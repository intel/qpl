/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TESTS_FUNCTIONAL_NEGATIVE_TESTS_LOW_LEVEL_API_COMMON_H_
#define QPL_TESTS_FUNCTIONAL_NEGATIVE_TESTS_LOW_LEVEL_API_COMMON_H_

#include "gtest/gtest.h"

// tests_common
#include "check_result.hpp"
#include "execution_wrapper.hpp"

#define QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(test_suite_name, test_fixture, test_name) \
    GTEST_TEST_(tn_c_api_##test_suite_name, test_name, test_fixture, testing::internal::GetTypeId<JobFixture>())

#define QPL_LOW_LEVEL_API_NEGATIVE_TEST(test_suite_name, test_name) GTEST_TEST(tn_c_api_##test_suite_name, test_name)

#endif //QPL_TESTS_FUNCTIONAL_NEGATIVE_TESTS_LOW_LEVEL_API_COMMON_H_
