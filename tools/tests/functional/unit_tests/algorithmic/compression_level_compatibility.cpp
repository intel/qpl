/*******************************************************************************
 * Copyright (C) 2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 * Backward compatibility tests for the compression levels feature: existing
 * enum values and aliases must keep their meaning so that pre-existing code
 * continues to behave as before.
 */

#include <cstdint>
#include <string>
#include <vector>

#include "qpl/qpl.h"

#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"
#include "t_common.hpp"

namespace qpl::test {

// Existing enum values must never change (ABI stability).
QPL_UNIT_API_ALGORITHMIC_TEST(compression_level_compatibility, enum_values_unchanged) {
    EXPECT_EQ(static_cast<int>(qpl_level_0), 0);
    EXPECT_EQ(static_cast<int>(qpl_level_1), 1);
    EXPECT_EQ(static_cast<int>(qpl_level_2), 2);
    EXPECT_EQ(static_cast<int>(qpl_level_3), 3);
    EXPECT_EQ(static_cast<int>(qpl_level_7), 7);
    EXPECT_EQ(static_cast<int>(qpl_level_9), 9);
}

// Aliases must keep pointing to the same levels as before the feature.
QPL_UNIT_API_ALGORITHMIC_TEST(compression_level_compatibility, aliases_unchanged) {
    EXPECT_EQ(qpl_default_level, qpl_level_1);
    EXPECT_EQ(qpl_fast_level, qpl_level_1);
    EXPECT_EQ(qpl_high_level, qpl_level_7);
}

// qpl_high_level (previously rejected on the hardware path) must now execute
// on every path thanks to level saturation.
QPL_UNIT_API_ALGORITHMIC_TEST(compression_level_compatibility, high_level_executes) {
    const qpl_path_t path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint32_t job_size = 0;
    ASSERT_EQ(qpl_get_job_size(path, &job_size), QPL_STS_OK);

    std::vector<uint8_t> job_buffer(job_size);
    auto*                job = reinterpret_cast<qpl_job*>(job_buffer.data());
    ASSERT_EQ(qpl_init_job(path, job), QPL_STS_OK);

    std::string text = "Backward compatibility payload for qpl_high_level.";
    for (int i = 0; i < 16; i++) {
        text += "ABCDEFGHIJKLMNOP";
    }
    std::vector<uint8_t> input(text.begin(), text.end());
    std::vector<uint8_t> output(input.size() * 2 + 64);

    job->op            = qpl_op_compress;
    job->level         = qpl_high_level;
    job->next_in_ptr   = input.data();
    job->available_in  = static_cast<uint32_t>(input.size());
    job->next_out_ptr  = output.data();
    job->available_out = static_cast<uint32_t>(output.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN;

    const qpl_status status = qpl_execute_job(job);
    qpl_fini_job(job);

    EXPECT_EQ(status, QPL_STS_OK) << "qpl_high_level must execute on all paths";
}

// qpl_gather_deflate_statistics used to accept only the qpl_default_level and
// qpl_high_level aliases. Moving qpl_high_level from 3 to 7 must not make a
// previously valid qpl_level_3 call fail: every level of the enum is accepted and
// saturated, like everywhere else in the API.
QPL_UNIT_API_ALGORITHMIC_TEST(compression_level_compatibility, statistics_accepts_all_levels) {
    const qpl_path_t path = util::TestEnvironment::GetInstance().GetExecutionPath();

    std::string text = "Statistics payload for every compression level.";
    for (int i = 0; i < 16; i++) {
        text += "ABCDEFGHIJKLMNOP";
    }
    std::vector<uint8_t> input(text.begin(), text.end());

    for (int level = qpl_level_0; level <= qpl_level_9; level++) {
        qpl_histogram histogram {};
        EXPECT_EQ(qpl_gather_deflate_statistics(input.data(), static_cast<uint32_t>(input.size()), &histogram,
                                                static_cast<qpl_compression_levels>(level), path),
                  QPL_STS_OK)
                << "Statistics gathering must support level " << level;
    }
}

// Levels outside the enum range stay rejected.
QPL_UNIT_API_ALGORITHMIC_TEST(compression_level_compatibility, statistics_rejects_invalid_level) {
    const qpl_path_t path = util::TestEnvironment::GetInstance().GetExecutionPath();

    uint8_t       source = 0U;
    qpl_histogram histogram {};

    EXPECT_EQ(qpl_gather_deflate_statistics(&source, 1U, &histogram,
                                            static_cast<qpl_compression_levels>(qpl_level_9 + 1), path),
              QPL_STS_UNSUPPORTED_COMPRESSION_LEVEL);
}

} // namespace qpl::test
