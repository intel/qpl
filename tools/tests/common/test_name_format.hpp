/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TESTS_COMMON_TEST_NAME_FORMAT_HPP_
#define QPL_TESTS_COMMON_TEST_NAME_FORMAT_HPP_

#define QPL_SUITE_NAME(type, api, tested_entity) type##_##api##_##tested_entity

#define QPL_LOW_LEVEL_SUITE_NAME(type, tested_entity) QPL_SUITE_NAME(type, c_api, tested_entity)

#define QPL_UNIT_SUITE_NAME(type, tested_entity) QPL_SUITE_NAME(type, unit_api, tested_entity)

#endif //QPL_TESTS_COMMON_TEST_NAME_FORMAT_HPP_
