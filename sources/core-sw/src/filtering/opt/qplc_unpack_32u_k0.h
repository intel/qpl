/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @brief Contains implementation of functions for unpacking 17..32-bit data to dwords
 * @date 08/02/2020
 *
 * @details Function list:
 *          - @ref k0_qplc_unpack_17u32u
 *          - @ref k0_qplc_unpack_18u32u
 *          - @ref k0_qplc_unpack_19u32u
 *          - @ref k0_qplc_unpack_20u32u
 *          - @ref k0_qplc_unpack_21u32u
 *          - @ref k0_qplc_unpack_22u32u
 *          - @ref k0_qplc_unpack_23u32u
 *          - @ref k0_qplc_unpack_24u32u
 *          - @ref k0_qplc_unpack_25u32u
 *          - @ref k0_qplc_unpack_26u32u
 *          - @ref k0_qplc_unpack_27u32u
 *          - @ref k0_qplc_unpack_28u32u
 *          - @ref k0_qplc_unpack_29u32u
 *          - @ref k0_qplc_unpack_30u32u
 *          - @ref k0_qplc_unpack_31u32u
 *          - @ref k0_qplc_unpack_32u32u
 *
 */
#pragma once

#include "own_qplc_defs.h"

// ------------------------------------ 17u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_17u_0[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 3U, 4U,
                                                                      4U, 5U, 5U, 6U, 6U, 7U, 7U, 8U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_17u_1[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 3U, 4U,
                                                                      4U, 5U, 5U, 6U, 6U, 7U, 7U, 8U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_17u_0[8])         = {0U, 2U, 4U, 6U, 8U, 10U, 12U, 14U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_17u_1[8])         = {15U, 13U, 11U, 9U, 7U, 5U, 3U, 1U};

// ------------------------------------ 18u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_18u_0[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 3U, 4U,
                                                                      4U, 5U, 5U, 6U, 6U, 7U, 7U, 8U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_18u_1[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 3U, 4U,
                                                                      5U, 6U, 6U, 7U, 7U, 8U, 8U, 9U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_18u_0[8])         = {0U, 4U, 8U, 12U, 16U, 20U, 24U, 28U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_18u_1[8])         = {14U, 10U, 6U, 2U, 30U, 26U, 22U, 18U};

// ------------------------------------ 19u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_19u_0[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 3U, 4U,
                                                                      4U, 5U, 5U, 6U, 7U, 8U, 8U, 9U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_19u_1[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 4U, 5U,
                                                                      5U, 6U, 6U, 7U, 7U, 8U, 8U, 9U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_19u_0[8])         = {0U, 6U, 12U, 18U, 24U, 30U, 4U, 10U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_19u_1[8])         = {13U, 7U, 1U, 27U, 21U, 15U, 9U, 3U};

// ------------------------------------ 20u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_20u_0[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 3U, 4U,
                                                                      5U, 6U, 6U, 7U, 7U, 8U, 8U, 9U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_20u_1[16]) = {0U, 1U, 1U, 2U, 3U, 4U, 4U, 5U,
                                                                      5U, 6U, 6U, 7U, 8U, 9U, 9U, 10U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_20u_0[8])         = {0U, 8U, 16U, 24U, 0U, 8U, 16U, 24U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_20u_1[8])         = {12U, 4U, 28U, 20U, 12U, 4U, 28U, 20U};

// ------------------------------------ 21u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_21u_0[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 3U, 4U,
                                                                      5U, 6U, 6U, 7U, 7U, 8U, 9U, 10U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_21u_1[16]) = {0U, 1U, 1U, 2U, 3U, 4U, 4U, 5U,
                                                                      5U, 6U, 7U, 8U, 8U, 9U, 9U, 10U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_21u_0[8])         = {0U, 10U, 20U, 30U, 8U, 18U, 28U, 6U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_21u_1[8])         = {11U, 1U, 23U, 13U, 3U, 25U, 15U, 5U};

