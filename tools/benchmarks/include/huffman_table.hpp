/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <string>

#include "qpl/qpl.h"

// path_e
#include "types.hpp"

namespace bench {

/**
 * @brief Helper to convert integer level used throughout the benchmarks
 * to qpl_compression_levels.
*/
static inline qpl_compression_levels level_to_qpl(std::int32_t level) {
    switch (level) {
        case 1: return qpl_level_1;
        case 2: return qpl_level_2;
        case 3: return qpl_level_3;
        case 4: return qpl_level_4;
        case 5: return qpl_level_5;
        case 6: return qpl_level_6;
        case 7: return qpl_level_7;
        case 8: return qpl_level_8;
        case 9: return qpl_level_9;
        default: return qpl_default_level;
    }
}

/**
 * @brief Helper to convert integer path_e enum value used throughout the benchmarks
 * to qpl_path_t.
*/
static inline qpl_path_t path_to_qpl(path_e path) {
    switch (path) {
        case path_e::cpu: return qpl_path_software;
        case path_e::iaa: return qpl_path_hardware;
        case path_e::auto_: return qpl_path_auto;
        default: return qpl_path_auto;
    }
}

/**
 * @brief Helper wrapper to construct qpl_huffman_table_t based on input table type, data, level and path.
 * @return Returns pointer to Huffman table.
*/
static qpl_huffman_table_t deflate_huffman_table_maker(const qpl_huffman_table_type_e type, const data_t& data,
                                                       const qpl_compression_levels level, const qpl_path_t path,
                                                       const allocator_t allocator) {
    qpl_huffman_table_t table_ptr = nullptr;

    qpl_status status = qpl_deflate_huffman_table_create(type, path, allocator, &table_ptr);
    if (status != QPL_STS_OK) table_ptr = nullptr;

    qpl_histogram huffman_histogram {};

    status = qpl_gather_deflate_statistics((std::uint8_t*)data.buffer.data(), data.buffer.size(), &huffman_histogram,
                                           level, path);
    if (status != QPL_STS_OK) {
        qpl_huffman_table_destroy(table_ptr);
        table_ptr = nullptr;
    }

    status = qpl_huffman_table_init_with_histogram(table_ptr, &huffman_histogram);
    if (status != QPL_STS_OK) {
        qpl_huffman_table_destroy(table_ptr);
        table_ptr = nullptr;
    }

    return table_ptr;
}

/**
 * @brief Helper wrapper for qpl_huffman_table_destroy.
*/
static auto any_huffman_table_deleter = [](qpl_huffman_table_t t) { qpl_huffman_table_destroy(t); };
} // namespace bench
