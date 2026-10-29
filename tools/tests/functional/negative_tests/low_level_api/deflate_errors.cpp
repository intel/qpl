/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/
#include <cstdint>

#include "tn_common.hpp"

// tests_common
#include "operation_test.hpp"

// tool_common
#include "dispatcher_checks.hpp"
#include "util.hpp"

namespace qpl::test {

/**
 * @brief Test fixed mode high level compression on @ref qpl_path_software will return
 * proper error (QPL_STS_MORE_OUTPUT_NEEDED) when the destination buffer is not large enough
 * to accommodate the compressed stream and the slop.
 *
 * @note The source buffer size and data is hardcoded to simulate this edge case.
 */
QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(deflate, JobFixture, fixed_high_overflow_with_slop) {
    if (is_effective_hardware(GetExecutionPath())) { GTEST_SKIP() << "Test asserts software-path compressed size."; }

    // This hardcoded source data and size will result in a compressed stream of 10 Bytes.
    // The destination buffer (14 Bytes) is not large enough to accommodate the compressed
    // stream and the slop
    const uint32_t fixed_source_size      = 14U;
    const uint32_t fixed_destination_size = fixed_source_size;
    source.resize(fixed_source_size);
    destination.resize(fixed_destination_size);
    source = {0x5c, 0x01, 0xef, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x5c};

    // Performing a compression operation
    job_ptr->op            = qpl_op_compress;
    job_ptr->level         = qpl_high_level;
    job_ptr->next_in_ptr   = source.data();
    job_ptr->next_out_ptr  = destination.data();
    job_ptr->available_in  = fixed_source_size;
    job_ptr->available_out = fixed_destination_size;
    job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY;

    // Compression
    auto compression_status = run_job_api(job_ptr);

    ASSERT_EQ(compression_status, QPL_STS_MORE_OUTPUT_NEEDED);
}
} // namespace qpl::test
