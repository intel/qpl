/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11.2.ELEMENTS_COUNT18
 * @brief Bad argument tests for qpl_set_membership function
 */

#include "gtest/gtest.h"
#include "tb_ll_common.hpp"
#include "../../../common/operation_test.hpp"
#include "util.hpp"

namespace qpl::test {

constexpr auto SET_MEMBERSHIP_MAX_ELEMENT_BIT_WIDTH = 15u;

static inline void check_large_set(qpl_job *const job_ptr, uint32_t flags) {
    std::vector<uint8_t> source(SOURCE_ARRAY_SIZE);
    std::vector<uint8_t> destination;
    std::vector<uint8_t> mask(MASK_ARRAY_SIZE);

    constexpr uint32_t input_bit_width     = SET_MEMBERSHIP_MAX_ELEMENT_BIT_WIDTH + 1u;
    constexpr uint32_t unique_values_count = 1u << (SET_MEMBERSHIP_MAX_ELEMENT_BIT_WIDTH);
    constexpr uint32_t needed_output_size  = (unique_values_count + 7u) >> 3u; //For no modification output

    destination.resize(needed_output_size);

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, input_bit_width, 16u, INPUT_FORMAT);
    set_output_stream(job_ptr, destination.data(), needed_output_size, qpl_ow_nom);
    set_mask_stream(job_ptr, mask.data(), MASK_ARRAY_SIZE, MASK_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, qpl_op_find_unique);

    EXPECT_EQ(run_job_api(job_ptr), QPL_STS_SET_TOO_LARGE_ERR) << "Fail on: large set check";
}

static inline void check_mask_length(qpl_job *const job_ptr, uint32_t flags) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source{};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination{};
    std::array<uint8_t, MASK_ARRAY_SIZE>        mask{};

    constexpr uint32_t input_bit_width     = 8u;
    constexpr uint32_t unique_values_count = 1u << (input_bit_width - 1u);
    constexpr uint32_t needed_mask_size    = (unique_values_count + 7u) >> 3u; //For no modification output

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, input_bit_width, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, qpl_op_set_membership);
    set_mask_stream(job_ptr, mask.data(), needed_mask_size - 1u, MASK_BIT_WIDTH);

    EXPECT_EQ(run_job_api(job_ptr), QPL_STS_SRC2_IS_SHORT_ERR) << "Fail on: incorrect drop bits";
}

static inline void check_incorrect_drop_bits(qpl_job *const job_ptr, uint32_t flags) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source{};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination{};
    std::array<uint8_t, MASK_ARRAY_SIZE>        mask{};

    constexpr uint32_t input_bit_width = 4u;

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, input_bit_width, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, qpl_op_set_membership);
    set_mask_stream(job_ptr, mask.data(), MASK_ARRAY_SIZE, MASK_BIT_WIDTH);
    set_range(job_ptr, input_bit_width / 2u, input_bit_width / 2u);

    EXPECT_EQ(run_job_api(job_ptr), QPL_STS_DROP_BITS_OVERFLOW_ERR) << "Fail on: incorrect drop bits";

    set_range(job_ptr, input_bit_width / 2u + 1u, input_bit_width / 2u);

    EXPECT_EQ(run_job_api(job_ptr), QPL_STS_DROP_BITS_OVERFLOW_ERR) << "Fail on: incorrect drop bits";

    set_range(job_ptr, input_bit_width / 2u, input_bit_width / 2u + 1u);

    EXPECT_EQ(run_job_api(job_ptr), QPL_STS_DROP_BITS_OVERFLOW_ERR) << "Fail on: incorrect drop bits";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(set_membership, source_errors) {
    check_input_stream_validation(job_ptr, qpl_op_set_membership, OPERATION_FLAGS);

    check_input_stream_validation(job_ptr, qpl_op_set_membership, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(set_membership, destination_errors) {
    check_output_stream_validation(job_ptr, qpl_op_set_membership, OPERATION_FLAGS);

    check_output_stream_validation(job_ptr, qpl_op_set_membership, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(set_membership, mask_errors) {
    check_mask_stream_validation(job_ptr, qpl_op_set_membership, OPERATION_FLAGS);

    check_mask_stream_validation(job_ptr, qpl_op_set_membership, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(set_membership, mask_length_error) {
    check_mask_length(job_ptr, OPERATION_FLAGS);

    check_mask_length(job_ptr, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(set_membership, incorrect_drop_bits) {
    check_incorrect_drop_bits(job_ptr, OPERATION_FLAGS);

    check_incorrect_drop_bits(job_ptr, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(set_membership, legacy_large_set) {
    check_large_set(job_ptr, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(set_membership, exceeded_limit_errors) {
    SKIP_TEST_FOR(qpl_path_software) << "Software haven't limitation on drop of decompressed bytes";

    check_double_source_stream_limits_validation(job_ptr, qpl_op_set_membership, OPERATION_FLAGS);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(set_membership, incorrect_initial_output_index) {
    check_initial_output_index_verification<operation_group_e::filter_double_source>(job_ptr,
                                                                                     qpl_op_set_membership,
                                                                                     OPERATION_FLAGS);

    check_initial_output_index_verification<operation_group_e::filter_double_source>(job_ptr,
                                                                                     qpl_op_set_membership,
                                                                                     OPERATION_FLAGS
                                                                                     | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(set_membership, drop_initial_bytes) {
    check_drop_initial_bytes_verification<operation_group_e::filter_double_source>(job_ptr,
                                                                                   qpl_op_set_membership,
                                                                                   OPERATION_FLAGS);

    check_drop_initial_bytes_verification<operation_group_e::filter_double_source>(job_ptr,
                                                                                   qpl_op_set_membership,
                                                                                   OPERATION_FLAGS
                                                                                   | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(set_membership, source_size_not_enough_to_hold_num_input_elements) {
    check_source_size_not_enough_to_hold_num_input_elements(job_ptr, qpl_op_set_membership, OPERATION_FLAGS);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(set_membership, buffer_overlap) {
    check_buffer_overlap<operation_group_e::filter_double_source>(job_ptr, qpl_op_set_membership, OPERATION_FLAGS);

    check_buffer_overlap<operation_group_e::filter_double_source>(job_ptr, qpl_op_set_membership, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

}
