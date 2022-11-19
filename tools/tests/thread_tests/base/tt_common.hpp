/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TESTS_FUNCTIONAL_THREAD_TESTS_LOW_LEVEL_API_COMMON_H_
#define QPL_TESTS_FUNCTIONAL_THREAD_TESTS_LOW_LEVEL_API_COMMON_H_

#include "../../common/test_cases.hpp"
#include "../../common/test_name_format.hpp"

#define QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(operation, test) \
    TEST(QPL_LOW_LEVEL_SUITE_NAME(tt, operation), test)

#endif //QPL_TESTS_FUNCTIONAL_THREAD_TESTS_LOW_LEVEL_API_COMMON_H_
