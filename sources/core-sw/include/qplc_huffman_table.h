/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Job API (public C API)
 */

#ifndef QPL_QPLC_HUFFMAN_TABLE_H_
#define QPL_QPLC_HUFFMAN_TABLE_H_

#include <stdint.h>
#include "qplc_deflate_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Structure that holds Huffman codes for compression
 *
 * There are two different Huffman tables:
 *  One for literals and match lengths
 *  One for offsets
 *
 * Both of them have the same format:
 *  Bits [14:0] - code itself
 *  Bits [18:15] - code length
 *
 * Code is not bit-reversed, stored in LE
 */
typedef struct {
    uint32_t literals_matches[LITERALS_MATCHES_TABLE_SIZE];  /**< Huffman table for literals and match lengths */
    uint32_t offsets[OFFSETS_TABLE_SIZE];                    /**< Huffman table for offsets */
} qplc_compression_huffman_table;

inline uint16_t qplc_huffman_table_get_ll_code(const qplc_compression_huffman_table *table, uint32_t index) {
    auto code_value = static_cast<uint16_t>(table->literals_matches[index] & HUFFMAN_CODE_MASK);
    return code_value;
}

inline void qplc_huffman_table_write_ll_code(qplc_compression_huffman_table *table, uint32_t index, uint16_t code) {
    uint32_t literal_length_value = static_cast<uint32_t>(code) & HUFFMAN_CODE_MASK;

    table->literals_matches[index] |= literal_length_value;
}

inline void qplc_huffman_table_write_ll_code_length(qplc_compression_huffman_table *table, uint32_t index, uint8_t code_length) {
    uint32_t code_length_value = static_cast<uint32_t>(code_length) & HUFFMAN_CODE_LENGTH_MASK;
    table->literals_matches[index] |= (code_length_value << HUFFMAN_CODE_LENGTH_OFFSET);
}

inline uint8_t qplc_huffman_table_get_ll_code_length(const qplc_compression_huffman_table *table, uint32_t index) {
    auto ll_code     = table->literals_matches[index];
    auto code_length = static_cast<uint8_t>((ll_code & LENGTH_MASK) >> HUFFMAN_CODE_LENGTH_OFFSET);
    return code_length;
}

inline uint16_t qplc_huffman_table_get_offset_code(const qplc_compression_huffman_table *table, uint32_t index) {
    auto code_value = static_cast<uint16_t>(table->offsets[index] & HUFFMAN_CODE_MASK);
    return code_value;
}

inline uint8_t qplc_huffman_table_get_offset_code_length(const qplc_compression_huffman_table *table, uint32_t index) {
    auto offset_code = table->offsets[index];
    auto code_length = static_cast<uint8_t>((offset_code & LENGTH_MASK) >> HUFFMAN_CODE_LENGTH_OFFSET);
    return code_length;
}

#ifdef __cplusplus
}
#endif

#endif //QPL_QPLC_HUFFMAN_TABLE_H_
