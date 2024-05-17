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

static inline void check_destination_overflow_prefetch(qpl_job *const job_ptr, qpl_operation operation, uint32_t flags) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source{};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination{};

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, 16U, INPUT_FORMAT);
    set_output_stream(job_ptr, destination.data(), 1U, OUTPUT_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, flags, operation);

    EXPECT_EQ(run_job_api(job_ptr), QPL_STS_DST_IS_SHORT_ERR) << "Fail on: output size error";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(scan, source_errors) {
    qpl::test::random random_scan_operation(qpl_op_scan_eq,
                                            qpl_op_scan_not_range,
                                            TestEnviroment::GetInstance().GetSeed());
    auto operation = (qpl_operation) ((uint32_t) random_scan_operation);

    check_input_stream_validation(job_ptr, operation, OPERATION_FLAGS);

    check_input_stream_validation(job_ptr, operation, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(scan, destination_errors) {
    qpl::test::random random_scan_operation(qpl_op_scan_eq,
                                            qpl_op_scan_not_range,
                                            TestEnviroment::GetInstance().GetSeed());
    auto operation = (qpl_operation) ((uint32_t) random_scan_operation);

    check_output_stream_validation(job_ptr, operation, OPERATION_FLAGS);

    check_output_stream_validation(job_ptr, operation, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(scan, destination_overflow_prefetch) {
    qpl::test::random random_scan_operation(qpl_op_scan_eq,
                                            qpl_op_scan_not_range,
                                            TestEnviroment::GetInstance().GetSeed());
    auto operation = (qpl_operation) ((uint32_t) random_scan_operation);

    check_destination_overflow_prefetch(job_ptr, operation, OPERATION_FLAGS);

    check_destination_overflow_prefetch(job_ptr, operation, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(scan, incorrect_initial_output_index) {
    qpl::test::random random_scan_operation(qpl_op_scan_eq,
        qpl_op_scan_not_range,
        TestEnviroment::GetInstance().GetSeed());
    auto operation = (qpl_operation)((uint32_t)random_scan_operation);

    check_initial_output_index_verification(job_ptr, operation, OPERATION_FLAGS);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(scan, source_size_not_enough_to_hold_num_input_elements) {
    qpl::test::random random_scan_operation(qpl_op_scan_eq,
        qpl_op_scan_not_range,
        TestEnviroment::GetInstance().GetSeed());
    auto operation = (qpl_operation)((uint32_t)random_scan_operation);

    check_source_size_not_enough_to_hold_num_input_elements(job_ptr, operation, OPERATION_FLAGS);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(scan, buffer_overlap) {
    qpl::test::random random_scan_operation(qpl_op_scan_eq,
                                            qpl_op_scan_not_range,
                                            TestEnviroment::GetInstance().GetSeed());

    auto operation = (qpl_operation)((uint32_t)random_scan_operation);

    check_buffer_overlap(job_ptr, operation, OPERATION_FLAGS);

    check_buffer_overlap(job_ptr, operation, OPERATION_FLAGS | QPL_FLAG_DECOMPRESS_ENABLE);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(scan, drop_initial_bytes) {
    qpl::test::random random_scan_operation(qpl_op_scan_eq,
                                            qpl_op_scan_not_range,
                                            TestEnviroment::GetInstance().GetSeed());

    auto operation = (qpl_operation)((uint32_t) random_scan_operation);

    check_drop_initial_bytes_verification<operation_group_e::filter_single_source>(job_ptr,
                                                                                   operation,
                                                                                   OPERATION_FLAGS);

    check_drop_initial_bytes_verification<operation_group_e::filter_single_source>(job_ptr,
                                                                                   operation,
                                                                                   OPERATION_FLAGS
                                                                                   | QPL_FLAG_DECOMPRESS_ENABLE);
}

/////////////////////////////////////
// Force Array Output Modification //
//                                 //
// Dependencies:                   //
// - iaa_features_checks.hpp       //
// - util.hpp                      //
/////////////////////////////////////

// If flag is set, output bit width is nominal, QPL_STS_OUT_FORMAT_ERR is expected
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(scan, force_array_output_nominal) {
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(is_iaa_force_array_output_mod_supported() == false, "Force array output modification is not supported. Skip the test.");
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Force array output modification not available on software path");

    qpl::test::random random_scan_operation(qpl_op_scan_eq,
                                            qpl_op_scan_not_range,
                                            TestEnviroment::GetInstance().GetSeed());
    auto operation = (qpl_operation) ((uint32_t) random_scan_operation);

    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source{};
    std::array<uint8_t, MASK_ARRAY_SIZE>        mask{};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination{};

    job_ptr->flags |= QPL_FLAG_FORCE_ARRAY_OUTPUT;

    // Preset correct parameters
    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    set_mask_stream(job_ptr, mask.data(), MASK_ARRAY_SIZE, MASK_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, OPERATION_FLAGS | QPL_FLAG_FORCE_ARRAY_OUTPUT, operation);

    // Force Array Output Bit Width check
    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, qpl_ow_nom);
    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_OUT_FORMAT_ERR) << "Fail on: force array output bit width error";
}

// If flag is set, force array output modification is not supported, QPL_STS_NOT_SUPPORTED_MODE_ERR is expected
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(scan, force_array_output_not_supported) {
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(is_iaa_force_array_output_mod_supported() == true, "Force array output modification is supported. Skip the test.");
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Force array output modification not available on software path");

    qpl::test::random random_scan_operation(qpl_op_scan_eq,
                                            qpl_op_scan_not_range,
                                            TestEnviroment::GetInstance().GetSeed());
    auto operation = (qpl_operation) ((uint32_t) random_scan_operation);

    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source{};
    std::array<uint8_t, MASK_ARRAY_SIZE>        mask{};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination{};

    job_ptr->flags |= QPL_FLAG_FORCE_ARRAY_OUTPUT;

    // Preset correct parameters
    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);
    set_mask_stream(job_ptr, mask.data(), MASK_ARRAY_SIZE, MASK_BIT_WIDTH);
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, OPERATION_FLAGS | QPL_FLAG_FORCE_ARRAY_OUTPUT, operation);

    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, qpl_ow_32);

    EXPECT_EQ(qpl::test::run_job_api(job_ptr), QPL_STS_NOT_SUPPORTED_MODE_ERR) << "Fail on: force array output modification is not supported";
}

}
