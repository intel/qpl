/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Test data generator
 */

#ifndef _GEN_DEFS_H_
#define _GEN_DEFS_H_

#if defined(_WIN32) || defined(_WIN64)
#define GEN_UINT64 unsigned __int64
#define GEN_INT64  __int64
#else
#define GEN_UINT64 unsigned long long
#define GEN_INT64  long long
#endif

/**
*@{
*@name basic types definitions
*@typedefs
*/
typedef unsigned char  Gen8u;  /**< unsigned 8bit  int */
typedef unsigned short Gen16u; /**< unsigned 16bit int */
typedef unsigned int   Gen32u; /**< unsigned 32bit int */
typedef int            Gen32s; /**< signed   32bit int */
typedef GEN_UINT64     Gen64u; /**< unsigned 64bit int */
typedef GEN_INT64      Gen64s; /**< signed   64bit int */
typedef int            GenStatus;
/** @} */

#define GEN_MAX_8U  (0xFFU)
#define GEN_MIN_8U  (0U)
#define GEN_MAX_16U (0xFFFFU)
#define GEN_MIN_16U (0U)
#define GEN_MAX_32U (0xFFFFFFFFU)
#define GEN_MIN_32U (0U)
#define GEN_MIN_32S (-2147483647 - 1)
#define GEN_MAX_32S (2147483647)

#if defined(_WIN32) || defined(_WIN64)
#define GEN_MAX_64U (0xFFFFFFFFFFFFFFFFL) // 18446744073709551615
#else
#define GEN_MAX_64U (0xFFFFFFFFFFFFFFFFLL) // 18446744073709551615
#endif

#define GEN_ONE_64U (1ULL)

#define GEN_MIN_64U (0U)

#define GEN_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define GEN_MIN(a, b) (((a) < (b)) ? (a) : (b))

#define LAST_FROM(count) ((count)-1)

/**
 * @brief list of definitions which was declared in accordance with DEFLATE STANDARD
 */

#define DEFAULT_LL_TABLE_LENGTH 286U /**> The real LL table size without reserved 287 and 288 LL codes*/
#define DEFAULT_D_TABLE_LENGTH  30U  /**> The real D table size without reserved 31 and 32 D codes*/
#define DEFAULT_CL_TABLE_LENGTH 19U  /**> The real CL table size*/

#define LITERALS_LOW_BORDER  0U   /**> The value of the first literal code in LL table*/
#define LITERALS_HIGH_BORDER 255U /**> The value of the last literal code in LL table*/
#define EOB_CODE             256U /**> The code of EOB literal */
#define MATCH_LOW_BORDER     257U /**> The value of the first match code in LL table*/
#define MATCH_HIGH_BORDER    285U /**> The value of the last match code in LL table*/

#define MAX_OFFSET             4096U /**> The maximum offset is limited by history window size (Intel QPL limit) */
#define DEFAULT_MAX_OFFSET     32768U /**> The maximum offset is limited by history window size (DEFLATE STANDARD limit) */
#define MIN_MATCH              3U     /**> The minimum match*/
#define MAX_MATCH              258U /**> The maximum match*/
#define MAX_CL_CODE_BIT_LENGTH 7U   /**> The maximum CL code bit length*/
#define MAX_LL_CODE_BIT_LENGTH 15U  /**> The maximum LL code bit length*/
#define MAX_D_CODE_BIT_LENGTH  15U  /**> The maximum D code bit length*/

/**
 * @brief list of generator errors
 */

#define GEN_OK                    0U /**> Generation of the test stream was completed successfully*/
#define GEN_ERR                   1U /**> Generation of the test stream was completed with unexpected error*/
#define GEN_UNSUPPORTED_GENERATOR 2U /**> Specified generator is not supported*/
#define GEN_TEST_FACTOR_NEEDED    3U /**> Specified generator is needed in special parameters*/
#define GEN_ALLOCATION_ERROR      4U /**> Specified generator can't allocate internal buffers*/

typedef enum {
    NO_ERR_DYNAMIC_BLOCK,
    NO_ERR_STORED_BLOCK,
    NO_ERR_FIXED_BLOCK,
    NO_ERR_HUFFMAN_ONLY,
    //Сanned tests
    CANNED_SMALL,
    CANNED_LARGE_LL,
    //Inflate functionality errors
    LARGE_HEADER,
    UNDEFINED_CL_CODE,
    FIRST_LL_CODE_16,
    FIRST_D_CODE_16,
    NO_LL_CODE,
    ALL_ZERO_LL_CODE,
    CL_CODES_SPAN_LL_WITH_D,
    TOO_MANY_D_CODES,
    TOO_MANY_D_CODES_V2,
    TOO_MANY_LL_CODES,
    OVERSUBSCRIBED_CL_TREE,
    OVERSUBSCRIBED_LL_TREE,
    OVERSUBSCRIBED_D_TREE,
    BLOCK_CONTAIN_UNALLOWABLE_D_CODE,
    BLOCK_CONTAIN_UNALLOWABLE_LL_CODE,
    INVALID_BLOCK_TYPE,
    BAD_STORED_LEN,
    BAD_DIST,
    DIST_BEFORE_START,
    BIG_REPEAT_COUNT_LL,
    BIG_REPEAT_COUNT_D,
    //Indexing functionality errors
    INDEX_DECOMPRESS_NO_ERR,
    INDEX_DECOMPRESS_DISTANCE_SPANS_MINIBLOCKS,
    INDEX_DECOMPRESS_MATCH_SPANS_MINIBLOCKS,
    INDEX_DECOMPRESS_INCORRECT_BLOCK_SIZE
} TestType;

typedef enum {
    ht_with_mapping_table = 0U,
    ht_with_mapping_cam   = 1U,
} GenHuffmanTableAecsFormat;

/*Qpl Huffman table*/
struct GenDecompressionHuffmanTable {
    Gen16u number_of_codes[15];
    Gen16u first_codes[15];
    Gen16u first_table_indexes[15];
    union {
        Gen8u  index_to_char[257];
        Gen16u lit_cam[265];
    };
    GenHuffmanTableAecsFormat format_stored;
};

struct SpecialTestOptions {
    bool                         bigEndian  = false;
    bool                         crc32c     = false;
    Gen32u                       blockCount = 1U;
    Gen32u                       mini_block_size;
    GenDecompressionHuffmanTable decompression_huffman_table {};
    bool                         is_aecs_format2_expected = false;
};

struct TestFactor {
    TestType           type = NO_ERR_DYNAMIC_BLOCK;
    Gen32u             seed = 0U;
    SpecialTestOptions specialTestOptions;
};

#endif //_GEN_DEFS_
