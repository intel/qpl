/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 08/07/2020
 *
 * @defgroup SW_KERNELS_RLE_BURST_API Rle_burst API
 * @ingroup  SW_KERNELS_PRIVATE_API
 * @{
 * @brief Contains Intel® Query Processing Library (Intel® QPL) Core API for `RLE Burst` operation
 *
 * @details Core APIs implement the following functionalities:
 *      -   Rle_burst analytics operation out-of-place kernels for 8u, 16u and 32u input/output data.
 *
 */

#include "qplc_defines.h"

#ifndef QPLC_RLE_BURST_H__
#define QPLC_RLE_BURST_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef qplc_status_t (*qplc_rle_burst_t_ptr)(const uint8_t *src1_ptr,
                                              uint32_t flag_be,
                                              const uint8_t *src2_ptr,
                                              uint32_t src_length,
                                              uint8_t **pp_dst,
                                              uint8_t *dst_end_ptr,
                                              uint32_t *count_ptr,
                                              uint32_t *index_ptr);

/**
 * @name qplc_rle_burst_<input bit-width>
 *
 * @brief Rle_burst analytics operation out-of-place kernels for 8u, 16u and 32u input and output data
 *
 * @param[in]      src1_ptr    pointer to source vector #1 (counters)
 * @param[in]      flag_be     indicator if vector #1 contains data in BE format
 * @param[in]      src2_ptr    pointer to source vector #2 (data to unpack)
 * @param[in]      src_length  length of both source vectors in elements; if source #1 contains 32u data
 *                             - it should have (src_length + 1u) elements
 * @param[out]     pp_dst      double pointer to destination vector
 * @param[in]      dst_end_ptr   pointer to destination vector upper boundary
 * @param[in,out]  count_ptr     pointer to counter of incomplete RLE group
 * @param[in,out]  index_ptr   pointer to source index if incomplete RLE group counter is non-zero
 *
 * @note Rle_burst operation for 8u and 16u src #1 data puts src2_ptr[i] value to dst_ptr a number of times defined
 *       by src1_ptr[i] value/counter; for 32u src #1 data it puts src2_ptr[i] value to dst_ptr a number of times
 *       defined by (src1_ptr[i+1] - src1_ptr[i]) value/counter; if the current counter defined by src1_ptr is zero
 *       - the corresponding src2_ptr[i] value is skipped.
 *
 * @return @ref qplc_status_t status of operation: QPLC_STS_OK, QPLC_STS_DST_IS_SHORT_ERR or QPLC_STS_INVALID_RLE_COUNT.
 * @{
 */
OWN_QPLC_API(qplc_status_t, qplc_rle_burst_8u, (const uint8_t *src1_ptr,
        uint32_t flag_be,
        const uint8_t *src2_ptr,
        uint32_t src_length,
        uint8_t **pp_dst,
        uint8_t *dst_end_ptr,
        uint32_t *count_ptr,
        uint32_t *index_ptr))

OWN_QPLC_API(qplc_status_t, qplc_rle_burst_8u16u, (const uint8_t *src1_ptr,
        uint32_t flag_be,
        const uint8_t *src2_ptr,
        uint32_t src_length,
        uint8_t **pp_dst,
        uint8_t *dst_end_ptr,
        uint32_t *count_ptr,
        uint32_t *index_ptr))

OWN_QPLC_API(qplc_status_t, qplc_rle_burst_8u32u, (const uint8_t *src1_ptr,
        uint32_t flag_be,
        const uint8_t *src2_ptr,
        uint32_t src_length,
        uint8_t **pp_dst,
        uint8_t *dst_end_ptr,
        uint32_t *count_ptr,
        uint32_t *index_ptr))

OWN_QPLC_API(qplc_status_t, qplc_rle_burst_16u8u, (const uint8_t *src1_ptr,
        uint32_t flag_be,
        const uint8_t *src2_ptr,
        uint32_t src_length,
        uint8_t **pp_dst,
        uint8_t *dst_end_ptr,
        uint32_t *count_ptr,
        uint32_t *index_ptr))

OWN_QPLC_API(qplc_status_t, qplc_rle_burst_16u, (const uint8_t *src1_ptr,
        uint32_t flag_be,
        const uint8_t *src2_ptr,
        uint32_t src_length,
        uint8_t **pp_dst,
        uint8_t *dst_end_ptr,
        uint32_t *count_ptr,
        uint32_t *index_ptr))

OWN_QPLC_API(qplc_status_t, qplc_rle_burst_16u32u, (const uint8_t *src1_ptr,
        uint32_t flag_be,
        const uint8_t *src2_ptr,
        uint32_t src_length,
        uint8_t **pp_dst,
        uint8_t *dst_end_ptr,
        uint32_t *count_ptr,
        uint32_t *index_ptr))

OWN_QPLC_API(qplc_status_t, qplc_rle_burst_32u8u, (const uint8_t *src1_ptr,
        uint32_t flag_be,
        const uint8_t *src2_ptr,
        uint32_t src_length,
        uint8_t **pp_dst,
        uint8_t *dst_end_ptr,
        uint32_t *count_ptr,
        uint32_t *index_ptr))

OWN_QPLC_API(qplc_status_t, qplc_rle_burst_32u16u, (const uint8_t *src1_ptr,
        uint32_t flag_be,
        const uint8_t *src2_ptr,
        uint32_t src_length,
        uint8_t **pp_dst,
        uint8_t *dst_end_ptr,
        uint32_t *count_ptr,
        uint32_t *index_ptr))

OWN_QPLC_API(qplc_status_t, qplc_rle_burst_32u, (const uint8_t *src1_ptr,
        uint32_t flag_be,
        const uint8_t *src2_ptr,
        uint32_t src_length,
        uint8_t **pp_dst,
        uint8_t *dst_end_ptr,
        uint32_t *count_ptr,
        uint32_t *index_ptr))
/** @} */

#ifdef __cplusplus
}
#endif

#endif // QPLC_RLE_BURST_H__
/** @} */
