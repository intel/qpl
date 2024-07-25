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
#include "ta_ll_common.hpp"

/*
 *  These test check the accelerator configuration, and will run tests for
 *  the OPCFG functionality if the configuration is correct for the test.
 */

namespace qpl::test {

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(opcfg, enabled_decompress) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Software path does not support OPCFG");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_opcfg_capabilities_present(), "OPCFG capabilities are not present");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(is_operation_disabled_on_all_wq_on_node(opcode_decompress),
                                   "Decompress disabled on all available workqueues");

    const qpl_path_t execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    const qpl_status status = run_decompress_op(execution_path);
    ASSERT_EQ(status, QPL_STS_OK);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(opcfg, enabled_compress) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Software path does not support OPCFG");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_opcfg_capabilities_present(), "OPCFG capabilities are not present");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(is_operation_disabled_on_all_wq_on_node(opcode_compress),
                                   "Compress disabled on all available workqueues");

    const qpl_path_t execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    const qpl_status status = run_compress_op(execution_path);
    ASSERT_EQ(status, QPL_STS_OK);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(opcfg, enabled_crc64) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Software path does not support OPCFG");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_opcfg_capabilities_present(), "OPCFG capabilities are not present");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(is_operation_disabled_on_all_wq_on_node(opcode_crc64),
                                   "CRC64 disabled on all available workqueues");

    const qpl_path_t execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    const qpl_status status = run_crc64_op(execution_path);
    ASSERT_EQ(status, QPL_STS_OK);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(opcfg, enabled_scan) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Software path does not support OPCFG");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_opcfg_capabilities_present(), "OPCFG capabilities are not present");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(is_operation_disabled_on_all_wq_on_node(opcode_scan),
                                   "Scan disabled on all available workqueues");

    const qpl_path_t execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    const qpl_status status = run_scan_op(execution_path);
    ASSERT_EQ(status, QPL_STS_OK);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(opcfg, enabled_extract) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Software path does not support OPCFG");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_opcfg_capabilities_present(), "OPCFG capabilities are not present");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(is_operation_disabled_on_all_wq_on_node(opcode_extract),
                                   "Extract disabled on all available workqueues");

    const qpl_path_t execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    const qpl_status status = run_extract_op(execution_path);
    ASSERT_EQ(status, QPL_STS_OK);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(opcfg, enabled_select) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Software path does not support OPCFG");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_opcfg_capabilities_present(), "OPCFG capabilities are not present");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(is_operation_disabled_on_all_wq_on_node(opcode_select),
                                   "Select disabled on all available workqueues");

    const qpl_path_t execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    const qpl_status status = run_select_op(execution_path);
    ASSERT_EQ(status, QPL_STS_OK);
}
QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(opcfg, enabled_expand) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Software path does not support OPCFG");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(!is_opcfg_capabilities_present(), "OPCFG capabilities are not present");
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(is_operation_disabled_on_all_wq_on_node(opcode_expand),
                                   "Expand disabled on all available workqueues");

    const qpl_path_t execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    const qpl_status status = run_expand_op(execution_path);
    ASSERT_EQ(status, QPL_STS_OK);
}

} // namespace qpl::test
