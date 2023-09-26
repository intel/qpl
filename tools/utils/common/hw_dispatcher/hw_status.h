/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TOOLS_UTILS_COMMON_HW_STATUS_H_
#define QPL_TOOLS_UTILS_COMMON_HW_STATUS_H_

/**
 * @brief Enumerates the hardware accelerator statuses
 */
typedef enum {
    QPL_TEST_HW_ACCELERATOR_STATUS_OK                    = 0u, /**< Accelerator returned success */
    QPL_TEST_HW_ACCELERATOR_SUPPORT_ERR                  = 1u, /**< System doesn't support accelerator */
    QPL_TEST_HW_ACCELERATOR_LIBACCEL_NOT_FOUND           = 2u, /**< Required version of libaccel-config is not found */
    QPL_TEST_HW_ACCELERATOR_LIBACCEL_ERROR               = 3u, /**< Accelerator instance can not be found */
    QPL_TEST_HW_ACCELERATOR_WORK_QUEUES_NOT_AVAILABLE    = 4u, /**< Enabled work queues are not found or no enabled devices */
    QPL_TEST_HW_ACCELERATOR_NULL_PTR_ERR                 = 5u, /**< Null pointer error */
    QPL_TEST_HW_ACCELERATOR_WQ_IS_BUSY                   = 6u, /**< Work queue is busy with task processing */
} qpl_test_hw_accelerator_status;

#endif //QPL_TOOLS_UTILS_COMMON_HW_STATUS_H_
