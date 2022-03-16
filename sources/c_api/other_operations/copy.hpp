/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Job API (public C API)
 */

#ifndef QPL_SOURCES_C_API_OTHER_OPERATIONS_COPY_HPP_
#define QPL_SOURCES_C_API_OTHER_OPERATIONS_COPY_HPP_

#include "qpl/c_api/defs.h"

/**
 * @anchor COPY_OPERATION
 * @brief Performs a simple memory copy. The application just defines the input and output buffers.
 *        The output buffer must be at least as large as the input buffer, and the two buffers cannot overlap.
 *
 * @param [in,out] job_ptr pointer onto user specified @ref qpl_job
 *
 * @details For operation execution, you must set the following parameters in `qpl_job_ptr`:
 *    - @ref qpl_job.next_in_ptr            - start address of the input stream
 *    - @ref qpl_job.available_in           - number of bytes in the input stream
 *    - @ref qpl_job.next_out_ptr           - start address of output stream
 *    - @ref qpl_job.available_in           - number of available output bytes
 *
 * @warning If any of @ref qpl_job.available_in, @ref qpl_job.available_out is 0,
 *          it will be error (@ref QPL_STS_SIZE_ERR).
 *
 * @return
 *    - @ref QPL_STS_OK
 *    - @ref QPL_STS_NULL_PTR_ERR
 *    - @ref QPL_STS_SIZE_ERR
 *    - @ref QPL_STS_SRC_IS_SHORT_ERR
 *    - @ref QPL_STS_DST_IS_SHORT_ERR
 *    - @ref QPL_STS_OPERATION_ERR
 *
 * Example of main usage:
 * @snippet low-level-api/copy_example.cpp QPL_LOW_LEVEL_COPY_EXAMPLE
 *
 */
uint32_t perform_copy(qpl_job *const job_ptr) noexcept;

#endif //QPL_SOURCES_C_API_OTHER_OPERATIONS_COPY_HPP_
