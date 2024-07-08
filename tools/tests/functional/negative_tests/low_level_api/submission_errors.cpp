/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "gtest/gtest.h"

#include "qpl/qpl.h"

// tool_common
#include "dispatcher_checks.hpp"
#include "util.hpp"

// test_common
#include "tn_common.hpp"
#include "run_operation.hpp"


namespace qpl::test {

QPL_LOW_LEVEL_API_NEGATIVE_TEST(submission, no_available_workqueues) {
    QPL_SKIP_TEST_FOR(qpl_path_software);
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(are_workqueues_available(), "At least one available workqueue");

    qpl_path_t execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    qpl_status status = run_decompress_op(execution_path);
    if (qpl_path_hardware == execution_path) {
        ASSERT_EQ(status, QPL_STS_INIT_WORK_QUEUES_NOT_AVAILABLE);
    } else {
        // qpl_path_auto
        ASSERT_EQ(status, QPL_STS_OK);
    }
}

}
