/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <iostream>
#include <random>
#include <vector>

#include "execution_wrapper.hpp"
#include "qpl_api_ref.h"
#include "source_provider.hpp"
#include "ta_ll_common.hpp"
#include "util.hpp"

namespace qpl::test {
struct AggregatesTestCase {
    uint32_t   element_count     = 0U;
    uint8_t    element_bit_width = 0U;
    qpl_parser parser            = qpl_p_le_packed_array;
};

static std::ostream& operator<<(std::ostream& os, const AggregatesTestCase& test_case) {
    os << "Number of elements: " << test_case.element_count << '\n';
    os << "Element bit width: " << test_case.element_bit_width << '\n';
    os << "Parser: " << ParserToString(test_case.parser) << '\n';

    return os;
}

class AggregatesTest : public JobFixtureWithTestCases<AggregatesTestCase> {
protected:
    void SetUpBeforeIteration() override {
        auto test_case = GetTestCase();

        source_provider source_generator(test_case.element_count, test_case.element_bit_width, GetSeed(),
                                         test_case.parser);

        ASSERT_NO_THROW(source = source_generator.get_source()); //NOLINT(cppcoreguidelines-avoid-goto)

        destination.resize(source.size());

        job_ptr->op                 = qpl_op_scan_eq;
        job_ptr->src1_bit_width     = test_case.element_bit_width;
        job_ptr->out_bit_width      = qpl_ow_nom;
        job_ptr->param_low          = 0U;
        job_ptr->param_high         = 0U;
        job_ptr->num_input_elements = test_case.element_count;
        job_ptr->parser             = test_case.parser;

        job_ptr->next_in_ptr   = source.data();
        job_ptr->available_in  = static_cast<uint32_t>(source.size());
        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_out = static_cast<uint32_t>(destination.size());
    }

private:
    void InitializeTestCases() override {
        for (uint32_t length = 500U; length < 1000U; length++) {
            AggregatesTestCase test_case;
            test_case.element_count     = length;
            test_case.element_bit_width = 8U;

            AddNewTestCase(test_case);
        }
    }

    void SetUp() override {
        JobFixture::SetUp();
        InitializeTestCases();
    }
};

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(integrity_control, crc32_gzip, AggregatesTest) {
    const uint32_t polynomial = 0x04C11DB7;

    auto status = run_job_api(job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    const uint32_t library_crc   = job_ptr->crc;
    const uint32_t reference_crc = ref_crc32(source.data(), static_cast<uint32_t>(source.size()), polynomial, 0U);

    EXPECT_EQ(reference_crc, library_crc);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(integrity_control, crc32_iscsi, AggregatesTest) {
    const uint32_t polynomial = 0x1EDC6F41; // this polynomial is used with QPL_FLAG_CRC32C
    job_ptr->flags            = QPL_FLAG_CRC32C;

    auto status = run_job_api(job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    const uint32_t library_crc   = job_ptr->crc;
    const uint32_t reference_crc = ref_crc32(source.data(), static_cast<uint32_t>(source.size()), polynomial, 0U);

    EXPECT_EQ(reference_crc, library_crc);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(integrity_control, xor_checksum, AggregatesTest) {
    auto status = run_job_api(job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    const uint32_t library_xor   = job_ptr->xor_checksum;
    const uint32_t reference_xor = ref_xor_checksum(source.data(), static_cast<uint32_t>(source.size()), 0);

    EXPECT_EQ(reference_xor, library_xor);
}

// Decompress into a small output buffer that is reused on every call after
// QPL_STS_MORE_OUTPUT_NEEDED. The final xor_checksum must cover the whole
// stream, and must not be computed from memory preceding the reused buffer.
QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(decompress_checksum, xor_reused_output_buffer, JobFixture) {
    constexpr uint16_t   data_size = 257U;
    std::vector<uint8_t> data(data_size);
    for (uint32_t i = 0U; i < data_size; i++) {
        data[i] = static_cast<uint8_t>(i * 7U + 3U);
    }

    // Single final stored (uncompressed) deflate block: BFINAL=1, BTYPE=00, LEN, NLEN, data
    std::vector<uint8_t> stream = {0x01U, static_cast<uint8_t>(data_size & 0xFFU),
                                   static_cast<uint8_t>(data_size >> 8U), static_cast<uint8_t>(~data_size & 0xFFU),
                                   static_cast<uint8_t>((~data_size >> 8U) & 0xFFU)};
    stream.insert(stream.end(), data.begin(), data.end());

    const uint32_t reference_xor = ref_xor_checksum(data.data(), data_size, 0U);

    // Odd chunk sizes exercise continuation at odd stream offsets
    for (const uint32_t chunk_size : {1U, 2U, 3U, 15U, 16U, 17U, 64U, 255U, 256U}) {
        ASSERT_EQ(QPL_STS_OK, qpl_init_job(GetExecutionPath(), job_ptr));

        // Place the reused buffer after a pseudo-random prefix so that any read
        // before its start changes the checksum
        std::vector<uint8_t> arena(data_size + chunk_size);
        std::minstd_rand     prefix_generator(chunk_size);
        for (uint32_t i = 0U; i < data_size; i++) {
            arena[i] = static_cast<uint8_t>(prefix_generator() >> 8U);
        }
        uint8_t* const output_ptr = arena.data() + data_size;

        job_ptr->op           = qpl_op_decompress;
        job_ptr->next_in_ptr  = stream.data();
        job_ptr->available_in = static_cast<uint32_t>(stream.size());
        job_ptr->flags        = QPL_FLAG_FIRST | QPL_FLAG_LAST;

        std::vector<uint8_t> recovered;
        qpl_status           status      = QPL_STS_MORE_OUTPUT_NEEDED;
        bool                 no_progress = false;

        while (status == QPL_STS_MORE_OUTPUT_NEEDED && !no_progress) {
            const uint32_t available_in = job_ptr->available_in;
            job_ptr->next_out_ptr       = output_ptr;
            job_ptr->available_out      = chunk_size;

            status = run_job_api(job_ptr);

            const uint32_t produced = chunk_size - job_ptr->available_out;
            recovered.insert(recovered.end(), output_ptr, output_ptr + produced);
            job_ptr->flags &= ~QPL_FLAG_FIRST;

            // Documented: unchanged available_in and available_out means the output buffer is too small
            no_progress = produced == 0U && job_ptr->available_in == available_in;
        }

        if (no_progress && GetExecutionPath() == qpl_path_hardware) {
            std::cout << "Hardware can't make progress with chunk_size = " << chunk_size << ", skipping it\n";
            continue;
        }

        ASSERT_EQ(QPL_STS_OK, status) << "chunk_size = " << chunk_size;
        ASSERT_EQ(data, recovered) << "chunk_size = " << chunk_size;
        EXPECT_EQ(reference_xor, job_ptr->xor_checksum) << "chunk_size = " << chunk_size;
    }
}
} // namespace qpl::test
