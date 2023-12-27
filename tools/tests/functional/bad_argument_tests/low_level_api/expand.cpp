/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "gtest/gtest.h"
#include "tb_ll_common.hpp"
#include "operation_test.hpp"
#include "util.hpp"

namespace qpl::test {

void check_less_elements_in_mask_than_required(qpl_job *job_ptr, uint64_t flags) {
    constexpr uint32_t source_length = 5;
    constexpr uint32_t element_bit_width = 8u;
    std::vector<uint8_t> source = { 1, 2, 3, 4, 5};
    constexpr uint32_t mask_length = 1;
    std::vector<uint8_t> mask = { 0b10010111 };
    std::vector<uint8_t> destination(DESTINATION_ARRAY_SIZE);

    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);
    set_mask_stream(job_ptr, mask.data(), mask_length, MASK_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, qpl_op_expand);

    // num_input_elements for expand operation reflects number of input elements in source-2
    set_input_stream(job_ptr, source.data(), source_length, element_bit_width, 50, qpl_p_le_packed_array);
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_SRC_IS_SHORT_ERR) << "Fail on: number of elements exceeds mask size";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(expand, source_errors) {
    check_input_stream_validation(job_ptr, qpl_op_expand, OPERATION_FLAGS);
    check_less_elements_in_mask_than_required(job_ptr, OPERATION_FLAGS);

    check_input_stream_validation(job_ptr, qpl_op_expand, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
    check_less_elements_in_mask_than_required(job_ptr, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(expand, destination_errors) {
    check_output_stream_validation(job_ptr, qpl_op_expand, OPERATION_FLAGS);

    check_output_stream_validation(job_ptr, qpl_op_expand, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(expand, mask_errors) {
    check_mask_stream_validation(job_ptr, qpl_op_expand, OPERATION_FLAGS);

    check_mask_stream_validation(job_ptr, qpl_op_expand, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(expand, exceeded_limit_errors)
{
    // Software path doesn't have a limitation on drop of decompressed bytes
    QPL_SKIP_TEST_FOR(qpl_path_software);

    check_double_source_stream_limits_validation(job_ptr, qpl_op_expand, OPERATION_FLAGS);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(expand, incorrect_initial_output_index) {
    check_initial_output_index_verification<operation_group_e::filter_double_source>(job_ptr,
                                                                                     qpl_op_expand,
                                                                                     OPERATION_FLAGS);

    check_initial_output_index_verification<operation_group_e::filter_double_source>(job_ptr,
                                                                                     qpl_op_expand,
                                                                                     OPERATION_FLAGS
                                                                                     | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(expand, drop_initial_bytes) {
    check_drop_initial_bytes_verification<operation_group_e::filter_double_source>(job_ptr,
                                                                                   qpl_op_expand,
                                                                                   OPERATION_FLAGS);

    check_drop_initial_bytes_verification<operation_group_e::filter_double_source>(job_ptr,
                                                                                   qpl_op_expand,
                                                                                   OPERATION_FLAGS
                                                                                   | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(expand, buffer_overlap) {
    check_buffer_overlap<operation_group_e::filter_double_source>(job_ptr, qpl_op_expand, OPERATION_FLAGS);

    check_buffer_overlap<operation_group_e::filter_double_source>(job_ptr, qpl_op_expand, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

}
