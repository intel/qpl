/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef HW_PATH_OWN_HW_DEFINITIONS_H_
#define HW_PATH_OWN_HW_DEFINITIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_BUF_SIZE (1024U * 1024 * 1024U) /**< @todo */

/* ------ Operations ------ */
/**
 * @name Operation code flags
 * @anchor HW_OPERATION_FLAGS
 * @todo Opcode values
 * @{
 */

// Read Src2 values
#define AD_RDSRC2_AECS     1U /**< @todo */
#define AD_RDSRC2_FF_INPUT 2U /**< @todo */

// Write Src2 values
#define AD_WRSRC2_NEVER  0U /**< @todo */
#define AD_WRSRC2_ALWAYS 1U /**< @todo */
#define AD_WRSRC2_MAYBE  2U /**< @todo */

// End Process values
#define AD_APPEND_EOB          1U /**< @todo */
#define AD_APPEND_EOB_FINAL_SB 3U /**< Append EOB and b_final Store Block */
/** @} */

/* ------ Setters ------ */
/**
 * @name Operations codes and operation flags setters
 * @anchor HW_OPERATIONS_AND_FLAGS_SETTERS
 * @todo
 * @{
 */
// opcode_opflags fields
#define ADOF_READ_SRC2(x)  (((x) & 3U) << 16U)    /**< @todo */
#define ADOF_WRITE_SRC2(x) (((x) & 3U) << 18U)    /**< @todo */
#define ADOF_CRC32C        (1U << 21U)            /**< @todo */
#define ADOF_AECS_SEL      (1U << 22U)            /**< @todo */
#define ADOF_OPCODE(x)     (((x) & 0xFFU) << 24U) /**< @todo */
#define ADOF_GET_OPCODE(x) (((x) >> 24U) & 0xFFU) /**< @todo */

// decompression flags
#define ADDF_ENABLE_DECOMP  (1U << 0U) /**< @todo */
#define ADDF_FLUSH_OUTPUT   (1U << 1U) /**< @todo */
#define ADDF_STOP_ON_EOB    (1U << 2U) /**< @todo */
#define ADDF_CHECK_FOR_EOB  (1U << 3U) /**< @todo */
#define ADDF_SEL_BFINAL_EOB (1U << 4U) /**< @todo */
#define ADDF_DECOMP_BE      (1U << 5U) /**< @todo */
#define ADDF_IGNORE_END_BITS(x) \
    (((x) & 7U) << 6U)                               /**< This 3-bit flag represents the number of bits
                                                              to ignore at the end of the compressed input stream.
                                                              Sometimes, a 4th bit may be supported (see below) */
#define ADDF_IGNORE_END_BITS_EXT (1U << 14U)         /**< The 4th high-order bit for Ignore End Bits */
#define ADDF_SUPPRESS_OUTPUT     (1U << 9U)          /**< @todo */
#define ADDF_ENABLE_IDXING(x)    (((x) & 7U) << 10U) /**< @todo */

// compression flags
#define ADCF_STATS_MODE        (1U << 0U)          /**< @todo */
#define ADCF_FLUSH_OUTPUT      (1U << 1U)          /**< @todo */
#define ADCF_END_PROC(x)       (((x) & 3U) << 2U)  /**< @todo */
#define ADCF_COMP_BE           (1U << 5U)          /**< @todo */
#define ADCF_ENABLE_HDR_GEN(x) (((x) & 7U) << 12U) /**< Enable Header Generation */
#define ADCF_WRITE_AECS_HT     (1U << 1U)          /**< Write AECS Huffman Tables */
#define ADCF_LOAD_DICT(x)      (((x) & 3U) << 10U) /**< Load dictionary */

// crc64 flags
#define ADC64F_INVCRC (1U << 14U) /**< @todo */
#define ADC64F_BE     (1U << 15U) /**< @todo */

/** @} */

/* ====== Macros ====== */

/**
 * @todo Random Access Body is FLAG_RND_ACCESS and not FLAG_FIRST
 */
#define IS_RND_ACCESS_BODY(flag) (((flag) & (QPL_FLAG_RND_ACCESS | QPL_FLAG_FIRST)) == QPL_FLAG_RND_ACCESS)

/**
 * Random Access Header is FLAG_RND_ACCESS and FLAG_FIRST
 */
#define IS_RND_ACCESS_HDR(flag) \
    (((flag) & (QPL_FLAG_RND_ACCESS | QPL_FLAG_FIRST)) == (QPL_FLAG_RND_ACCESS | QPL_FLAG_FIRST))

/**
 * @todo
 */
#define GET_DCFG(p) ((hw_iaa_aecs_analytic*)(((uint8_t*)((p)->dcfg)) + ((p)->aecs_hw_read_offset)))

#ifdef __cplusplus
}
#endif

#endif //HW_PATH_OWN_HW_DEFINITIONS_H_
