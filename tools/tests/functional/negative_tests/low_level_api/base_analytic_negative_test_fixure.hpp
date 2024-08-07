/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TESTS_NEGATIVE_TESTS_BASE_ANALYTIC_NEGATIVE_TEST_FIXTURE_HPP_
#define QPL_TESTS_NEGATIVE_TESTS_BASE_ANALYTIC_NEGATIVE_TEST_FIXTURE_HPP_

#include "tn_common.hpp"

// tests_common
#include "analytic_fixture.hpp"
#include "job_helper.hpp"

namespace qpl::test {

constexpr bool ENABLE_COMPRESSION = true;

class BaseAnalyticsNegativeTestFixture : public JobFixture {
public:
    virtual void SetUpDefaultCase() = 0;

    void SetUp() override {
        JobFixture::SetUp();
        SetUpDefaultCase();
        this->PrepareJobBuffers();
    }

    void PrepareJobBuffers() {
        source_provider source_gen(job_ptr->num_input_elements, job_ptr->src1_bit_width, GetSeed(), job_ptr->parser);

        ASSERT_NO_THROW(source = source_gen.get_source()); //NOLINT(cppcoreguidelines-avoid-goto)

        uint32_t actual_out_bit_width =
                (job_ptr->out_bit_width == qpl_ow_nom) ? job_ptr->src1_bit_width : 4U << (job_ptr->out_bit_width);

        uint32_t dest_size =
                job_ptr->num_input_elements * ((actual_out_bit_width + max_bit_index) >> bit_to_byte_shift_offset);

        std::fill(destination.begin(), destination.end(), 0);

        destination.resize(dest_size);
        job_ptr->total_in  = 0U;
        job_ptr->total_out = 0U;

        job_ptr->available_in = static_cast<uint32_t>(source.size());
        job_ptr->next_in_ptr  = source.data();

        job_ptr->available_out = static_cast<uint32_t>(destination.size());
        job_ptr->next_out_ptr  = destination.data();
    }

    void CompressSource() {
        uint32_t job_size = 0U;
        auto     status   = qpl_get_job_size(GetExecutionPath(), &job_size);

        ASSERT_EQ(QPL_STS_OK, status) << "Couldn't get compression job size\n";

        auto     job_buffer      = std::make_unique<uint8_t[]>(job_size);
        qpl_job* deflate_job_ptr = reinterpret_cast<qpl_job*>(job_buffer.get());

        status = qpl_init_job(GetExecutionPath(), deflate_job_ptr);

        if (QPL_STS_OK != status) { throw std::runtime_error("Couldn't init compression job\n"); }

        const uint32_t MINIMAL_DESTINATION_SIZE = 100U;
        uint32_t       destination_size         = static_cast<uint32_t>(source.size()) * 2;
        destination_size = (destination_size < MINIMAL_DESTINATION_SIZE) ? MINIMAL_DESTINATION_SIZE : destination_size;

        m_compressed_source.resize(destination_size, 0);

        deflate_job_ptr->op    = qpl_op_compress;
        deflate_job_ptr->level = qpl_default_level;
        deflate_job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN | QPL_FLAG_OMIT_VERIFY;
        deflate_job_ptr->available_in  = static_cast<uint32_t>(source.size());
        deflate_job_ptr->next_in_ptr   = source.data();
        deflate_job_ptr->available_out = static_cast<uint32_t>(m_compressed_source.size());
        deflate_job_ptr->next_out_ptr  = m_compressed_source.data();

        status = run_job_api(deflate_job_ptr);

        if (QPL_STS_OK != status) {
            std::string error_message = "Compression returned " + std::to_string(status) + " status\n";
            throw std::runtime_error(error_message);
        }

        m_compressed_source.resize(deflate_job_ptr->total_out);

        job_ptr->available_in = static_cast<uint32_t>(m_compressed_source.size());
        job_ptr->next_in_ptr  = m_compressed_source.data();
        job_ptr->flags |= QPL_FLAG_DECOMPRESS_ENABLE;
    }

