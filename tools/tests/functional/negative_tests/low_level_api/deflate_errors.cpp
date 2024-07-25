/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "tn_common.hpp"

// tests_common
#include "operation_test.hpp"

// tool_common
#include "compression_huffman_table.hpp"
#include "huffman_table_unique.hpp"
#include "util.hpp"

// tool_generator
#include <algorithm>

#include "random_generator.h"

namespace qpl::test {
enum compression_mode { fixed_compression, static_compression, dynamic_compression, canned_compression };

constexpr uint32_t source_size      = 64 * 1024;
constexpr uint32_t destination_size = source_size + 4;

// Functions to perform compression
template <compression_mode mode>
qpl_status compress_create_indices(std::vector<uint8_t>& source, std::vector<uint8_t>& destination, qpl_job* job_ptr,
                                   qpl_huffman_table_t table_ptr, qpl_compression_levels level) {
    return QPL_STS_OK;
}

template <>
qpl_status compress_create_indices<compression_mode::dynamic_compression>(
        std::vector<uint8_t>& source, std::vector<uint8_t>& destination, qpl_job* job_ptr,
        qpl_huffman_table_t    table_ptr, //NOLINT(misc-unused-parameters)
        qpl_compression_levels level) {
    std::vector<uint64_t> indices(100, 0);

    // Configure job
    job_ptr->op    = qpl_op_compress;
    job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_DYNAMIC_HUFFMAN | QPL_FLAG_LAST;

    job_ptr->available_in  = static_cast<uint32_t>(source.size());
    job_ptr->available_out = static_cast<uint32_t>(destination.size());

    job_ptr->next_in_ptr  = source.data();
    job_ptr->next_out_ptr = destination.data();

    job_ptr->level = level;

    job_ptr->idx_array    = indices.data();
    job_ptr->idx_max_size = static_cast<uint32_t>(indices.size());

    job_ptr->mini_block_size = qpl_mini_block_size::qpl_mblk_size_512;

    // Compress
    auto result = run_job_api(job_ptr);

    return result;
}

template <>
qpl_status compress_create_indices<compression_mode::static_compression>(std::vector<uint8_t>&  source,
                                                                         std::vector<uint8_t>&  destination,
                                                                         qpl_job*               job_ptr,
                                                                         qpl_huffman_table_t    table_ptr,
                                                                         qpl_compression_levels level) {
    std::vector<uint64_t> indices(100, 0);
    // Configure job
    job_ptr->op = qpl_op_compress;

    job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST;

    job_ptr->available_in  = static_cast<uint32_t>(source.size());
    job_ptr->available_out = static_cast<uint32_t>(destination.size());

    job_ptr->next_in_ptr  = source.data();
    job_ptr->next_out_ptr = destination.data();

    job_ptr->huffman_table = table_ptr;
    job_ptr->level         = level;

    job_ptr->idx_array    = indices.data();
    job_ptr->idx_max_size = static_cast<uint32_t>(indices.size());

    job_ptr->mini_block_size = qpl_mini_block_size::qpl_mblk_size_16k;

    // Compress
    auto result = run_job_api(job_ptr);

    return result;
}

template <>
qpl_status compress_create_indices<compression_mode::fixed_compression>(
        std::vector<uint8_t>& source, std::vector<uint8_t>& destination, qpl_job* job_ptr,
        qpl_huffman_table_t    table_ptr, //NOLINT(misc-unused-parameters)
        qpl_compression_levels level) {
    std::vector<uint64_t> indices(100, 0);
    // Configure job
    job_ptr->op = qpl_op_compress;

    job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST;

    job_ptr->available_in  = static_cast<uint32_t>(source.size());
    job_ptr->available_out = static_cast<uint32_t>(destination.size());

    job_ptr->next_in_ptr  = source.data();
    job_ptr->next_out_ptr = destination.data();

    job_ptr->level = level;

    job_ptr->idx_array    = indices.data();
    job_ptr->idx_max_size = static_cast<uint32_t>(indices.size());

    job_ptr->mini_block_size = qpl_mini_block_size::qpl_mblk_size_16k;

    // Compress
    auto result = run_job_api(job_ptr);
    return result;
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(deflate, JobFixture, dynamic_default_stored_block_overflow) {
    qpl::test::random random_number(0U, UINT8_MAX, GetSeed());

    source.resize(source_size);
    destination.resize(destination_size);

    std::generate(source.begin(), source.end(), [&random_number]() { return static_cast<uint8_t>(random_number); });

    auto compression_status = compress_create_indices<compression_mode::dynamic_compression>(
            source, destination, job_ptr, nullptr, qpl_default_level);

    // For qpl_path_auto, the test will first try to run on accelerator, but since accelerator should return error,
    // the test will fall back to host. Therefore, auto path should get the same error as sw path
    if (GetExecutionPath() == qpl_path_software || GetExecutionPath() == qpl_path_auto) {
        ASSERT_EQ(compression_status, QPL_STS_MORE_OUTPUT_NEEDED);
    } else {
        ASSERT_EQ(compression_status, QPL_STS_INDEX_GENERATION_ERR);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(deflate, JobFixture, dynamic_high_stored_block_overflow) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_hardware, "High level compression is not supported on hardware path.");

    qpl::test::random random_number(0U, UINT8_MAX, GetSeed());

    source.resize(source_size);
    destination.resize(destination_size);

    std::generate(source.begin(), source.end(), [&random_number]() { return static_cast<uint8_t>(random_number); });

    auto compression_status = compress_create_indices<compression_mode::dynamic_compression>(
            source, destination, job_ptr, nullptr, qpl_high_level);

    ASSERT_EQ(compression_status, QPL_STS_MORE_OUTPUT_NEEDED);
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(deflate, JobFixture, static_default_stored_block_overflow) {
    qpl::test::random random_number(0U, UINT8_MAX, GetSeed());

    source.resize(source_size);
    destination.resize(destination_size);

    std::generate(source.begin(), source.end(), [&random_number]() { return static_cast<uint8_t>(random_number); });

    const unique_huffman_table table(
            deflate_huffman_table_maker(compression_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C),
            any_huffman_table_deleter);
    ASSERT_NE(table.get(), nullptr) << "Huffman Table creation failed\n";

    auto status = fill_compression_table(table.get());
    ASSERT_EQ(status, QPL_STS_OK) << "Compression table failed to be filled";

    status = compress_create_indices<compression_mode::static_compression>(source, destination, job_ptr, table.get(),
                                                                           qpl_default_level);

    // For qpl_path_auto, the test will first try to run on accelerator, but since accelerator should return error,
    // the test will fall back to host. Therefore, auto path should get the same error as sw path
    if (GetExecutionPath() == qpl_path_software || GetExecutionPath() == qpl_path_auto) {
        ASSERT_EQ(status, QPL_STS_MORE_OUTPUT_NEEDED);
    } else {
        ASSERT_EQ(status, QPL_STS_INDEX_GENERATION_ERR);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(deflate, JobFixture, static_high_stored_block_overflow) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_hardware, "High level compression is not supported on hardware path.");

    qpl::test::random random_number(0U, UINT8_MAX, GetSeed());

    source.resize(source_size);
    destination.resize(destination_size);

    std::generate(source.begin(), source.end(), [&random_number]() { return static_cast<uint8_t>(random_number); });

    const unique_huffman_table table(
            deflate_huffman_table_maker(compression_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C),
            any_huffman_table_deleter);
    ASSERT_NE(table.get(), nullptr) << "Huffman Table creation failed\n";

    auto status = fill_compression_table(table.get());
    ASSERT_EQ(status, QPL_STS_OK) << "Compression table failed to be filled";

    status = compress_create_indices<compression_mode::static_compression>(source, destination, job_ptr, table.get(),
                                                                           qpl_high_level);
    ASSERT_EQ(status, QPL_STS_MORE_OUTPUT_NEEDED);
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(deflate, JobFixture, fixed_default_stored_block_overflow) {
    qpl::test::random random_number(0U, UINT8_MAX, GetSeed());

    source.resize(source_size);
    destination.resize(destination_size);

    std::generate(source.begin(), source.end(), [&random_number]() { return static_cast<uint8_t>(random_number); });

    auto compression_status = compress_create_indices<compression_mode::fixed_compression>(source, destination, job_ptr,
                                                                                           nullptr, qpl_default_level);

    // For qpl_path_auto, the test will first try to run on accelerator, but since accelerator should return error,
    // the test will fall back to host. Therefore, auto path should get the same error as sw path
    if (GetExecutionPath() == qpl_path_software || GetExecutionPath() == qpl_path_auto) {
        ASSERT_EQ(compression_status, QPL_STS_MORE_OUTPUT_NEEDED);
    } else {
        ASSERT_EQ(compression_status, QPL_STS_INDEX_GENERATION_ERR);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(deflate, JobFixture, fixed_high_stored_block_overflow) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_hardware, "High level compression is not supported on hardware path.");

    qpl::test::random random_number(0U, UINT8_MAX, GetSeed());

    source.resize(source_size);
    destination.resize(destination_size);

    std::generate(source.begin(), source.end(), [&random_number]() { return static_cast<uint8_t>(random_number); });

    auto compression_status = compress_create_indices<compression_mode::fixed_compression>(source, destination, job_ptr,
                                                                                           nullptr, qpl_high_level);

    ASSERT_EQ(compression_status, QPL_STS_MORE_OUTPUT_NEEDED);
}

/**
 * @brief Test fixed mode high level compression on @ref qpl_path_software will return
 * proper error (QPL_STS_MORE_OUTPUT_NEEDED) when the destination buffer is not large enough
 * to accommodate the compressed stream and the slop.
 *
 * @note The source buffer size and data is hardcoded to simulate this edge case.
 */
QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(deflate, JobFixture, fixed_high_overflow_with_slop) {
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_hardware, "High level compression is not supported on hardware path.");

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
