/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_CORE_SW_SRC_FILTERING_OPT_QPLC_UNPACK_16U_K0_H
#define QPL_SOURCES_CORE_SW_SRC_FILTERING_OPT_QPLC_UNPACK_16U_K0_H

/**
 * @brief Contains implementation of functions for unpacking 9..16-bit data to words
 * @date 08/02/2020
 *
 * @details Function list:
 *          - @ref k0_qplc_unpack_9u16u
 *          - @ref k0_qplc_unpack_10u16u
 *          - @ref k0_qplc_unpack_11u16u
 *          - @ref k0_qplc_unpack_12u16u
 *          - @ref k0_qplc_unpack_13u16u
 *          - @ref k0_qplc_unpack_14u16u
 *          - @ref k0_qplc_unpack_15u16u
 *          - @ref k0_qplc_unpack_16u16u
 *
 */

#include "own_qplc_defs.h"

// ------------------------------------ 9u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_9u_0[32]) = {
        0U, 1U,  1U,  2U,  2U,  3U,  3U,  4U,  4U,  5U,  5U,  6U,  6U,  7U,  7U,  8U,
        9U, 10U, 10U, 11U, 11U, 12U, 12U, 13U, 13U, 14U, 14U, 15U, 15U, 16U, 16U, 17U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_9u_1[32]) = {
        0U, 1U,  1U,  2U,  2U,  3U,  3U,  4U,  5U,  6U,  6U,  7U,  7U,  8U,  8U,  9U,
        9U, 10U, 10U, 11U, 11U, 12U, 12U, 13U, 14U, 15U, 15U, 16U, 16U, 17U, 17U, 18U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_9u_0[16]) = {0U, 2U, 4U, 6U, 8U, 10U, 12U, 14U,
                                                              0U, 2U, 4U, 6U, 8U, 10U, 12U, 14U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_9u_1[16]) = {7U, 5U, 3U, 1U, 15U, 13U, 11U, 9U,
                                                              7U, 5U, 3U, 1U, 15U, 13U, 11U, 9U};

// ------------------------------------ 10u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_10u_0[32]) = {
        0U,  1U,  1U,  2U,  2U,  3U,  3U,  4U,  5U,  6U,  6U,  7U,  7U,  8U,  8U,  9U,
        10U, 11U, 11U, 12U, 12U, 13U, 13U, 14U, 15U, 16U, 16U, 17U, 17U, 18U, 18U, 19U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_10u_1[32]) = {
        0U,  1U,  1U,  2U,  3U,  4U,  4U,  5U,  5U,  6U,  6U,  7U,  8U,  9U,  9U,  10U,
        10U, 11U, 11U, 12U, 13U, 14U, 14U, 15U, 15U, 16U, 16U, 17U, 18U, 19U, 19U, 20U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_10u_0[16]) = {0U, 4U, 8U, 12U, 0U, 4U, 8U, 12U,
                                                               0U, 4U, 8U, 12U, 0U, 4U, 8U, 12U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_10u_1[16]) = {6U, 2U, 14U, 10U, 6U, 2U, 14U, 10U,
                                                               6U, 2U, 14U, 10U, 6U, 2U, 14U, 10U};

// ------------------------------------ 11u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_11u_0[32]) = {
        0U,  1U,  1U,  2U,  2U,  3U,  4U,  5U,  5U,  6U,  6U,  7U,  8U,  9U,  9U,  10U,
        11U, 12U, 12U, 13U, 13U, 14U, 15U, 16U, 16U, 17U, 17U, 18U, 19U, 20U, 20U, 21U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_11u_1[32]) = {
        0U,  1U,  2U,  3U,  3U,  4U,  4U,  5U,  6U,  7U,  7U,  8U,  8U,  9U,  10U, 11U,
        11U, 12U, 13U, 14U, 14U, 15U, 15U, 16U, 17U, 18U, 18U, 19U, 19U, 20U, 21U, 22U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_11u_0[16]) = {0U, 6U, 12U, 2U, 8U, 14U, 4U, 10U,
                                                               0U, 6U, 12U, 2U, 8U, 14U, 4U, 10U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_11u_1[16]) = {5U, 15U, 9U, 3U, 13U, 7U, 1U, 11U,
                                                               5U, 15U, 9U, 3U, 13U, 7U, 1U, 11U};

