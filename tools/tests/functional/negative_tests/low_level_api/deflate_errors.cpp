/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "operation_test.hpp"
#include "compression_huffman_table.hpp"
#include "tn_common.hpp"
#include "random_generator.h"

namespace qpl::test
{
enum compression_mode {
    fixed_compression,
    static_compression,
    dynamic_compression,
    canned_compression
};

constexpr uint32_t source_size = 64 * 1024;
constexpr uint32_t destination_size = source_size + 4;

// Functions to perform compression
template<compression_mode mode>
qpl_status compress_create_indices(std::vector<uint8_t> &source,
                                   std::vector<uint8_t> &destination,
                                   qpl_job *job_ptr,
                                   qpl_huffman_table_t table_ptr,
                                   qpl_compression_levels level) { return QPL_STS_OK; }

template<>
qpl_status compress_create_indices<compression_mode::dynamic_compression>(std::vector<uint8_t> &source,
                                                                          std::vector<uint8_t> &destination,
                                                                          qpl_job *job_ptr,
                                                                          qpl_huffman_table_t table_ptr,
                                                                          qpl_compression_levels level) {
    std::vector<uint64_t> indices(100, 0);

    // Configure job
    job_ptr->op = qpl_op_compress;
    job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_DYNAMIC_HUFFMAN | QPL_FLAG_LAST;

    job_ptr->available_in = static_cast<uint32_t>(source.size());
    job_ptr->available_out = static_cast<uint32_t>(destination.size());

    job_ptr->next_in_ptr = source.data();
    job_ptr->next_out_ptr = destination.data();

    job_ptr->level = level;

    job_ptr->idx_array = indices.data();
    job_ptr->idx_max_size = static_cast<uint32_t>(indices.size());

    job_ptr->mini_block_size = qpl_mini_block_size::qpl_mblk_size_512;

    // Compress
    auto result = run_job_api(job_ptr);

    return result;
}

template<>
qpl_status compress_create_indices<compression_mode::static_compression>(std::vector<uint8_t> &source,
                                                                         std::vector<uint8_t> &destination,
                                                                         qpl_job *job_ptr,
                                                                         qpl_huffman_table_t table_ptr,
                                                                         qpl_compression_levels level) {
    std::vector<uint64_t> indices(100, 0);
    // Configure job
    job_ptr->op = qpl_op_compress;

    job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST;

    job_ptr->available_in = static_cast<uint32_t>(source.size());
    job_ptr->available_out = static_cast<uint32_t>(destination.size());

    job_ptr->next_in_ptr = source.data();
    job_ptr->next_out_ptr = destination.data();

    job_ptr->huffman_table = table_ptr;
    job_ptr->level = level;

    job_ptr->idx_array = indices.data();
    job_ptr->idx_max_size = static_cast<uint32_t>(indices.size());

    job_ptr->mini_block_size = qpl_mini_block_size::qpl_mblk_size_16k;

    // Compress
    auto result = run_job_api(job_ptr);

    return result;
}

template<>
qpl_status compress_create_indices<compression_mode::fixed_compression>(std::vector<uint8_t> &source,
                                                                        std::vector<uint8_t> &destination,
                                                                        qpl_job *job_ptr,
                                                                        qpl_huffman_table_t table_ptr,
                                                                        qpl_compression_levels level) {
    std::vector<uint64_t> indices(100, 0);
    // Configure job
    job_ptr->op = qpl_op_compress;

    job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST;

    job_ptr->available_in = static_cast<uint32_t>(source.size());
    job_ptr->available_out = static_cast<uint32_t>(destination.size());

    job_ptr->next_in_ptr = source.data();
    job_ptr->next_out_ptr = destination.data();

    job_ptr->level = level;

    job_ptr->idx_array = indices.data();
    job_ptr->idx_max_size = static_cast<uint32_t>(indices.size());

    job_ptr->mini_block_size = qpl_mini_block_size::qpl_mblk_size_16k;

    // Compress
    auto result = run_job_api(job_ptr);
    return result;
}


QPL_LOW_LEVEL_API_NEGATIVE_TEST(deflate, dynamic_default_stored_block_overflow) {
    std::vector<uint8_t> source(source_size);
    std::generate(source.begin(), source.end(), std::rand);

    std::vector<uint8_t> destination(destination_size);

    auto execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();
    uint32_t job_size = 0;

    auto status = qpl_get_job_size(execution_path, &job_size);
    ASSERT_EQ(QPL_STS_OK, status) << "Failed to get job size\n";

    // Allocate buffers for decompression job
    auto job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto job_ptr = reinterpret_cast<qpl_job *>(job_buffer.get());

    // Initialize decompression job
    status = qpl_init_job(execution_path, job_ptr);
    ASSERT_EQ(QPL_STS_OK, status) << "Failed to initialize job\n";

    auto compression_status = compress_create_indices<compression_mode::dynamic_compression>(source, destination,
                                                                                             job_ptr, nullptr,
                                                                                             qpl_default_level);
    if (execution_path == qpl_path_software) {
        ASSERT_EQ(compression_status, QPL_STS_MORE_OUTPUT_NEEDED);
    } else {
        ASSERT_EQ(compression_status, QPL_STS_INDEX_GENERATION_ERR);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST(deflate, dynamic_high_stored_block_overflow) {
    std::vector<uint8_t> source(source_size);
    std::generate(source.begin(), source.end(), std::rand);

    std::vector<uint8_t> destination(destination_size);

    auto execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    if (execution_path == qpl_path_hardware) {
        GTEST_SKIP_("High compression is not supported on hw path");
    }
    uint32_t job_size = 0;

    auto status = qpl_get_job_size(execution_path, &job_size);
    ASSERT_EQ(QPL_STS_OK, status) << "Failed to get job size\n";

    // Allocate buffers for decompression job
    auto job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto job_ptr = reinterpret_cast<qpl_job *>(job_buffer.get());

    // Initialize decompression job
    status = qpl_init_job(execution_path, job_ptr);
    ASSERT_EQ(QPL_STS_OK, status) << "Failed to initialize job\n";

    auto compression_status = compress_create_indices<compression_mode::dynamic_compression>(source, destination,
                                                                                             job_ptr, nullptr,
                                                                                             qpl_default_level);
    if (execution_path == qpl_path_software) {
        ASSERT_EQ(compression_status, QPL_STS_MORE_OUTPUT_NEEDED);
    } else {
        ASSERT_EQ(compression_status, QPL_STS_INDEX_GENERATION_ERR);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST(deflate, static_default_stored_block_overflow) {
    std::vector<uint8_t> source(source_size);
    std::generate(source.begin(), source.end(), std::rand);

    std::vector<uint8_t> destination(destination_size);

    auto execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();
    uint32_t job_size = 0;

    auto status = qpl_get_job_size(execution_path, &job_size);
    ASSERT_EQ(QPL_STS_OK, status) << "Failed to get job size\n";

    // Allocate buffers for decompression job
    auto job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto job_ptr = reinterpret_cast<qpl_job *>(job_buffer.get());

    // Initialize decompression job
    status = qpl_init_job(execution_path, job_ptr);
    ASSERT_EQ(QPL_STS_OK, status) << "Failed to initialize job\n";

    qpl_huffman_table_t huffman_table_ptr;

    status = qpl_deflate_huffman_table_create(compression_table_type,
                                              execution_path,
                                              DEFAULT_ALLOCATOR_C,
                                              &huffman_table_ptr);
    ASSERT_EQ(status, QPL_STS_OK) << "Table creation failed";

    fill_compression_table(huffman_table_ptr);

    auto compression_status = compress_create_indices<compression_mode::static_compression>(source, destination,
                                                                                             job_ptr,
                                                                                             huffman_table_ptr,
                                                                                             qpl_default_level);
    if (execution_path == qpl_path_software) {
        ASSERT_EQ(compression_status, QPL_STS_MORE_OUTPUT_NEEDED);
    } else {
        ASSERT_EQ(compression_status, QPL_STS_INDEX_GENERATION_ERR);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST(deflate, static_high_stored_block_overflow) {
    std::vector<uint8_t> source(source_size);
    std::generate(source.begin(), source.end(), std::rand);

    std::vector<uint8_t> destination(destination_size);

    auto execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    if (execution_path == qpl_path_hardware) {
        GTEST_SKIP_("High compression is not supported on hw path");
    }
    uint32_t job_size = 0;

    auto status = qpl_get_job_size(execution_path, &job_size);
    ASSERT_EQ(QPL_STS_OK, status) << "Failed to get job size\n";

    // Allocate buffers for decompression job
    auto job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto job_ptr = reinterpret_cast<qpl_job *>(job_buffer.get());

    // Initialize decompression job
    status = qpl_init_job(execution_path, job_ptr);
    ASSERT_EQ(QPL_STS_OK, status) << "Failed to initialize job\n";

    qpl_huffman_table_t huffman_table_ptr;

    status = qpl_deflate_huffman_table_create(compression_table_type,
                                              execution_path,
                                              DEFAULT_ALLOCATOR_C,
                                              &huffman_table_ptr);
    ASSERT_EQ(status, QPL_STS_OK) << "Table creation failed";

    fill_compression_table(huffman_table_ptr);

    auto compression_status = compress_create_indices<compression_mode::static_compression>(source, destination,
                                                                                             job_ptr,
                                                                                             huffman_table_ptr,
                                                                                             qpl_high_level);
    if (execution_path == qpl_path_software) {
        ASSERT_EQ(compression_status, QPL_STS_MORE_OUTPUT_NEEDED);
    } else {
        ASSERT_EQ(compression_status, QPL_STS_INDEX_GENERATION_ERR);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST(deflate, fixed_default_stored_block_overflow) {
    std::vector<uint8_t> source(source_size);
    std::generate(source.begin(), source.end(), std::rand);

    std::vector<uint8_t> destination(destination_size);

    auto execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();
    uint32_t job_size = 0;

    auto status = qpl_get_job_size(execution_path, &job_size);
    ASSERT_EQ(QPL_STS_OK, status) << "Failed to get job size\n";

    // Allocate buffers for decompression job
    auto job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto job_ptr = reinterpret_cast<qpl_job *>(job_buffer.get());

    // Initialize decompression job
    status = qpl_init_job(execution_path, job_ptr);
    ASSERT_EQ(QPL_STS_OK, status) << "Failed to initialize job\n";

    auto compression_status = compress_create_indices<compression_mode::fixed_compression>(source, destination,
                                                                                           job_ptr,
                                                                                           nullptr,
                                                                                           qpl_default_level);
    if (execution_path == qpl_path_software) {
        ASSERT_EQ(compression_status, QPL_STS_MORE_OUTPUT_NEEDED);
    } else {
        ASSERT_EQ(compression_status, QPL_STS_INDEX_GENERATION_ERR);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST(deflate, fixed_high_stored_block_overflow) {
    std::vector<uint8_t> source(source_size);
    std::generate(source.begin(), source.end(), std::rand);

    std::vector<uint8_t> destination(destination_size);

    auto execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

    if (execution_path == qpl_path_hardware) {
        GTEST_SKIP_("High compression is not supported on hw path");
    }

    uint32_t job_size = 0;

    auto status = qpl_get_job_size(execution_path, &job_size);
    ASSERT_EQ(QPL_STS_OK, status) << "Failed to get job size\n";

    // Allocate buffers for decompression job
    auto job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto job_ptr = reinterpret_cast<qpl_job *>(job_buffer.get());

    // Initialize decompression job
    status = qpl_init_job(execution_path, job_ptr);
    ASSERT_EQ(QPL_STS_OK, status) << "Failed to initialize job\n";

    auto compression_status = compress_create_indices<compression_mode::fixed_compression>(source, destination,
                                                                                           job_ptr,
                                                                                           nullptr,
                                                                                           qpl_high_level);
    if (execution_path == qpl_path_software) {
        ASSERT_EQ(compression_status, QPL_STS_MORE_OUTPUT_NEEDED);
    } else {
        ASSERT_EQ(compression_status, QPL_STS_INDEX_GENERATION_ERR);
    }
}



}
