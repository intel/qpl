/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#if defined(__linux__)

#include "qpl/qpl.h"

#include "gtest/gtest.h"

// tool_common
#include "util.hpp"

// test_common
#include "run_operation.hpp"
#include "ta_ll_common.hpp"

/*
 *  These tests will check if auto path works as expected in case of accelerator
 *  initialization failure.
 */

namespace qpl::test {

/*
 *  These tests should be run on a host without Intel® In-Memory Analytics Accelerator (Intel® IAA).
 *  Each test first submits a job on the hardware path and expects QPL_STS_INIT_WORK_QUEUES_NOT_AVAILABLE.
 *  Then it submits a job on the auto path and expects successful execution because
 *  host fallback is enabled.
 */
QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(auto_path_hw_init, deflate_no_wq) {
    qpl_path_t execution_path = qpl_path_hardware;
    qpl_status status         = run_compress_op(execution_path);
    if (status != QPL_STS_INIT_WORK_QUEUES_NOT_AVAILABLE) {
        GTEST_SKIP() << "Work queue is initiated without issue, skip this test";
    }

    execution_path = qpl_path_auto;
    status         = run_compress_op(execution_path);
    ASSERT_EQ(status, QPL_STS_OK);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(auto_path_hw_init, inflate_no_wq) {
    qpl_path_t execution_path = qpl_path_hardware;
    qpl_status status         = run_decompress_op(execution_path);
    if (status != QPL_STS_INIT_WORK_QUEUES_NOT_AVAILABLE) {
        GTEST_SKIP() << "Work queue is initiated without issue, skip this test";
    }

    execution_path = qpl_path_auto;
    status         = run_decompress_op(execution_path);
    ASSERT_EQ(status, QPL_STS_OK);
}

} // namespace qpl::test

#endif // #if defined(__linux__)