// ------------------------------------ 12u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_12u_0[32]) = {
        0U,  1U,  1U,  2U,  3U,  4U,  4U,  5U,  6U,  7U,  7U,  8U,  9U,  10U, 10U, 11U,
        12U, 13U, 13U, 14U, 15U, 16U, 16U, 17U, 18U, 19U, 19U, 20U, 21U, 22U, 22U, 23U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_12u_1[32]) = {
        0U,  1U,  2U,  3U,  3U,  4U,  5U,  6U,  6U,  7U,  8U,  9U,  9U,  10U, 11U, 12U,
        12U, 13U, 14U, 15U, 15U, 16U, 17U, 18U, 18U, 19U, 20U, 21U, 21U, 22U, 23U, 24U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_12u_0[16]) = {0U, 8U, 0U, 8U, 0U, 8U, 0U, 8U,
                                                               0U, 8U, 0U, 8U, 0U, 8U, 0U, 8U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_12u_1[16]) = {4U, 12U, 4U, 12U, 4U, 12U, 4U, 12U,
                                                               4U, 12U, 4U, 12U, 4U, 12U, 4U, 12U};

// ------------------------------------ 13u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_13u_0[32]) = {
        0U,  1U,  1U,  2U,  3U,  4U,  4U,  5U,  6U,  7U,  8U,  9U,  9U,  10U, 11U, 12U,
        13U, 14U, 14U, 15U, 16U, 17U, 17U, 18U, 19U, 20U, 21U, 22U, 22U, 23U, 24U, 25U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_13u_1[32]) = {
        0U,  1U,  2U,  3U,  4U,  5U,  5U,  6U,  7U,  8U,  8U,  9U,  10U, 11U, 12U, 13U,
        13U, 14U, 15U, 16U, 17U, 18U, 18U, 19U, 20U, 21U, 21U, 22U, 23U, 24U, 25U, 26U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_13u_0[16]) = {0U, 10U, 4U, 14U, 8U, 2U, 12U, 6U,
                                                               0U, 10U, 4U, 14U, 8U, 2U, 12U, 6U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_13u_1[16]) = {3U, 9U, 15U, 5U, 11U, 1U, 7U, 13U,
                                                               3U, 9U, 15U, 5U, 11U, 1U, 7U, 13U};

// ------------------------------------ 14u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_14u_0[32]) = {
        0U,  1U,  1U,  2U,  3U,  4U,  5U,  6U,  7U,  8U,  8U,  9U,  10U, 11U, 12U, 13U,
        14U, 15U, 15U, 16U, 17U, 18U, 19U, 20U, 21U, 22U, 22U, 23U, 24U, 25U, 26U, 27U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_14u_1[32]) = {
        0U,  1U,  2U,  3U,  4U,  5U,  6U,  7U,  7U,  8U,  9U,  10U, 11U, 12U, 13U, 14U,
        14U, 15U, 16U, 17U, 18U, 19U, 20U, 21U, 21U, 22U, 23U, 24U, 25U, 26U, 27U, 28U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_14u_0[16]) = {0U, 12U, 8U, 4U, 0U, 12U, 8U, 4U,
                                                               0U, 12U, 8U, 4U, 0U, 12U, 8U, 4U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_14u_1[16]) = {2U, 6U, 10U, 14U, 2U, 6U, 10U, 14U,
                                                               2U, 6U, 10U, 14U, 2U, 6U, 10U, 14U};

