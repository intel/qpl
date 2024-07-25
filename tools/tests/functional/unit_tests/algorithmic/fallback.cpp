/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tests
 */

#include "job.hpp"
#include "t_common.hpp"

namespace qpl::test {

using namespace qpl::job;

constexpr auto incorrect_operation = static_cast<qpl_operation>(0x3FU);
constexpr auto no_flags            = 0U;

QPL_UNIT_API_ALGORITHMIC_TEST(fallback, filtering) {
    qpl_job job;

    job.op = qpl_op_extract;
    EXPECT_TRUE(is_extract(&job));

    job.op = incorrect_operation;
    EXPECT_FALSE(is_extract(&job));

    job.op = qpl_op_select;
    EXPECT_TRUE(is_select(&job));

    job.op = incorrect_operation;
    EXPECT_FALSE(is_select(&job));

    job.op = qpl_op_expand;
    EXPECT_TRUE(is_expand(&job));

    job.op = incorrect_operation;
    EXPECT_FALSE(is_expand(&job));

    job.op = qpl_op_scan_eq;
    EXPECT_TRUE(is_scan(&job));

    job.op = qpl_op_scan_ne;
    EXPECT_TRUE(is_scan(&job));

    job.op = qpl_op_scan_lt;
    EXPECT_TRUE(is_scan(&job));

    job.op = qpl_op_scan_le;
    EXPECT_TRUE(is_scan(&job));

    job.op = qpl_op_scan_gt;
    EXPECT_TRUE(is_scan(&job));

    job.op = qpl_op_scan_ge;
    EXPECT_TRUE(is_scan(&job));

    job.op = qpl_op_scan_range;
    EXPECT_TRUE(is_scan(&job));

    job.op = qpl_op_scan_not_range;
    EXPECT_TRUE(is_scan(&job));
}

QPL_UNIT_API_ALGORITHMIC_TEST(fallback, inflate) {
    qpl_job job;

    job.op = qpl_op_decompress;
    EXPECT_TRUE(is_decompression(&job));

    job.op    = qpl_op_decompress;
    job.flags = QPL_FLAG_RND_ACCESS;
    EXPECT_TRUE(is_random_decompression(&job));

    job.op    = qpl_op_decompress;
    job.flags = QPL_FLAG_NO_HDRS;
    EXPECT_TRUE(is_huffman_only_decompression(&job));

    job.op    = qpl_op_decompress;
    job.flags = QPL_FLAG_CANNED_MODE;
    EXPECT_TRUE(is_canned_mode_decompression(&job));

    job.op = incorrect_operation;
    EXPECT_FALSE(is_decompression(&job));

    job.op    = qpl_op_decompress;
    job.flags = no_flags;
    EXPECT_FALSE(is_random_decompression(&job));

    job.op    = incorrect_operation;
    job.flags = QPL_FLAG_RND_ACCESS;
    EXPECT_FALSE(is_random_decompression(&job));

    job.op    = qpl_op_decompress;
    job.flags = no_flags;
    EXPECT_FALSE(is_huffman_only_decompression(&job));

    job.op    = incorrect_operation;
    job.flags = QPL_FLAG_NO_HDRS;
    EXPECT_FALSE(is_huffman_only_decompression(&job));

    job.op    = qpl_op_decompress;
    job.flags = no_flags;
    EXPECT_FALSE(is_canned_mode_decompression(&job));

    job.op    = incorrect_operation;
    job.flags = QPL_FLAG_CANNED_MODE;
    EXPECT_FALSE(is_canned_mode_decompression(&job));
}

QPL_UNIT_API_ALGORITHMIC_TEST(fallback, deflate) {
    qpl_job job;

    job.op    = qpl_op_compress;
    job.flags = QPL_FLAG_CANNED_MODE; // @todo fix condition
    EXPECT_TRUE(is_compression(&job));
    job.flags = QPL_FLAG_CANNED_MODE;
    EXPECT_TRUE(is_canned_mode_compression(&job));

    job.level = qpl_high_level;
    EXPECT_TRUE(is_high_level_compression(&job));

    job.op    = qpl_op_compress;
    job.flags = QPL_FLAG_GEN_LITERALS;
    EXPECT_TRUE(is_huffman_only_compression(&job));

    job.op = incorrect_operation;
    EXPECT_FALSE(is_compression(&job));

    job.op    = qpl_op_compress;
    job.flags = QPL_FLAG_FIRST | QPL_FLAG_LAST;
    EXPECT_FALSE(is_canned_mode_compression(&job));

    job.op    = incorrect_operation;
    job.flags = QPL_FLAG_CANNED_MODE;
    EXPECT_FALSE(is_canned_mode_compression(&job));

    job.op    = qpl_op_compress;
    job.level = qpl_default_level;
    EXPECT_FALSE(is_high_level_compression(&job));

    job.op    = incorrect_operation;
    job.flags = qpl_high_level;
    EXPECT_FALSE(is_high_level_compression(&job));

    job.op    = incorrect_operation;
    job.flags = QPL_FLAG_GEN_LITERALS;
    EXPECT_FALSE(is_huffman_only_compression(&job));

    job.op    = qpl_op_compress;
    job.flags = no_flags;
    EXPECT_FALSE(is_huffman_only_compression(&job));
}

QPL_UNIT_API_ALGORITHMIC_TEST(fallback, others) {
    qpl_job job;

    // @todo call to middle layer
    job.op = qpl_op_crc64;
}

QPL_UNIT_API_ALGORITHMIC_TEST(fallback, multi_job_support) {
    qpl_job job;

    job.op = qpl_op_compress;

    // Single job checks
    job.flags = 0U;
    EXPECT_FALSE(is_single_job(&job));

    job.flags = QPL_FLAG_FIRST;
    EXPECT_FALSE(is_single_job(&job));

    job.flags = QPL_FLAG_LAST;
    EXPECT_FALSE(is_single_job(&job));

    job.flags = QPL_FLAG_FIRST | QPL_FLAG_LAST;
    EXPECT_TRUE(is_single_job(&job));

    // Multi job checks
    job.flags = 0U;
    EXPECT_TRUE(is_multi_job(&job));

    job.flags = QPL_FLAG_FIRST;
    EXPECT_TRUE(is_multi_job(&job));

    job.flags = QPL_FLAG_LAST;
    EXPECT_TRUE(is_multi_job(&job));

    job.flags = QPL_FLAG_FIRST | QPL_FLAG_LAST;
    EXPECT_FALSE(is_multi_job(&job));
}
} // namespace qpl::test
