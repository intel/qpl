/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "tn_common.hpp"

// tests_common
#include "execution_wrapper.hpp"
#include "operation_test.hpp"

// tool_generator
#include "gendefs.hpp"
#include "igenerator.h"

namespace qpl::test {
class InflateErrorsTest : public JobFixture {
public:
    void SetUp() override { JobFixture::SetUp(); }

    qpl_status InflateRunBrokenStream(TestType test_type) {
        std::vector<uint8_t> encoded_data_buffer(0);
        std::vector<uint8_t> decoded_data_buffer(0);

        GenStatus  generator_status = GEN_OK;
        TestFactor test_factor;
        test_factor.seed = GetSeed();
        test_factor.type = test_type;

        const gz_generator::InflateGenerator data_generator;

        generator_status =
                gz_generator::InflateGenerator::generate(encoded_data_buffer, decoded_data_buffer, test_factor);

        EXPECT_EQ(GEN_OK, generator_status);

        source.resize(encoded_data_buffer.size());
        std::copy(encoded_data_buffer.begin(), encoded_data_buffer.end(), source.begin());

        auto destination_size = decoded_data_buffer.size();

        if (destination_size == 0) { destination_size = 1; }

        destination.resize(destination_size);
        std::copy(decoded_data_buffer.begin(), decoded_data_buffer.end(), destination.begin());

        job_ptr->op    = qpl_op_decompress;
        job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST;

        job_ptr->next_in_ptr   = source.data();
        job_ptr->available_in  = static_cast<uint32_t>(source.size());
        job_ptr->next_out_ptr  = destination.data();
        job_ptr->available_out = static_cast<uint32_t>(destination.size());

        const qpl_status job_status = run_job_api(job_ptr);

        return job_status;
    }

    void TearDown() override { JobFixture::TearDown(); }
};

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, DISABLED_large_header) {
    const TestType   test_type      = LARGE_HEADER;
    const qpl_status inflate_status = InflateRunBrokenStream(test_type);

    const qpl_status expected_status = QPL_STS_BIG_HEADER_ERR;

    EXPECT_EQ(expected_status, inflate_status);
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, undefined_code_length_code) {
    const TestType   test_type      = UNDEFINED_CL_CODE;
    const qpl_status inflate_status = InflateRunBrokenStream(test_type);

    const auto expected_status = QPL_STS_UNDEF_CL_CODE_ERR;

    EXPECT_EQ(expected_status, inflate_status);
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, first_literal_lengths_code_is_16) {
    const TestType   test_type      = FIRST_LL_CODE_16;
    const qpl_status inflate_status = InflateRunBrokenStream(test_type);

    const auto expected_status = QPL_STS_FIRST_LL_CODE_16_ERR;

    EXPECT_EQ(expected_status, inflate_status);
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, first_distance_code_is_16) {
    // Could not be returned by the isa-l because ll codes parsed with dist codes in the same loop
    if (qpl_path_hardware == GetExecutionPath()) {
        const TestType   test_type      = FIRST_D_CODE_16;
        const qpl_status inflate_status = InflateRunBrokenStream(test_type);

        const auto expected_status = QPL_STS_FIRST_D_CODE_16_ERR;

        EXPECT_EQ(expected_status, inflate_status);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, no_literal_lengths_code) {
    const TestType   test_type      = NO_LL_CODE;
    const qpl_status inflate_status = InflateRunBrokenStream(test_type);

    // Check AD_ERROR_CODE_BAD_LL_CODE code
    if (qpl_path_hardware == GetExecutionPath()) {
        EXPECT_EQ(QPL_STS_BAD_LL_CODE_ERR, inflate_status);
    } else {
        EXPECT_EQ(QPL_STS_NO_LL_CODE_ERR, inflate_status);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, all_zero_literal_lengths_code) {
    const TestType   test_type      = ALL_ZERO_LL_CODE;
    const qpl_status inflate_status = InflateRunBrokenStream(test_type);

    EXPECT_EQ(QPL_STS_NO_LL_CODE_ERR, inflate_status);
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, strading_code_length_codes) {
    if (qpl_path_hardware == GetExecutionPath()) {
        // Could not be returned by the isa-l
        const TestType   test_type      = CL_CODES_SPAN_LL_WITH_D;
        const qpl_status inflate_status = InflateRunBrokenStream(test_type);

        const auto expected_status = QPL_STS_WRONG_NUM_LL_CODES_ERR;

        EXPECT_EQ(expected_status, inflate_status);
    }
}

/**
    * @brief Check Situation when too many expected LL length has been declared in the dynamic block header
    */
QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, many_literal_length_codes_declared) {
    const TestType   test_type      = TOO_MANY_LL_CODES;
    const qpl_status inflate_status = InflateRunBrokenStream(test_type);

    // Check AD_ERROR_CODE_TOO_MANY_LL_CODES code
    if (qpl_path_hardware == GetExecutionPath()) {
        EXPECT_EQ(235, inflate_status);
    } else {
        EXPECT_EQ(206, inflate_status);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, big_count_repeating_literal_length_codes) {
    if (qpl_path_hardware == GetExecutionPath()) {
        const TestType   test_type      = BIG_REPEAT_COUNT_LL;
        const qpl_status inflate_status = InflateRunBrokenStream(test_type);

        const auto expected_status = QPL_STS_WRONG_NUM_LL_CODES_ERR;

        EXPECT_EQ(expected_status, inflate_status);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, many_distance_codes) {
    const TestType   test_type      = TOO_MANY_D_CODES;
    const qpl_status inflate_status = InflateRunBrokenStream(test_type);

    const auto expected_status = QPL_STS_WRONG_NUM_DIST_CODES_ERR;

    EXPECT_EQ(expected_status, inflate_status);
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, many_distance_codes_declared) {
    const TestType   test_type      = TOO_MANY_D_CODES_V2;
    const qpl_status inflate_status = InflateRunBrokenStream(test_type);

    // Check AD_ERROR_CODE_TOO_MANY_D_CODES code
    if (qpl_path_hardware == GetExecutionPath()) {
        EXPECT_EQ(236, inflate_status);
    } else {
        EXPECT_EQ(207, inflate_status);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, large_count_repeating_distance_codes) {
    const TestType   test_type      = BIG_REPEAT_COUNT_D;
    const qpl_status inflate_status = InflateRunBrokenStream(test_type);

    const auto expected_status = QPL_STS_WRONG_NUM_DIST_CODES_ERR;

    EXPECT_EQ(expected_status, inflate_status);
}

/**
    * @brief Test case, then encoded lengths of code lengths huffman codes, have been rewritten
    */
QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, oversubscribed_code_lengths_tree) {
    const TestType   test_type      = OVERSUBSCRIBED_CL_TREE;
    const qpl_status inflate_status = InflateRunBrokenStream(test_type);

    const auto expected_status = QPL_STS_BAD_CL_CODE_LEN_ERR;

    EXPECT_EQ(expected_status, inflate_status);
}

/**
    * @brief Test case, then encoded lengths of literal/match huffman codes have been rewritten
    */
QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, oversubscribed_literal_lengths_tree) {
    const TestType   test_type      = OVERSUBSCRIBED_LL_TREE;
    const qpl_status inflate_status = InflateRunBrokenStream(test_type);

    const auto expected_status = QPL_STS_BAD_LL_CODE_LEN_ERR;

    EXPECT_EQ(expected_status, inflate_status);
}

/**
    * @brief Test case, then encoded lengths of offset huffman codes, have been rewritten
    */
QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, oversubscribed_distance_lengths_tree) {
    const TestType   test_type      = OVERSUBSCRIBED_D_TREE;
    const qpl_status inflate_status = InflateRunBrokenStream(test_type);

    const auto expected_status = QPL_STS_BAD_DIST_CODE_LEN_ERR;

    EXPECT_EQ(expected_status, inflate_status);
}
/**
    * @brief Test to check if inflate returns a correct status in case of reserved literal/match code is read
    */
QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, using_reserved_literal_length_code) {
    const TestType   test_type      = BLOCK_CONTAIN_UNALLOWABLE_LL_CODE;
    const qpl_status inflate_status = InflateRunBrokenStream(test_type);

    const auto expected_status = QPL_STS_BAD_LL_CODE_ERR;

    EXPECT_EQ(expected_status, inflate_status);
}

