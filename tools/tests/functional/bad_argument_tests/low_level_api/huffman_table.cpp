/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tests
 */

#include "qpl/c_api/huffman_table.h"

#include <algorithm>

#include "tb_ll_common.hpp"

// tests_common
#include "operation_test.hpp"

namespace qpl::test {

void* bad_malloc(size_t size) {
    return NULL;
}

// ------ DEFLATE HUFFMAN TABLE ------ //

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, deflate_table_create) {
    qpl_huffman_table_t huffman_table {};

    auto status = qpl_deflate_huffman_table_create(static_cast<qpl_huffman_table_type_e>(0xFF), GetExecutionPath(),
                                                   {malloc, free}, &huffman_table);

    EXPECT_EQ(QPL_STS_HUFFMAN_TABLE_TYPE_ERROR, status) << "incorrect table type used";

    status = qpl_deflate_huffman_table_create(combined_table_type, static_cast<qpl_path_t>(0xFF), {malloc, free},
                                              &huffman_table);

    EXPECT_EQ(QPL_STS_PATH_ERR, status) << "incorrect table path used";

    status = qpl_deflate_huffman_table_create(combined_table_type, GetExecutionPath(), {bad_malloc, free},
                                              &huffman_table);

    EXPECT_EQ(QPL_STS_OBJECT_ALLOCATION_ERR, status) << "bad allocator provided";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, huffman_only_table_create) {
    qpl_huffman_table_t huffman_table {};

    auto status = qpl_huffman_only_table_create(static_cast<qpl_huffman_table_type_e>(0xFF), GetExecutionPath(),
                                                {malloc, free}, &huffman_table);

    EXPECT_EQ(QPL_STS_HUFFMAN_TABLE_TYPE_ERROR, status) << "incorrect table type used";

    status = qpl_huffman_only_table_create(combined_table_type, static_cast<qpl_path_t>(0xFF), {malloc, free},
                                           &huffman_table);

    EXPECT_EQ(QPL_STS_PATH_ERR, status) << "incorrect table path used";

    status = qpl_huffman_only_table_create(combined_table_type, GetExecutionPath(), {bad_malloc, free}, &huffman_table);

    EXPECT_EQ(QPL_STS_OBJECT_ALLOCATION_ERR, status) << "bad allocator provided";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, destroy) {
    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_destroy(nullptr));
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, deflate_table_init) {
    qpl_histogram histogram;
    std::fill(reinterpret_cast<uint8_t*>(&histogram), reinterpret_cast<uint8_t*>(&histogram) + sizeof(histogram), 0);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_histogram(nullptr, &histogram));

    qpl_huffman_table_t table {};

    auto status =
            qpl_deflate_huffman_table_create(combined_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C, &table);
    ASSERT_EQ(QPL_STS_OK, status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_histogram(table, nullptr));

    histogram.reserved_distances[0] = 1U;
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, qpl_huffman_table_init_with_histogram(table, &histogram));
    histogram.reserved_distances[0] = 0U;

    histogram.reserved_distances[1] = 1U;
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, qpl_huffman_table_init_with_histogram(table, &histogram));
    histogram.reserved_distances[1] = 0U;

    histogram.reserved_literal_lengths[0] = 1U;
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, qpl_huffman_table_init_with_histogram(table, &histogram));
    histogram.reserved_literal_lengths[0] = 0U;

    histogram.reserved_literal_lengths[1] = 1U;
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, qpl_huffman_table_init_with_histogram(table, &histogram));
    histogram.reserved_literal_lengths[1] = 0U;

    ASSERT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(table));
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, huffman_only_table_init) {
    qpl_histogram histogram;
    std::fill(reinterpret_cast<uint8_t*>(&histogram), reinterpret_cast<uint8_t*>(&histogram) + sizeof(histogram), 0);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_histogram(nullptr, &histogram));

    qpl_huffman_table_t table {};

    auto status = qpl_huffman_only_table_create(combined_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C, &table);
    ASSERT_EQ(QPL_STS_OK, status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_histogram(table, nullptr));

    histogram.reserved_distances[0] = 1U;
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, qpl_huffman_table_init_with_histogram(table, &histogram));
    histogram.reserved_distances[0] = 0U;

    histogram.reserved_distances[1] = 1U;
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, qpl_huffman_table_init_with_histogram(table, &histogram));
    histogram.reserved_distances[1] = 0U;

    histogram.reserved_literal_lengths[0] = 1U;
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, qpl_huffman_table_init_with_histogram(table, &histogram));
    histogram.reserved_literal_lengths[0] = 0U;

    histogram.reserved_literal_lengths[1] = 1U;
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, qpl_huffman_table_init_with_histogram(table, &histogram));
    histogram.reserved_literal_lengths[1] = 0U;

    ASSERT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(table));
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, deflate_table_init_with_triplet) {
    constexpr auto      TRIPLET_COUNT = 256;
    qpl_huffman_triplet triplet[TRIPLET_COUNT];

    std::fill(reinterpret_cast<uint8_t*>(&triplet), reinterpret_cast<uint8_t*>(&triplet) + sizeof(triplet), 0);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_triplets(nullptr, triplet, TRIPLET_COUNT));

    qpl_huffman_table_t table {};

    auto status =
            qpl_deflate_huffman_table_create(combined_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C, &table);
    ASSERT_EQ(QPL_STS_OK, status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_triplets(table, nullptr, TRIPLET_COUNT));

    EXPECT_EQ(QPL_STS_INVALID_HUFFMAN_TABLE_ERR, qpl_huffman_table_init_with_triplets(table, triplet, TRIPLET_COUNT));

    ASSERT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(table));
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, huffman_only_table_init_with_triplet) {
    constexpr auto      TRIPLET_COUNT = 256;
    qpl_huffman_triplet triplet[TRIPLET_COUNT];

    std::fill(reinterpret_cast<uint8_t*>(&triplet), reinterpret_cast<uint8_t*>(&triplet) + sizeof(triplet), 0);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_triplets(nullptr, triplet, TRIPLET_COUNT));

    qpl_huffman_table_t table {};

    auto status = qpl_huffman_only_table_create(combined_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C, &table);
    ASSERT_EQ(QPL_STS_OK, status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_triplets(table, nullptr, TRIPLET_COUNT));

    EXPECT_EQ(QPL_STS_SIZE_ERR, qpl_huffman_table_init_with_triplets(table, triplet, TRIPLET_COUNT - 1));

    ASSERT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(table));
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, deflate_table_init_with_other) {
    qpl_huffman_table_t table {};

    auto status =
            qpl_deflate_huffman_table_create(combined_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C, &table);
    ASSERT_EQ(QPL_STS_OK, status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_other(table, nullptr));
    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_other(nullptr, table));

    qpl_huffman_table_t another_table {};

    status =
            qpl_huffman_only_table_create(combined_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C, &another_table);
    if (status != QPL_STS_OK) {
        qpl_huffman_table_destroy(table);
        FAIL() << "Huffman Table creation failed\n";
    }

    EXPECT_EQ(QPL_STS_INVALID_HUFFMAN_TABLE_ERR, qpl_huffman_table_init_with_other(table, another_table));

    EXPECT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(table));
    EXPECT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(another_table));
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, huffman_only_table_init_with_other) {
    qpl_huffman_table_t table {};

    auto status = qpl_huffman_only_table_create(combined_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C, &table);
    ASSERT_EQ(QPL_STS_OK, status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_other(table, nullptr));
    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_other(nullptr, table));

    qpl_huffman_table_t another_table {};

    status = qpl_deflate_huffman_table_create(combined_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C,
                                              &another_table);
    if (status != QPL_STS_OK) {
        qpl_huffman_table_destroy(table);
        FAIL() << "Huffman Table creation failed\n";
    }

    EXPECT_EQ(QPL_STS_INVALID_HUFFMAN_TABLE_ERR, qpl_huffman_table_init_with_other(table, another_table));

    EXPECT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(table));
    EXPECT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(another_table));
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, deflate_table_get_type) {
    qpl_huffman_table_type_e type {};
    qpl_huffman_table_t      table {};

    auto status =
            qpl_deflate_huffman_table_create(combined_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C, &table);
    ASSERT_EQ(QPL_STS_OK, status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_get_type(nullptr, &type));

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_get_type(table, nullptr));

    EXPECT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(table));
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, huffman_only_table_get_type) {
    qpl_huffman_table_type_e type {};
    qpl_huffman_table_t      table {};

    auto status = qpl_huffman_only_table_create(combined_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C, &table);
    ASSERT_EQ(QPL_STS_OK, status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_get_type(nullptr, &type));

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_get_type(table, nullptr));

    EXPECT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(table));
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, get_serialized_size) {
    size_t table_size = 0;

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR,
              qpl_huffman_table_get_serialized_size(nullptr, DEFAULT_SERIALIZATION_OPTIONS, &table_size));

    qpl_huffman_table_t table {};

    auto status = qpl_huffman_only_table_create(combined_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C, &table);
    ASSERT_EQ(QPL_STS_OK, status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR,
              qpl_huffman_table_get_serialized_size(table, DEFAULT_SERIALIZATION_OPTIONS, nullptr));

    const serialization_options_t bad_options = {static_cast<qpl_serialization_format_e>(0xFF), 0};

    EXPECT_EQ(QPL_STS_SERIALIZATION_FORMAT_ERROR,
              qpl_huffman_table_get_serialized_size(table, bad_options, &table_size));

    EXPECT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(table));
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, serialize) {
    uint8_t      buffer_ptr[1];
    const size_t buffer_size = 1;

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR,
              qpl_huffman_table_serialize(nullptr, buffer_ptr, buffer_size, DEFAULT_SERIALIZATION_OPTIONS));

    qpl_huffman_table_t table {};

    auto status = qpl_huffman_only_table_create(combined_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C, &table);
    ASSERT_EQ(QPL_STS_OK, status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR,
              qpl_huffman_table_serialize(table, nullptr, buffer_size, DEFAULT_SERIALIZATION_OPTIONS));

    EXPECT_EQ(QPL_STS_SIZE_ERR, qpl_huffman_table_serialize(table, buffer_ptr, 0, DEFAULT_SERIALIZATION_OPTIONS));

    const serialization_options_t bad_options = {static_cast<qpl_serialization_format_e>(0xFF), 0};

    EXPECT_EQ(QPL_STS_SERIALIZATION_FORMAT_ERROR,
              qpl_huffman_table_serialize(table, buffer_ptr, buffer_size, bad_options));

    EXPECT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(table));
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, deserialize) {
    uint8_t      buffer_ptr[1];
    const size_t buffer_size = 1;

    qpl_huffman_table_t table {};

    auto status = qpl_huffman_only_table_create(combined_table_type, GetExecutionPath(), DEFAULT_ALLOCATOR_C, &table);
    ASSERT_EQ(QPL_STS_OK, status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_deserialize(nullptr, buffer_size, DEFAULT_ALLOCATOR_C, &table));

    EXPECT_EQ(QPL_STS_SIZE_ERR, qpl_huffman_table_deserialize(buffer_ptr, 0, DEFAULT_ALLOCATOR_C, &table));

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR,
              qpl_huffman_table_deserialize(buffer_ptr, buffer_size, DEFAULT_ALLOCATOR_C, nullptr));

    EXPECT_EQ(QPL_STS_OK, qpl_huffman_table_destroy(table));
}

} // namespace qpl::test
