/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/13/2018
 * @brief Internal definitions for reference implementation of aggregates calculation
 *
 * @defgroup REFERENCE_AGGREGATES Aggregates
 * @ingroup REFERENCE_PRIVATE
 * @{
 */

#include "own_ref_defs.h"

#ifndef REF_MIN_MAX_SUM_H_
#define REF_MIN_MAX_SUM_H_

#define REF_BYTE_BIT_LEN     8                                     /**< @todo */
#define REF_BIT_BUF_LEN      (sizeof(uint64_t) * REF_BYTE_BIT_LEN) /**< @todo */
#define REF_BIT_BUF_LEN_HALF (REF_BIT_BUF_LEN >> 1)                /**< @todo */
#define REF_BUFL             REF_BIT_BUF_LEN_HALF                  /**< @todo */

/**
 * @todo
 */
#define REF_BITSIZEOF(a) (sizeof(a) * REF_BYTE_BIT_LEN)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculates min, max, sum aggregates
 *
 * @param[in]   src_ptr                  pointer to the input data byte buffer
 * @param[in]   len                      len of the buffer (in bytes)
 * @param[in]   last_bit_offset          offset of the last bit in the last byte of src (is used for total src
 *                                       bit len calculation)
 * @param[in]   elem_format              bit width format for vector element
 * @param[in]   value_bit_width          bit width of the real value in element of input buffer
 *                                       (vary from 1 till 32bits)
 * @param[in]   input_be                 1 means that input data is in BE(BigEndian) format, 0 - means LE(LittleEndian)
 * @param[in]   initial_output_index     initial output index, it is used only for case when output is a
 *                                       bit vector (elem_bit_width == 1)
 * @param[out]  first_idx_min_value_ptr  min value over array or index of first 1 value over bit vector input
 * @param[out]  last_idx_max_value_ptr   max value over array or index of last 1 value over bit vector input
 * @param[out]  sum_value_ptr            sum of all values or number of 1 values over bit vector input
 *
 * @note value_bit_width should be <= elem_bit_width (in other case function will work incorrectly)
 *       it is used to support cases when bitVector output was expanded to uint array
 *
 * @note value_bit_width == 1 means that input data is a bit vector (elem_bit_width can have values 1,8,16 or 32)
 *
 * @return @ref QPL_STS_OK in case of successful execution, or non-zero value, otherwise
 * @todo
 *
 */
qpl_status ref_min_max_sum(const uint8_t* src_ptr, uint32_t len, uint32_t last_bit_offset, qpl_out_format elem_format,
                           uint8_t value_bit_width, uint32_t input_be, uint32_t initial_output_index,
                           uint32_t* first_idx_min_value_ptr, uint32_t* last_idx_max_value_ptr,
                           uint32_t* sum_value_ptr);

#ifdef __cplusplus
}
#endif

#endif // REF_MIN_MAX_SUM_H_

/** @} */
