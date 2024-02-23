/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tests
 */

#include "array"
#include "gtest/gtest.h"

// middle-layer
#include "accelerator/hw_accelerator_api.h"
#include "dispatcher/hw_dispatcher.hpp"

// core-iaa
#include "hw_status.h"

#define QPL_INIT_BAD_ARGUMENT_TEST(test) TEST(tb_init, test) /**< Bad argument tests register */

QPL_INIT_BAD_ARGUMENT_TEST(context_null) {
    hw_accelerator_status status = hw_accelerator_get_context(nullptr);

    EXPECT_EQ(status, HW_ACCELERATOR_NULL_PTR_ERR);
}
