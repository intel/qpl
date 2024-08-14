/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/qpl.h"

#include "gtest/gtest.h"

// tool_common
#include "dispatcher_checks.hpp"
#include "util.hpp"

// test_common
#include "run_operation.hpp"
#include "tn_common.hpp"

namespace qpl::test {

QPL_LOW_LEVEL_API_NEGATIVE_TEST(submission, no_available_workqueues) {
    QPL_SKIP_TEST_FOR(qpl_path_software);
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(are_workqueues_available(), "At least one available workqueue");

    const qpl_path_t execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    const qpl_status status = run_decompress_op(execution_path);
    if (qpl_path_hardware == execution_path) {
        ASSERT_EQ(status, QPL_STS_INIT_WORK_QUEUES_NOT_AVAILABLE);
    } else {
        // qpl_path_auto
        ASSERT_EQ(status, QPL_STS_OK);
    }
}

/* This test is designed to submit enough jobs to ensure a busy queue error is returned.
 * It does this by finding out the maximum # of jobs that can be submitted, and then submitting long jobs
 * to ensure that jobs are completed while we submit.
 */
QPL_LOW_LEVEL_API_NEGATIVE_TEST(submission, work_queues_are_busy) {
    QPL_SKIP_TEST_FOR(qpl_path_software);
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!are_workqueues_available(), "No workqueues are available");

    const qpl_path_t execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    constexpr uint32_t source_size = 9U; // 1 byte for input bit width, 4 bytes for RLE header, 4 bytes for 32-bit value
    constexpr uint32_t input_bit_width = 0U; // necessary for parquet format
    constexpr uint32_t value_to_find   = 1U; // value to find
    constexpr uint32_t num_elements =
            268435454U / 2; // 134217727 is the maximum number of elements in a single parquet RLE run

    // parquet representation of 32-bit value 0 repeated 134217727 times
    // this job is chosen for taking a long time to complete (~.25 seconds), without any output.
    std::vector<uint8_t> source = {32,                      // input bit width
                                   0xFE, 0xFF, 0xFF, 0x7F,  // RLE header varint (repeat count << 1)
                                   0x00, 0x00, 0x00, 0x00}; // 32-bit value 0
    std::vector<uint8_t> destination(4, 0);

    const uint32_t number_of_jobs = max_descriptor_submissions() + 1;

    auto job_buffer = std::vector<std::unique_ptr<uint8_t[]>>(number_of_jobs);

    uint32_t size = 0U;

    qpl_status status = qpl_get_job_size(execution_path, &size);
    ASSERT_EQ(status, QPL_STS_OK);

    int job_index = 0;
    for (job_index = 0; job_index < number_of_jobs; job_index++) {
        job_buffer[job_index] = std::make_unique<uint8_t[]>(size);
        qpl_job* job          = reinterpret_cast<qpl_job*>(job_buffer[job_index].get());

        status = qpl_init_job(execution_path, job);

        job->next_in_ptr        = source.data();
        job->available_in       = source_size;
        job->next_out_ptr       = destination.data();
        job->available_out      = static_cast<uint32_t>(destination.size());
        job->op                 = qpl_op_scan_gt;
        job->parser             = qpl_p_parquet_rle;
        job->src1_bit_width     = input_bit_width;
        job->num_input_elements = num_elements;
        job->out_bit_width      = qpl_ow_32;
        job->param_low          = value_to_find;

        status = qpl_submit_job(job);
        if (status != QPL_STS_OK) { break; }
    }

    for (int i = 0; i < job_index; i++) {
        qpl_job* job = reinterpret_cast<qpl_job*>(job_buffer[i].get());

        ASSERT_EQ(qpl_wait_job(job), QPL_STS_OK) << "Expected job submission to complete with success";
    }

    if (qpl_path_hardware == execution_path) {
        ASSERT_EQ(status, QPL_STS_QUEUES_ARE_BUSY_ERR);
    } else {
        // qpl_path_auto
        ASSERT_EQ(status, QPL_STS_OK);
    }
}

} // namespace qpl::test
