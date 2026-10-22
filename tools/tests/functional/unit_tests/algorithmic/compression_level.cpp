/*******************************************************************************
 * Copyright (C) 2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 * Correctness tests for deflate compression levels. Validated by invariants:
 *   1. Round-trip   - every supported level decompresses back to the original.
 *   2. SW saturation- every level is accepted: 0-1 behave as the default level,
 *                     2-9 saturate to the high level.
 *   3. HW saturation- IAA gen3: 0-7 native, 8-9 saturate to 7; gen<=2: all == 1.
 *   4. Levels live  - native-range levels must not all collapse to one output.
 *   5. Canned mode  - statistics gathering accepts every level, so canned
 *                     round-trips at 0-9.
 */

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <set>
#include <vector>

#include "qpl/qpl.h"

#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"
#include "t_common.hpp"

// tool_common
#include "dispatcher_checks.hpp"
#include "iaa_features_checks.hpp"

namespace qpl::test {

namespace {

// Compresses input at the given level, empty vector on failure. Every level uses
// dynamic Huffman. Correctness is validated by a separate decompress pass.
std::vector<uint8_t> compress(qpl_path_t path, const std::vector<uint8_t>& input, int level) {
    uint32_t job_size = 0;
    if (qpl_get_job_size(path, &job_size) != QPL_STS_OK) { return {}; }

    std::vector<uint8_t> job_buffer(job_size);
    auto*                job = reinterpret_cast<qpl_job*>(job_buffer.data());
    if (qpl_init_job(path, job) != QPL_STS_OK) { return {}; }

    std::vector<uint8_t> output(qpl_get_safe_deflate_compression_buffer_size(static_cast<uint32_t>(input.size())) +
                                64U);

    job->op            = qpl_op_compress;
    job->level         = static_cast<qpl_compression_levels>(level);
    job->next_in_ptr   = const_cast<uint8_t*>(input.data());
    job->available_in  = static_cast<uint32_t>(input.size());
    job->next_out_ptr  = output.data();
    job->available_out = static_cast<uint32_t>(output.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY | QPL_FLAG_DYNAMIC_HUFFMAN;

    const qpl_status status = qpl_execute_job(job);
    const uint32_t   total  = job->total_out;
    qpl_fini_job(job);

    if (status != QPL_STS_OK) { return {}; }
    output.resize(total);
    return output;
}

// Same as compress() but drives the job through the async submit/wait API.
std::vector<uint8_t> compress_async(qpl_path_t path, const std::vector<uint8_t>& input, int level) {
    uint32_t job_size = 0;
    if (qpl_get_job_size(path, &job_size) != QPL_STS_OK) { return {}; }

    std::vector<uint8_t> job_buffer(job_size);
    auto*                job = reinterpret_cast<qpl_job*>(job_buffer.data());
    if (qpl_init_job(path, job) != QPL_STS_OK) { return {}; }

    std::vector<uint8_t> output(qpl_get_safe_deflate_compression_buffer_size(static_cast<uint32_t>(input.size())) +
                                64U);

    job->op            = qpl_op_compress;
    job->level         = static_cast<qpl_compression_levels>(level);
    job->next_in_ptr   = const_cast<uint8_t*>(input.data());
    job->available_in  = static_cast<uint32_t>(input.size());
    job->next_out_ptr  = output.data();
    job->available_out = static_cast<uint32_t>(output.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY | QPL_FLAG_DYNAMIC_HUFFMAN;

    qpl_status status = qpl_submit_job(job);
    if (status == QPL_STS_OK) { status = qpl_wait_job(job); }
    const uint32_t total = job->total_out;
    qpl_fini_job(job);

    if (status != QPL_STS_OK) { return {}; }
    output.resize(total);
    return output;
}

// Decompresses input, returns the exact decompressed bytes. Empty on failure.
std::vector<uint8_t> decompress(qpl_path_t path, const std::vector<uint8_t>& compressed, size_t original_size) {
    uint32_t job_size = 0;
    if (qpl_get_job_size(path, &job_size) != QPL_STS_OK) { return {}; }

    std::vector<uint8_t> job_buffer(job_size);
    auto*                job = reinterpret_cast<qpl_job*>(job_buffer.data());
    if (qpl_init_job(path, job) != QPL_STS_OK) { return {}; }

    std::vector<uint8_t> output(original_size);

    job->op            = qpl_op_decompress;
    job->next_in_ptr   = const_cast<uint8_t*>(compressed.data());
    job->available_in  = static_cast<uint32_t>(compressed.size());
    job->next_out_ptr  = output.data();
    job->available_out = static_cast<uint32_t>(output.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;

    const qpl_status status = qpl_execute_job(job);
    const uint32_t   total  = job->total_out;
    qpl_fini_job(job);

    if (status != QPL_STS_OK) { return {}; }
    output.resize(total);
    return output;
}

// Returns the largest dataset file (best level separation), empty if unavailable.
std::vector<uint8_t> largest_dataset_file() {
    const auto&                 dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();
    const std::vector<uint8_t>* best    = nullptr;
    for (const auto& [name, data] : dataset.get_data()) {
        if (best == nullptr || data.size() > best->size()) { best = &data; }
    }
    return (best != nullptr) ? *best : std::vector<uint8_t> {};
}

// Builds a canned-mode Huffman table from the input statistics gathered at the given
// level, exactly like the benchmark and canned-mode callers do. Null on failure.
qpl_huffman_table_t canned_table(qpl_path_t path, const std::vector<uint8_t>& input, int level) {
    qpl_huffman_table_t table     = nullptr;
    const allocator_t   allocator = DEFAULT_ALLOCATOR_C;
    if (qpl_deflate_huffman_table_create(combined_table_type, path, allocator, &table) != QPL_STS_OK) {
        return nullptr;
    }

    qpl_histogram histogram {};
    qpl_status    status =
            qpl_gather_deflate_statistics(const_cast<uint8_t*>(input.data()), static_cast<uint32_t>(input.size()),
                                          &histogram, static_cast<qpl_compression_levels>(level), path);
    if (status == QPL_STS_OK) { status = qpl_huffman_table_init_with_histogram(table, &histogram); }

    if (status != QPL_STS_OK) {
        qpl_huffman_table_destroy(table);
        return nullptr;
    }
    return table;
}

// Canned compress/decompress with the given table, true on a successful round-trip.
bool canned_round_trip(qpl_path_t path, const std::vector<uint8_t>& input, int level, qpl_huffman_table_t table) {
    uint32_t job_size = 0;
    if (qpl_get_job_size(path, &job_size) != QPL_STS_OK) { return false; }

    std::vector<uint8_t> job_buffer(job_size);
    auto*                job = reinterpret_cast<qpl_job*>(job_buffer.data());
    if (qpl_init_job(path, job) != QPL_STS_OK) { return false; }

    std::vector<uint8_t> compressed(qpl_get_safe_deflate_compression_buffer_size(static_cast<uint32_t>(input.size())) +
                                    64U);

    job->op            = qpl_op_compress;
    job->level         = static_cast<qpl_compression_levels>(level);
    job->next_in_ptr   = const_cast<uint8_t*>(input.data());
    job->available_in  = static_cast<uint32_t>(input.size());
    job->next_out_ptr  = compressed.data();
    job->available_out = static_cast<uint32_t>(compressed.size());
    job->huffman_table = table;
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY | QPL_FLAG_CANNED_MODE;

    qpl_status status = qpl_execute_job(job);

    // The same job is reused for decompression, qpl_init_job is called once per allocation
    std::vector<uint8_t> restored(input.size());
    if (status == QPL_STS_OK) {
        compressed.resize(job->total_out);

        job->op            = qpl_op_decompress;
        job->next_in_ptr   = compressed.data();
        job->available_in  = static_cast<uint32_t>(compressed.size());
        job->next_out_ptr  = restored.data();
        job->available_out = static_cast<uint32_t>(restored.size());
        job->huffman_table = table;
        job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE;

        status = qpl_execute_job(job);
    }
    qpl_fini_job(job);

    return status == QPL_STS_OK && restored == input;
}

qpl_path_t current_path() {
    return util::TestEnvironment::GetInstance().GetExecutionPath();
}

bool effective_hardware() {
    return is_effective_hardware(current_path());
}

} // namespace

// Every level 0-9 is accepted and round-trips on every dataset file, printing the ratio.
QPL_UNIT_API_ALGORITHMIC_TEST(compression_levels, round_trip_all_levels) {
    const qpl_path_t path    = current_path();
    const auto&      dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();
    if (dataset.get_data().empty()) { GTEST_SKIP() << "Algorithmic dataset not available"; }

    for (const auto& [name, data] : dataset.get_data()) {
        if (data.empty()) { continue; }
        std::cout << name << ":";
        for (int level = 0; level <= 9; level++) {
            const std::vector<uint8_t> compressed = compress(path, data, level);
            ASSERT_FALSE(compressed.empty()) << "Compression failed. file=" << name << " level=" << level;

            const std::vector<uint8_t> restored = decompress(path, compressed, data.size());
            ASSERT_EQ(restored, data) << "Round-trip mismatch. file=" << name << " level=" << level;

            const double ratio = static_cast<double>(data.size()) / static_cast<double>(compressed.size());
            std::cout << " L" << level << "=" << ratio;
        }
        std::cout << "\n";
    }
}

// Software path saturates to its highest level: qpl_high_level and levels above it
// produce output identical to qpl_level_3. Runs on the software path, and on the auto
// path when no hardware is available (auto then falls back to software).
QPL_UNIT_API_ALGORITHMIC_TEST(compression_levels, software_saturation) {
    if (effective_hardware()) { GTEST_SKIP() << "Runs on the effective software path only"; }

    const qpl_path_t           path = current_path();
    const std::vector<uint8_t> data = largest_dataset_file();
    if (data.empty()) { GTEST_SKIP() << "Algorithmic dataset not available"; }

    // Level 0 maps down to the default level (1), not to the high level.
    EXPECT_EQ(compress(path, data, 0), compress(path, data, 1)) << "SW level 0 must behave as the default level";

    const std::vector<uint8_t> reference = compress(path, data, 3);
    ASSERT_FALSE(reference.empty());
    for (const int level : {static_cast<int>(qpl_high_level), 4, 5, 6, 7, 8, 9}) {
        EXPECT_EQ(compress(path, data, level), reference) << "SW level " << level << " must saturate to level 3";
    }
}

// Hardware path saturation, generation dependent. Runs on the hardware path, and on
// the auto path when hardware is available.
QPL_UNIT_API_ALGORITHMIC_TEST(compression_levels, hardware_saturation) {
    if (!effective_hardware()) { GTEST_SKIP() << "Runs on the effective hardware path only"; }

    const qpl_path_t           path = current_path();
    const std::vector<uint8_t> data = largest_dataset_file();
    if (data.empty()) { GTEST_SKIP() << "Algorithmic dataset not available"; }

    if (are_iaa_gen_3_min_capabilities_present()) {
        // Gen3: levels 0-7 native, 8-9 saturate to 7.
        const std::vector<uint8_t> reference = compress(path, data, 7);
        ASSERT_FALSE(reference.empty());
        EXPECT_EQ(compress(path, data, 8), reference) << "Gen3 level 8 must saturate to level 7";
        EXPECT_EQ(compress(path, data, 9), reference) << "Gen3 level 9 must saturate to level 7";
    } else {
        // Gen <=2: only the default level is supported, all levels behave as level 1.
        const std::vector<uint8_t> reference = compress(path, data, 1);
        ASSERT_FALSE(reference.empty());
        for (int level = 0; level <= 9; level++) {
            if (level == 1) { continue; }
            EXPECT_EQ(compress(path, data, level), reference) << "Gen<=2 level " << level << " must behave as level 1";
        }
    }
}

// Native levels must produce more than one distinct output (level field reaches
// the compressor). Skipped where all levels coincide (HW gen <=2). The auto path
// runs on hardware when available, otherwise it falls back to software.
QPL_UNIT_API_ALGORITHMIC_TEST(compression_levels, native_levels_are_live) {
    const qpl_path_t path        = current_path();
    const bool       on_hardware = effective_hardware();
    if (on_hardware && !are_iaa_gen_3_min_capabilities_present()) {
        GTEST_SKIP() << "HW gen <=2 maps every level to the default level";
    }

    const std::vector<uint8_t> data = largest_dataset_file();
    if (data.empty()) { GTEST_SKIP() << "Algorithmic dataset not available"; }

    // Distinct compressed sizes are enough to prove the level reaches the compressor.
    // Level 0 saturates to the default level, so it duplicates level 1 (deduped by the set).
    std::set<size_t> distinct_sizes;
    // SW distinguishes default(1) from high(3); gen3 exposes native levels 1-7.
    const int max_level = on_hardware ? 7 : 3;
    for (int level = 0; level <= max_level; level++) {
        const auto out = compress(path, data, level);
        ASSERT_FALSE(out.empty()) << "Compression failed. level=" << level;
        distinct_sizes.insert(out.size());
    }

    // Gen3 native levels should span several distinct sizes; SW has just default vs high.
    const size_t min_distinct = on_hardware ? 4U : 2U;
    EXPECT_GE(distinct_sizes.size(), min_distinct)
            << "Native levels produced too few distinct outputs - level is likely not being applied";
}

// Canned mode must work at every level: qpl_gather_deflate_statistics is the gate here,
// it has to accept the whole enum range and saturate instead of rejecting levels.
QPL_UNIT_API_ALGORITHMIC_TEST(compression_levels, canned_round_trip_all_levels) {
    const qpl_path_t           path = current_path();
    const std::vector<uint8_t> data = largest_dataset_file();
    if (data.empty()) { GTEST_SKIP() << "Algorithmic dataset not available"; }

    for (int level = 0; level <= 9; level++) {
        qpl_huffman_table_t table = canned_table(path, data, level);
        ASSERT_NE(table, nullptr) << "Canned table creation failed. level=" << level;

        const bool restored = canned_round_trip(path, data, level, table);
        qpl_huffman_table_destroy(table);

        EXPECT_TRUE(restored) << "Canned round-trip failed. level=" << level;
    }
}

// The async submit/wait API must succeed and apply the level exactly like the sync
// API: output must be non-empty and byte-identical at every level. Combined with
// round_trip_all_levels (sync decompresses), this also proves async round-trips.
QPL_UNIT_API_ALGORITHMIC_TEST(compression_levels, async_matches_sync) {
    const qpl_path_t path    = current_path();
    const auto&      dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();
    if (dataset.get_data().empty()) { GTEST_SKIP() << "Algorithmic dataset not available"; }

    for (const auto& [name, data] : dataset.get_data()) {
        if (data.empty()) { continue; }
        for (int level = 0; level <= 9; level++) {
            const std::vector<uint8_t> async_out = compress_async(path, data, level);
            ASSERT_FALSE(async_out.empty()) << "Async compression failed. file=" << name << " level=" << level;
            EXPECT_EQ(async_out, compress(path, data, level))
                    << "Async output differs from sync. file=" << name << " level=" << level;
        }
    }
}

} // namespace qpl::test
