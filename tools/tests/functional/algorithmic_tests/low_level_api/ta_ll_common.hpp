/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TESTS_FUNCTIONAL_ALGORITHMIC_TESTS_LOW_LEVEL_API_COMMON_H_
#define QPL_TESTS_FUNCTIONAL_ALGORITHMIC_TESTS_LOW_LEVEL_API_COMMON_H_

// tests_common
#include "check_result.hpp"
#include "execution_wrapper.hpp"
#include "test_cases.hpp"
#include "test_name_format.hpp"

#define QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(operation, test) TEST(ta##_c_api_##operation, test)

#define QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(operation, test, test_fixture) \
    QPL_FIXTURE_TEST(ta##_c_api_##operation, test, test_fixture)

#define QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(operation, test, test_fixture) \
    QPL_TEST_TC_(ta##_c_api_##operation, test, test_fixture, testing::internal::GetTypeId<test_fixture>())

#endif //QPL_TESTS_FUNCTIONAL_ALGORITHMIC_TESTS_LOW_LEVEL_API_COMMON_H_
