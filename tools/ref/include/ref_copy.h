/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/19/2018
 * @brief Contains internal definitions for reference implementation of Expand/Extract/Select/Copy functionality
 *
 * @defgroup REFERENCE_COPY Copy
 * @ingroup REFERENCE_COMMONS
 * @{
 *
 */

#ifndef _QPL_REF_COPY_H_
#define _QPL_REF_COPY_H_

#include "own_ref_defs.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief ref_own_copy_8u performs copy operation of byte data
 *
 * @param[in]      src_ptr  - source byte buffer
 * @param[in,out]  dst_ptr  - pointer to destination byte buffer
 * @param[in]      len      - number of bytes to copy
 *
 * @return always success - void - as all parameters must be checked at the caller level
 */
void ref_own_copy_8u(uint8_t* src_ptr, uint8_t* dst_ptr, uint32_t len);

/**
 * @todo ref_common
 * @param vector_ptr
 * @param vector_size
 */
void ref_fill_by_zero_8u(uint8_t* vector_ptr, uint32_t vector_size);

/**
 * @brief ref_store_val performs copy operation of one N-bit value in LE format to LE or BE format
 *
 * @param[in]      value          - source N-bit data in LE format (N can vary from 1 to 32)
 * @param[in,out]  dst_ptr        - pointer to destination buffer
 * @param[in,out]  idx_ptr        - index of N-bit data element in destination buffer
 * @param[in]      current_idx    - current index for non-zero values for nominal bit-vector
 * @param[in]      s_bit          - element width in bits
 * @param[in]      flag_be        - destination format - LE if 0, BE otherwise
 * @param[in]      o_fmt          - output format specification - qpl_ow_nom/8/16/32
 *
 * @return always success - void - as all parameters must be checked at the caller level
 */
qpl_status ref_store_val(uint32_t value, uint8_t* dst_ptr, uint32_t* idx_ptr, uint32_t current_idx, uint32_t s_bit,
                         uint32_t flag_be, qpl_out_format o_fmt);

/**
 * @brief ref_copy_le_le_nu performs copy operation of Nu (s_bit) data in LE format
 *
 * @param[in]      src_ptr         - source bit buffer of data in LE format
 * @param[in]      src_bit_offset  - offset of first bit in byte to start copy from.
 *                                 LE format - in Intel notation - offset from position 0
 * @param[in,out]  pp_dst          - pointer to pointer to current destination byte
 * @param[in]      i_offset        - offset of first index for nominal bit vector and qpl_ow_8|16|32
 * @param[in]      s_bit           - input element bit width
 * @param[in]      len             - number of s_bit elements to copy
 * @param[in]      o_fmt           - output format specification - qpl_ow_nom/8/16/32
 *
 * @return @ref QPL_STS_OK in case of successful execution, or non-zero value, otherwise
 * Return values:
 *   - QPL_STS_OK
 *   - QPL_STS_OUTPUT_OVERFLOW_ERR - in case qpl_ow_8|16|32 format and nominal bit-vector - if index exceeds
 *                                  max value for used data type
 */
qpl_status ref_copy_le_le_nu(uint8_t* src_ptr, uint32_t src_bit_offset, uint8_t** pp_dst, uint32_t i_offset,
                             uint32_t s_bit, uint32_t len, qpl_out_format o_fmt);

/**
 * @brief ref_copy_le_be_nu performs copy operation of Nu (s_bit) data from LE to BE format
 *
 * @param[in]      src_ptr         - source bit buffer of data in LE format
 * @param[in]      src_bit_offset  - offset of first bit in byte to start copy from.
 *                                   LE format - in Intel notation - offset from position 0
 * @param[in,out]  pp_dst          - pointer to pointer to current destination byte
 * @param[in]      i_offset        - offset of first index for nominal bit vector and qpl_ow_8|16|32
 * @param[in]      s_bit           - input element bit width
 * @param[in]      len             - number of s_bit elements to copy
 * @param[in]      o_fmt           - output format specification - qpl_ow_nom/8/16/32
 *
 * @return @ref QPL_STS_OK in case of successful execution, or non-zero value, otherwise
 * Return values:
 *   - QPL_STS_OK
 *   - QPL_STS_OUTPUT_OVERFLOW_ERR  - in case qpl_ow_8|16|32 format and nominal bit-vector - if index exceeds
 *                                    max value for used data type
 */
qpl_status ref_copy_le_be_nu(uint8_t* src_ptr, uint32_t src_bit_offset, uint8_t** pp_dst, uint32_t i_offset,
                             uint32_t s_bit, uint32_t len, qpl_out_format o_fmt);

/**
 * @brief ref_copy_be_le_nu performs copy operation of Nu (s_bit) data from BE to LE format
 *
 * @param[in]      src_ptr         - source bit buffer of data in LE format
 * @param[in]      src_bit_offset  - offset of first bit in byte to start copy from.
 *                                   LE format - in Intel notation - offset from position 0
 * @param[in,out]  pp_dst          - pointer to pointer to current destination byte
 * @param[in]      i_offset        - offset of first index for nominal bit vector and qpl_ow_8|16|32
 * @param[in]      s_bit           - input element bit width
 * @param[in]      len             - number of s_bit elements to copy
 * @param[in]      o_fmt           - output format specification - qpl_ow_nom/8/16/32
 *
 * @return @ref QPL_STS_OK in case of successful execution, or non-zero value, otherwise
 * Return values:
 *   - QPL_STS_OK
 *   - QPL_STS_OUTPUT_OVERFLOW_ERR  - in case qpl_ow_8|16|32 format and nominal bit-vector - if index exceeds
 *                                    max value for used data type
 */
qpl_status ref_copy_be_le_nu(uint8_t* src_ptr, uint32_t src_bit_offset, uint8_t** pp_dst, uint32_t i_offset,
                             uint32_t s_bit, uint32_t len, qpl_out_format o_fmt);

/**
 * @brief ref_copy_be_be_nu performs copy operation of Nu (s_bit) data from BE to BE format
 *
 * @param[in]      src_ptr         - source bit buffer of data in LE format
 * @param[in]      src_bit_offset  - offset of first bit in byte to start copy from.
 *                                   LE format - in Intel notation - offset from position 0
 * @param[in,out]  pp_dst          - pointer to pointer to current destination byte
 * @param[in]      i_offset        - offset of first index for nominal bit vector and qpl_ow_8|16|32
 * @param[in]      s_bit           - input element bit width
 * @param[in]      len             - number of s_bit elements to copy
 * @param[in]      o_fmt           - output format specification - qpl_ow_nom/8/16/32
 *
 * @return @ref QPL_STS_OK in case of successful execution, or non-zero value, otherwise
 * Return values:
 *   - QPL_STS_OK
 *   - QPL_STS_OUTPUT_OVERFLOW_ERR  - in case qpl_ow_8|16|32 format and nominal bit-vector - if index exceeds
 *                                    max value for used data type
 */
qpl_status ref_copy_be_be_nu(uint8_t* src_ptr, uint32_t src_bit_offset, uint8_t** pp_dst, uint32_t i_offset,
                             uint32_t s_bit, uint32_t len, qpl_out_format o_fmt);

/**
 * @brief ref_copy_mask_le_le_nu performs copy with mask operation from LE to LE format - if current mask_ptr bit
 *                               is 1 - than corresponding src_ptr n-bit value is copied to dst_ptr,
 *                               if 0 - than skipped
 *
 * @param[in]      src_ptr   - source N-bit data in LE format (N can vary from 1 to 32)
 * @param[in]      mask_ptr  - mask 1-bit data
 * @param[in]      mask_be   - >0 if mask_ptr points to BE data, 0 if LE
 * @param[in,out]  pp_dst    - pointer to destination buffer
 * @param[in]      i_offset  - initial index offset for nominal bit-vector with output modification qpl_ow_8|16|32
 * @param[in]      o_bits    - ptr to number of bits in dst buffer
 * @param[in]      s_bit     - element width in bits
 * @param[in]      len       - number of elements to process
 * @param[in]      o_fmt     - output format specification - qpl_ow_nom/8/16/32
 *
 * @return @ref QPL_STS_OK in case of successful execution, or non-zero value, otherwise
 * Return values:
 *   - QPL_STS_OK
 *   - QPL_STS_DST_IS_SHORT_ERR     - in case of not enough place in destination buffer
 *   - QPL_STS_OUTPUT_OVERFLOW_ERR  - in case qpl_ow_8|16|32 format and nominal bit-vector - if index exceeds
 *                                    max value for used data type
 */
qpl_status ref_copy_mask_le_le_nu(uint8_t* src_ptr, uint8_t* mask_ptr, uint32_t mask_be, uint8_t** pp_dst,
                                  uint32_t i_offset, uint64_t* o_bits, uint32_t s_bit, uint32_t len,
                                  qpl_out_format o_fmt);

/**
 * @brief ref_copy_mask_le_be_nu performs copy with mask operation from LE to BE format - if current mask_ptr bit
 *                               is 1 - than corresponding src_ptr n-bit value is copied to dst_ptr,
 *                               if 0 - than skipped
 *
 * @param[in]      src_ptr   - source N-bit data in LE format (N can vary from 1 to 32)
 * @param[in]      mask_ptr  - mask 1-bit data
 * @param[in]      mask_be   - >0 if mask_ptr points to BE data, 0 if LE
 * @param[in,out]  pp_dst    - pointer to destination buffer
 * @param[in]      i_offset  - initial index offset for nominal bit-vector and output modification qpl_ow_8|16|32
 * @param[in]      o_bits    - ptr to number of bits in dst buffer
 * @param[in]      s_bit     - element width in bits
 * @param[in]      len       - number of elements to process
 * @param[in]      o_fmt     - output format specification - qpl_ow_nom/8/16/32
 *
 * @return @ref QPL_STS_OK in case of successful execution, or non-zero value, otherwise
 * Return values:
 *   - QPL_STS_OK
 *   - QPL_STS_DST_IS_SHORT_ERR     - in case of not enough place in destination buffer
 *   - QPL_STS_OUTPUT_OVERFLOW_ERR  - in case qpl_ow_8|16|32 format and nominal bit-vector - if index exceeds
 *                                    max value for used data type
 */
qpl_status ref_copy_mask_le_be_nu(uint8_t* src_ptr, uint8_t* mask_ptr, uint32_t mask_be, uint8_t** pp_dst,
                                  uint32_t i_offset, uint64_t* o_bits, uint32_t s_bit, uint32_t len,
                                  qpl_out_format o_fmt);

/**
 * @brief ref_copy_mask_be_le__nu performs copy with mask operation from BE to LE format - if current mask_ptr
 *                                bit is 1 - than corresponding src_ptr n-bit value is copied to dst_ptr,
 *                                if 0 - than skipped
 *
 * @param[in]      src_ptr   - source N-bit data in BE format (N can vary from 1 to 32)
 * @param[in]      mask_ptr  - mask 1-bit data
 * @param[in]      mask_be   - >0 if mask_ptr points to BE data, 0 if LE
 * @param[in,out]  pp_dst    - pointer to destination buffer
 * @param[in]      i_offset  - initial index offset for nominal bit-vector with output modification qpl_ow_8|16|32
 * @param[in]      o_bits    - ptr to number of bits in dst buffer
 * @param[in]      s_bit     - element width in bits
 * @param[in]      len       - number of elements to process
 * @param[in]      o_fmt     - output format specification - qpl_ow_nom/8/16/32
 *
 * @return @ref QPL_STS_OK in case of successful execution, or non-zero value, otherwise
 * Return values:
 *   - QPL_STS_OK
 *   - QPL_STS_DST_IS_SHORT_ERR     - in case of not enough place in destination buffer
 *   - QPL_STS_OUTPUT_OVERFLOW_ERR  - in case qpl_ow_8|16|32 format and nominal bit-vector - if index exceeds
 *                                    max value for used data type
 */
qpl_status ref_copy_mask_be_le__nu(uint8_t* src_ptr, uint8_t* mask_ptr, uint32_t mask_be, uint8_t** pp_dst,
                                   uint32_t i_offset, uint64_t* o_bits, uint32_t s_bit, uint32_t len,
                                   qpl_out_format o_fmt);

/**
 * @brief ref_copy_mask_be_be_nu performs copy with mask operation from BE to BE format - if current mask_ptr bit
 *                               is 1 - than corresponding src_ptr n-bit value is copied to dst_ptr,
 *                               if 0 - than skipped
 *
 * @param[in]      src_ptr   - source N-bit data in BE format (N can vary from 1 to 32)
 * @param[in]      mask_ptr  - mask 1-bit data
 * @param[in]      mask_be   - >0 if mask_ptr points to BE data, 0 if LE
 * @param[in,out]  pp_dst    - pointer to destination buffer
 * @param[in]      i_offset  - initial index offset for nominal bit-vector and output modification qpl_ow_8|16|32
 * @param[in]      o_bits    - ptr to number of bits in dst buffer
 * @param[in]      s_bit     - element width in bits
 * @param[in]      len       - number of elements to process
 * @param[in]      o_fmt     - output format specification - qpl_ow_nom/8/16/32
 *
 * @return @ref QPL_STS_OK in case of successful execution, or non-zero value, otherwise
 * Return values:
 *   - QPL_STS_OK
 *   - QPL_STS_DST_IS_SHORT_ERR     - in case of not enough place in destination buffer
 *   - QPL_STS_OUTPUT_OVERFLOW_ERR  - in case qpl_ow_8|16|32 format and nominal bit-vector - if index exceeds
 *                                    max value for used data type
 */
qpl_status ref_copy_mask_be_be_nu(uint8_t* src_ptr, uint8_t* mask_ptr, uint32_t mask_be, uint8_t** pp_dst,
                                  uint32_t i_offset, uint64_t* o_bits, uint32_t s_bit, uint32_t len,
                                  qpl_out_format o_fmt);

/**
 * @brief ref_expand_mask_le_le_nu performs copy with expand operation for LE format - if current mask_ptr bit
 *                                 is 1 - than corresponding src_ptr n-bit value is copied to dst_ptr,
 *                                 if 0 - than zero is put to dst_ptr
 *
 * @param[in]      src_ptr   - source N-bit data in LE format (N can vary from 1 to 32)
 * @param[in]      mask_ptr  - mask 1-bit data
 * @param[in]      mask_be   - >0 if mask_ptr points to BE data, 0 if LE
 * @param[in,out]  pp_dst    - pointer to destination buffer
 * @param[in]      i_offset  - initial index offset for nominal bit-vector and output modification qpl_ow_8|16|32
 * @param[in]      i_bytes   - ptr to number of bytes in source buffer
 * @param[in]      s_bit     - element width in bits
 * @param[in]      len       - number of elements to process
 * @param[in]      o_fmt     - output format specification - qpl_ow_nom/8/16/32
 *
 * @return @ref QPL_STS_OK in case of successful execution, or non-zero value, otherwise
 * Return values:
 *   - QPL_STS_OK
 *   - QPL_STS_DST_IS_SHORT_ERR     - in case of not enough place in destination buffer
 *   - QPL_STS_OUTPUT_OVERFLOW_ERR  - in case qpl_ow_8|16|32 format and nominal bit-vector - if index exceeds
 *                                    max value for used data type
 */
qpl_status ref_expand_mask_le_le_nu(uint8_t* src_ptr, uint8_t* mask_ptr, uint32_t mask_be, uint8_t** pp_dst,
                                    uint32_t i_offset, uint32_t* i_bytes, uint32_t s_bit, uint32_t len,
                                    qpl_out_format o_fmt);

/**
 * @brief ref_expand_mask_le_be_nu performs copy with expand operation from LE to BE format - if current mask_ptr bit
 *                                 is 1 - than corresponding src_ptr n-bit value is copied to dst_ptr,
 *                                 if 0 - than zero is put to dst_ptr
 *
 * @param[in]      src_ptr   - source N-bit data in LE format (N can vary from 1 to 32)
 * @param[in]      mask_ptr  - mask 1-bit data
 * @param[in]      mask_be   - >0 if mask_ptr points to BE data, 0 if LE
 * @param[in,out]  pp_dst    - pointer to destination buffer
 * @param[in]      i_offset  - initial index offset for nominal bit-vector and output modification qpl_ow_8|16|32
 * @param[in]      i_bytes   - ptr to number of bytes in source buffer
 * @param[in]      s_bit     - element width in bits
 * @param[in]      len       - number of elements to process
 * @param[in]      o_fmt     - output format specification - qpl_ow_nom/8/16/32
 *
 * @return @ref QPL_STS_OK in case of successful execution, or non-zero value, otherwise
 * Return values:
 *   - QPL_STS_OK
 *   - QPL_STS_DST_IS_SHORT_ERR  - in case of not enough place in destination buffer
 */
qpl_status ref_expand_mask_le_be_nu(uint8_t* src_ptr, uint8_t* mask_ptr, uint32_t mask_be, uint8_t** pp_dst,
                                    uint32_t i_offset, uint32_t* i_bytes, uint32_t s_bit, uint32_t len,
                                    qpl_out_format o_fmt);

/**
 * @brief ref_expand_mask_be_le_nu performs copy with expand operation from BE to LE format - if current mask_ptr bit
 *                                 is 1 - than corresponding src_ptr n-bit value is copied to dst_ptr,
 *                                 if 0 - than zero is put to dst_ptr
 *
 * @param[in]      src_ptr   - source N-bit data in LE format (N can vary from 1 to 32)
 * @param[in]      mask_ptr  - mask 1-bit data
 * @param[in]      mask_be   - >0 if mask_ptr points to BE data, 0 if LE
 * @param[in,out]  pp_dst    - pointer to destination buffer
 * @param[in]      i_offset  - initial index offset for nominal bit-vector and output modification qpl_ow_8|16|32
 * @param[in]      i_bytes   - ptr to number of bytes in source buffer
 * @param[in]      s_bit     - element width in bits
 * @param[in]      len       - number of elements to process
 * @param[in]      o_fmt     - output format specification - qpl_ow_nom/8/16/32
 *
 * @return @ref QPL_STS_OK in case of successful execution, or non-zero value, otherwise
 * Return values:
 *   - QPL_STS_OK
 *   - QPL_STS_DST_IS_SHORT_ERR     - in case of not enough place in destination buffer
 *   - QPL_STS_OUTPUT_OVERFLOW_ERR  - in case qpl_ow_8|16|32 format and nominal bit-vector - if index exceeds
 *                                    max value for used data type
 */
qpl_status ref_expand_mask_be_le_nu(uint8_t* src_ptr, uint8_t* mask_ptr, uint32_t mask_be, uint8_t** pp_dst,
                                    uint32_t i_offset, uint32_t* i_bytes, uint32_t s_bit, uint32_t len,
                                    qpl_out_format o_fmt);

/**
 * @brief ref_expand_mask_be_be_nu performs copy with expand operation from BE to BE format - if current mask_ptr bit is 1 -
 *        than corresponding src_ptr n-bit value is copied to dst_ptr, if 0 - than zero is put to dst_ptr
 *
 * @param[in]      src_ptr   - source N-bit data in LE format (N can vary from 1 to 32)
 * @param[in]      mask_ptr  - mask 1-bit data
 * @param[in]      mask_be   - >0 if mask_ptr points to BE data, 0 if LE
 * @param[in,out]  pp_dst    - pointer to destination buffer
 * @param[in]      i_offset  - initial index offset for nominal bit-vector and output modification qpl_ow_8|16|32
 * @param[in]      i_bytes   - ptr to number of bytes in source buffer
 * @param[in]      s_bit     - element width in bits
 * @param[in]      len       - number of elements to process
 * @param[in]      o_fmt     - output format specification - qpl_ow_nom/8/16/32
 *
 * @return @ref QPL_STS_OK in case of successful execution, or non-zero value, otherwise
 * Return values:
 *   - QPL_STS_OK
 *   - QPL_STS_DST_IS_SHORT_ERR     - in case of not enough place in destination buffer
 *   - QPL_STS_OUTPUT_OVERFLOW_ERR  - in case qpl_ow_8|16|32 format and nominal bit-vector - if index exceeds
 *                                    max value for used data type
 */
qpl_status ref_expand_mask_be_be_nu(uint8_t* src_ptr, uint8_t* mask_ptr, uint32_t mask_be, uint8_t** pp_dst,
                                    uint32_t i_offset, uint32_t* i_bytes, uint32_t s_bit, uint32_t len,
                                    qpl_out_format o_fmt);

/**
 * @todo ref_copy
 * @param z
 * @return
 */
REF_INLINE uint16_t ref_cvt_le_2_be_16u(uint16_t z) {
    union {
        uint16_t u_int;
        uint8_t  u_byte[2];
    } x, y;

    x.u_int     = (uint16_t)z; // reverse bytes for BE
    y.u_byte[0] = x.u_byte[1];
    y.u_byte[1] = x.u_byte[0];

    return y.u_int;
}

/**
 * @todo ref_copy
 * @param z
 * @return
 */
REF_INLINE uint32_t ref_cvt_le_2_be_32u(uint32_t z) {
    union {
        uint32_t u_int;
        uint8_t  u_byte[4];
    } x, y;

    x.u_int     = (uint32_t)z; // reverse bytes for BE
    y.u_byte[0] = x.u_byte[3];
    y.u_byte[1] = x.u_byte[2];
    y.u_byte[2] = x.u_byte[1];
    y.u_byte[3] = x.u_byte[0];

    return y.u_int;
}

/**
 * @todo ref_copy
 * @param fmt
 * @param s_bit
 * @return
 */
REF_INLINE uint32_t ref_fmt_2_bits(qpl_out_format fmt, uint32_t s_bit) {
    if (qpl_ow_nom == fmt) {
        return s_bit;
    } else if (qpl_ow_8 == fmt) {
        return REF_8U_BITS;
    } else if (qpl_ow_16 == fmt) {
        return REF_16U_BITS;
    } else {
        return REF_32U_BITS;
    }
}

#if defined(__cplusplus)
}
#endif

#endif // _QPL_REF_COPY_H_

/** @} */
