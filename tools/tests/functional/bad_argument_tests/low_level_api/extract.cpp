/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "gtest/gtest.h"
#include "tb_ll_common.hpp"

// utils_common
#include "iaa_features_checks.hpp"

// tool_common
#include "util.hpp"

// tests_common
#include "operation_test.hpp"

namespace qpl::test {

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(extract, source_errors) {
    check_input_stream_validation(job_ptr, qpl_op_extract, OPERATION_FLAGS);

    check_input_stream_validation(job_ptr, qpl_op_extract, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(extract, destination_errors) {
    check_output_stream_validation(job_ptr, qpl_op_extract, OPERATION_FLAGS);

    check_output_stream_validation(job_ptr, qpl_op_extract, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(extract, param_low_greater_param_high) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    job_ptr->next_in_ptr   = source.data();
    job_ptr->available_in  = SOURCE_ARRAY_SIZE;
    job_ptr->next_out_ptr  = destination.data();
    job_ptr->available_out = DESTINATION_ARRAY_SIZE;

    job_ptr->first_index_min_value = 0U;
    job_ptr->last_index_max_value  = 0U;
    job_ptr->last_bit_offset       = 0U;
    job_ptr->sum_value             = 0U;
    job_ptr->crc                   = 0U;

    job_ptr->num_input_elements = SOURCE_ARRAY_SIZE;
    job_ptr->src1_bit_width     = 8U;
    job_ptr->param_low          = SOURCE_ARRAY_SIZE / 2U;
    job_ptr->param_high         = SOURCE_ARRAY_SIZE / 4U;
    job_ptr->parser             = qpl_p_le_packed_array;
    job_ptr->out_bit_width      = qpl_ow_nom;

    job_ptr->op = qpl_op_extract;

    const qpl_status status = run_job_api(job_ptr);

    ASSERT_EQ(status, QPL_STS_OK);

    ASSERT_EQ(job_ptr->next_in_ptr, source.data());
    ASSERT_EQ(job_ptr->available_in, SOURCE_ARRAY_SIZE);
    ASSERT_EQ(job_ptr->next_out_ptr, destination.data());
    ASSERT_EQ(job_ptr->available_out, DESTINATION_ARRAY_SIZE);

    ASSERT_EQ(job_ptr->first_index_min_value, 0U);
    ASSERT_EQ(job_ptr->last_index_max_value, 0U);
    ASSERT_EQ(job_ptr->last_bit_offset, 0U);
    ASSERT_EQ(job_ptr->sum_value, 0U);
    ASSERT_EQ(job_ptr->crc, 0U);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(extract, incorrect_initial_output_index) {
    check_initial_output_index_verification(job_ptr, qpl_op_extract, OPERATION_FLAGS);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(extract, source_size_not_enough_to_hold_num_input_elements) {
    check_source_size_not_enough_to_hold_num_input_elements(job_ptr, qpl_op_extract, OPERATION_FLAGS);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(extract, incorrect_num_elements_with_index_output) {
    analytic_with_mask_check_num_elements_with_index_output(job_ptr, qpl_op_extract, OPERATION_FLAGS);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(extract, buffer_overlap) {
    check_buffer_overlap(job_ptr, qpl_op_extract, OPERATION_FLAGS);

    check_buffer_overlap(job_ptr, qpl_op_extract, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(extract, drop_initial_bytes) {
    check_drop_initial_bytes_verification<operation_group_e::filter_single_source>(job_ptr, qpl_op_extract,
                                                                                   OPERATION_FLAGS);

    check_drop_initial_bytes_verification<operation_group_e::filter_single_source>(
            job_ptr, qpl_op_extract, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

/////////////////////////////////////
// Force Array Output Modification //
//                                 //
// Dependencies:                   //
// - iaa_features_checks.hpp       //
// - util.hpp                      //
/////////////////////////////////////

// If flag is set, output bit width is nominal, QPL_STS_OUT_FORMAT_ERR is expected
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(extract, force_array_output_nominal) {
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(is_iaa_force_array_output_mod_supported() == false,
                                   "Force array output modification is not supported. Skip the test.");
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Force array output modification not available on software path");

    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, MASK_ARRAY_SIZE>        mask {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    job_ptr->flags |= QPL_FLAG_FORCE_ARRAY_OUTPUT;

    // Preset correct parameters
    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    set_mask_stream(job_ptr, mask.data(), MASK_ARRAY_SIZE, MASK_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, OPERATION_FLAGS | QPL_FLAG_FORCE_ARRAY_OUTPUT,
                             qpl_op_extract);

    // Force Array Output Bit Width check
    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, qpl_ow_nom);
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_OUT_FORMAT_ERR) << "Fail on: force array output bit width error";
}

// If flag is set, force array output modification is not supported, QPL_STS_NOT_SUPPORTED_MODE_ERR is expected
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(extract, force_array_output_not_supported) {
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(is_iaa_force_array_output_mod_supported() == true,
                                   "Force array output modification is supported. Skip the test.");

    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, MASK_ARRAY_SIZE>        mask {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    job_ptr->flags |= QPL_FLAG_FORCE_ARRAY_OUTPUT;

    // Preset correct parameters
    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    set_mask_stream(job_ptr, mask.data(), MASK_ARRAY_SIZE, MASK_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, OPERATION_FLAGS | QPL_FLAG_FORCE_ARRAY_OUTPUT,
                             qpl_op_extract);

    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, qpl_ow_32);

    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_NOT_SUPPORTED_MODE_ERR)
            << "Fail on: force array output modification is not supported";
}

} // namespace qpl::test
