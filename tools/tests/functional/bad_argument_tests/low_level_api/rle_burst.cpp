/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/09/2018
 * @brief Bad Args tests for qpl_expand_rle function
 * 
 */

#include "gtest/gtest.h"
#include "qpl/qpl.h"
#include "../../../common/operation_test.hpp"
#include "tb_ll_common.hpp"
#include "util.hpp"

#include <array>

namespace qpl::test {

constexpr int            BYTE_BIT_LEN            = 8u;
constexpr uint32_t       TEST_ARRAY_SIZE         = 2u;
constexpr uint32_t       TEST_ELEMENTS_COUNT     = 2;
constexpr uint8_t        TEST_COUNTERS_BIT_WIDTH = 8;
constexpr uint8_t        TEST_ELEMENTS_BIT_WIDTH = 7;
constexpr qpl_out_format TEST_OUTPUT_FORMAT      = qpl_ow_8;

static inline void validate_input_stream(qpl_job *const job_ptr, qpl_operation operation, uint32_t flags) {
    std::array<uint8_t, TEST_ARRAY_SIZE * 2> counters{};
    std::array<uint8_t, TEST_ARRAY_SIZE * 2> source{};
    std::array<uint8_t, TEST_ARRAY_SIZE * 2> destination{};

    counters[0] = 1;
    counters[1] = 1;

    source[0] = 1;
    source[1] = 1;

    // Preset
    job_ptr->op = qpl_op_rle_burst;
    set_input_stream(job_ptr,
                     counters.data(),
                     TEST_ARRAY_SIZE,
                     TEST_COUNTERS_BIT_WIDTH,
                     TEST_ELEMENTS_COUNT,
                     INPUT_FORMAT);
    set_mask_stream(job_ptr, source.data(), TEST_ARRAY_SIZE, TEST_ELEMENTS_BIT_WIDTH);
    set_output_stream(job_ptr, destination.data(), TEST_ARRAY_SIZE, TEST_OUTPUT_FORMAT);

    ASSERT_EQ(QPL_STS_NULL_PTR_ERR, run_job_api(nullptr));

    job_ptr->next_src2_ptr = nullptr;
    ASSERT_EQ(QPL_STS_NULL_PTR_ERR, run_job_api(job_ptr));
    job_ptr->next_src2_ptr = source.data();

    job_ptr->available_src2 = 0u;
    ASSERT_EQ(QPL_STS_SIZE_ERR, run_job_api(job_ptr));
    job_ptr->available_src2 = TEST_ARRAY_SIZE;

    // check for correct processing situation when num_of_elements * src2_bit_width exceeds available bits in src2
    // check for src1 use len which exceeds the number of bits required for loading num_of_elements, and use too small val for src2Len
    job_ptr->available_in       = (((TEST_ARRAY_SIZE + 1) * TEST_COUNTERS_BIT_WIDTH) / BYTE_BIT_LEN) + 1;
    job_ptr->available_src2     = (((TEST_ARRAY_SIZE) * TEST_ELEMENTS_BIT_WIDTH) / BYTE_BIT_LEN);
    job_ptr->num_input_elements = TEST_ARRAY_SIZE + 1;
    ASSERT_EQ(QPL_STS_SRC2_IS_SHORT_ERR, run_job_api(job_ptr));
    job_ptr->available_src2     = TEST_ARRAY_SIZE;
    job_ptr->available_in       = TEST_ARRAY_SIZE;
    job_ptr->num_input_elements = TEST_ELEMENTS_COUNT;

    job_ptr->src2_bit_width = 0;
    ASSERT_EQ(QPL_STS_BIT_WIDTH_ERR, run_job_api(job_ptr));
    job_ptr->src2_bit_width = TEST_ELEMENTS_BIT_WIDTH;

    job_ptr->src2_bit_width = 33;
    ASSERT_EQ(QPL_STS_BIT_WIDTH_ERR, run_job_api(job_ptr));
    job_ptr->src2_bit_width = TEST_ELEMENTS_BIT_WIDTH;
}

static inline void validate_counters_stream(qpl_job *const job_ptr, qpl_operation operation, uint32_t flags) {
    std::array<uint8_t, TEST_ARRAY_SIZE * 2> counters{};
    std::array<uint8_t, TEST_ARRAY_SIZE * 2> source{};
    std::array<uint8_t, TEST_ARRAY_SIZE * 2> destination{};

    // Preset
    set_input_stream(job_ptr,
                     counters.data(),
                     TEST_ARRAY_SIZE,
                     TEST_COUNTERS_BIT_WIDTH,
                     TEST_ELEMENTS_COUNT,
                     INPUT_FORMAT);
    set_mask_stream(job_ptr, source.data(), TEST_ARRAY_SIZE, TEST_ELEMENTS_BIT_WIDTH);
    set_output_stream(job_ptr, destination.data(), TEST_ARRAY_SIZE, TEST_OUTPUT_FORMAT);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, operation);

