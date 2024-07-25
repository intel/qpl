/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @brief Bad Args tests for index_table functions
 *
 */

#include "qpl/c_api/index_table.h"

#include "tb_ll_common.hpp"

// tests_common
#include "operation_test.hpp"

namespace qpl::test {

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(index_table, get_index_table_size) {
    const uint32_t mini_block_count      = 1U;
    const uint32_t mini_blocks_per_block = 1U;
    size_t         size                  = 0U;
    size_t*        size_ptr              = &size;

    auto status = qpl_get_index_table_size(mini_block_count, mini_blocks_per_block, nullptr);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, status);

    status = qpl_get_index_table_size(mini_block_count, 0, size_ptr);

    EXPECT_EQ(QPL_STS_SIZE_ERR, status);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(index_table, set_mini_block_location) {
    const uint32_t start_bit = 1U;
    const uint32_t last_bit  = 1U;

    uint32_t first_bit_offset = 1U;
    uint32_t last_bit_offset  = 1U;
    uint32_t compressed_size  = 1U;
    uint8_t  source           = 1U;

    uint8_t*  source_ptr           = &source;
    uint8_t** source_pptr          = &source_ptr;
    uint32_t* first_bit_offset_ptr = &first_bit_offset;
    uint32_t* last_bit_offset_ptr  = &last_bit_offset;
    uint32_t* compressed_size_ptr  = &compressed_size;

    auto status = qpl_set_mini_block_location(1, 0, source_pptr, first_bit_offset_ptr, last_bit_offset_ptr,
                                              compressed_size_ptr);

    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, status);

    status = qpl_set_mini_block_location(start_bit, last_bit, nullptr, first_bit_offset_ptr, last_bit_offset_ptr,
                                         compressed_size_ptr);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, status);

    status = qpl_set_mini_block_location(start_bit, last_bit, source_pptr, nullptr, last_bit_offset_ptr,
                                         compressed_size_ptr);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, status);

    status = qpl_set_mini_block_location(start_bit, last_bit, source_pptr, first_bit_offset_ptr, nullptr,
                                         compressed_size_ptr);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, status);

    status = qpl_set_mini_block_location(start_bit, last_bit, source_pptr, first_bit_offset_ptr, last_bit_offset_ptr,
                                         nullptr);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, status);

    uint8_t* source_null_ptr = nullptr;
    source_pptr              = &source_null_ptr;

    status = qpl_set_mini_block_location(start_bit, last_bit, source_pptr, first_bit_offset_ptr, last_bit_offset_ptr,
                                         compressed_size_ptr);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, status);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(index_table, find_header_block_index) {
    const uint32_t  mini_block_number = 1U;
    uint32_t        block_index       = 1U;
    uint32_t*       block_index_ptr   = &block_index;
    qpl_index_table table;
    table.mini_blocks_per_block = 0;
    table.mini_block_count      = 0;
    qpl_index_table* table_ptr  = &table;

    auto status = qpl_find_header_block_index(table_ptr, mini_block_number, block_index_ptr);

    EXPECT_EQ(QPL_STS_SIZE_ERR, status);
    table.mini_blocks_per_block = 1;

    status = qpl_find_header_block_index(nullptr, mini_block_number, block_index_ptr);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, status);

    status = qpl_find_header_block_index(table_ptr, mini_block_number, nullptr);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, status);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(index_table, find_mini_block_index) {
    const uint32_t   mini_block_number = 1U;
    uint32_t         block_index       = 1U;
    uint32_t*        block_index_ptr   = &block_index;
    qpl_index_table  table;
    qpl_index_table* table_ptr  = &table;
    table.mini_blocks_per_block = 0;
    table.mini_block_count      = 0;

    auto status = qpl_find_mini_block_index(table_ptr, mini_block_number, block_index_ptr);

    EXPECT_EQ(QPL_STS_SIZE_ERR, status);
    table.mini_blocks_per_block = 1;

    status = qpl_find_mini_block_index(nullptr, mini_block_number, block_index_ptr);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, status);

    status = qpl_find_mini_block_index(table_ptr, mini_block_number, nullptr);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, status);
}
} // namespace qpl::test
