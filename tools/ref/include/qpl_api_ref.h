/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Reference library
 */

/**
 * @date 10/25/2018
 * @brief Include file for Intel(R) Query Processing Library (Intel(R) QPL) reference library
 *
 * @addtogroup REFERENCE_PUBLIC
 * @{
 *
 */

#ifndef QPL_API_REF_H__
#define QPL_API_REF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "qpl/c_api/job.h"
#include "qpl/c_api/status.h"

#include "own_ref_defs.h"

/**
 * @brief Calculates CRC32
 *
 * @b Purpose: Intel QPL other operations - CRC32.
 *
 * @param[in]  buf      Pointer to the input data vector
 * @param[in]  len      Length of the input vector
 * @param[in]  poly     CRC32 polynom which is used for CRC calculation
 * @param[in]  init_crc  CRC32 init value
 *
 * @todo fix description
 * @return CRC32 value
 *
 * @todo fix description
 */
extern uint32_t ref_crc32(const uint8_t* buf, uint32_t len, uint32_t poly, uint32_t init_crc);

/**
 * @brief Calculates XOR checksum
 *
 * @b Purpose: Intel QPL other operations - XOR checksum.
 *
 * @param[in]  buf       Pointer to the input data vector
 * @param[in]  len       Length of the input vector
 * @param[in]  init_xor  XOR checksum init value
 *
 * @todo fix description
 * @return XOR checksum value
 */
extern uint32_t ref_xor_checksum(const uint8_t* buf, uint32_t len, uint32_t init_xor);

/**
 * @brief ref_crc64 - CRC64 calculation
 *
 * @param[in,out] qpl_job_ptr  Pointer to the initialized @ref qpl_job structure
 *
 * @todo used fields: next_in_ptr, available_in, crc64, crc64_poly, flags
 *
 * @note Field flags - function looks for values QPL_FLAG_CRC64_BE, QPL_FLAG_CRC64_INV.
 * @note Result crc64 is available in crc64 field
 *
 * @return
 *    - @ref QPL_STS_OK
 *    - @ref QPL_STS_NULL_PTR_ERR if any of qpl_job_ptr|next_in_ptr|next_out_ptr pointers is NULL
 *    - @ref QPL_STS_CRC64_BAD_POLYNOM in case if crc64_poly is 0
 */
extern qpl_status ref_crc64(qpl_job* const qpl_job_ptr);

/**
 * @brief  Replicates each element in src2_ptr a number of times defined by src1_ptr element with the same index.
 *
 * @param[in,out] qpl_job_ptr  Pointer to the initialized @ref qpl_job structure
 *
 * @todo used fields: next_in_ptr, available_in, next_src2_ptr, available_src2, next_out_ptr, available_out,
 *               num_input_elements, src1_bit_width, src2_bit_width, out_bit_width;
 *
 * @note  The next_in_ptr is an array of counters, which specify how many times element from next_src2_ptr should be
 *        repeated in output array.
 *        The bit width (src1_bit_width field) of counters' elements can be 8, 16 or 32.
 *        The next_src2_ptr is a packed array of unsigned integers of any width (src2_bit_width field) from 1 to 32 bits.
 *
 * @note  If the src1_bit_width is 8 or 16, each counter specifies the number of times to replicate
 *        the corresponding element in src2. If the value is 0, the corre.spond.ing element is dropped.
 *        If the bit width of src1 is 32, each element of src1 specifies the cumulative number of elements in the
 *        output to that point.
 *
 * @note  Field num_input_elements specifies number of elements in counters array (src1).
 *        If the src1_bit_width is 8 or 16, number of elements in src2 == number of counters.
 *        If the src1_bit_width is 32, number of elements in src2 == numberOfCounters - 1, due to each element of src1
 *        in this case specifies the cumulative number of elements in the output to that point..
 *
 * @note  Bit width of elements in output buffer is determine by value of out_bit_width field and can be
 *        the same as for elements in src2 or can be expanded up to 8, 16 or 32 bits (NOTE: it can't be less
 *        than src2_bit_width).
 *
 * @return
 *    - @ref QPL_STS_OK
 *    - @ref QPL_STS_NULL_PTR_ERR        - if any of qpl_job_ptr | next_in_ptr | next_src2_ptr | next_out_ptr
 *                                         pointers is NULL
 *    - @ref QPL_STS_SIZE_ERR            - if any of available_in | available_src2 | available_out |
 *                                         num_input_elements is 0
 *    - @ref QPL_STS_BIT_WIDTH_ERR       - in case of src1_bit_width or src2_bit_width has incorrect value
 *    - @ref QPL_STS_SRC_IS_SHORT_ERR    - in case of num_input_elements exceeds number of available elements
 *                                         in next_in_ptr or next_src2_ptr arrays
 *    - @ref QPL_STS_DST_IS_SHORT_ERR    - in case of next_out_ptr buffer isn't enough to store all output elements
 *    - @ref QPL_STS_OUT_FORMAT_ERR      - in case of out_bit_width field has incorrect value or result outputBitWidth
 *                                         is less than src2_bit_width
 */