// ------------------------------------ 15u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_15u_0[32]) = {
        0U,  1U,  1U,  2U,  3U,  4U,  5U,  6U,  7U,  8U,  9U,  10U, 11U, 12U, 13U, 14U,
        15U, 16U, 16U, 17U, 18U, 19U, 20U, 21U, 22U, 23U, 24U, 25U, 26U, 27U, 28U, 29U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_15u_1[32]) = {
        0U,  1U,  2U,  3U,  4U,  5U,  6U,  7U,  8U,  9U,  10U, 11U, 12U, 13U, 14U, 15U,
        15U, 16U, 17U, 18U, 19U, 20U, 21U, 22U, 23U, 24U, 25U, 26U, 27U, 28U, 29U, 30U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_15u_0[16]) = {0U, 14U, 12U, 10U, 8U, 6U, 4U, 2U,
                                                               0U, 14U, 12U, 10U, 8U, 6U, 4U, 2U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_15u_1[16]) = {1U, 3U, 5U, 7U, 9U, 11U, 13U, 15U,
                                                               1U, 3U, 5U, 7U, 9U, 11U, 13U, 15U};

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

OWN_QPLC_INLINE(void, px_qplc_unpack_Nu16u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint32_t bit_width,
                 uint8_t* dst_ptr)) {
    uint32_t  mask        = OWN_BIT_MASK(bit_width);
    uint32_t  next_word   = 0U;
    uint32_t  bits_in_buf = OWN_WORD_WIDTH - start_bit;
    uint16_t* src16u_ptr  = (uint16_t*)src_ptr;
    uint16_t* dst16u_ptr  = (uint16_t*)dst_ptr;
    uint32_t  src         = (uint32_t)((*src16u_ptr) >> start_bit);
    src16u_ptr++;

    while (1U < num_elements) {
        if (bit_width > bits_in_buf) {
            next_word = (uint32_t)(*src16u_ptr);
            src16u_ptr++;
            next_word = next_word << bits_in_buf;
            src       = src | next_word;
            bits_in_buf += OWN_WORD_WIDTH;
        }
        *dst16u_ptr = (uint16_t)(src & mask);
        src         = src >> bit_width;
        bits_in_buf -= bit_width;
        dst16u_ptr++;
        num_elements--;
    }

    if (bit_width > bits_in_buf) {
        next_word = (uint32_t)(bit_width - bits_in_buf > 8U ? *src16u_ptr : *((uint8_t*)src16u_ptr));
        next_word = next_word << bits_in_buf;
        src       = src | next_word;
    }
    *dst16u_ptr = (uint16_t)(src & mask);
}

