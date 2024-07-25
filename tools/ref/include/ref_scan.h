/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/07/2018
 * @brief Internal definitions for reference implementation of Compare/Scan functionality
 *
 * @addtogroup REFERENCE_SCAN
 * @{
 */

#include "own_ref_defs.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief ref_store_result stores result of Compare (Scan) function to destination buffer
 *
 * @param[in]      dst_buf      bit buffer with Compare/Scan result
 * @param[in]      bit_idx      bit index (index of bit to store)
 * @param[in,out]  pp_dst       pointer to current destination byte pointer
 * @param[in]      dst_end_ptr  @todo
 * @param[in,out]  idx_ptr      pointer to current destination index value in case of output modification
 * @param[in]      o_format     output format specifier
 *
 * @return
 *      - @ref QPL_STS_OK
 *      - @ref QPL_STS_OUTPUT_OVERFLOW_ERR - if current index is greater than max value for current data type
 */
qpl_status ref_store_result(uint32_t dst_buf, uint32_t bit_idx, uint8_t** pp_dst, uint8_t* dst_end_ptr,
                            uint32_t* idx_ptr, qpl_out_format o_format);

/**
 *  @brief ref_store_1_bit stores 1 result of Compare (Scan) function to destination buffer
 *
 *  @param[in]      destination_buffer   bit buffer with Compare/Scan result
 *  @param[in]      bit_index            index of bit to store
 *  @param[in,out]  pp_destination       pointer to current destination byte pointer
 *  @param[in]      destination_end_ptr  pointer to destination buffer end
 *  @param[in,out]  index_ptr            pointer to current destination index value in case of output modification
 *  @param[in]      output_format        output format specifier QPL_FLAG_OUT_BE
 *
 *  @return
 *      - @ref QPL_STS_OK
 *      - @ref QPL_STS_OUTPUT_OVERFLOW_ERR  - if current index is greater than max value for current data type
 *      - @ref QPL_STS_DST_IS_SHORT_ERR     - in case of attempt to store beyond destination buffer end
 *
 */
qpl_status ref_store_1_bit(uint32_t destination_buffer, uint32_t bit_index, uint8_t** const pp_destination,
                           const uint8_t* const destination_end_ptr, uint32_t* const index_ptr,
                           const qpl_out_format output_format);

#if defined(__cplusplus)
}
#endif

/** @} */