extern qpl_status ref_expand_rle(qpl_job* const qpl_job_ptr);

/**
 * @brief ref_compare_C_LT - Scans input vector for values that satisfy the condition
 *
 * @param[in,out] qpl_job_ptr  Pointer to the initialized @ref qpl_job structure
 *
 * @todo used fields: next_in_ptr, available_in, next_out_ptr, available_out, num_input_elements, src1_bit_width,
 *               param_low, param_high parser, out_bit_width, op;
 *
 *
 * @note ref_compare_C_LT - Scans input vector for values that are less than some const
 *       If qpl_job_ptr->next_in_ptr[i] is LT than qpl_job_ptr->param_low, than qpl_job_ptr->next_out_ptr[i] = 1,
 *       otherwise = 0. In case of output modification used, qpl_job_ptr->next_out_ptr[i] is an index
 *       "i" if LT condition is satisfied, or skipped otherwise.
 *
 * @note ref_compare_C_LE - Scans input vector for values that are less than or equal to some const
 *       If qpl_job_ptr->next_in_ptr[i] is LE than qpl_job_ptr->param_low, than qpl_job_ptr->next_out_ptr[i] = 1,
 *       otherwise = 0. In case of output modification used, qpl_job_ptr->next_out_ptr[i] is an index
 *       "i" if LE condition is satisfied, or skipped otherwise.
 *
 * @note ref_compare_C_GT - Scans input vector for values that are greater than some const
 *       If qpl_job_ptr->next_in_ptr[i] is GT than qpl_job_ptr->param_low, than qpl_job_ptr->next_out_ptr[i] = 1,
 *       otherwise = 0. In case of output modification used, qpl_job_ptr->next_out_ptr[i] is an index
 *       "i" if GT condition is satisfied, or skipped otherwise.
 *
 * @note ref_compare_C_GE - Scans input vector for values that are greater than or equal to some const
 *       If qpl_job_ptr->next_in_ptr[i] is GE than qpl_job_ptr->param_low, than qpl_job_ptr->next_out_ptr[i] = 1,
 *       otherwise = 0. In case of output modification used, qpl_job_ptr->next_out_ptr[i] is an index
 *       "i" if GE condition is satisfied, or skipped otherwise.
 *
 * @note ref_compare_C_EQ - Scans input vector for values that are equal to some const
 *       If qpl_job_ptr->next_in_ptr[i] is EQ to qpl_job_ptr->param_low, than qpl_job_ptr->next_out_ptr[i] = 1,
 *       otherwise = 0. In case of output modification used, qpl_job_ptr->next_out_ptr[i] is an index
 *       "i" if EQ condition is satisfied, or skipped otherwise.
 *
 * @note ref_compare_C_NE - Scans input vector for values that are not equal to some const
 *       If qpl_job_ptr->next_in_ptr[i] is NE to qpl_job_ptr->param_low, than qpl_job_ptr->next_out_ptr[i] = 1,
 *       otherwise = 0. In case of output modification used, qpl_job_ptr->next_out_ptr[i] is an index
 *       "i" if NE condition is satisfied, or skipped otherwise.
 *
 * @note ref_compare_range_C_EQ - Scans input vector for values that are in range between 2 const
 *       If qpl_job_ptr->next_in_ptr[i] is GE than qpl_job_ptr->param_low and LE than qpl_job_ptr->param_high,
 *       than qpl_job_ptr->next_out_ptr[i] = 1, otherwise = 0. In case of output modification used,
 *       qpl_job_ptr->next_out_ptr[i] is an index "i" if condition above is satisfied, or skipped otherwise.
 *
 * @note ref_compare_range_C_NE - Scans input vector for values that are beyond the range between 2 const
 *       If qpl_job_ptr->next_in_ptr[i] is LT than qpl_job_ptr->param_low or GT than qpl_job_ptr->param_high,
 *       than qpl_job_ptr->next_out_ptr[i] = 1, otherwise = 0. In case of output modification used,
 *       qpl_job_ptr->next_out_ptr[i] is an index "i" if condition above is satisfied, or skipped otherwise.
 *
 * @return
 *    - @ref QPL_STS_OK
 *    - @ref QPL_STS_NULL_PTR_ERR           - if any of qpl_job_ptr|next_in_ptr|next_out_ptr pointers is NULL
 *    - @ref QPL_STS_SIZE_ERR               - if any of available_in|available_out|num_input_elements is 0
 *    - @ref QPL_STS_BIT_WIDTH_ERR          - if src1_bit_width is 0 or greater than 32
 *    - @ref QPL_STS_SRC_IS_SHORT_ERR       - if num_input_elements has not been processed while available_in archived
 *    - @ref QPL_STS_DST_IS_SHORT_ERR       - if num_input_elements has not been processed while available_out archived
 *    - @ref QPL_STS_OUT_FORMAT_ERR         - in case of bad value in the out_bit_width field
 *    - @ref QPL_STS_PARSER_ERR             - in case of bad (non-supported) value in the parser field
 *    - @ref QPL_STS_OPERATION_ERR          - in case of bad (non-supported) value in the op field
 *    - @ref QPL_STS_OUTPUT_OVERFLOW_ERR    - if output format is qpl_ow_8/16/32 and current index exceeds max value
 *                                            for this data type
 */
