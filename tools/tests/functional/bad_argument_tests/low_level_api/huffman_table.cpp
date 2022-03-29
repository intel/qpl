/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tests
 */

#include <algorithm>

#include "../../../common/operation_test.hpp"
#include "tb_ll_common.hpp"

#include "qpl/c_api/huffman_table.h"

namespace qpl::test {

// ------ DEFLATE HUFFMAN TABLE ------ //

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, deflate_table_create) {
    qpl_huffman_table_t huffman_table{};

    auto status = qpl_deflate_huffman_table_create(static_cast<qpl_huffman_table_type_e>(0xFF),
                                                   GetExecutionPath(),
                                                   {malloc, free},
                                                   &huffman_table);

    EXPECT_EQ(QPL_STS_HUFFMAN_TABLE_TYPE_ERROR, status) << "incorrect table type used";

    status = qpl_deflate_huffman_table_create(combined_table_type,
                                              static_cast<qpl_path_t>(0xFF),
                                              {malloc, free},
                                              &huffman_table);

    EXPECT_EQ(QPL_STS_PATH_ERR, status) << "incorrect table path used";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, huffman_only_table_create) {
    qpl_huffman_table_t huffman_table{};

    auto status = qpl_huffman_only_table_create(static_cast<qpl_huffman_table_type_e>(0xFF),
                                                GetExecutionPath(),
                                                {malloc, free},
                                                &huffman_table);

    EXPECT_EQ(QPL_STS_HUFFMAN_TABLE_TYPE_ERROR, status) << "incorrect table type used";

    status = qpl_huffman_only_table_create(combined_table_type,
                                           static_cast<qpl_path_t>(0xFF),
                                           {malloc, free},
                                           &huffman_table);

    EXPECT_EQ(QPL_STS_PATH_ERR, status) << "incorrect table path used";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, destroy) {
    qpl_huffman_table_destroy(nullptr);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, deflate_table_init) {
    qpl_histogram histogram;
    std::fill(reinterpret_cast<uint8_t *>(&histogram), reinterpret_cast<uint8_t *>(&histogram) + sizeof(histogram), 0);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init(nullptr, &histogram));

    qpl_huffman_table_t table{};

    auto status = qpl_deflate_huffman_table_create(combined_table_type,
                                                   GetExecutionPath(),
                                                   DEFAULT_ALLOCATOR_C,
                                                   &table);
    ASSERT_FALSE(status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init(table, nullptr));

    histogram.reserved_distances[0] = 1u;
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, qpl_huffman_table_init(table, &histogram));
    histogram.reserved_distances[0] = 0u;

    histogram.reserved_distances[1] = 1u;
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, qpl_huffman_table_init(table, &histogram));
    histogram.reserved_distances[1] = 0u;

    histogram.reserved_literal_lengths[0] = 1u;
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, qpl_huffman_table_init(table, &histogram));
    histogram.reserved_literal_lengths[0] = 0u;

    histogram.reserved_literal_lengths[1] = 1u;
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, qpl_huffman_table_init(table, &histogram));
    histogram.reserved_literal_lengths[1] = 0u;

    qpl_huffman_table_destroy(table);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, huffman_only_table_init) {
    qpl_histogram histogram;
    std::fill(reinterpret_cast<uint8_t *>(&histogram), reinterpret_cast<uint8_t *>(&histogram) + sizeof(histogram), 0);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init(nullptr, &histogram));

    qpl_huffman_table_t table{};

    auto status = qpl_huffman_only_table_create(combined_table_type,
                                                GetExecutionPath(),
                                                DEFAULT_ALLOCATOR_C,
                                                &table);
    ASSERT_FALSE(status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init(table, nullptr));

    histogram.reserved_distances[0] = 1u;
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, qpl_huffman_table_init(table, &histogram));
    histogram.reserved_distances[0] = 0u;

    histogram.reserved_distances[1] = 1u;
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, qpl_huffman_table_init(table, &histogram));
    histogram.reserved_distances[1] = 0u;

    histogram.reserved_literal_lengths[0] = 1u;
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, qpl_huffman_table_init(table, &histogram));
    histogram.reserved_literal_lengths[0] = 0u;

    histogram.reserved_literal_lengths[1] = 1u;
    EXPECT_EQ(QPL_STS_INVALID_PARAM_ERR, qpl_huffman_table_init(table, &histogram));
    histogram.reserved_literal_lengths[1] = 0u;

    qpl_huffman_table_destroy(table);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, deflate_table_init_with_triplet) {
    constexpr auto      TRIPLET_COUNT = 256;
    qpl_huffman_triplet triplet[TRIPLET_COUNT];

    std::fill(reinterpret_cast<uint8_t *>(&triplet), reinterpret_cast<uint8_t *>(&triplet) + sizeof(triplet), 0);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_triplet(nullptr, triplet, TRIPLET_COUNT));

    qpl_huffman_table_t table{};

    auto status = qpl_deflate_huffman_table_create(combined_table_type,
                                                   GetExecutionPath(),
                                                   DEFAULT_ALLOCATOR_C,
                                                   &table);
    ASSERT_FALSE(status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_triplet(table, nullptr, TRIPLET_COUNT));

    EXPECT_EQ(QPL_STS_INVALID_HUFFMAN_TABLE_ERR, qpl_huffman_table_init_with_triplet(table, triplet, TRIPLET_COUNT));

    qpl_huffman_table_destroy(table);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, huffman_only_table_init_with_triplet) {
    constexpr auto      TRIPLET_COUNT = 256;
    qpl_huffman_triplet triplet[TRIPLET_COUNT];

    std::fill(reinterpret_cast<uint8_t *>(&triplet), reinterpret_cast<uint8_t *>(&triplet) + sizeof(triplet), 0);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_triplet(nullptr, triplet, TRIPLET_COUNT));

    qpl_huffman_table_t table{};

    auto status = qpl_huffman_only_table_create(combined_table_type,
                                                GetExecutionPath(),
                                                DEFAULT_ALLOCATOR_C,
                                                &table);
    ASSERT_FALSE(status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_triplet(table, nullptr, TRIPLET_COUNT));

    EXPECT_EQ(QPL_STS_SIZE_ERR, qpl_huffman_table_init_with_triplet(table, triplet, TRIPLET_COUNT - 1));

    qpl_huffman_table_destroy(table);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, deflate_table_init_with_other) {
    qpl_huffman_table_t table{};

    auto status = qpl_deflate_huffman_table_create(combined_table_type,
                                                   GetExecutionPath(),
                                                   DEFAULT_ALLOCATOR_C,
                                                   &table);
    ASSERT_FALSE(status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_other(table, nullptr));
    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_other(nullptr, table));

    qpl_huffman_table_t another_table{};

    status = qpl_huffman_only_table_create(combined_table_type,
                                           GetExecutionPath(),
                                           DEFAULT_ALLOCATOR_C,
                                           &another_table);
    ASSERT_FALSE(status);

    EXPECT_EQ(QPL_STS_INVALID_HUFFMAN_TABLE_ERR, qpl_huffman_table_init_with_other(table, another_table));

    qpl_huffman_table_destroy(table);
    qpl_huffman_table_destroy(another_table);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, huffman_only_table_init_with_other) {
    qpl_huffman_table_t table{};

    auto status = qpl_huffman_only_table_create(combined_table_type,
                                                GetExecutionPath(),
                                                DEFAULT_ALLOCATOR_C,
                                                &table);
    ASSERT_FALSE(status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_other(table, nullptr));
    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_init_with_other(nullptr, table));

    qpl_huffman_table_t another_table{};

    status = qpl_deflate_huffman_table_create(combined_table_type,
                                              GetExecutionPath(),
                                              DEFAULT_ALLOCATOR_C,
                                              &another_table);
    ASSERT_FALSE(status);

    EXPECT_EQ(QPL_STS_INVALID_HUFFMAN_TABLE_ERR, qpl_huffman_table_init_with_other(table, another_table));

    qpl_huffman_table_destroy(table);
    qpl_huffman_table_destroy(another_table);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, deflate_table_get_type) {
    qpl_huffman_table_type_e type{};
    qpl_huffman_table_t      table{};

    auto status = qpl_deflate_huffman_table_create(combined_table_type,
                                                   GetExecutionPath(),
                                                   DEFAULT_ALLOCATOR_C,
                                                   &table);
    ASSERT_FALSE(status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_get_type(nullptr, &type));

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_get_type(table, nullptr));
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(huffman_table, huffman_only_table_get_type) {
    qpl_huffman_table_type_e type{};
    qpl_huffman_table_t      table{};

    auto status = qpl_huffman_only_table_create(combined_table_type,
                                                GetExecutionPath(),
                                                DEFAULT_ALLOCATOR_C,
                                                &table);
    ASSERT_FALSE(status);

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_get_type(nullptr, &type));

    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_huffman_table_get_type(table, nullptr));
}
}
