/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Core API (private C++ API)
 */

#ifndef QPL_SOURCES_CORE_INCLUDE_QPLC_COMPRESSION_H_
#define QPL_SOURCES_CORE_INCLUDE_QPLC_COMPRESSION_H_

#include "../src/compression/include/deflate_hash_table.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Position in Huffman table where extra bits are used
 */
#define OWN_EXTRA_BITS_START_POSITION 264u

/**
 * Step for adding extra bits length (matches codes extracting)
 */
#define OWN_LENGTH_EXTRA_BITS_INTERVAL 4u

/**
 * Step for adding extra bits length (offsets codes extracting)
 */
#define OWN_OFFSETS_EXTRA_BITS_INTERVAL 2u

/**
 * Number of bits that are used to store code length in ISA-L
 */
#define OWN_CODE_LENGTH_BIT_LENGTH 5u

/**
 * Number of bits that are used for extra Huffman code
 */
#define OWN_OFFSETS_BEGIN_VALUE 3u

/**
 * Length of packed offsets Huffman table
 */
#define OWN_OFFSET_TABLE_SIZE 2u

/**
 * 4 bits [3:0] (shift by Huffman code length before use)
 */
#define OWN_QPL_HUFFMAN_CODE_LENGTH_MASK 15u

/**
 * Number of bytes that is used for hash calculation
 */
#define OWN_BYTES_FOR_HASH_CALCULATION 4u

/**
 * Minimal match length used during match search
 */
#define OWN_MINIMAL_MATCH_LENGTH 4u

/**
 * Maximal offset for match
 */
#define OWN_MAXIMAL_OFFSET 4096u

typedef struct isal_mod_hist      isal_mod_hist;
typedef struct deflate_icf_stream deflate_icf_stream;

typedef struct isal_hufftables isal_hufftables;
typedef struct BitBuf2         BitBuf2;

typedef uint32_t(*qplc_slow_deflate_body_t_ptr)(uint8_t *current_ptr,
                                                const uint8_t *const lower_bound_ptr,
                                                const uint8_t *const upper_bound_ptr,
                                                deflate_hash_table_t *hash_table_ptr,
                                                struct isal_hufftables *huffman_tables_ptr,
                                                struct BitBuf2 *bit_writer_ptr);

typedef uint32_t(*qplc_slow_deflate_icf_body_t_ptr)(uint8_t *current_ptr,
                                                    const uint8_t *const lower_bound_ptr,
                                                    const uint8_t *const upper_bound_ptr,
                                                    deflate_hash_table_t *hash_table_ptr,
                                                    isal_mod_hist *histogram_ptr,
                                                    deflate_icf_stream *icf_stream_ptr);

typedef void(*qplc_setup_dictionary_t_ptr)(uint8_t *dictionary_ptr,
                                           uint32_t dictionary_size,
                                           deflate_hash_table_t *hash_table_ptr);

#ifdef __cplusplus
}
#endif

#endif //QPL_SOURCES_CORE_INCLUDE_QPLC_COMPRESSION_H_
