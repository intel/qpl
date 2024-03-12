/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "gtest/gtest.h"

#include "tb_ll_common.hpp"

// tests_common
#include "operation_test.hpp"

// tool_common
#include "util.hpp"

namespace qpl::test {

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(select, source_errors) {
    check_input_stream_validation(job_ptr, qpl_op_select, OPERATION_FLAGS);

    check_input_stream_validation(job_ptr, qpl_op_select, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(select, destination_errors) {
    check_output_stream_validation(job_ptr, qpl_op_select, OPERATION_FLAGS);

    check_output_stream_validation(job_ptr, qpl_op_select, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(select, mask_errors) {
    check_mask_stream_validation(job_ptr, qpl_op_select, OPERATION_FLAGS);

    check_mask_stream_validation(job_ptr, qpl_op_select, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(select, exceeded_limit_errors) {
    // Software path doesn't have a limitation on drop of decompressed bytes
    QPL_SKIP_TEST_FOR(qpl_path_software);

    check_double_source_stream_limits_validation(job_ptr, qpl_op_select, OPERATION_FLAGS);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(select, incorrect_initial_output_index) {
    check_initial_output_index_verification<operation_group_e::filter_double_source>(job_ptr,
                                                                                     qpl_op_select,
                                                                                     OPERATION_FLAGS);

    check_initial_output_index_verification<operation_group_e::filter_double_source>(job_ptr,
                                                                                     qpl_op_select,
                                                                                     OPERATION_FLAGS
                                                                                     | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(select, drop_initial_bytes) {
    check_drop_initial_bytes_verification<operation_group_e::filter_double_source>(job_ptr,
                                                                                   qpl_op_select,
                                                                                   OPERATION_FLAGS);

    check_drop_initial_bytes_verification<operation_group_e::filter_double_source>(job_ptr,
                                                                                   qpl_op_select,
                                                                                   OPERATION_FLAGS
                                                                                   | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(select, source_size_not_enough_to_hold_num_input_elements) {
    check_source_size_not_enough_to_hold_num_input_elements(job_ptr, qpl_op_select, OPERATION_FLAGS);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(select, incorrect_num_elements_with_index_output) {
        analytic_with_mask_check_num_elements_with_index_output(job_ptr, qpl_op_select, OPERATION_FLAGS);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(select, buffer_overlap) {
    check_buffer_overlap<operation_group_e::filter_double_source>(job_ptr, qpl_op_select, OPERATION_FLAGS);

    check_buffer_overlap<operation_group_e::filter_double_source>(job_ptr, qpl_op_select, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

}