OWN_OPT_FUN(void, k0_qplc_unpack_9u16u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 9U, 16U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu16u(src_ptr, align, start_bit, 9U, dst_ptr);
        src_ptr += ((align * 9U) + start_bit) >> 3;
        dst_ptr += align * 2;
        num_elements -= align;
    }

    if (num_elements >= 32U) {
        __mmask32 read_mask   = OWN_BIT_MASK(OWN_BITS_2_WORD(9U * OWN_DWORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi16(OWN_BIT_MASK(9U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_9u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_9u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_9u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_9u_1);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi16(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi32(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi16(zmm[0], 0xAAAAAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 4U * 9U;
            dst_ptr += 64U;
            num_elements -= 32U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu16u(src_ptr, num_elements, 0U, 9U, dst_ptr); }
}

OWN_OPT_FUN(void, k0_qplc_unpack_10u16u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 10U, 16U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu16u(src_ptr, align, start_bit, 10U, dst_ptr);
        src_ptr += ((align * 10U) + start_bit) >> 3;
        dst_ptr += align * 2;
        num_elements -= align;
    }

    if (num_elements >= 32U) {
        __mmask32 read_mask   = OWN_BIT_MASK(OWN_BITS_2_WORD(10U * OWN_DWORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi16(OWN_BIT_MASK(10U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_10u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_10u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_10u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_10u_1);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi16(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi32(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi16(zmm[0], 0xAAAAAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 4U * 10U;
            dst_ptr += 64U;
            num_elements -= 32U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu16u(src_ptr, num_elements, 0U, 10U, dst_ptr); }
}

OWN_OPT_FUN(void, k0_qplc_unpack_11u16u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 11U, 16U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu16u(src_ptr, align, start_bit, 11U, dst_ptr);
        src_ptr += ((align * 11U) + start_bit) >> 3;
        dst_ptr += align * 2;
        num_elements -= align;
    }

    if (num_elements >= 32U) {
        __mmask32 read_mask   = OWN_BIT_MASK(OWN_BITS_2_WORD(11U * OWN_DWORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi16(OWN_BIT_MASK(11U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_11u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_11u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_11u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_11u_1);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi16(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi32(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi16(zmm[0], 0xAAAAAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 4U * 11U;
            dst_ptr += 64U;
            num_elements -= 32U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu16u(src_ptr, num_elements, 0U, 11U, dst_ptr); }
}

OWN_OPT_FUN(void, k0_qplc_unpack_12u16u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 12U, 16U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu16u(src_ptr, align, start_bit, 12U, dst_ptr);
        src_ptr += ((align * 12U) + start_bit) >> 3;
        dst_ptr += align * 2;
        num_elements -= align;
    }

    if (num_elements >= 32U) {
        __mmask32 read_mask   = OWN_BIT_MASK(OWN_BITS_2_WORD(12U * OWN_DWORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi16(OWN_BIT_MASK(12U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_12u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_12u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_12u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_12u_1);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi16(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi32(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi16(zmm[0], 0xAAAAAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 4U * 12U;
            dst_ptr += 64U;
            num_elements -= 32U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu16u(src_ptr, num_elements, 0U, 12U, dst_ptr); }
}

OWN_OPT_FUN(void, k0_qplc_unpack_13u16u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 13U, 16U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu16u(src_ptr, align, start_bit, 13U, dst_ptr);
        src_ptr += ((align * 13U) + start_bit) >> 3;
        dst_ptr += align * 2;
        num_elements -= align;
    }

    if (num_elements >= 32U) {
        __mmask32 read_mask   = OWN_BIT_MASK(OWN_BITS_2_WORD(13U * OWN_DWORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi16(OWN_BIT_MASK(13U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_13u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_13u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_13u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_13u_1);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi16(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi32(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi16(zmm[0], 0xAAAAAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 4U * 13U;
            dst_ptr += 64U;
            num_elements -= 32U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu16u(src_ptr, num_elements, 0U, 13U, dst_ptr); }
}

OWN_OPT_FUN(void, k0_qplc_unpack_14u16u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 14U, 16U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu16u(src_ptr, align, start_bit, 14U, dst_ptr);
        src_ptr += ((align * 14U) + start_bit) >> 3;
        dst_ptr += align * 2;
        num_elements -= align;
    }

    if (num_elements >= 32U) {
        __mmask32 read_mask   = OWN_BIT_MASK(OWN_BITS_2_WORD(14U * OWN_DWORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi16(OWN_BIT_MASK(14U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_14u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_14u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_14u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_14u_1);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi16(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi32(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi16(zmm[0], 0xAAAAAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 4U * 14U;
            dst_ptr += 64U;
            num_elements -= 32U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu16u(src_ptr, num_elements, 0U, 14U, dst_ptr); }
}

OWN_OPT_FUN(void, k0_qplc_unpack_15u16u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 15U, 16U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_Nu16u(src_ptr, align, start_bit, 15U, dst_ptr);
        src_ptr += ((align * 15U) + start_bit) >> 3;
        dst_ptr += align * 2;
        num_elements -= align;
    }

    if (num_elements >= 32U) {
        __mmask32 read_mask   = OWN_BIT_MASK(OWN_BITS_2_WORD(15U * OWN_DWORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi16(OWN_BIT_MASK(15U));

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_15u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_15u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_15u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_15u_1);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi16(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi32(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi16(zmm[0], 0xAAAAAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 4U * 15U;
            dst_ptr += 64U;
            num_elements -= 32U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_Nu16u(src_ptr, num_elements, 0U, 15U, dst_ptr); }
}

#endif