/**
    * @brief Test to check if inflate returns a correct status in case of reserved distance code is read
    */
QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, using_reserved_distance_length_code) {
    const TestType   test_type      = BLOCK_CONTAIN_UNALLOWABLE_D_CODE;
    const qpl_status inflate_status = InflateRunBrokenStream(test_type);

    const auto expected_status = QPL_STS_BAD_D_CODE_ERR;

    EXPECT_EQ(expected_status, inflate_status);
}

/**
    * @brief Test to check if inflate returns a correct status in case of block code is 3 (0b11)
    */
QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, invalid_block_type) {
    const TestType   test_type      = INVALID_BLOCK_TYPE;
    const qpl_status inflate_status = InflateRunBrokenStream(test_type);

    const auto expected_status = QPL_STS_INVALID_BLOCK_TYPE;

    EXPECT_EQ(expected_status, inflate_status);
}

/**
    * @brief Test to check if inflate returns a correct status in case of stored block length is not correct
    */
QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, invalid_stored_length) {
    const TestType   test_type      = BAD_STORED_LEN;
    const qpl_status inflate_status = InflateRunBrokenStream(test_type);

    const auto expected_status = QPL_STS_INVALID_STORED_LEN_ERR;

    EXPECT_EQ(expected_status, inflate_status);
}

/**
    * @brief Test to check if inflate returns a correct status in case of link with offset,
    * which is greater then maximal history value
    */
QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, bad_distance) {
    const TestType   test_type      = BAD_DIST;
    const qpl_status inflate_status = InflateRunBrokenStream(test_type);

    const auto expected_status = QPL_STS_BAD_DIST_ERR;

    EXPECT_EQ(expected_status, inflate_status);
}

/**
    * @brief Test to check if inflate returns a correct status in case of link with offset,
    * which is greater then decoded symbols count
    */
QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(inflate, InflateErrorsTest, buffer_over_reading_via_distance) {
    const TestType   test_type      = DIST_BEFORE_START;
    const qpl_status inflate_status = InflateRunBrokenStream(test_type);

    const auto expected_status = QPL_STS_REF_BEFORE_START_ERR;

    EXPECT_EQ(expected_status, inflate_status);
}
} // namespace qpl::test
