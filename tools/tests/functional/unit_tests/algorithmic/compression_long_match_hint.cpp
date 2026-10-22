/*******************************************************************************
 * Copyright (C) 2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 * Correctness tests for the QPL_FLAG_LONG_MATCH_HINT compression flag. The hint
 * is advisory: it may change the compressed size but must never break round-trip
 * correctness, and it must be accepted (silently ignored) on paths that do not
 * support it (software and accelerator generations below 3.0).
 */

#include <cstdint>
#include <vector>

#include "qpl/qpl.h"

#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"
#include "t_common.hpp"

// tool_common
#include "iaa_features_checks.hpp"
#include "util.hpp"

namespace qpl::test {

namespace {

// Compresses input, optionally with the long-match hint. Empty vector on failure.
std::vector<uint8_t> compress(qpl_path_t path, const std::vector<uint8_t>& input, bool hint) {
    uint32_t job_size = 0;
    if (qpl_get_job_size(path, &job_size) != QPL_STS_OK) { return {}; }

    std::vector<uint8_t> job_buffer(job_size);
    auto*                job = reinterpret_cast<qpl_job*>(job_buffer.data());
    if (qpl_init_job(path, job) != QPL_STS_OK) { return {}; }

    std::vector<uint8_t> output(qpl_get_safe_deflate_compression_buffer_size(static_cast<uint32_t>(input.size())) +
                                64U);

    job->op            = qpl_op_compress;
    job->next_in_ptr   = const_cast<uint8_t*>(input.data());
    job->available_in  = static_cast<uint32_t>(input.size());
    job->next_out_ptr  = output.data();
    job->available_out = static_cast<uint32_t>(output.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY | QPL_FLAG_DYNAMIC_HUFFMAN;
    if (hint) { job->flags |= QPL_FLAG_LONG_MATCH_HINT; }

    const qpl_status status = qpl_execute_job(job);
    const uint32_t   total  = job->total_out;
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

qpl_path_t current_path() {
    return util::TestEnvironment::GetInstance().GetExecutionPath();
}

} // namespace

// With the hint enabled, every dataset file must still compress and round-trip.
QPL_UNIT_API_ALGORITHMIC_TEST(compression_long_match_hint, round_trip) {
    const qpl_path_t path    = current_path();
    const auto&      dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();
    if (dataset.get_data().empty()) { GTEST_SKIP() << "Algorithmic dataset not available"; }

    for (const auto& [name, data] : dataset.get_data()) {
        if (data.empty()) { continue; }

        const std::vector<uint8_t> compressed = compress(path, data, true);
        ASSERT_FALSE(compressed.empty()) << "Compression with hint failed. file=" << name;

        const std::vector<uint8_t> restored = decompress(path, compressed, data.size());
        ASSERT_EQ(restored, data) << "Round-trip mismatch with hint. file=" << name;
    }
}

// On the software path the hint is unsupported and must be silently ignored:
// the output must be byte-identical to compression without the hint.
QPL_UNIT_API_ALGORITHMIC_TEST(compression_long_match_hint, ignored_on_software) {
    QPL_SKIP_TEST_FOR(qpl_path_hardware);
    QPL_SKIP_TEST_FOR(qpl_path_auto);

    const auto& dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();
    if (dataset.get_data().empty()) { GTEST_SKIP() << "Algorithmic dataset not available"; }

    for (const auto& [name, data] : dataset.get_data()) {
        if (data.empty()) { continue; }
        EXPECT_EQ(compress(qpl_path_software, data, true), compress(qpl_path_software, data, false))
                << "SW long-match hint must not change the output. file=" << name;
    }
}

// On accelerator generation 3.0 the hint must actually reach the hardware: for an input
// known to benefit from long matches, the hinted output must be strictly smaller. Skipped
// on hardware generations that do not support the hint.
QPL_UNIT_API_ALGORITHMIC_TEST(compression_long_match_hint, improves_ratio_on_gen3) {
    QPL_SKIP_TEST_FOR(qpl_path_software);
    QPL_SKIP_TEST_FOR(qpl_path_auto);
    if (!are_iaa_gen_3_min_capabilities_present()) {
        GTEST_SKIP() << "Long-match hint requires accelerator generation 3.0";
    }

    const auto& dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();
    if (dataset.get_data().find("file2") == dataset.get_data().end()) {
        GTEST_SKIP() << "Reference file 'file2' not available in dataset";
    }
    const std::vector<uint8_t>& data = dataset["file2"];

    const std::vector<uint8_t> without_hint = compress(qpl_path_hardware, data, false);
    const std::vector<uint8_t> with_hint    = compress(qpl_path_hardware, data, true);
    ASSERT_FALSE(without_hint.empty());
    ASSERT_FALSE(with_hint.empty());

    EXPECT_LT(with_hint.size(), without_hint.size())
            << "Long-match hint must improve the ratio of file2 on accelerator generation 3.0";
}

} // namespace qpl::test