    testing::AssertionResult ValidateDecompressionErrorHandling() {
        CompressSource();

        // Corrupt Compressed source
        m_compressed_source[0] |= 0b000000110U; // [4:0] - data; [6:5] - block type; [7:7] - block final marker;

        return RunStatusTest(QPL_STS_INVALID_BLOCK_TYPE);
    }

    testing::AssertionResult ValidateDecompressAndDropInitialBytesHandling() {
        CompressSource();

        // Corrupt Compressed source
        m_compressed_source[0] |= 0b000000110U; // [4:0] - data; [6:5] - block type; [7:7] - block final marker;
        job_ptr->drop_initial_bytes = job_ptr->available_in / 8U;

        auto expected_status = QPL_STS_INVALID_BLOCK_TYPE;

        return RunStatusTest(expected_status);
    }

    testing::AssertionResult ValidateCompressedInputErrorHandling() {
        source.resize(source.size() / 2);
        CompressSource();

        return RunStatusTest(QPL_STS_SRC_IS_SHORT_ERR);
    }

    testing::AssertionResult ValidateRLEInputErrorHandling() {
        source_provider source_gen(job_ptr->num_input_elements / 4, job_ptr->src1_bit_width, GetSeed(),
                                   job_ptr->parser);

        auto source_is_short = source_gen.get_source();

        job_ptr->available_in = static_cast<uint32_t>(source_is_short.size());
        job_ptr->next_in_ptr  = source_is_short.data();

        testing::AssertionResult testStatus = RunStatusTest(QPL_STS_SRC_IS_SHORT_ERR);

        job_ptr->available_in = static_cast<uint32_t>(source.size());
        job_ptr->next_in_ptr  = source.data();

        return testStatus;
    }

    testing::AssertionResult ValidateMoreOutputErrorHandling(bool compressed) {
        job_ptr->available_out /= 2;

        if (compressed) { CompressSource(); }

        return RunStatusTest(QPL_STS_DST_IS_SHORT_ERR);
    }

protected:
    testing::AssertionResult RunStatusTest(qpl_status expected_status) {
        auto status = run_job_api(job_ptr);

        if (status != expected_status /* Invalid block type*/) {
            return testing::AssertionFailure() << " Library status: " << status << " VS "
                                               << " Expected status: " << expected_status << "\n";
        } else {
            return testing::AssertionSuccess();
        }
    }

    std::vector<uint8_t> m_compressed_source;
};

#define REGISTER_BASE_NEGATIVE_TESTS(operation, fixture, postfix)                                        \
    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(operation, fixture, tn_deflate_error_handling_##postfix) {         \
        ASSERT_TRUE(ValidateDecompressionErrorHandling());                                               \
    }                                                                                                    \
    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(operation, fixture, DISABLED_tn_output_error_handling_##postfix) { \
        ASSERT_TRUE(ValidateMoreOutputErrorHandling(ENABLE_COMPRESSION));                                \
    }

#define REGISTER_NEGATIVE_TESTS_LE(operation, fixture)                                                      \
    REGISTER_BASE_NEGATIVE_TESTS(operation, fixture, le)                                                    \
    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(operation, fixture, DISABLED_tn_compressed_le_input_error_handling) { \
        ASSERT_TRUE(ValidateCompressedInputErrorHandling());                                                \
    }

#define REGISTER_NEGATIVE_TESTS_RLE(operation, fixture)                                                      \
    REGISTER_BASE_NEGATIVE_TESTS(operation, fixture, rle)                                                    \
    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(operation, fixture, tn_rle_input_error_handling) {                     \
        ASSERT_TRUE(ValidateRLEInputErrorHandling());                                                        \
    }                                                                                                        \
    QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(operation, fixture, DISABLED_tn_compressed_rle_input_error_handling) { \
        ASSERT_TRUE(ValidateCompressedInputErrorHandling());                                                 \
    }
} // namespace qpl::test
#endif //QPL_TESTS_NEGATIVE_TESTS_BASE_ANALYTIC_NEGATIVE_TEST_FIXTURE_HPP_
