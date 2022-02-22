/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "gtest/gtest.h"
#include "../../../common/operation_test.hpp"
#include "tb_ll_common.hpp"

namespace qpl::test {
    QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(copy, input_stream_validation) {
        std::array<uint8_t, SOURCE_ARRAY_SIZE>      source{};
        std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination{};

        // Parameter preset
        set_output_stream(job_ptr,
                          destination.data(),
                          DESTINATION_ARRAY_SIZE,
                          static_cast<qpl_out_format>( NOT_APPLICABLE_PARAMETER));
        job_ptr->op = qpl_op_memcpy;

        // Source checks
        set_input_stream(job_ptr,
                         nullptr,
                         SOURCE_ARRAY_SIZE,
                         NOT_APPLICABLE_PARAMETER,
                         NOT_APPLICABLE_PARAMETER,
                         static_cast<qpl_parser>( NOT_APPLICABLE_PARAMETER));
        EXPECT_EQ(run_job_api(job_ptr), QPL_STS_NULL_PTR_ERR) << "Fail on: source ==  null";

        set_input_stream(job_ptr,
                         source.data(),
                         0u,
                         NOT_APPLICABLE_PARAMETER,
                         NOT_APPLICABLE_PARAMETER,
                         static_cast<qpl_parser>( NOT_APPLICABLE_PARAMETER));
        EXPECT_EQ(run_job_api(job_ptr), QPL_STS_SIZE_ERR) << "Fail on: source size == 0";
    }

    QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(copy, output_stream_validation) {
        std::array<uint8_t, SOURCE_ARRAY_SIZE>      source{};
        std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination{};

        job_ptr->op = qpl_op_memcpy;

        // Parameter preset
        set_input_stream(job_ptr,
                         source.data(),
                         SOURCE_ARRAY_SIZE,
                         NOT_APPLICABLE_PARAMETER,
                         NOT_APPLICABLE_PARAMETER,
                         static_cast<qpl_parser>( NOT_APPLICABLE_PARAMETER));

        // Source checks
        set_output_stream(job_ptr,
                          nullptr,
                          DESTINATION_ARRAY_SIZE,
                          static_cast<qpl_out_format>( NOT_APPLICABLE_PARAMETER));
        EXPECT_EQ(run_job_api(job_ptr), QPL_STS_NULL_PTR_ERR) << "Fail on: destination ==  null";

        set_output_stream(job_ptr,
                          destination.data(),
                          0u,
                          static_cast<qpl_out_format>( NOT_APPLICABLE_PARAMETER));
        EXPECT_EQ(run_job_api(job_ptr), QPL_STS_SIZE_ERR) << "Fail on: destination size == 0";

        set_output_stream(job_ptr,
                          destination.data(),
                          SOURCE_ARRAY_SIZE - 1u,
                          static_cast<qpl_out_format>( NOT_APPLICABLE_PARAMETER));
        EXPECT_EQ(run_job_api(job_ptr), QPL_STS_DST_IS_SHORT_ERR) << "Fail on: destination size == source size - 1";
    }

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(copy, buffers_overlap) {
    check_buffer_overlap(job_ptr, qpl_op_memcpy, OPERATION_FLAGS);
}
}
