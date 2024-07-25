/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "tn_common.hpp"

// qplref
#include "qpl_api_ref.h"

// tests_common
#include "execution_wrapper.hpp"
#include "operation_test.hpp"

namespace qpl::test {
/*
     * Purpose of this test is to control situation when bit width of job_ptr->param_low
     * is greater than bit width of the input stream. Here we test the following situation:
     *
     * param_low = 0b...11111111, and ignoring upper bits results 0b11111111, and input vector
     * contains 0b11111111 values, so scan should output all input elements
     */
QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(scan, ReferenceFixture, param_low_greater_than_input_bitwidth_fulloutput) {
    source.resize(100);
    destination.resize(100);
    reference_destination.resize(100);

    for (auto& source_elem : source) {
        source_elem = (1ULL << 8) - 1;
    }

    job_ptr->available_in = static_cast<uint32_t>(source.size());
    job_ptr->next_in_ptr  = source.data();

    reference_job_ptr->available_in = static_cast<uint32_t>(source.size());
    reference_job_ptr->next_in_ptr  = source.data();

    job_ptr->next_out_ptr  = destination.data();
    job_ptr->available_out = static_cast<uint32_t>(destination.size());

    reference_job_ptr->next_out_ptr  = reference_destination.data();
    reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());

    job_ptr->op           = qpl_op_scan_eq;
    reference_job_ptr->op = qpl_op_scan_eq;

    job_ptr->param_low           = (1ULL << 9) - 1;
    reference_job_ptr->param_low = (1ULL << 9) - 1;

    job_ptr->num_input_elements           = 100;
    reference_job_ptr->num_input_elements = 100;

    job_ptr->src1_bit_width           = 8;
    reference_job_ptr->src1_bit_width = 8;

    auto status = run_job_api(job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    auto reference_status = ref_compare(reference_job_ptr);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareTotalInOutWithReference());
    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));

    for (uint32_t i = 0; i < job_ptr->total_out - 1; i++) {
        const uint32_t expected_elem = (1ULL << 8) - 1;
        ASSERT_EQ(destination[i], expected_elem);
    }
}

/*
     * Purpose of this test is to control situation when bit width of job_ptr->param_low
     * is greater than bit width of the input stream. Here we test the following situation:
     *
     * param_low = 0b...100000000, and ignoring upper bits results 0b0, and input vector
     * contains 0b11111111 values, so scan should output no elements
     */
QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(scan, ReferenceFixture, param_low_greater_than_input_bitwidth_no_output) {
    source.resize(100);
    destination.resize(100);
    reference_destination.resize(100);

    for (auto& source_elem : source) {
        source_elem = (1ULL << 8) - 1;
    }

    job_ptr->available_in = static_cast<uint32_t>(source.size());
    job_ptr->next_in_ptr  = source.data();

    reference_job_ptr->available_in = static_cast<uint32_t>(source.size());
    reference_job_ptr->next_in_ptr  = source.data();

    job_ptr->next_out_ptr  = destination.data();
    job_ptr->available_out = static_cast<uint32_t>(destination.size());

    reference_job_ptr->next_out_ptr  = reference_destination.data();
    reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());

    job_ptr->op           = qpl_op_scan_eq;
    reference_job_ptr->op = qpl_op_scan_eq;

    job_ptr->param_low           = (1ULL << 9);
    reference_job_ptr->param_low = (1ULL << 9);

    job_ptr->num_input_elements           = 100;
    reference_job_ptr->num_input_elements = 100;

    job_ptr->src1_bit_width           = 8;
    reference_job_ptr->src1_bit_width = 8;

    auto status = run_job_api(job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    auto reference_status = ref_compare(reference_job_ptr);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareTotalInOutWithReference());
    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));

    for (uint32_t i = 0; i < job_ptr->total_out - 1; i++) {
        const uint32_t expected_elem = 0;
        ASSERT_EQ(destination[i], expected_elem);
    }
}
} // namespace qpl::test