    job_ptr->next_in_ptr = nullptr;
    ASSERT_EQ(QPL_STS_NULL_PTR_ERR, run_job_api(job_ptr));
    job_ptr->next_in_ptr = counters.data();

    job_ptr->available_in = 0u;
    ASSERT_EQ(QPL_STS_SIZE_ERR, run_job_api(job_ptr));
    job_ptr->available_in = TEST_ARRAY_SIZE;

    job_ptr->num_input_elements = 0u;
    ASSERT_EQ(QPL_STS_SIZE_ERR, run_job_api(job_ptr));
    job_ptr->num_input_elements = TEST_ELEMENTS_COUNT;

    // There's no way to check on short source size
    if (flags != QPL_FLAG_DECOMPRESS_ENABLE) {
        job_ptr->num_input_elements = TEST_ARRAY_SIZE + 1;
        job_ptr->available_src2     = ((TEST_ARRAY_SIZE + 1) * TEST_ELEMENTS_BIT_WIDTH + 7u) >> 3u;
        ASSERT_EQ(QPL_STS_SRC_IS_SHORT_ERR, run_job_api(job_ptr));
        job_ptr->num_input_elements = TEST_ELEMENTS_COUNT;
        job_ptr->available_src2     = TEST_ARRAY_SIZE;
    }

    job_ptr->src1_bit_width = 13;
    uint32_t new_source_length = ((job_ptr->src1_bit_width + 7u) >> 3u) * job_ptr->num_input_elements;
    job_ptr->available_in = new_source_length;
    ASSERT_EQ(QPL_STS_BIT_WIDTH_ERR, run_job_api(job_ptr));
    job_ptr->src1_bit_width = TEST_COUNTERS_BIT_WIDTH;
}

static inline void validate_output_stream(qpl_job *const job_ptr, qpl_operation operation, uint32_t flags) {
    std::array<uint8_t, TEST_ARRAY_SIZE> counters{};
    std::array<uint8_t, TEST_ARRAY_SIZE + 1> source{};
    std::array<uint8_t, TEST_ARRAY_SIZE> destination{};

    // Preset
    set_input_stream(job_ptr,
                     counters.data(),
                     TEST_ARRAY_SIZE,
                     TEST_COUNTERS_BIT_WIDTH,
                     TEST_ELEMENTS_COUNT,
                     INPUT_FORMAT);
    set_mask_stream(job_ptr, source.data(), TEST_ARRAY_SIZE, TEST_ELEMENTS_BIT_WIDTH);
    set_output_stream(job_ptr, destination.data(), TEST_ARRAY_SIZE, TEST_OUTPUT_FORMAT);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, operation);

    job_ptr->next_out_ptr = nullptr;
    ASSERT_EQ(QPL_STS_NULL_PTR_ERR, run_job_api(job_ptr));
    job_ptr->next_out_ptr = destination.data();

    job_ptr->available_out = 0u;
    ASSERT_EQ(QPL_STS_SIZE_ERR, run_job_api(job_ptr));
    job_ptr->available_out = TEST_ARRAY_SIZE;

    job_ptr->out_bit_width = static_cast<qpl_out_format>(13);
    ASSERT_EQ(QPL_STS_OUT_FORMAT_ERR, run_job_api(job_ptr));
    job_ptr->out_bit_width = TEST_OUTPUT_FORMAT;

    job_ptr->available_src2 = TEST_ARRAY_SIZE + 1;
    job_ptr->src2_bit_width = 10;
    job_ptr->out_bit_width  = static_cast<qpl_out_format>(13);
    ASSERT_EQ(QPL_STS_OUT_FORMAT_ERR, run_job_api(job_ptr));
}

