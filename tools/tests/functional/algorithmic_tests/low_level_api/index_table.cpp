/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @brief Algorithmic tests for index_table functions
 *
 */

#include <queue>

#include "qpl/c_api/index_table.h"
#include "ta_ll_common.hpp"

namespace qpl::test {

size_t calculate_table_size(const uint32_t mini_block_count, const uint32_t mini_blocks_per_block) {
    const size_t index_size = sizeof(uint64_t);
    const uint32_t blocks_in_deflate_count =
        (mini_block_count + mini_blocks_per_block - 1U) / mini_blocks_per_block;
    const uint32_t number_of_index_entries = (blocks_in_deflate_count * 2U + mini_block_count + 1U);
    const size_t table_size = number_of_index_entries * index_size;
    return table_size;
}

uint32_t calculate_header_block_index(uint32_t mini_block_per_block, uint32_t mini_block_to_decompress_number){
    const uint32_t block_number = mini_block_to_decompress_number/mini_block_per_block;
    const uint32_t index_block_header_start = block_number * (mini_block_per_block + 2U);
    return index_block_header_start;
}

uint32_t calculate_mini_block_index(uint32_t mini_block_per_block, uint32_t mini_block_to_decompress_number){
    const uint32_t block_number = mini_block_to_decompress_number/mini_block_per_block;
    const uint32_t index_block_header_start = block_number * (mini_block_per_block + 2U);
    const uint32_t mini_block_number_in_block = mini_block_to_decompress_number - block_number * mini_block_per_block;
    const uint32_t mini_block_index = index_block_header_start + 1U + mini_block_number_in_block;
    return mini_block_index;
}


QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(index_table, get_index_table_size) {
    size_t ret_size = 0U;
    size_t *ret_size_ptr = &ret_size;

    auto status = qpl_get_index_table_size(0, 1, ret_size_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during getting index table size\n";
    EXPECT_EQ(*ret_size_ptr,calculate_table_size(0, 1));
    status = qpl_get_index_table_size(0, 2, ret_size_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during getting index table size\n";
    EXPECT_EQ(*ret_size_ptr, calculate_table_size(0, 2));
    EXPECT_EQ(QPL_STS_OK, status) << "Error during getting index table size\n";
    status = qpl_get_index_table_size(1, 1, ret_size_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during getting index table size\n";
    EXPECT_EQ(*ret_size_ptr, calculate_table_size(1, 1));
    status = qpl_get_index_table_size(1, 2, ret_size_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during getting index table size\n";
    EXPECT_EQ(*ret_size_ptr, calculate_table_size(1, 2));
    status = qpl_get_index_table_size(2, 1, ret_size_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during getting index table size\n";
    EXPECT_EQ(*ret_size_ptr, calculate_table_size(2, 1));
    status = qpl_get_index_table_size(2, 2, ret_size_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during getting index table size\n";
    EXPECT_EQ(*ret_size_ptr, calculate_table_size(2, 2));

}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(index_table, DISABLED_set_mini_block_location) {
    uint32_t start_bit = 0U;
    uint32_t last_bit = 1U;

    uint32_t first_bit_offset = 0U;
    uint32_t last_bit_offset = 0U;
    uint32_t compressed_size = 0U;

    uint8_t source = 0U;
    uint8_t *source_ptr = &source;
    uint8_t **source_pptr = &source_ptr;

    uint8_t next_in = 0U;
    uint8_t *next_in_ptr = &next_in;
    uint8_t **next_in_pptr = &next_in_ptr;

    uint32_t *first_bit_offset_ptr = &first_bit_offset;
    uint32_t *last_bit_offset_ptr = &last_bit_offset;
    uint32_t *compressed_size_ptr = &compressed_size;

    uint32_t ignore_start_bits = start_bit & 7;
    uint32_t ignore_end_bits = 7 & (0 - last_bit);
    uint32_t available_in = ((last_bit + 7) / 8) - (start_bit / 8);
    *next_in_pptr += start_bit / 8;

    auto status = qpl_set_mini_block_location(start_bit,
                                              last_bit,
                                              source_pptr,
                                              first_bit_offset_ptr,
                                              last_bit_offset_ptr,
                                              compressed_size_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during setting mini block location\n";
    EXPECT_EQ(**source_pptr, **next_in_pptr);
    EXPECT_EQ(*first_bit_offset_ptr, ignore_start_bits);
    EXPECT_EQ(*last_bit_offset_ptr, ignore_end_bits);
    EXPECT_EQ(*compressed_size_ptr, available_in);

    start_bit = 1U;
    last_bit = 2U;
    ignore_start_bits = start_bit & 7;
    ignore_end_bits = 7 & (0 - last_bit);
    available_in = ((last_bit + 7) / 8) - (start_bit / 8);
    *next_in_pptr += start_bit / 8;

    status = qpl_set_mini_block_location(start_bit,
                                              last_bit,
                                              source_pptr,
                                              first_bit_offset_ptr,
                                              last_bit_offset_ptr,
                                              compressed_size_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during setting mini block location\n";
    EXPECT_EQ(**source_pptr, **next_in_pptr);
    EXPECT_EQ(*first_bit_offset_ptr, ignore_start_bits);
    EXPECT_EQ(*last_bit_offset_ptr, ignore_end_bits);
    EXPECT_EQ(*compressed_size_ptr, available_in);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(index_table, find_header_block_index){
    qpl_index_table table;
    qpl_index_table *table_ptr = &table;
    uint32_t block_index = 0U;
    uint32_t *block_index_ptr = &block_index;
    table.block_count = 10;

    table.mini_block_count = 10;
    table.mini_blocks_per_block = 1;
    ASSERT_EQ(table.mini_block_count, table.block_count * table.mini_blocks_per_block) << "Incompatible mini_block_count";
    auto status = qpl_find_header_block_index(table_ptr, 0, block_index_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during search of header block index\n";
    EXPECT_EQ(*block_index_ptr, calculate_header_block_index(1, 0));

    status = qpl_find_header_block_index(table_ptr, 1, block_index_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during search of header block index\n";
    EXPECT_EQ(*block_index_ptr, calculate_header_block_index(1, 1));

    status = qpl_find_header_block_index(table_ptr, 2, block_index_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during search of header block index\n";
    EXPECT_EQ(*block_index_ptr, calculate_header_block_index(1, 2));

    table.mini_block_count = 20;
    table.mini_blocks_per_block = 2;
    ASSERT_EQ(table.mini_block_count, table.block_count * table.mini_blocks_per_block) << "Incompatible mini_block_count";
    status = qpl_find_header_block_index(table_ptr, 0, block_index_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during search of header block index\n";
    EXPECT_EQ(*block_index_ptr, calculate_header_block_index(2, 0));

    status = qpl_find_header_block_index(table_ptr, 1, block_index_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during search of header block index\n";
    EXPECT_EQ(*block_index_ptr, calculate_header_block_index(2, 1));

    status = qpl_find_header_block_index(table_ptr, 2, block_index_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during search of header block index\n";
    EXPECT_EQ(*block_index_ptr, calculate_header_block_index(2, 2));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(index_table, find_mini_block_index){
    // works on HW path, not on SW path
    qpl_index_table table;
    qpl_index_table *table_ptr = &table;
    uint32_t block_index = 0U;
    uint32_t *block_index_ptr = &block_index;
    table.block_count = 10;

    table.mini_block_count = 10;
    table.mini_blocks_per_block = 1;
    ASSERT_EQ(table.mini_block_count, table.block_count * table.mini_blocks_per_block) << "Incompatible mini_block_count";
    auto status = qpl_find_mini_block_index(table_ptr, 0, block_index_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during search mini block index\n";
    EXPECT_EQ(*block_index_ptr, calculate_mini_block_index(1, 0));

    status = qpl_find_mini_block_index(table_ptr, 1, block_index_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during search mini block index\n";
    EXPECT_EQ(*block_index_ptr, calculate_mini_block_index(1, 1));

    status = qpl_find_mini_block_index(table_ptr, 2, block_index_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during search mini block index\n";
    EXPECT_EQ(*block_index_ptr, calculate_mini_block_index(1, 2));

    table.mini_block_count = 20;
    table.mini_blocks_per_block = 2;
    ASSERT_EQ(table.mini_block_count, table.block_count * table.mini_blocks_per_block) << "Incompatible mini_block_count";
    status = qpl_find_mini_block_index(table_ptr, 0, block_index_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during search mini block index\n";
    EXPECT_EQ(*block_index_ptr, calculate_mini_block_index(2, 0));

    status = qpl_find_mini_block_index(table_ptr, 1, block_index_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during search mini block index\n";
    EXPECT_EQ(*block_index_ptr, calculate_mini_block_index(2, 1));

    status = qpl_find_mini_block_index(table_ptr, 2, block_index_ptr);
    EXPECT_EQ(QPL_STS_OK, status) << "Error during search mini block index\n";
    EXPECT_EQ(*block_index_ptr, calculate_mini_block_index(2, 2));
}

} // namespace qpl::test
