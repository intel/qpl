/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_ALGORITHMIC_TESTS_HIGH_LEVEL_API_TA_COMMON_H_
#define QPL_ALGORITHMIC_TESTS_HIGH_LEVEL_API_TA_COMMON_H_

#include "../../../common/test_cases.hpp"
#include "../../../common/test_name_format.hpp"

#define QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(operation, test) \
    TEST(ta##_cpp_api_##operation, test)

#define QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_F(operation, test, test_fixture) \
    QPL_FIXTURE_TEST(ta##_cpp_api_##operation, test, test_fixture)

#define QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(operation, test, test_fixture) \
    QPL_TEST_TC_(ta##_cpp_api_##operation, test, test_fixture, testing::internal::GetTypeId<test_fixture>())

#endif //QPL_ALGORITHMIC_TESTS_HIGH_LEVEL_API_TA_COMMON_H_