extern qpl_status ref_compare(qpl_job* const qpl_job_ptr);

/**
 * @brief qpl_extract - Extracts a sub-vector from input vector starting from index
 *                      param_low and finishing at index param_high
 *
 * @param[in,out]  qpl_job_ptr  Pointer to the initialized @ref qpl_job structure
 *
 * @todo used fields: next_in_ptr, available_in, next_out_ptr, available_out, num_input_elements, src1_bit_width,
 *               param_low, param_high, parser, op;
 *
 * @remarks  This operation outputs those input elements whose indices (starting at 0) fall within the inclusive range
 *           defined by job fields param_low and param_high. So the bit width of the output is the same as the
 *           bit width of the input, and the number of output elements should be (param_high - param_low + 1).
 *
 * @todo fix description
 * @return
 *    - @ref QPL_STS_OK
 *    - @ref QPL_STS_NULL_PTR_ERR        - if any of qpl_job_ptr|next_in_ptr|next_out_ptr pointers is NULL
 *    - @ref QPL_STS_SIZE_ERR            - if any of available_in|available_out|num_input_elements is 0
 *    - @ref QPL_STS_BIT_WIDTH_ERR       - if src1_bit_width is 0 or greater than 32
 *    - @ref QPL_STS_SRC_IS_SHORT_ERR    - if num_input_elements has not been processed while available_in archived
 *    - @ref QPL_STS_DST_IS_SHORT_ERR    - if num_input_elements has not been processed while available_out archived
 *    - @ref QPL_STS_PARSER_ERR          - in case of bad (non-supported) value in the parser field
 *    - @ref QPL_STS_OPERATION_ERR       - in case of bad (non-supported) value in the op field
 */