// ------------------------------------ 22u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_22u_0[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 4U, 5U,
                                                                      5U, 6U, 6U, 7U, 8U, 9U, 9U, 10U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_22u_1[16]) = {0U, 1U, 2U, 3U, 3U, 4U, 4U,  5U,
                                                                      6U, 7U, 7U, 8U, 8U, 9U, 10U, 11U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_22u_0[8])         = {0U, 12U, 24U, 4U, 16U, 28U, 8U, 20U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_22u_1[8])         = {10U, 30U, 18U, 6U, 26U, 14U, 2U, 22U};

// ------------------------------------ 23u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_23u_0[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 4U,  5U,
                                                                      5U, 6U, 7U, 8U, 8U, 9U, 10U, 11U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_23u_1[16]) = {0U, 1U, 2U, 3U, 3U, 4U,  5U,  6U,
                                                                      6U, 7U, 7U, 8U, 9U, 10U, 10U, 11U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_23u_0[8])         = {0U, 14U, 28U, 10U, 24U, 6U, 20U, 2U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_23u_1[8])         = {9U, 27U, 13U, 31U, 17U, 3U, 21U, 7U};

// ------------------------------------ 24u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_24u_0[16]) = {0U, 1U, 1U, 2U, 3U, 4U,  4U,  5U,
                                                                      6U, 7U, 7U, 8U, 9U, 10U, 10U, 11U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_24u_1[16]) = {0U, 1U, 2U, 3U, 3U, 4U,  5U,  6U,
                                                                      6U, 7U, 8U, 9U, 9U, 10U, 11U, 12U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_24u_0[8])         = {0U, 16U, 0U, 16U, 0U, 16U, 0U, 16U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_24u_1[8])         = {8U, 24U, 8U, 24U, 8U, 24U, 8U, 24U};

// ------------------------------------ 25u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_25u_0[16]) = {0U, 1U, 1U, 2U, 3U, 4U,  4U,  5U,
                                                                      6U, 7U, 7U, 8U, 9U, 10U, 10U, 11U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_25u_1[16]) = {0U, 1U, 2U, 3U, 3U,  4U,  5U,  6U,
                                                                      7U, 8U, 8U, 9U, 10U, 11U, 11U, 12U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_25u_0[8])         = {0U, 18U, 4U, 22U, 8U, 26U, 12U, 30U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_25u_1[8])         = {7U, 21U, 3U, 17U, 31U, 13U, 27U, 9U};

// ------------------------------------ 26u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_26u_0[16]) = {0U, 1U, 1U, 2U, 3U, 4U,  4U,  5U,
                                                                      6U, 7U, 8U, 9U, 9U, 10U, 11U, 12U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_26u_1[16]) = {0U, 1U, 2U, 3U, 4U,  5U,  5U,  6U,
                                                                      7U, 8U, 8U, 9U, 10U, 11U, 12U, 13U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_26u_0[8])         = {0U, 20U, 8U, 28U, 16U, 4U, 24U, 12U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_26u_1[8])         = {6U, 18U, 30U, 10U, 22U, 2U, 14U, 26U};

// ------------------------------------ 27u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_27u_0[16]) = {0U, 1U, 1U, 2U, 3U,  4U,  5U,  6U,
                                                                      6U, 7U, 8U, 9U, 10U, 11U, 11U, 12U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_27u_1[16]) = {0U, 1U, 2U, 3U,  4U,  5U,  5U,  6U,
                                                                      7U, 8U, 9U, 10U, 10U, 11U, 12U, 13U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_27u_0[8])         = {0U, 22U, 12U, 2U, 24U, 14U, 4U, 26U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_27u_1[8])         = {5U, 15U, 25U, 3U, 13U, 23U, 1U, 11U};

// ------------------------------------ 28u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_28u_0[16]) = {0U, 1U, 1U, 2U, 3U,  4U,  5U,  6U,
                                                                      7U, 8U, 8U, 9U, 10U, 11U, 12U, 13U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_28u_1[16]) = {0U, 1U, 2U, 3U,  4U,  5U,  6U,  7U,
                                                                      7U, 8U, 9U, 10U, 11U, 12U, 13U, 14U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_28u_0[8])         = {0U, 24U, 16U, 8U, 0U, 24U, 16U, 8U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_28u_1[8])         = {4U, 12U, 20U, 28U, 4U, 12U, 20U, 28U};

