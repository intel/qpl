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

#include "qplc_compression_consts.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Flag to specify whether Mapping Table or Mapping CAM
 * is stored in Huffman Table of qplc_huffman_table_flat_format type.
*/
typedef enum {
    ht_with_mapping_table = 0U,
    ht_with_mapping_cam   = 1U,
} huffman_table_aecs_format;

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
    uint32_t literals_matches[QPLC_DEFLATE_LL_TABLE_SIZE]; /**< Huffman table for literals and match lengths */
    uint32_t offsets[QPLC_DEFLATE_D_TABLE_SIZE];           /**< Huffman table for offsets */
} qplc_huffman_table_default_format;

/**
 * @brief Structure that holds information to build Huffman table
 */
typedef struct {
    /**
     * Number of codes of given length, i.e.
     * (N-1)-th element of this array contains number of codes of the length N
     */
    uint16_t number_of_codes[QPLC_HUFFMAN_CODES_PROPERTIES_TABLE_SIZE];

    /**
     * First code of given length
     */
    uint16_t first_codes[QPLC_HUFFMAN_CODES_PROPERTIES_TABLE_SIZE];

    /**
     * Index of the first code of given length
     */
    uint16_t first_table_indexes[QPLC_HUFFMAN_CODES_PROPERTIES_TABLE_SIZE];

    union {
        /**
         * Symbol of code of given index
         * The codes are sorted by the rule specified in the RFC1951 3.2.2.
         */
        uint8_t index_to_char[QPLC_INDEX_TO_CHAR_TABLE_SIZE];

        /**
         * Mapping CAM (for AECS Format-2 compatibility).
         * For each entry the index is the input symbol,
         * the value is the pair of input code length and (input code - first code),
         * stored in first 4 bits and next 4 bits respectively.
         * CAM size is exactly 265, that is number of len codes without those requiring extra bits.
         */
        uint16_t lit_cam[QPLC_LIT_CAM_SIZE];
    };

    huffman_table_aecs_format format_stored;
} qplc_huffman_table_flat_format;

inline uint16_t qplc_huffman_table_get_ll_code(const qplc_huffman_table_default_format* table, uint32_t index) {
    const uint16_t code_value = (uint16_t)(table->literals_matches[index] & QPLC_HUFFMAN_CODE_MASK);

    return code_value;
}

inline void qplc_huffman_table_write_ll_code(qplc_huffman_table_default_format* table, uint32_t index, uint16_t code) {
    const uint32_t literal_length_value = ((uint32_t)code) & QPLC_HUFFMAN_CODE_MASK;
    table->literals_matches[index] |= literal_length_value;
}

inline void qplc_huffman_table_write_ll_code_length(qplc_huffman_table_default_format* table, uint32_t index,
                                                    uint8_t code_length) {
    const uint32_t code_length_value = ((uint32_t)code_length) & QPLC_HUFFMAN_CODE_LENGTH_MASK;
    table->literals_matches[index] |= (code_length_value << QPLC_HUFFMAN_CODE_LENGTH_OFFSET);
}

inline uint8_t qplc_huffman_table_get_ll_code_length(const qplc_huffman_table_default_format* table, uint32_t index) {
    const uint32_t ll_code     = table->literals_matches[index];
    const uint8_t  code_length = (uint8_t)((ll_code & QPLC_LENGTH_MASK) >> QPLC_HUFFMAN_CODE_LENGTH_OFFSET);

    return code_length;
}

inline uint16_t qplc_huffman_table_get_offset_code(const qplc_huffman_table_default_format* table, uint32_t index) {
    const uint16_t code_value = (uint16_t)(table->offsets[index] & QPLC_HUFFMAN_CODE_MASK);

    return code_value;
}

inline uint8_t qplc_huffman_table_get_offset_code_length(const qplc_huffman_table_default_format* table,
                                                         uint32_t                                 index) {
    const uint32_t offset_code = table->offsets[index];
    const uint8_t  code_length = (uint8_t)((offset_code & QPLC_LENGTH_MASK) >> QPLC_HUFFMAN_CODE_LENGTH_OFFSET);

    return code_length;
}

#ifdef __cplusplus
}
#endif

#endif //QPL_QPLC_HUFFMAN_TABLE_H_
