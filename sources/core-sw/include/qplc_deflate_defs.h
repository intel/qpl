/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Job API (public C API)
 */

#ifndef QPL_QPLC_DEFLATE_DEFS_H_
#define QPL_QPLC_DEFLATE_DEFS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define OWN_EXTRA_BITS_START_POSITION    264u  /**< Position in Huffman table where extra bits are used */
#define OWN_LENGTH_EXTRA_BITS_INTERVAL   4u    /**< Step for adding extra bits length (matches codes extracting */
#define OWN_OFFSETS_EXTRA_BITS_INTERVAL  2u    /**< Step for adding extra bits length (offsets codes extracting */
#define OWN_CODE_LENGTH_BIT_LENGTH       5u    /**< Number of bits that are used to store code length in ISA-L */
#define OWN_OFFSETS_BEGIN_VALUE          3u    /**< Number of bits that are used for extra Huffman code */
#define OWN_OFFSET_TABLE_SIZE            2u    /**< Length of packed offsets Huffman table */
#define OWN_QPL_HUFFMAN_CODE_LENGTH_MASK 15u   /**< 4 bits [3:0] (shift by Huffman code length before use */
#define OWN_BYTES_FOR_HASH_CALCULATION   4u    /**< Number of bytes that is used for hash calculation */
#define OWN_MINIMAL_MATCH_LENGTH         4u    /**< Minimal match length used during match search */
#define OWN_MAXIMAL_OFFSET               4096u /**< Maximal offset for match */

#define LITERALS_COUNT                   256u  /**< Literals count  */
#define LITERALS_TABLE_SIZE              257u  /**< Deflate literals codes count */
#define MATCHES_TABLE_SIZE               29u   /**< Deflate matches codes count */
#define LITERALS_MATCHES_TABLE_SIZE      (LITERALS_TABLE_SIZE + MATCHES_TABLE_SIZE) /**< Deflate Literals Lengths count */
#define OFFSETS_TABLE_SIZE               30u   /**< Deflate offset codes count */
#define HUFFMAN_CODE_LENGTH_MASK         0xFu  /**< Huffman code lengths task */
#define HUFFMAN_CODE_LENGTH_OFFSET       15u   /**< Huffman code length offset */
#define LENGTH_MASK                      (HUFFMAN_CODE_LENGTH_MASK << HUFFMAN_CODE_LENGTH_OFFSET)
#define HUFFMAN_CODE_MASK                ((1u << HUFFMAN_CODE_LENGTH_OFFSET) - 1u) /**< Huffman code mask */
/*
#define maximum_code_length               16u
#define codes_properties_table_size       (maximum_code_length - 1u)
#define index_to_char_table_size          257u
*/
#ifdef __cplusplus
}
#endif

#endif //QPL_QPLC_DEFLATE_DEFS_H_
