/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file
 * @brief implementation of deflate service functions
 */

/* ------ Includes ------ */

#include "own_defs.h"
#include "common/bit_reverse.hpp"
#include "util/memory.hpp"
#include "compression/huffman_table/canned_utils.hpp"
#include "qplc_compression.h"
#include "own_deflate_job.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OWN_HUFFMAN_CODE_BIT_LENGTH 15u /**< Number of bits used to store Huffman code */

typedef enum {
    little_endian,
    big_endian
} own_endianness;

struct own_huffman_code {
    uint16_t code;               /**< Huffman code */
    uint8_t  extra_bit_count;    /**< Number of extra bits */
    uint8_t  length;             /**< Huffman code length */
};


OWN_FUN(uint32_t, own_count_significant_bits, (uint32_t value));

/**
 * @brief Returns correct Huffman code and code length of the match according to given Huffman table
 *
 * @param[in]   huffman_table_ptr  pointer to isal_hufftables structure that contains code
 * @param[in]   match_length        length of the match
 * @param[out]  code_ptr           pointer to memory where to put Huffman code
 * @param[out]  code_length_ptr    pointer to memory where to put length of the Huffman code
 */
OWN_FUN(void, own_get_match_length_code, (const struct isal_hufftables *const huffman_table_ptr,
        const uint32_t match_length,
        uint64_t *const code_ptr,
        uint32_t *const code_length_ptr));

/**
 * @brief Returns correct Huffman code and code length of the literal according to given Huffman table
 *
 * @param[in]   huffman_table_ptr  pointer to isal_hufftables structure that contains code
 * @param[in]   literal            symbol for which we want to get Huffman code
 * @param[out]  code_ptr           pointer to memory where to put Huffman code
 * @param[out]  code_length_ptr    pointer to memory where to put length of the Huffman code
 */
OWN_FUN(void, own_get_literal_code, (const struct isal_hufftables *const huffman_table_ptr,
        const uint32_t literal,
        uint64_t *const code_ptr,
        uint32_t *const code_length_ptr));

/**
 * @brief Returns correct Huffman code and code length of the offset according to given Huffman table
 *
 * @param[in]   huffman_table_ptr  pointer to isal_hufftables structure that contains code
 * @param[in]   offset             offset for which we want to get Huffman code
 * @param[out]  code_ptr           pointer to memory where to put Huffman code
 * @param[out]  code_length_ptr    pointer to memory where to put length of the Huffman code
 */
OWN_FUN(void, own_get_offset_code, (const struct isal_hufftables *const huffman_table_ptr,
        uint32_t offset,
        uint64_t *const code_ptr,
        uint32_t *const code_length_ptr));

/* ------ Internal functions implementation ------ */

static inline void own_create_code_tables(uint16_t *const code_table_ptr,
                                          uint8_t *const code_length_table_ptr,
                                          const uint32_t length,
                                          const struct own_huffman_code *const huffman_table_ptr) {
    for (uint32_t i = 0; i < length; i++) {
        code_table_ptr[i]        = huffman_table_ptr[i].code;
        code_length_table_ptr[i] = huffman_table_ptr[i].length;
    }
}

static inline void own_create_packed_match_lengths_table(uint32_t *const packed_table_ptr,
                                                         const struct own_huffman_code *const huffman_table_ptr) {
    // Variables
    uint8_t count             = 0;
    uint16_t extra_bits_count = 0;
    uint16_t gain_extra_bits  = OWN_EXTRA_BITS_START_POSITION;

    // Main cycle
    for (uint32_t i = 257; i < OWN_LITERALS_MATCHES_TABLE_SIZE - 1; i++) {
        for (uint16_t extra_bits = 0; extra_bits < (1u << extra_bits_count); extra_bits++) {
            if (count > 254) {
                break;
            }

            packed_table_ptr[count++] = (extra_bits << (huffman_table_ptr[i].length + OWN_CODE_LENGTH_BIT_LENGTH)) |
                                        (huffman_table_ptr[i].code << OWN_CODE_LENGTH_BIT_LENGTH) |
                                        (huffman_table_ptr[i].length + extra_bits_count);
        }

        if (i == gain_extra_bits) {
            gain_extra_bits += OWN_LENGTH_EXTRA_BITS_INTERVAL;
            extra_bits_count += 1;
        }
    }

    packed_table_ptr[count] =
            (huffman_table_ptr[OWN_LITERALS_MATCHES_TABLE_SIZE - 1].code << OWN_CODE_LENGTH_BIT_LENGTH) |
            (huffman_table_ptr[OWN_LITERALS_MATCHES_TABLE_SIZE - 1].length);
}

