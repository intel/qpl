/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TESTS_FUNCTIONAL_UNIT_TESTS_API_COMMON_H_
#define QPL_TESTS_FUNCTIONAL_UNIT_TESTS_API_COMMON_H_

#include "gtest/gtest.h"

// tests_common
#include "check_result.hpp"
#include "test_name_format.hpp"

#define QPL_UNIT_API_ALGORITHMIC_TEST(entity, test) TEST(QPL_UNIT_SUITE_NAME(ta, entity), test)

#define QPL_UNIT_API_BAD_ARGUMENT_TEST(entity, test) TEST(QPL_UNIT_SUITE_NAME(tb, entity), test)

#endif //QPL_TESTS_FUNCTIONAL_UNIT_TESTS_API_COMMON_H_
