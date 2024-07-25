/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TEST_TOOLS_UTILS_COMMON_HUFFMAN_TABLE_UNIQUE_HPP
#define QPL_TEST_TOOLS_UTILS_COMMON_HUFFMAN_TABLE_UNIQUE_HPP

#include <memory>

#include "qpl/qpl.h"

namespace qpl::test {
static qpl_huffman_table_t deflate_huffman_table_maker(const qpl_huffman_table_type_e type, const qpl_path_t path,
                                                       const allocator_t allocator) {
    qpl_huffman_table_t table_ptr = nullptr;
    qpl_status          status    = qpl_deflate_huffman_table_create(type, path, allocator, &table_ptr);
    if (status != QPL_STS_OK) table_ptr = nullptr;
    return table_ptr;
}

static qpl_huffman_table_t huffman_only_huffman_table_maker(const qpl_huffman_table_type_e type, const qpl_path_t path,
                                                            const allocator_t allocator) {
    qpl_huffman_table_t table_ptr = nullptr;
    qpl_status          status    = qpl_huffman_only_table_create(type, path, allocator, &table_ptr);
    if (status != QPL_STS_OK) table_ptr = nullptr;
    return table_ptr;
}

static auto any_huffman_table_deleter = [](qpl_huffman_table_t t) { qpl_huffman_table_destroy(t); };

using unique_huffman_table = std::unique_ptr<qpl_huffman_table, decltype(any_huffman_table_deleter)>;

} // namespace qpl::test

#endif //QPL_TEST_TOOLS_UTILS_COMMON_HUFFMAN_TABLE_UNIQUE_HPP