/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tests
 */

#ifndef QPL_TEST_ARGUMENTS_LIST
#define QPL_TEST_ARGUMENTS_LIST

#include <cstdint>
#include <string>
#include <vector>

#include "qpl/c_api/defs.h"

// All Intel QPL test arguments defined there:
#define QPL_ARG_SEED         "--seed"
#define QPL_ARG_PATH         "--path"
#define QPL_ARG_DATASET_PATH "--dataset"
#define QPL_ARG_TEST_CASE_ID "--testid"
#define QPL_ARG_ASYNC        "--async"
#define QPL_ARG_HELP         "--qpl-tests-help"

namespace qpl::test::util {
/**
     * @briеf Structure that holds parsed arguments
     */
struct arguments_list_t {
    qpl_path_t  execution_path       = qpl_path_software;
    uint32_t    seed                 = {};
    std::string path_to_dataset      = {};
    uint32_t    test_case_id         = {};
    bool        is_test_case_id_used = false;
    bool        is_async_testing     = false;
};
} // namespace qpl::test::util

#endif //QPL_TEST_ARGUMENTS_LIST
