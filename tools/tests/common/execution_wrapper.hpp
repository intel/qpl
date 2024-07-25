/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Test API (private C++ API)
 */

#ifndef QPL_TESTS_COMMON_EXECUTION_WRAPPER_HPP_
#define QPL_TESTS_COMMON_EXECUTION_WRAPPER_HPP_

#include "qpl/qpl.h"

#include "qpl_test_environment.hpp"

namespace qpl::test {
static inline auto run_job_api(qpl_job* job_ptr) {
    if (!util::TestEnvironment::GetInstance().IsAsynchronousApiTesting()) {
        return qpl_execute_job(job_ptr);
    } else {
        auto status = qpl_submit_job(job_ptr);

        if (status) { return status; }

        return qpl_wait_job(job_ptr);
    }
}
} // namespace qpl::test

#endif //QPL_TESTS_COMMON_EXECUTION_WRAPPER_HPP_