qpl_status ref_extract(qpl_job* const qpl_job_ptr);

/**
 * @brief qpl_select - Extracts a sub-vector from input vector; this sub-vector contains
 *                     only those input values, for which src2-bit-vector provides non-zero mask (bit)
 *
 * @param[in,out]  qpl_job_ptr  Pointer to the initialized @ref qpl_job structure
 *
 * @todo used fields: next_in_ptr, available_in, next_out_ptr, available_out, num_input_elements, src1_bit_width,
 *                    next_src2_ptr, available_src2, src2_bit_width, parser, op, flags;
 *
 * @remarks  For this operation source-2 is a bit-vector which should have at least as many elements as source-1.
 *           Those source-1 items which correspond to 1-bits in source-2 will be output.
 *
 * @todo fix description
 * @return
 *    - @ref QPL_STS_OK
 *    - @ref QPL_STS_NULL_PTR_ERR        - if any of qpl_job_ptr | next_in_ptr | next_out_ptr | next_src2_ptr
 *                                         pointers is NULL
 *    - @ref QPL_STS_SIZE_ERR            - if any of available_in | available_src2 | available_out |
 *                                         num_input_elements is 0
 *    - @ref QPL_STS_BIT_WIDTH_ERR       - if src1_bit_width is 0 or greater than 32 or src2_bit_width differs from 1
 *    - @ref QPL_STS_SRC_IS_SHORT_ERR    - in case of num_input_elements has not been processed while available_in
 *                                         or available_src2 archived
 *    - @ref QPL_STS_DST_IS_SHORT_ERR    - if num_input_elements has not been processed while available_out archived
 *    - @ref QPL_STS_PARSER_ERR          - in case of bad (non-supported) value in the parser field
 *    - @ref QPL_STS_OPERATION_ERR       - in case of bad (non-supported) value in the op field
 */
qpl_status ref_select(qpl_job* const qpl_job_ptr);

/**
 * @brief qpl_expand - Bit-width of the output is the same as the bit width of src1_ptr,
 *                     but the number of output elements is equal to the number of input elements in src2_ptr.
 *                     So for this function, the job field "num_input_elements" actually contains the number
 *                     of elements in src2_ptr rather than in src1_ptr. Each 0-bit from src2_ptr writes a zero to the
 *                     output. Each 1-bit writes the next entry from src1_ptr.
 *
 * @param[in,out]  qpl_job_ptr  Pointer to the initialized @ref qpl_job structure
 *
 * @todo used fields: next_in_ptr, available_in, next_out_ptr, available_out, num_input_elements, src1_bit_width,
 *                    next_src2_ptr, available_src2, src2_bit_width, parser, op, flags;
 *
 * @remarks  For this operation source-2 is a bit-vector which defines the size of output.
 *
 * @todo fix description
 * @return
 *    - @ref QPL_STS_OK
 *    - @ref QPL_STS_NULL_PTR_ERR        - if any of qpl_job_ptr | next_in_ptr | next_out_ptr | next_src2_ptr
 *                                         pointers is NULL
 *    - @ref QPL_STS_SIZE_ERR            - if any of available_in | available_src2 | available_out |
 *                                         num_input_elements is 0
 *    - @ref QPL_STS_BIT_WIDTH_ERR       - if src1_bit_width is 0 or greater than 32 or src2_bit_width differs from 1
 *    - @ref QPL_STS_SRC_IS_SHORT_ERR    - in case of num_input_elements has not been processed while available_in
 *                                         or available_src2 archived
 *    - @ref QPL_STS_DST_IS_SHORT_ERR    - if num_input_elements has not been processed while available_out archived
 *    - @ref QPL_STS_PARSER_ERR          - in case of bad (non-supported) value in the parser field
 *    - @ref QPL_STS_OPERATION_ERR       - in case of bad (non-supported) value in the op field
 */
qpl_status ref_expand(qpl_job* const qpl_job_ptr);

#ifdef __cplusplus
}
#endif

#endif //QPL_API_REF_H__

/** @} */