// ------------------------------------ 29u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_29u_0[16]) = {0U, 1U, 1U, 2U,  3U,  4U,  5U,  6U,
                                                                      7U, 8U, 9U, 10U, 10U, 11U, 12U, 13U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_29u_1[16]) = {0U, 1U, 2U, 3U,  4U,  5U,  6U,  7U,
                                                                      8U, 9U, 9U, 10U, 11U, 12U, 13U, 14U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_29u_0[8])         = {0U, 26U, 20U, 14U, 8U, 2U, 28U, 22U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_29u_1[8])         = {3U, 9U, 15U, 21U, 27U, 1U, 7U, 13U};

// ------------------------------------ 30u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_30u_0[16]) = {0U, 1U, 1U, 2U,  3U,  4U,  5U,  6U,
                                                                      7U, 8U, 9U, 10U, 11U, 12U, 13U, 14U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_30u_1[16]) = {0U, 1U, 2U,  3U,  4U,  5U,  6U,  7U,
                                                                      8U, 9U, 10U, 11U, 12U, 13U, 14U, 15U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_30u_0[8])         = {0U, 28U, 24U, 20U, 16U, 12U, 8U, 4U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_30u_1[8])         = {2U, 6U, 10U, 14U, 18U, 22U, 26U, 30U};

// ------------------------------------ 31u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_31u_0[16]) = {0U, 1U, 1U, 2U,  3U,  4U,  5U,  6U,
                                                                      7U, 8U, 9U, 10U, 11U, 12U, 13U, 14U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_31u_1[16]) = {0U, 1U, 2U,  3U,  4U,  5U,  6U,  7U,
                                                                      8U, 9U, 10U, 11U, 12U, 13U, 14U, 15U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_31u_0[8])         = {0U, 30U, 28U, 26U, 24U, 22U, 20U, 18U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_31u_1[8])         = {1U, 3U, 5U, 7U, 9U, 11U, 13U, 15U};

OWN_QPLC_INLINE(uint32_t, own_get_align, (uint32_t start_bit, uint32_t base, uint32_t bitsize)) {
    uint32_t remnant   = bitsize - start_bit;
    uint32_t ret_value = 0xFFFFFFFF;
    for (uint32_t i = 0U; i < bitsize; ++i) {
        uint32_t test_value = (i * base) % bitsize;
        if (test_value == remnant) {
            ret_value = i;
            break;
        }
    }
    return ret_value;
}

OWN_QPLC_INLINE(void, px_qplc_unpack_Nu32u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint32_t bit_width,
                 uint8_t* dst_ptr)) {
    uint64_t  mask        = OWN_BIT_MASK(bit_width);
    uint64_t  next_dword  = 0U;
    uint32_t* src32u_ptr  = (uint32_t*)src_ptr;
    uint8_t*  src8u_ptr   = (uint8_t*)src_ptr;
    uint32_t* dst32u_ptr  = (uint32_t*)dst_ptr;
    uint32_t  bits_in_buf = 0U;
    uint64_t  src         = 0U;

    if (2U < num_elements) {
        bits_in_buf = OWN_DWORD_WIDTH - start_bit;
        src         = (uint64_t)((*src32u_ptr) >> start_bit);

        src32u_ptr++;

        while (2U < num_elements) {
            if (bit_width > bits_in_buf) {
                next_dword = (uint64_t)(*src32u_ptr);
                src32u_ptr++;
                next_dword = next_dword << bits_in_buf;
                src        = src | next_dword;
                bits_in_buf += OWN_DWORD_WIDTH;
            }
            *dst32u_ptr = (uint32_t)(src & mask);
            src         = src >> bit_width;
            bits_in_buf -= bit_width;
            dst32u_ptr++;
            num_elements--;
        }

        src8u_ptr = (uint8_t*)src32u_ptr;
    } else {
        while (start_bit > bits_in_buf) {
            next_dword = (uint64_t)(*src8u_ptr);
            src8u_ptr++;
            next_dword = next_dword << bits_in_buf;
            src        = src | next_dword;
            bits_in_buf += OWN_BYTE_WIDTH;
        }

        src >>= start_bit;
        bits_in_buf -= start_bit;
    }

    while (0U < num_elements) {
        while (bit_width > bits_in_buf) {
            next_dword = (uint64_t)(*src8u_ptr);
            src8u_ptr++;
            next_dword = next_dword << bits_in_buf;
            src        = src | next_dword;
            bits_in_buf += OWN_BYTE_WIDTH;
        }
        *dst32u_ptr = (uint32_t)(src & mask);
        src         = src >> bit_width;
        bits_in_buf -= bit_width;
        dst32u_ptr++;
        num_elements--;
    }
}

