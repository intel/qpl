/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

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
} // namespace qpl::test
