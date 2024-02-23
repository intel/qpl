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

#define QPL_INIT_ALGORITHMIC_TEST(test) TEST(ta_init, test) /**< Algorithmic tests register */

QPL_INIT_ALGORITHMIC_TEST(try_init) {
    hw_accelerator_context context;
    hw_accelerator_status status = hw_accelerator_get_context(&context);

    EXPECT_EQ(status, HW_ACCELERATOR_STATUS_OK);
}

QPL_INIT_ALGORITHMIC_TEST(all_devices_found) {
    hw_accelerator_context context;
    hw_accelerator_status status = hw_accelerator_get_context(&context);
    qpl::ml::dispatcher::hw_dispatcher::get_instance();

    EXPECT_EQ(status, HW_ACCELERATOR_STATUS_OK);
}

QPL_INIT_ALGORITHMIC_TEST(all_shared_wq_found) {
    hw_accelerator_context context;
    hw_accelerator_status status = hw_accelerator_get_context(&context);
    qpl::ml::dispatcher::hw_dispatcher::get_instance();

    EXPECT_EQ(status, HW_ACCELERATOR_STATUS_OK);
}

QPL_INIT_ALGORITHMIC_TEST(numa_configuration_read_correctly) {
    hw_accelerator_context context;
    hw_accelerator_status status = hw_accelerator_get_context(&context);
    qpl::ml::dispatcher::hw_dispatcher::get_instance();

    EXPECT_EQ(status, HW_ACCELERATOR_STATUS_OK);
}

QPL_INIT_ALGORITHMIC_TEST(priority_read_correctly) {
    hw_accelerator_context context;
    hw_accelerator_status status = hw_accelerator_get_context(&context);
    qpl::ml::dispatcher::hw_dispatcher::get_instance();

    EXPECT_EQ(status, HW_ACCELERATOR_STATUS_OK);
}

QPL_INIT_ALGORITHMIC_TEST(gencap_read_correctly) {
    hw_accelerator_context context;
    hw_accelerator_status status = hw_accelerator_get_context(&context);
    qpl::ml::dispatcher::hw_dispatcher::get_instance();

    EXPECT_EQ(status, HW_ACCELERATOR_STATUS_OK);
}
