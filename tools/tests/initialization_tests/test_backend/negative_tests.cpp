/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tests
 */

#include "gtest/gtest.h"

// middle-layer
#include "accelerator/hw_accelerator_api.h"

// core-iaa
#include "hw_status.h"

/**
 * @brief Negative tests register
 */
#define QPL_INIT_NEGATIVE_TEST(test, expected_value) \
TEST(tn_init, test) { \
    hw_accelerator_context context; \
    hw_accelerator_status  status = hw_accelerator_get_context(&context); \
    EXPECT_EQ(status, expected_value); } \

QPL_INIT_NEGATIVE_TEST(libaccel_not_found, HW_ACCELERATOR_LIBACCEL_NOT_FOUND);
QPL_INIT_NEGATIVE_TEST(instance_not_found, HW_ACCELERATOR_LIBACCEL_ERROR);
QPL_INIT_NEGATIVE_TEST(work_queues_not_available, HW_ACCELERATOR_WORK_QUEUES_NOT_AVAILABLE);
