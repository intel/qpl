/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Own Includes
 */

/**
 * @brief Internal API for `Deflate`
 */

#ifndef OWN_DEFLATE_H_
#define OWN_DEFLATE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ------ Includes ------ */

#include "igzip_lib.h"
#include "deflate_defs.h"
#include "bit_writer.h"
#include "deflate_hash_table.h"
#include "deflate_histogram.h"

#include "own_defs.h"
#include "own_deflate_job.h"
#include "canned_utility.h"

#include <types.h>
#include <assert.h>
#include <string.h>

/* ------ Internal defines ------ */

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

/**
 * Simple stub callback for @link own_deflate_job @endlink
 */
#define OWN_STUB_CALLBACK own_deflate_job_callback_stub

/**
 * Simple stub callback for @link own_deflate_job @endlink
 */
#define OWN_STUB_PREDICATE own_deflate_job_predicate_stub

/**
 * Size of internal buffer for the isal level buffer
 */
#define ISAL_LEVEL_BUFFER_SIZE 348160u


/* ------ Internal tables ------ */

static const uint8_t own_match_lengths_extra_bits[29] = {
        0, 0, 0, 0,
        0, 0, 0, 0,
        1, 1, 1, 1,
        2, 2, 2, 2,
        3, 3, 3, 3,
        4, 4, 4, 4,
        5, 5, 5, 5,
        0
};

/* ------ Internal types ------ */

typedef enum {
    little_endian,
    big_endian
} own_endianness;

struct own_huffman_code {
    uint16_t code;               /**< Huffman code */
    uint8_t  extra_bit_count;    /**< Number of extra bits */
    uint8_t  length;             /**< Huffman code length */
};

/* ------ Own functions API ------ */

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

/**
 * @brief Performs conversion between Intel QPL Huffman table format and ISA-L one
 *
 * Intel QPL and ISA-L Huffman table formats are different.
 * Intel QPL table is present like: [18:15][14:0] corresponding to [code length][code]
 * In the same time ISA-L uses different arrays to store different information:
 *  It has own array with offsets codes called dcodes
 *  It has own array with offsets code lengths called dcodes_sizes
 *  It has own array with literals codes called lit_table
 *  It has own array with literals code lengths called lit_table_sizes
 *  It has own array that maps match length to code with appended extra bits called len_table (format [31:5][4:0])
 *  It has own array that maps offset to code with appended extra bits called dist_table (format [31:5][4:0])
 *
 * Also ISA-L may use reversed bit order for Huffman codes while Intel QPL Huffman code is stored directly
 *
 * @param[in]   qpl_table_ptr   pointer to Intel QPL table
 * @param[out]  isal_table_ptr  pointer to converted ISA-L format
 * @param[in]   endian          Huffman table endian mode
 */
OWN_FUN(void, own_qpl_huffman_table_to_isal, (qpl_compression_huffman_table *const qpl_table_ptr,
        struct isal_hufftables *const isal_table_ptr,
        const own_endianness endian));

/**
 * @brief Simple stub callback that does nothing
 *
 * @note implements @link own_deflate_job_callback @endlink
 */
OWN_FUN(void, own_deflate_job_callback_stub, (own_deflate_job *job_ptr));

/**
 * @brief Simple stub predicate that always returns `true`
 *
 * @note implements @link own_deflate_job_predicate @endlink
 */
OWN_FUN(uint8_t, own_deflate_job_predicate_stub, (own_deflate_job *const job_ptr));

/**
 * @brief Updates histogram for given @ref own_deflate_job
 */
OWN_FUN(void, own_update_deflate_histogram_high_level, (own_deflate_job *deflate_job_ptr));

#ifdef __cplusplus
}
#endif

#endif // OWN_DEFLATE_H_
