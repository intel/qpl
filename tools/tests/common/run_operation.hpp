/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TOOLS_TESTS_COMMON_RUN_OPERATION_HPP_
#define QPL_TOOLS_TESTS_COMMON_RUN_OPERATION_HPP_

#include "qpl/qpl.h"

namespace qpl::test {

/**
 * @brief Runs an example-like decompress operation without
 * any calls to any other operations for setup/error-checking.
 * Returns the status of execution or error on initialization
*/
qpl_status run_decompress_op(qpl_path_t execution_path);

/**
 * @brief Runs an example-like compression operation without
 * any calls to any other operations for setup/error-checking.
 * Returns the status of execution or error on initialization
*/
qpl_status run_compress_op(qpl_path_t execution_path);

/**
 * @brief Runs an example-like crc64 operation without
 * any calls to any other operations for setup/error-checking.
 * Returns the status of execution or error on initialization
*/
qpl_status run_crc64_op(qpl_path_t execution_path);

/**
 * @brief Runs an example-like scan operation without
 * any calls to any other operations for setup/error-checking.
 * Returns the status of execution or error on initialization
*/
qpl_status run_scan_op(qpl_path_t execution_path);

/**
 * @brief Runs an example-like extract operation without
 * any calls to any other operations for setup/error-checking.
 * Returns the status of execution or error on initialization
*/
qpl_status run_extract_op(qpl_path_t execution_path);

/**
 * @brief Runs an example-like select operation without
 * any calls to any other operations for setup/error-checking.
 * Returns the status of execution or error on initialization
*/
qpl_status run_select_op(qpl_path_t execution_path);

/**
 * @brief Runs an example-like expand operation without
 * any calls to any other operations for setup/error-checking.
 * Returns the status of execution or error on initialization
*/
qpl_status run_expand_op(qpl_path_t execution_path);

} // namespace qpl::test

#endif // QPL_TOOLS_TESTS_COMMON_RUN_OPERATION_HPP_