static inline void own_create_packed_offset_table(uint32_t *const packed_table_ptr,
                                                  const uint32_t length,
                                                  const struct own_huffman_code *const huffman_table_ptr) {
    // Variables
    uint32_t count            = 0;
    uint16_t extra_bits_count = 0;
    uint16_t gain_extra_bits  = OWN_OFFSETS_BEGIN_VALUE;

    // Main cycle
    for (uint32_t i = 0; i < QPL_DEFAULT_OFFSETS_NUMBER; i++) {
        for (uint16_t extra_bits = 0; extra_bits < (1u << extra_bits_count); extra_bits++) {
            if (count >= length) {
                return;
            }

            packed_table_ptr[count++] = (extra_bits << (huffman_table_ptr[i].length + OWN_CODE_LENGTH_BIT_LENGTH)) |
                                        (huffman_table_ptr[i].code << OWN_CODE_LENGTH_BIT_LENGTH) |
                                        (huffman_table_ptr[i].length + extra_bits_count);
        }

        if (i == gain_extra_bits) {
            gain_extra_bits += OWN_OFFSETS_EXTRA_BITS_INTERVAL;
            extra_bits_count += 1;
        }
    }
}

static inline void own_compute_offset_code(const struct isal_hufftables *huffman_table_ptr,
                                           uint16_t offset,
                                           uint64_t *const code_ptr,
                                           uint32_t *const code_length_ptr) {
    // Variables
    uint32_t significant_bits;
    uint32_t number_of_extra_bits;
    uint32_t extra_bits;
    uint32_t symbol;
    uint32_t length;
    uint32_t code;

    offset -= 1u;
    significant_bits = own_count_significant_bits(offset);

    number_of_extra_bits = significant_bits - 2u;
    extra_bits           = offset & ((1u << number_of_extra_bits) - 1u);
    offset >>= number_of_extra_bits;
    symbol               = offset + 2 * number_of_extra_bits;

    // Extracting information from table
    code   = huffman_table_ptr->dcodes[symbol];
    length = huffman_table_ptr->dcodes_sizes[symbol];

    // Return of the calculated results
    *code_ptr        = code | (extra_bits << length);
    *code_length_ptr = length + number_of_extra_bits;
}

/* ------ Own functions implementation ------ */

uint32_t own_count_significant_bits(uint32_t value) {
    // Variables
    uint32_t significant_bits = 0;

    // Main cycle
    while (value > 0) {
        significant_bits++;
        value >>= 1u;
    }

    return significant_bits;
}