static inline void validate_destination_overflow(qpl_job *const job_ptr, qpl_operation operation, uint32_t flags) {
    std::array<uint8_t, TEST_ARRAY_SIZE> counters{};
    std::array<uint8_t, TEST_ARRAY_SIZE> source{};
    std::array<uint8_t, TEST_ARRAY_SIZE> destination{};

    // Preset
    set_input_stream(job_ptr,
                     counters.data(),
                     TEST_ARRAY_SIZE,
                     TEST_COUNTERS_BIT_WIDTH,
                     TEST_ELEMENTS_COUNT,
                     INPUT_FORMAT);
    set_mask_stream(job_ptr, source.data(), TEST_ARRAY_SIZE, TEST_ELEMENTS_BIT_WIDTH);
    set_output_stream(job_ptr, destination.data(), TEST_ARRAY_SIZE, TEST_OUTPUT_FORMAT);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, operation);

    //!!! TODO It would be better to make following 2 tests in more general format
    // check for correct processing of the situation when dst is too small
    // fill counters' values
    counters[0] = 2;
    counters[1] = 1;
    // => required dst size in bits is 21 (src2BWidth = 7), but it is 16 (just 5 bits less)
    source[0]   = 5; // just any number
    source[1]   = 7; // just any number

    ASSERT_EQ(QPL_STS_DST_IS_SHORT_ERR, run_job_api(job_ptr));

    // check for correct processing of the situation when dst is too small
    // fill counters' values
    counters[0] = 2;
    counters[1] = 4;
    // => required dst size in bits is 42 (src2BWidth = 7), but it is 16
    source[0]   = 5; // just any number
    source[1]   = 7; // just any number

    ASSERT_EQ(QPL_STS_DST_IS_SHORT_ERR, run_job_api(job_ptr));
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(rle_burst, counters_errors) {
    validate_counters_stream(job_ptr, qpl_op_rle_burst, OPERATION_FLAGS);

    validate_counters_stream(job_ptr, qpl_op_rle_burst, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(rle_burst, source_errors) {
    validate_input_stream(job_ptr, qpl_op_rle_burst, OPERATION_FLAGS);

    validate_input_stream(job_ptr, qpl_op_rle_burst, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(rle_burst, destination_errors) {
    validate_output_stream(job_ptr, qpl_op_rle_burst, OPERATION_FLAGS);

    validate_output_stream(job_ptr, qpl_op_rle_burst, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(rle_burst, destination_overflow_errors) {
    validate_destination_overflow(job_ptr, qpl_op_rle_burst, OPERATION_FLAGS);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(rle_burst, incorrect_initial_output_index) {
    check_initial_output_index_verification<operation_group_e::filter_double_source>(job_ptr,
                                                                                     qpl_op_rle_burst,
                                                                                     OPERATION_FLAGS);

    check_initial_output_index_verification<operation_group_e::filter_double_source>(job_ptr,
                                                                                     qpl_op_rle_burst,
                                                                                     OPERATION_FLAGS
                                                                                     | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(rle_burst, drop_initial_bytes) {
    check_drop_initial_bytes_verification<operation_group_e::filter_double_source>(job_ptr,
                                                                                   qpl_op_rle_burst,
                                                                                   OPERATION_FLAGS);

    check_drop_initial_bytes_verification<operation_group_e::filter_double_source>(job_ptr,
                                                                                   qpl_op_rle_burst,
                                                                                   OPERATION_FLAGS
                                                                                   | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(rle_burst, exceeded_limit_errors) {
    SKIP_TEST_FOR(qpl_path_software) << "Software haven't limitation on drop of decompressed bytes";

    check_double_source_stream_limits_validation(job_ptr, qpl_op_rle_burst, OPERATION_FLAGS, 8u);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(rle_burst, short_counters_stream) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      counters{};
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source{};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination{};

    job_ptr->next_in_ptr    = counters.data();
    job_ptr->available_in   = SOURCE_ARRAY_SIZE;
    job_ptr->next_src2_ptr  = source.data();
    job_ptr->available_src2 = SOURCE_ARRAY_SIZE;
    job_ptr->next_out_ptr   = destination.data();
    job_ptr->available_out  = DESTINATION_ARRAY_SIZE;

    job_ptr->num_input_elements = 1u;
    job_ptr->src1_bit_width     = 32u;
    job_ptr->src2_bit_width     = 8u;
    job_ptr->parser             = qpl_p_le_packed_array;
    job_ptr->out_bit_width      = qpl_ow_nom;

    job_ptr->op = qpl_op_rle_burst;

    qpl_status status = run_job_api(job_ptr);

    ASSERT_EQ(status, QPL_STS_SIZE_ERR);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(rle_burst, buffer_overlap) {
    check_buffer_overlap<operation_group_e::filter_double_source>(job_ptr, qpl_op_rle_burst, OPERATION_FLAGS);

    check_buffer_overlap<operation_group_e::filter_double_source>(job_ptr, qpl_op_rle_burst, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

}
