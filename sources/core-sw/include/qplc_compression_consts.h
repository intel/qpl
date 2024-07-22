/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Job API (public C API)
 */

#ifndef QPL_QPLC_COMPRESSION_CONSTS_H_
#define QPL_QPLC_COMPRESSION_CONSTS_H_

#ifdef __cplusplus
extern "C" {
#endif

// ------ Deflate table constants in accordance with rfc 1951 ------ //
#define QPLC_LITERALS_COUNT         256U /**< Literals count  */
#define QPLC_DEFLATE_LITERALS_COUNT 257U /**< Deflate literals codes count */
#define QPLC_DEFLATE_MATCHES_COUNT  29U  /**< Deflate matches codes count */
#define QPLC_DEFLATE_OFFSETS_COUNT  30U  /**< Deflate offset codes count */
#define QPLC_DEFLATE_LL_TABLE_SIZE \
    (QPLC_DEFLATE_LITERALS_COUNT + QPLC_DEFLATE_MATCHES_COUNT) /**< Deflate Literals Lengths count */
#define QPLC_DEFLATE_D_TABLE_SIZE QPLC_DEFLATE_OFFSETS_COUNT   /**< Deflate offset table size */

// ------ Deflate extra bits constants in accordance with rfc 1951 ------ //
#define QPLC_DEFLATE_EXTRA_BITS_START_POSITION   264U /**< Position in Huffman table where extra bits are used */
#define QPLC_DEFLATE_LENGTH_EXTRA_BITS_INTERVAL  4U /**< Step for adding extra bits length (matches codes extracting */
#define QPLC_DEFLATE_OFFSETS_EXTRA_BITS_INTERVAL 2U /**< Step for adding extra bits length (offsets codes extracting */
#define QPLC_DEFLATE_EXTRA_OFFSETS_BEGIN_VALUE   3U /**< Number of bits that are used for extra Huffman code */

// ------ Deflate algorithm constants in accordance with rfc 1951 ------ //
#define QPLC_DEFLATE_MINIMAL_MATCH_LENGTH       4U    /**< Minimal match length used during match search */
#define QPLC_DEFLATE_BYTES_FOR_HASH_CALCULATION 4U    /**< Number of bytes that is used for hash calculation */
#define QPLC_DEFLATE_MAXIMAL_OFFSET             4096U /**< Maximal offset for match */

// ------ Other constants ------ //
#define QPLC_CODE_LENGTH_BIT_LENGTH     5U   /**< Number of bits that are used to store code length in ISA-L */
#define QPLC_OFFSET_TABLE_SIZE          2U   /**< Length of packed offsets Huffman table */
#define QPLC_HUFFMAN_CODE_LENGTH_MASK   0xFU /**< 4 bits [3:0] (shift by Huffman code length before use */
#define QPLC_HUFFMAN_CODE_LENGTH_OFFSET 0xFU /**< Huffman code length offset */
#define QPLC_LENGTH_MASK                (QPLC_HUFFMAN_CODE_LENGTH_MASK << QPLC_HUFFMAN_CODE_LENGTH_OFFSET)
#define QPLC_HUFFMAN_CODE_MASK          ((1U << QPLC_HUFFMAN_CODE_LENGTH_OFFSET) - 1U) /**< Huffman code mask */
#define BFINAL_BIT                      1U /**< Bfinal bit value in deflate header */

// ------ Huffman Only table decompression related sizes ------ //
#define QPLC_HUFFMAN_CODE_BIT_LENGTH \
    15U /**< Number of bits used to store Huffman code (maximum supported by hardware path) */
#define QPLC_HUFFMAN_CODES_PROPERTIES_TABLE_SIZE (QPLC_HUFFMAN_CODE_BIT_LENGTH)
#define QPLC_INDEX_TO_CHAR_TABLE_SIZE            257U /**< Mapping table size (corresponds to AECS Format-1) */
#define QPLC_LIT_CAM_SIZE                        265U /**< Mapping CAM size (corresponds to AECS Format-2) */

#ifdef __cplusplus
}
#endif

#endif //QPL_QPLC_COMPRESSION_CONSTS_H_
