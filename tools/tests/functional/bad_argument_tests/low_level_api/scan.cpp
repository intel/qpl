/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "gtest/gtest.h"

#include "tb_ll_common.hpp"

// tests_common
#include "operation_test.hpp"

namespace qpl::test {

static inline void check_destination_overflow_prefetch(qpl_job *const job_ptr, qpl_operation operation, uint32_t flags) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source{};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination{};

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, 16u, INPUT_FORMAT);
    set_output_stream(job_ptr, destination.data(), 1u, OUTPUT_BIT_WIDTH);
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

}
