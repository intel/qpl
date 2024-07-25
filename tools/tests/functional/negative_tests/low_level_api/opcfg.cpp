/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/qpl.h"

#include "gtest/gtest.h"

// tool_common
#include "opcfg_checks.hpp"
#include "util.hpp"

// test_common
#include "run_operation.hpp"
#include "tn_common.hpp"

/*
 *  These test check the accelerator configuration, and will run tests for
 *  ensuring the correct error is returned for that configuration for the
 *  OPCFG functionality.
 *
 *  On Intel® In-Memory Analytics Accelerator (Intel® IAA) 2.0 or later, these test are ran when operations are all disabled,
 *  adding this line to workqueue configuration to disable all operations:
 *
 *  "op_config":"00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000"
 */

namespace qpl::test {

QPL_LOW_LEVEL_API_NEGATIVE_TEST(opcfg, disabled_decompress) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Software path does not support OPCFG");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_opcfg_capabilities_present(), "OPCFG capabilities are not present");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_operation_disabled_on_all_wq_on_node(opcode_decompress),
                                   "Decompress enabled on an available workqueue");

    const qpl_path_t execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    const qpl_status status = run_decompress_op(execution_path);
    if (execution_path == qpl_path_auto) {
        // Expect fallback on sw path to be successful
        ASSERT_EQ(status, QPL_STS_OK);
    } else {
        ASSERT_EQ(status, QPL_STS_NOT_SUPPORTED_BY_WQ);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST(opcfg, disabled_compress) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Software path does not support OPCFG");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_opcfg_capabilities_present(), "OPCFG capabilities are not present");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_operation_disabled_on_all_wq_on_node(opcode_compress),
                                   "Compress enabled on an available workqueue");

    const qpl_path_t execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    const qpl_status status = run_compress_op(execution_path);
    if (execution_path == qpl_path_auto) {
        // Expect fallback on sw path to be successful
        ASSERT_EQ(status, QPL_STS_OK);
    } else {
        ASSERT_EQ(status, QPL_STS_NOT_SUPPORTED_BY_WQ);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST(opcfg, disabled_crc64) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Software path does not support OPCFG");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_opcfg_capabilities_present(), "OPCFG capabilities are not present");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_operation_disabled_on_all_wq_on_node(opcode_crc64),
                                   "CRC64 enabled on an available workqueue");

    const qpl_path_t execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    const qpl_status status = run_crc64_op(execution_path);
    if (execution_path == qpl_path_auto) {
        // Expect fallback on sw path to be successful
        ASSERT_EQ(status, QPL_STS_OK);
    } else {
        ASSERT_EQ(status, QPL_STS_NOT_SUPPORTED_BY_WQ);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST(opcfg, disabled_scan) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Software path does not support OPCFG");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_opcfg_capabilities_present(), "OPCFG capabilities are not present");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_operation_disabled_on_all_wq_on_node(opcode_scan),
                                   "Scan enabled on an available workqueue");

    const qpl_path_t execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    const qpl_status status = run_scan_op(execution_path);
    if (execution_path == qpl_path_auto) {
        // Expect fallback on sw path to be successful
        ASSERT_EQ(status, QPL_STS_OK);
    } else {
        ASSERT_EQ(status, QPL_STS_NOT_SUPPORTED_BY_WQ);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST(opcfg, disabled_extract) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Software path does not support OPCFG");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_opcfg_capabilities_present(), "OPCFG capabilities are not present");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_operation_disabled_on_all_wq_on_node(opcode_extract),
                                   "Extract enabled on an available workqueue");

    const qpl_path_t execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    const qpl_status status = run_extract_op(execution_path);
    if (execution_path == qpl_path_auto) {
        // Expect fallback on sw path to be successful
        ASSERT_EQ(status, QPL_STS_OK);
    } else {
        ASSERT_EQ(status, QPL_STS_NOT_SUPPORTED_BY_WQ);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST(opcfg, disabled_select) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Software path does not support OPCFG");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_opcfg_capabilities_present(), "OPCFG capabilities are not present");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_operation_disabled_on_all_wq_on_node(opcode_select),
                                   "Select enabled on an available workqueue");

    const qpl_path_t execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    const qpl_status status = run_select_op(execution_path);
    if (execution_path == qpl_path_auto) {
        // Expect fallback on sw path to be successful
        ASSERT_EQ(status, QPL_STS_OK);
    } else {
        ASSERT_EQ(status, QPL_STS_NOT_SUPPORTED_BY_WQ);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST(opcfg, disabled_expand) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Software path does not support OPCFG");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_opcfg_capabilities_present(), "OPCFG capabilities are not present");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_operation_disabled_on_all_wq_on_node(opcode_expand),
                                   "Expand enabled on an available workqueue");

    const qpl_path_t execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    const qpl_status status = run_expand_op(execution_path);
    if (execution_path == qpl_path_auto) {
        // Expect fallback on sw path to be successful
        ASSERT_EQ(status, QPL_STS_OK);
    } else {
        ASSERT_EQ(status, QPL_STS_NOT_SUPPORTED_BY_WQ);
    }
}

} // namespace qpl::test