// ********************** 17u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_17u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 17U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu32u(src_ptr, align, start_bit, 17U, dst_ptr);
        src_ptr += ((align * 17U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask32 read_mask   = OWN_BIT_MASK(17U);
        __m512i   parse_mask0 = _mm512_set1_epi32(OWN_BIT_MASK(17U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_17u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_17u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_17u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_17u_1);

        while (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 17U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu32u(src_ptr, num_elements, 0U, 17U, dst_ptr); }
}

// ********************** 18u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_18u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 18U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu32u(src_ptr, align, start_bit, 18U, dst_ptr);
        src_ptr += ((align * 18U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask16 read_mask   = OWN_BIT_MASK(OWN_BITS_2_DWORD(18U * OWN_WORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi32(OWN_BIT_MASK(18U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_18u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_18u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_18u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_18u_1);

        while (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi32(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 18U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu32u(src_ptr, num_elements, 0U, 18U, dst_ptr); }
}

// ********************** 19u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_19u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 19U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu32u(src_ptr, align, start_bit, 19U, dst_ptr);
        src_ptr += ((align * 19U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask32 read_mask   = OWN_BIT_MASK(19U);
        __m512i   parse_mask0 = _mm512_set1_epi32(OWN_BIT_MASK(19U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_19u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_19u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_19u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_19u_1);

        while (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 19U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu32u(src_ptr, num_elements, 0U, 19U, dst_ptr); }
}

// ********************** 20u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_20u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 20U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu32u(src_ptr, align, start_bit, 20U, dst_ptr);
        src_ptr += ((align * 20U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask16 read_mask   = OWN_BIT_MASK(OWN_BITS_2_DWORD(20U * OWN_WORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi32(OWN_BIT_MASK(20U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_20u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_20u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_20u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_20u_1);

        while (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi32(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 20U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu32u(src_ptr, num_elements, 0U, 20U, dst_ptr); }
}

// ********************** 21u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_21u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 21U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu32u(src_ptr, align, start_bit, 21U, dst_ptr);
        src_ptr += ((align * 21U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask32 read_mask   = OWN_BIT_MASK(21U);
        __m512i   parse_mask0 = _mm512_set1_epi32(OWN_BIT_MASK(21U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_21u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_21u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_21u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_21u_1);

        while (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 21U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu32u(src_ptr, num_elements, 0U, 21U, dst_ptr); }
}

// ********************** 22u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_22u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 22U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu32u(src_ptr, align, start_bit, 22U, dst_ptr);
        src_ptr += ((align * 22U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask16 read_mask   = OWN_BIT_MASK(OWN_BITS_2_DWORD(22U * OWN_WORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi32(OWN_BIT_MASK(22U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_22u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_22u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_22u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_22u_1);

        while (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi32(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 22U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu32u(src_ptr, num_elements, 0U, 22U, dst_ptr); }
}

// ********************** 23u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_23u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 23U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu32u(src_ptr, align, start_bit, 23U, dst_ptr);
        src_ptr += ((align * 23U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask32 read_mask   = OWN_BIT_MASK(23U);
        __m512i   parse_mask0 = _mm512_set1_epi32(OWN_BIT_MASK(23U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_23u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_23u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_23u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_23u_1);

        while (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 23U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu32u(src_ptr, num_elements, 0U, 23U, dst_ptr); }
}

// ********************** 24u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_24u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 24U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu32u(src_ptr, align, start_bit, 24U, dst_ptr);
        src_ptr += ((align * 24U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask16 read_mask   = OWN_BIT_MASK(OWN_BITS_2_DWORD(24U * OWN_WORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi32(OWN_BIT_MASK(24U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_24u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_24u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_24u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_24u_1);

        while (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi32(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 24U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu32u(src_ptr, num_elements, 0U, 24U, dst_ptr); }
}

// ********************** 25u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_25u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 25U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu32u(src_ptr, align, start_bit, 25U, dst_ptr);
        src_ptr += ((align * 25U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask32 read_mask   = OWN_BIT_MASK(25U);
        __m512i   parse_mask0 = _mm512_set1_epi32(OWN_BIT_MASK(25U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_25u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_25u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_25u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_25u_1);

        while (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 25U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu32u(src_ptr, num_elements, 0U, 25U, dst_ptr); }
}

// ********************** 26u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_26u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 26U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu32u(src_ptr, align, start_bit, 26U, dst_ptr);
        src_ptr += ((align * 26U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask16 read_mask   = OWN_BIT_MASK(OWN_BITS_2_DWORD(26U * OWN_WORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi32(OWN_BIT_MASK(26U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_26u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_26u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_26u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_26u_1);

        while (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi32(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 26U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu32u(src_ptr, num_elements, 0U, 26U, dst_ptr); }
}

// ********************** 27u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_27u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 27U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu32u(src_ptr, align, start_bit, 27U, dst_ptr);
        src_ptr += ((align * 27U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask32 read_mask   = OWN_BIT_MASK(27U);
        __m512i   parse_mask0 = _mm512_set1_epi32(OWN_BIT_MASK(27U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_27u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_27u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_27u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_27u_1);

        while (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 27U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu32u(src_ptr, num_elements, 0U, 27U, dst_ptr); }
}

// ********************** 28u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_28u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 28U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu32u(src_ptr, align, start_bit, 28U, dst_ptr);
        src_ptr += ((align * 28U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask16 read_mask   = OWN_BIT_MASK(OWN_BITS_2_DWORD(28U * OWN_WORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi32(OWN_BIT_MASK(28U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_28u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_28u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_28u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_28u_1);

        while (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi32(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 28U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu32u(src_ptr, num_elements, 0U, 28U, dst_ptr); }
}

// ********************** 29u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_29u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 29U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu32u(src_ptr, align, start_bit, 29U, dst_ptr);
        src_ptr += ((align * 29U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask32 read_mask   = OWN_BIT_MASK(29U);
        __m512i   parse_mask0 = _mm512_set1_epi32(OWN_BIT_MASK(29U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_29u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_29u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_29u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_29u_1);

        while (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 29U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu32u(src_ptr, num_elements, 0U, 29U, dst_ptr); }
}

// ********************** 30u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_30u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 30U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu32u(src_ptr, align, start_bit, 30U, dst_ptr);
        src_ptr += ((align * 30U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask16 read_mask   = OWN_BIT_MASK(OWN_BITS_2_DWORD(30U * OWN_WORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi32(OWN_BIT_MASK(30U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_30u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_30u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_30u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_30u_1);

        while (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi32(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 30U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu32u(src_ptr, num_elements, 0U, 30U, dst_ptr); }
}

// ********************** 31u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_31u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 31U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu32u(src_ptr, align, start_bit, 31U, dst_ptr);
        src_ptr += ((align * 31U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask32 read_mask   = OWN_BIT_MASK(31U);
        __m512i   parse_mask0 = _mm512_set1_epi32(OWN_BIT_MASK(31U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_31u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_31u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_31u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_31u_1);

        while (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 31U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu32u(src_ptr, num_elements, 0U, 31U, dst_ptr); }
}
