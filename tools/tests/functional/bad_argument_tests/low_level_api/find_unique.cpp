/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11.2.2018
 * @brief Bad argument tests for qplFindUnique function
 * 
 */

#include "gtest/gtest.h"
#include "tb_ll_common.hpp"
#include "../../../common/operation_test.hpp"

namespace qpl::test {

static inline void check_destination_overflow(qpl_job *const job_ptr, uint32_t flags) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source{};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination{};

    constexpr uint32_t input_bit_width       = 8u;
    constexpr uint32_t unique_values_count   = 1u << (input_bit_width - 1u);
    constexpr uint32_t needed_output_size    = (unique_values_count + 7u) >> 3u; //For no modification output
    constexpr uint32_t corrupted_output_size = needed_output_size - 1u;

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, input_bit_width, 16u, INPUT_FORMAT);
    set_output_stream(job_ptr, destination.data(), corrupted_output_size, qpl_ow_nom);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, qpl_op_find_unique);

    EXPECT_EQ(run_job_api(job_ptr), QPL_STS_DST_IS_SHORT_ERR) << "Fail on: output size error";
}

static inline void check_incorrect_drop_bits(qpl_job *const job_ptr, uint32_t flags) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source{};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination{};

    constexpr uint32_t input_bit_width = 4u;

    auto input_parser = INPUT_FORMAT;

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, input_bit_width, ELEMENTS_TO_PROCESS, input_parser);
    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, qpl_op_find_unique);
    set_range(job_ptr, input_bit_width / 2u, input_bit_width / 2u);

    EXPECT_EQ(run_job_api(job_ptr), QPL_STS_DROP_BITS_OVERFLOW_ERR) << "Fail on: incorrect drop bits";

    set_range(job_ptr, input_bit_width / 2u + 1u, input_bit_width / 2u);

    EXPECT_EQ(run_job_api(job_ptr), QPL_STS_DROP_BITS_OVERFLOW_ERR) << "Fail on: incorrect drop bits";

    set_range(job_ptr, input_bit_width / 2u, input_bit_width / 2u + 1u);

    EXPECT_EQ(run_job_api(job_ptr), QPL_STS_DROP_BITS_OVERFLOW_ERR) << "Fail on: incorrect drop bits";

    // Now set parser to PRLE, and set 1st byte of the source to be input bit width
    if (!(flags & QPL_FLAG_DECOMPRESS_ENABLE)) {
        input_parser = qpl_p_parquet_rle;
        source[0] = input_bit_width;
        set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, 0u, ELEMENTS_TO_PROCESS, input_parser);
        set_range(job_ptr, input_bit_width / 2u, input_bit_width / 2u);

        EXPECT_EQ(run_job_api(job_ptr), QPL_STS_DROP_BITS_OVERFLOW_ERR) << "Fail on: incorrect drop bits for PRLE format";

        set_range(job_ptr, input_bit_width / 2u + 1u, input_bit_width / 2u);

        EXPECT_EQ(run_job_api(job_ptr), QPL_STS_DROP_BITS_OVERFLOW_ERR) << "Fail on: incorrect drop bits for PRLE format";

        set_range(job_ptr, input_bit_width / 2u, input_bit_width / 2u + 1u);

        EXPECT_EQ(run_job_api(job_ptr), QPL_STS_DROP_BITS_OVERFLOW_ERR) << "Fail on: incorrect drop bits for PRLE format";
    }
}


constexpr auto FIND_UNIQUE_MAX_ELEMENT_BIT_WIDTH = 15u;
static inline void check_large_set(qpl_job *const job_ptr, uint32_t flags) {
    std::vector<uint8_t> source(SOURCE_ARRAY_SIZE);
    std::vector<uint8_t> destination;

    constexpr uint32_t input_bit_width       = FIND_UNIQUE_MAX_ELEMENT_BIT_WIDTH + 1u;
    constexpr uint32_t unique_values_count   = 1u << (FIND_UNIQUE_MAX_ELEMENT_BIT_WIDTH);
    constexpr uint32_t needed_output_size    = (unique_values_count + 7u) >> 3u; //For no modification output

    destination.resize(needed_output_size);

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, input_bit_width, 16u, INPUT_FORMAT);
    set_output_stream(job_ptr, destination.data(), needed_output_size, qpl_ow_nom);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, qpl_op_find_unique);

    EXPECT_EQ(run_job_api(job_ptr), QPL_STS_SET_TOO_LARGE_ERR) << "Fail on: large set check";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(find_unique, source_errors) {
    check_input_stream_validation(job_ptr, qpl_op_find_unique, OPERATION_FLAGS);

    check_input_stream_validation(job_ptr, qpl_op_find_unique, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(find_unique, destination_errors) {
    check_output_stream_validation(job_ptr, qpl_op_find_unique, OPERATION_FLAGS);

    check_output_stream_validation(job_ptr, qpl_op_find_unique, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(find_unique, destination_overflow) {
    check_destination_overflow(job_ptr, OPERATION_FLAGS);

    check_destination_overflow(job_ptr, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(find_unique, incorrect_drop_bits) {
    check_incorrect_drop_bits(job_ptr, OPERATION_FLAGS);

    check_incorrect_drop_bits(job_ptr, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(find_unique, large_set) {
    check_large_set(job_ptr, OPERATION_FLAGS);

    check_large_set(job_ptr, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(find_unique, source_size_not_enough_to_hold_num_input_elements) {
    check_source_size_not_enough_to_hold_num_input_elements(job_ptr, qpl_op_find_unique, OPERATION_FLAGS);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(find_unique, buffer_overlap) {
    check_buffer_overlap(job_ptr, qpl_op_find_unique, OPERATION_FLAGS);

    check_buffer_overlap(job_ptr, qpl_op_find_unique, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(find_unique, drop_initial_bytes) {
    check_drop_initial_bytes_verification<operation_group_e::filter_single_source>(job_ptr,
                                                                                   qpl_op_find_unique,
                                                                                   OPERATION_FLAGS);

    check_drop_initial_bytes_verification<operation_group_e::filter_single_source>(job_ptr,
                                                                                   qpl_op_find_unique,
                                                                                   OPERATION_FLAGS
                                                                                   | QPL_FLAG_DECOMPRESS_ENABLE);
}

}
