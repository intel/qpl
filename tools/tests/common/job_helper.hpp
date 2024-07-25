/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_JOB_HELPER_HPP
#define QPL_JOB_HELPER_HPP

#include "qpl/qpl.h"

// tests_common
#include "test_sources.hpp"

namespace qpl::test::job_helper {

template <qpl_operation, class... Arguments>
inline auto fill_job(qpl_job* const job_ptr, Arguments... args);

static inline auto is_two_source_filtering(qpl_job* const job_ptr) {
    return job_ptr->op == qpl_op_expand || job_ptr->op == qpl_op_select;
}

} // namespace qpl::test::job_helper

#endif // QPL_JOB_HELPER_HPP