void own_qpl_huffman_table_to_isal(qpl_compression_huffman_table *const qpl_table_ptr,
                                   struct isal_hufftables *const isal_table_ptr,
                                   const own_endianness endian) {
    // Variables
    const uint32_t qpl_code_mask     = (1u << OWN_HUFFMAN_CODE_BIT_LENGTH) - 1u;
    // First 15 bits [14:0]
    const uint32_t qpl_length_mask   = OWN_QPL_HUFFMAN_CODE_LENGTH_MASK << OWN_HUFFMAN_CODE_BIT_LENGTH; // Bits [18:15]
    uint32_t       header_byte_size  = 0;
    uint32_t       header_extra_bits = 0;

    struct own_huffman_code literals_matches_table[OWN_LITERALS_MATCHES_TABLE_SIZE] = {{0u, 0u, 0u}};
    struct own_huffman_code offsets_huffman_table[OWN_OFFSETS_TABLE_SIZE]           = {{0u, 0u, 0u}};

    // Memory initialization
    qpl::ml::util::set_zeros((uint8_t *) isal_table_ptr, sizeof(struct isal_hufftables));

    // Copying literals and match lengths Huffman table
    for (uint32_t i = 0; i < OWN_LITERALS_MATCHES_TABLE_SIZE; i++) {
        const uint16_t code   = (uint16_t) (get_literals_lengths_table_ptr(qpl_table_ptr)[i] & qpl_code_mask);
        const uint8_t  length = (uint8_t) ((get_literals_lengths_table_ptr(qpl_table_ptr)[i] & qpl_length_mask) >> 15u);

        literals_matches_table[i].length = length;
        literals_matches_table[i].code   = endian == little_endian ? reverse_bits(code, length) : code;
    }

    // Copying offsets Huffman table
    for (uint32_t i = 0; i < OWN_OFFSETS_TABLE_SIZE; i++) {
        const uint16_t code   = (uint16_t) (get_offsets_table_ptr(qpl_table_ptr)[i] & qpl_code_mask);
        const uint8_t  length = (uint8_t) ((get_offsets_table_ptr(qpl_table_ptr)[i] & qpl_length_mask) >> 15u);

        offsets_huffman_table[i].length = length;
        offsets_huffman_table[i].code   = endian == little_endian ? reverse_bits(code, length) : code;
    }

    // Generating ISA-L tables
    own_create_code_tables(isal_table_ptr->dcodes,
                           isal_table_ptr->dcodes_sizes,
                           OWN_OFFSETS_TABLE_SIZE,
                           offsets_huffman_table);
    own_create_code_tables(isal_table_ptr->lit_table,
                           isal_table_ptr->lit_table_sizes,
                           QPL_DEFAULT_LITERALS_NUMBER,
                           literals_matches_table);

    own_create_packed_match_lengths_table(isal_table_ptr->len_table, literals_matches_table);
    own_create_packed_offset_table(isal_table_ptr->dist_table, OWN_OFFSET_TABLE_SIZE, offsets_huffman_table);

    // Setting header information
    header_extra_bits = get_deflate_header_bits_size(qpl_table_ptr) % 8;
    header_byte_size  = (get_deflate_header_bits_size(qpl_table_ptr) / 8) + (header_extra_bits == 0 ? 0 : 1);

    isal_table_ptr->deflate_hdr_count      = get_deflate_header_bits_size(qpl_table_ptr) / 8;
    isal_table_ptr->deflate_hdr_extra_bits = header_extra_bits;

    uint8_t *deflate_header_ptr = get_deflate_header_ptr(qpl_table_ptr);

    for (uint32_t i = 0; i < header_byte_size; i++) {
        isal_table_ptr->deflate_hdr[i] = deflate_header_ptr[i];
    }

    // Forcedly set final bit of header, ISA-L will reset it if current block not final
    isal_table_ptr->deflate_hdr[0] |= 1u;
}

void own_get_offset_code(const struct isal_hufftables *const huffman_table_ptr,
                         uint32_t offset,
                         uint64_t *const code_ptr,
                         uint32_t *const code_length_ptr) {
    if (offset < 1) {
        offset = 0;
    }

    if (offset <= IGZIP_DIST_TABLE_SIZE) {
        const uint64_t offset_info = huffman_table_ptr->dist_table[offset - 1];

        *code_ptr        = offset_info >> 5u;
        *code_length_ptr = offset_info & 0x1Fu;
    } else {
        own_compute_offset_code(huffman_table_ptr, offset, code_ptr, code_length_ptr);
    }
}

void own_get_match_length_code(const struct isal_hufftables *const huffman_table_ptr,
                               const uint32_t match_length,
                               uint64_t *const code_ptr,
                               uint32_t *const code_length_ptr) {
    const uint64_t match_length_info = huffman_table_ptr->len_table[match_length - 3u];

    *code_ptr        = match_length_info >> 5u;
    *code_length_ptr = match_length_info & 0x1Fu;
}

void own_get_literal_code(const struct isal_hufftables *const huffman_table_ptr,
                          const uint32_t literal,
                          uint64_t *const code_ptr,
                          uint32_t *const code_length_ptr) {
    *code_ptr        = huffman_table_ptr->lit_table[literal];
    *code_length_ptr = huffman_table_ptr->lit_table_sizes[literal];
}

#ifdef __cplusplus
}
#endif
