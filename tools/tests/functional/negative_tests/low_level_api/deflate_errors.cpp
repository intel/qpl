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


QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(deflate, JobFixture, dynamic_default_stored_block_overflow) {
    qpl::test::random random_number(0u, UINT8_MAX, GetSeed());

    source.resize(source_size);
    destination.resize(destination_size);

    std::generate(source.begin(), source.end(), [&random_number](){return static_cast<uint8_t>(random_number);});

    auto compression_status = compress_create_indices<compression_mode::dynamic_compression>(source, destination,
                                                                                             job_ptr, nullptr,
                                                                                             qpl_default_level);
    if (GetExecutionPath() == qpl_path_software) {
        ASSERT_EQ(compression_status, QPL_STS_MORE_OUTPUT_NEEDED);
    } else {
        ASSERT_EQ(compression_status, QPL_STS_INDEX_GENERATION_ERR);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(deflate, JobFixture, dynamic_high_stored_block_overflow) {
    if (GetExecutionPath() == qpl_path_hardware) {
        GTEST_SKIP() << "High level compression is not supported on hardware path.";
    }

    qpl::test::random random_number(0u, UINT8_MAX, GetSeed());

    source.resize(source_size);
    destination.resize(destination_size);

    std::generate(source.begin(), source.end(), [&random_number](){return static_cast<uint8_t>(random_number);});

    auto compression_status = compress_create_indices<compression_mode::dynamic_compression>(source, destination,
                                                                                             job_ptr, nullptr,
                                                                                             qpl_high_level);

    ASSERT_EQ(compression_status, QPL_STS_MORE_OUTPUT_NEEDED);
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(deflate, JobFixture, static_default_stored_block_overflow) {
    qpl::test::random random_number(0u, UINT8_MAX, GetSeed());

    source.resize(source_size);
    destination.resize(destination_size);

    std::generate(source.begin(), source.end(), [&random_number](){return static_cast<uint8_t>(random_number);});

    qpl_huffman_table_t huffman_table_ptr;
    auto status = qpl_deflate_huffman_table_create(compression_table_type,
                                                   GetExecutionPath(),
                                                   DEFAULT_ALLOCATOR_C,
                                                   &huffman_table_ptr);
    ASSERT_EQ(status, QPL_STS_OK) << "Huffman table creation failed";

    status = fill_compression_table(huffman_table_ptr);
    ASSERT_EQ(status, QPL_STS_OK) << "Compression table failed to be filled";

    auto compression_status = compress_create_indices<compression_mode::static_compression>(source, destination,
                                                                                             job_ptr,
                                                                                             huffman_table_ptr,
                                                                                             qpl_default_level);

    EXPECT_EQ(qpl_huffman_table_destroy(huffman_table_ptr), QPL_STS_OK);
    if (GetExecutionPath() == qpl_path_software) {
        ASSERT_EQ(compression_status, QPL_STS_MORE_OUTPUT_NEEDED);
    } else {
        ASSERT_EQ(compression_status, QPL_STS_INDEX_GENERATION_ERR);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(deflate, JobFixture, static_high_stored_block_overflow) {
    if (GetExecutionPath() == qpl_path_hardware) {
        GTEST_SKIP() << "High level compression is not supported on hardware path.";
    }

    qpl::test::random random_number(0u, UINT8_MAX, GetSeed());

    source.resize(source_size);
    destination.resize(destination_size);

    std::generate(source.begin(), source.end(), [&random_number](){return static_cast<uint8_t>(random_number);});

    qpl_huffman_table_t huffman_table_ptr;
    auto status = qpl_deflate_huffman_table_create(compression_table_type,
                                                   GetExecutionPath(),
                                                   DEFAULT_ALLOCATOR_C,
                                                   &huffman_table_ptr);
    ASSERT_EQ(status, QPL_STS_OK) << "Huffman table creation failed";

    status = fill_compression_table(huffman_table_ptr);
    if(QPL_STS_OK != status){
        EXPECT_EQ(qpl_huffman_table_destroy(huffman_table_ptr), QPL_STS_OK);
    }
    ASSERT_EQ(status, QPL_STS_OK) << "Compression table failed to be filled";

    auto compression_status = compress_create_indices<compression_mode::static_compression>(source, destination,
                                                                                             job_ptr,
                                                                                             huffman_table_ptr,
                                                                                             qpl_high_level);

    EXPECT_EQ(qpl_huffman_table_destroy(huffman_table_ptr), QPL_STS_OK);
    ASSERT_EQ(compression_status, QPL_STS_MORE_OUTPUT_NEEDED);
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(deflate, JobFixture, fixed_default_stored_block_overflow) {
    qpl::test::random random_number(0u, UINT8_MAX, GetSeed());

    source.resize(source_size);
    destination.resize(destination_size);

    std::generate(source.begin(), source.end(), [&random_number](){return static_cast<uint8_t>(random_number);});

    auto compression_status = compress_create_indices<compression_mode::fixed_compression>(source, destination,
                                                                                           job_ptr,
                                                                                           nullptr,
                                                                                           qpl_default_level);
    if (GetExecutionPath() == qpl_path_software) {
        ASSERT_EQ(compression_status, QPL_STS_MORE_OUTPUT_NEEDED);
    } else {
        ASSERT_EQ(compression_status, QPL_STS_INDEX_GENERATION_ERR);
    }
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(deflate, JobFixture, fixed_high_stored_block_overflow) {
    if (GetExecutionPath() == qpl_path_hardware) {
        GTEST_SKIP() << "High level compression is not supported on hardware path.";
    }

    qpl::test::random random_number(0u, UINT8_MAX, GetSeed());

    source.resize(source_size);
    destination.resize(destination_size);

    std::generate(source.begin(), source.end(), [&random_number](){return static_cast<uint8_t>(random_number);});

    auto compression_status = compress_create_indices<compression_mode::fixed_compression>(source, destination,
                                                                                           job_ptr,
                                                                                           nullptr,
                                                                                           qpl_high_level);

    ASSERT_EQ(compression_status, QPL_STS_MORE_OUTPUT_NEEDED);
}

}
