/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tests
 */

#include "qpl/qpl.h"
#include "../t_common.hpp"

extern uint32_t perform_copy(qpl_job *const job_ptr) noexcept;

namespace qpl::test {

    QPL_UNIT_API_BAD_ARGUMENT_TEST(qpl_copy_8u, base_interface_test) {
        qpl_job    job;
        job.op = qpl_op_crc64;

        auto status = perform_copy(nullptr);

        EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on job_ptr == nullptr";

        status = perform_copy(&job);

        EXPECT_EQ(status, QPL_STS_OPERATION_ERR) << "Failed on op != Copy";
    }
}