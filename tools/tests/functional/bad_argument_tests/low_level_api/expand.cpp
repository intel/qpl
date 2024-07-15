/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "gtest/gtest.h"

#include "tb_ll_common.hpp"

// utils_common
#include "iaa_features_checks.hpp"

// test_common
#include "operation_test.hpp"

// tool_common
#include "util.hpp"

namespace qpl::test {

void check_less_elements_in_mask_than_required(qpl_job *job_ptr, uint64_t flags) {
    constexpr uint32_t source_length = 5;
    constexpr uint32_t element_bit_width = 8U;
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

/////////////////////////////////////
// Force Array Output Modification //
//                                 //
// Dependencies:                   //
// - iaa_features_checks.hpp       //
// - util.hpp                      //
/////////////////////////////////////

// If flag is set, output bit width is nominal, QPL_STS_OUT_FORMAT_ERR is expected
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(expand, force_array_output_nominal) {
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(is_iaa_force_array_output_mod_supported() == false, "Force array output modification is not supported. Skip the test.");
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Force array output modification not available on software path");

    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source{};
    std::array<uint8_t, MASK_ARRAY_SIZE>        mask{};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination{};

    job_ptr->flags |= QPL_FLAG_FORCE_ARRAY_OUTPUT;

    // Preset correct parameters
    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    set_mask_stream(job_ptr, mask.data(), MASK_ARRAY_SIZE, MASK_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, OPERATION_FLAGS | QPL_FLAG_FORCE_ARRAY_OUTPUT, qpl_op_expand);

    // Force Array Output Bit Width check
    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, qpl_ow_nom);
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_OUT_FORMAT_ERR) << "Fail on: force array output bit width error";
}

// If flag is set, force array output modification is not supported, QPL_STS_NOT_SUPPORTED_MODE_ERR is expected
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(expand, force_array_output_not_supported) {
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(is_iaa_force_array_output_mod_supported() == true, "Force array output modification is supported. Skip the test.");

    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source{};
    std::array<uint8_t, MASK_ARRAY_SIZE>        mask{};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination{};

    job_ptr->flags |= QPL_FLAG_FORCE_ARRAY_OUTPUT;

    // Preset correct parameters
    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    set_mask_stream(job_ptr, mask.data(), MASK_ARRAY_SIZE, MASK_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, OPERATION_FLAGS | QPL_FLAG_FORCE_ARRAY_OUTPUT, qpl_op_expand);

    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, qpl_ow_32);

    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_NOT_SUPPORTED_MODE_ERR) << "Fail on: force array output modification is not supported";
}

}
