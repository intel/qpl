/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_CORE_SW_SRC_FILTERING_OPT_QPLC_UNPACK_BE_8U_K0_H
#define QPL_SOURCES_CORE_SW_SRC_FILTERING_OPT_QPLC_UNPACK_BE_8U_K0_H

/**
 * @brief Contains implementation of functions for unpacking 1..8-bit BE data to bytes
 * @date 07/06/2020
 *
 * @details Function list:
 *          - @ref k0_qplc_unpack_be_1u8u
 *          - @ref k0_qplc_unpack_be_2u8u
 *          - @ref k0_qplc_unpack_be_3u8u
 *          - @ref k0_qplc_unpack_be_4u8u
 *          - @ref k0_qplc_unpack_be_5u8u
 *          - @ref k0_qplc_unpack_be_6u8u
 *          - @ref k0_qplc_unpack_be_7u8u
 *          - @ref k0_qplc_unpack_be_8u8u
 *
 */

#include "own_qplc_defs.h"
#include "qplc_memop.h"

// ------------------------------------ 3u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_3u_0[64]) = {
        1U, 0U, 1U, 0U, 2U, 1U, 3U, 2U, 4U, 3U, 4U, 3U, 5U, 4U, 6U, 5U, 1U, 0U, 1U, 0U, 2U, 1U,
        3U, 2U, 4U, 3U, 4U, 3U, 5U, 4U, 6U, 5U, 1U, 0U, 1U, 0U, 2U, 1U, 3U, 2U, 4U, 3U, 4U, 3U,
        5U, 4U, 6U, 5U, 1U, 0U, 1U, 0U, 2U, 1U, 3U, 2U, 4U, 3U, 4U, 3U, 5U, 4U, 6U, 5U};
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_3u_1[64]) = {
        0U, 0U, 1U, 0U, 2U, 1U, 3U, 2U, 3U, 2U, 4U, 3U, 5U, 4U, 6U, 5U, 0U, 0U, 1U, 0U, 2U, 1U,
        3U, 2U, 3U, 2U, 4U, 3U, 5U, 4U, 6U, 5U, 0U, 0U, 1U, 0U, 2U, 1U, 3U, 2U, 3U, 2U, 4U, 3U,
        5U, 4U, 6U, 5U, 0U, 0U, 1U, 0U, 2U, 1U, 3U, 2U, 3U, 2U, 4U, 3U, 5U, 4U, 6U, 5U};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_table_3u_0[32])      = {13U, 7U,  9U,  11U, 13U, 7U,  9U,  11U, 13U, 7U,  9U,
                                                                   11U, 13U, 7U,  9U,  11U, 13U, 7U,  9U,  11U, 13U, 7U,
                                                                   9U,  11U, 13U, 7U,  9U,  11U, 13U, 7U,  9U,  11U};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_table_3u_1[32])      = {6U, 4U, 2U, 0U, 6U, 4U, 2U, 0U, 6U, 4U, 2U,
                                                                   0U, 6U, 4U, 2U, 0U, 6U, 4U, 2U, 0U, 6U, 4U,
                                                                   2U, 0U, 6U, 4U, 2U, 0U, 6U, 4U, 2U, 0U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_3u[32]) = {
        0U, 1U, 2U, 0x0, 0x0, 0x0, 0x0, 0x0, 3U, 4U,  5U,  0x0, 0x0, 0x0, 0x0, 0x0,
        6U, 7U, 8U, 0x0, 0x0, 0x0, 0x0, 0x0, 9U, 10U, 11U, 0x0, 0x0, 0x0, 0x0, 0x0};

// ------------------------------------ 5u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_5u_0[64]) = {
        1U, 0U, 2U, 1U, 3U, 2U, 4U, 3U, 6U, 5U, 7U, 6U, 8U, 7U, 9U, 8U, 1U, 0U, 2U, 1U, 3U, 2U,
        4U, 3U, 6U, 5U, 7U, 6U, 8U, 7U, 9U, 8U, 1U, 0U, 2U, 1U, 3U, 2U, 4U, 3U, 6U, 5U, 7U, 6U,
        8U, 7U, 9U, 8U, 1U, 0U, 2U, 1U, 3U, 2U, 4U, 3U, 6U, 5U, 7U, 6U, 8U, 7U, 9U, 8U};
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_5u_1[64]) = {
        1U, 0U, 2U,  1U, 3U, 2U, 5U, 4U, 6U,  5U, 7U, 6U, 8U, 7U, 10U, 9U, 1U, 0U, 2U,  1U, 3U, 2U,
        5U, 4U, 6U,  5U, 7U, 6U, 8U, 7U, 10U, 9U, 1U, 0U, 2U, 1U, 3U,  2U, 5U, 4U, 6U,  5U, 7U, 6U,
        8U, 7U, 10U, 9U, 1U, 0U, 2U, 1U, 3U,  2U, 5U, 4U, 6U, 5U, 7U,  6U, 8U, 7U, 10U, 9U};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_table_5u_0[32])      = {11U, 9U,  7U,  5U, 11U, 9U,  7U,  5U, 11U, 9U,  7U,
                                                                   5U,  11U, 9U,  7U, 5U,  11U, 9U,  7U, 5U,  11U, 9U,
                                                                   7U,  5U,  11U, 9U, 7U,  5U,  11U, 9U, 7U,  5U};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_table_5u_1[32])      = {2U, 4U, 6U, 0U, 2U, 4U, 6U, 0U, 2U, 4U, 6U,
                                                                   0U, 2U, 4U, 6U, 0U, 2U, 4U, 6U, 0U, 2U, 4U,
                                                                   6U, 0U, 2U, 4U, 6U, 0U, 2U, 4U, 6U, 0U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_5u[32]) = {
        0U,  1U,  2U,  3U,  4U,  0x0, 0x0, 0x0, 5U,  6U,  7U,  8U,  9U,  0x0, 0x0, 0x0,
        10U, 11U, 12U, 13U, 14U, 0x0, 0x0, 0x0, 15U, 16U, 17U, 18U, 19U, 0x0, 0x0, 0x0};

// ------------------------------------ 6u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_6u_0[64]) = {
        1U,  0U, 2U,  1U,  4U, 3U, 5U,  4U, 7U,  6U,  8U, 7U, 10U, 9U, 11U, 10U, 1U,  0U, 2U,  1U, 4U, 3U,
        5U,  4U, 7U,  6U,  8U, 7U, 10U, 9U, 11U, 10U, 1U, 0U, 2U,  1U, 4U,  3U,  5U,  4U, 7U,  6U, 8U, 7U,
        10U, 9U, 11U, 10U, 1U, 0U, 2U,  1U, 4U,  3U,  5U, 4U, 7U,  6U, 8U,  7U,  10U, 9U, 11U, 10U};
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_6u_1[64]) = {
        1U,  0U, 3U,  2U,  4U, 3U, 6U,  5U, 7U,  6U,  9U, 8U, 10U, 9U, 12U, 11U, 1U,  0U, 3U,  2U, 4U, 3U,
        6U,  5U, 7U,  6U,  9U, 8U, 10U, 9U, 12U, 11U, 1U, 0U, 3U,  2U, 4U,  3U,  6U,  5U, 7U,  6U, 9U, 8U,
        10U, 9U, 12U, 11U, 1U, 0U, 3U,  2U, 4U,  3U,  6U, 5U, 7U,  6U, 9U,  8U,  10U, 9U, 12U, 11U};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_table_6u_0[32]) = {10U, 6U,  10U, 6U,  10U, 6U,  10U, 6U,  10U, 6U,  10U,
                                                              6U,  10U, 6U,  10U, 6U,  10U, 6U,  10U, 6U,  10U, 6U,
                                                              10U, 6U,  10U, 6U,  10U, 6U,  10U, 6U,  10U, 6U};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_table_6u_1[32]) = {4U, 0U, 4U, 0U, 4U, 0U, 4U, 0U, 4U, 0U, 4U,
                                                              0U, 4U, 0U, 4U, 0U, 4U, 0U, 4U, 0U, 4U, 0U,
                                                              4U, 0U, 4U, 0U, 4U, 0U, 4U, 0U, 4U, 0U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_6u[16]) = {0U, 1U, 2U, 0x0, 3U, 4U,  5U,  0x0,
                                                                   6U, 7U, 8U, 0x0, 9U, 10U, 11U, 0x0};

// ------------------------------------ 7u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_7u_0[64]) = {
        1U,  0U,  2U,  1U,  4U, 3U, 6U,  5U,  8U,  7U,  9U, 8U, 11U, 10U, 13U, 12U, 1U,  0U,  2U,  1U, 4U, 3U,
        6U,  5U,  8U,  7U,  9U, 8U, 11U, 10U, 13U, 12U, 1U, 0U, 2U,  1U,  4U,  3U,  6U,  5U,  8U,  7U, 9U, 8U,
        11U, 10U, 13U, 12U, 1U, 0U, 2U,  1U,  4U,  3U,  6U, 5U, 8U,  7U,  9U,  8U,  11U, 10U, 13U, 12U};
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_7u_1[64]) = {
        1U,  0U,  3U,  2U,  5U,  4U, 7U,  6U,  8U,  7U,  10U, 9U, 12U, 11U, 14U, 13U, 1U,  0U,  3U,  2U, 5U,  4U,
        7U,  6U,  8U,  7U,  10U, 9U, 12U, 11U, 14U, 13U, 1U,  0U, 3U,  2U,  5U,  4U,  7U,  6U,  8U,  7U, 10U, 9U,
        12U, 11U, 14U, 13U, 1U,  0U, 3U,  2U,  5U,  4U,  7U,  6U, 8U,  7U,  10U, 9U,  12U, 11U, 14U, 13U};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_table_7u_0[32])      = {9U, 3U, 5U, 7U, 9U, 3U, 5U, 7U, 9U, 3U, 5U,
                                                                   7U, 9U, 3U, 5U, 7U, 9U, 3U, 5U, 7U, 9U, 3U,
                                                                   5U, 7U, 9U, 3U, 5U, 7U, 9U, 3U, 5U, 7U};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_table_7u_1[32])      = {6U, 4U, 2U, 0U, 6U, 4U, 2U, 0U, 6U, 4U, 2U,
                                                                   0U, 6U, 4U, 2U, 0U, 6U, 4U, 2U, 0U, 6U, 4U,
                                                                   2U, 0U, 6U, 4U, 2U, 0U, 6U, 4U, 2U, 0U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_7u[32]) = {
        0U,  1U,  2U,  3U,  4U,  5U,  6U,  0x0, 7U,  8U,  9U,  10U, 11U, 12U, 13U, 0x0,
        14U, 15U, 16U, 17U, 18U, 19U, 20U, 0x0, 21U, 22U, 23U, 24U, 25U, 26U, 27U, 0x0};

OWN_ALIGNED_64_ARRAY(static uint64_t reverse_mask_table_1u[8]) = {
        0x0001020304050607, 0x08090A0B0C0D0E0F, 0x1011121314151617, 0x18191A1B1C1D1E1F,
        0x2021222324252627, 0x28292A2B2C2D2E2F, 0x3031323334353637, 0x38393A3B3C3D3E3F};

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

// For BE start_bit is bit index from the top of a byte
OWN_QPLC_INLINE(void, px_qplc_unpack_be_Nu8u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint32_t bit_width,
                 uint8_t* dst_ptr)) {
    uint16_t next_byte   = 0U;
    uint32_t bits_in_buf = OWN_BYTE_WIDTH - start_bit;
    uint32_t shift       = OWN_WORD_WIDTH - bit_width;
    uint16_t src         = ((uint16_t)(*src_ptr)) << (start_bit + OWN_BYTE_WIDTH);
    src_ptr++;

    while (0U < num_elements) {
        if (bit_width > bits_in_buf) {
            next_byte = (uint16_t)(*src_ptr);
            src_ptr++;
            next_byte = next_byte << (OWN_BYTE_WIDTH - bits_in_buf);
            src       = src | next_byte;
            bits_in_buf += OWN_BYTE_WIDTH;
        }
        *dst_ptr = (uint8_t)(src >> shift);
        src      = src << bit_width;
        bits_in_buf -= bit_width;
        dst_ptr++;
        num_elements--;
    }
}

// ********************** 1u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_1u8u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (0U < start_bit) {
        uint32_t align = own_get_align(start_bit, 1U, 8U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu8u(src_ptr, align, start_bit, 1U, dst_ptr);
        src_ptr += (align + start_bit) >> 3U;
        dst_ptr += align;
        num_elements -= align;
    }

    if (num_elements >= 64U) {
        __m512i reverse_mask_1u = _mm512_load_si512(reverse_mask_table_1u);
        while (num_elements >= 64U) {
            uint64_t src_64 = *(uint64_t*)src_ptr;
            // convert mask to 512-bit register. 0 --> 0x00, 1 --> 0xFF
            __m512i srcmm = _mm512_movm_epi8(src_64);
            // make 0x00 --> 0x00, 0xFF --> 0x01
            srcmm = _mm512_abs_epi8(srcmm);
            srcmm = _mm512_shuffle_epi8(srcmm, reverse_mask_1u);
            _mm512_storeu_si512(dst_ptr, srcmm);

            src_ptr += 8U * 1U;
            dst_ptr += 64U;
            num_elements -= 64U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu8u(src_ptr, num_elements, 0U, 1U, dst_ptr); }
}

// ********************** 2u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_2u8u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (0U < start_bit) {
        uint32_t align = own_get_align(start_bit, 2U, 8U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu8u(src_ptr, align, start_bit, 2U, dst_ptr);
        src_ptr += ((align * 2U) + start_bit) >> 3U;
        dst_ptr += align;
        num_elements -= align;
    }

    if (num_elements >= 64U) {
        __mmask64 read_mask  = OWN_MAX_16U;               // first 16 bytes (64 elements)
        __m512i   parse_mask = _mm512_set1_epi16(0x0303); // 2 times 1 then (8 - 2) times 0
        while (num_elements >= 64U) {
            __m512i srcmm3 = _mm512_maskz_loadu_epi8(read_mask, src_ptr);
            __m512i srcmm0, srcmm1, srcmm2, tmpmm;

            srcmm2 = _mm512_srli_epi16(srcmm3, 2U);
            srcmm1 = _mm512_srli_epi16(srcmm3, 4U);
            srcmm0 = _mm512_srli_epi16(srcmm3, 6U);

            // turn 2 bit_width into 8 by zeroing 3 of each 4 elements.
            // move them into their places
            // srcmm0: a e i m 0 0 0 0 0 0 0 0 0 0 0 0
            // srcmm1: b f j n 0 0 0 0 0 0 0 0 0 0 0 0
            tmpmm  = _mm512_unpacklo_epi8(srcmm0, srcmm1);      // ab ef 00 00 00 00 00 00
            srcmm0 = _mm512_unpackhi_epi8(srcmm0, srcmm1);      // ij mn 00 00 00 00 00 00
            srcmm0 = _mm512_shuffle_i64x2(tmpmm, srcmm0, 0x00); // ab ef ab ef ij mn ij mn

            // srcmm2: c g k o 0 0 0 0 0 0 0 0 0 0 0 0
            // srcmm3: d h l p 0 0 0 0 0 0 0 0 0 0 0 0
            tmpmm  = _mm512_unpacklo_epi8(srcmm2, srcmm3);      // cd gh 00 00 00 00 00 00
            srcmm1 = _mm512_unpackhi_epi8(srcmm2, srcmm3);      // kl op 00 00 00 00 00 00
            srcmm1 = _mm512_shuffle_i64x2(tmpmm, srcmm1, 0x00); // cd gh cd gh kl op kl op

            tmpmm  = _mm512_unpacklo_epi16(srcmm0, srcmm1);      // abcd abcd ijkl ijkl
            srcmm0 = _mm512_unpackhi_epi16(srcmm0, srcmm1);      // efgh efgh mnop mnop
            srcmm0 = _mm512_shuffle_i64x2(tmpmm, srcmm0, 0x88);  // abcd ijkl efgh mnop
            srcmm0 = _mm512_shuffle_i64x2(srcmm0, srcmm0, 0xD8); // abcd efgh ijkl mnop

            srcmm0 = _mm512_and_si512(srcmm0, parse_mask);

            _mm512_storeu_si512(dst_ptr, srcmm0);

            src_ptr += 8U * 2U;
            dst_ptr += 64U;
            num_elements -= 64U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu8u(src_ptr, num_elements, 0U, 2U, dst_ptr); }
}

// ********************** 3u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_3u8u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (0U < start_bit) {
        uint32_t align = own_get_align(start_bit, 3U, 8U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu8u(src_ptr, align, start_bit, 3U, dst_ptr);
        src_ptr += ((align * 3U) + start_bit) >> 3U;
        dst_ptr += align;
        num_elements -= align;
    }

    if (num_elements >= 64U) {
        __mmask64 read_mask  = OWN_BIT_MASK(OWN_BITS_2_BYTE(3U * OWN_QWORD_WIDTH));
        __m512i   parse_mask = _mm512_set1_epi8(OWN_BIT_MASK(3U));

        __m512i permutex_idx = _mm512_load_si512(permutex_idx_table_3u);

        __m512i shuffle_idx_ptr[2];
        shuffle_idx_ptr[0] = _mm512_load_si512(shuffle_idx_table_3u_0);
        shuffle_idx_ptr[1] = _mm512_load_si512(shuffle_idx_table_3u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_3u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_3u_1);

        while (num_elements >= 64U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi8(read_mask, src_ptr);
            srcmm = _mm512_permutexvar_epi16(permutex_idx, srcmm);

            // shuffling so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[0]);
            zmm[1] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[1]);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi16(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi16(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi8(zmm[0], 0xAAAAAAAAAAAAAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 8U * 3U;
            dst_ptr += 64U;
            num_elements -= 64U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu8u(src_ptr, num_elements, 0U, 3U, dst_ptr); }
}

// ********************** 4u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_4u8u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    uint32_t bit_width = 4U;

    if (0U < start_bit) {
        uint32_t align = own_get_align(start_bit, bit_width, 8U);
        px_qplc_unpack_be_Nu8u(src_ptr, align, start_bit, bit_width, dst_ptr);
        src_ptr += ((align * bit_width) + start_bit) >> 3U;
        dst_ptr += align;
        num_elements -= align;
    }

    if (num_elements >= 64U) {
        __mmask64 read_mask  = OWN_MAX_32U;               // first 32 bytes (64 elements)
        __m512i   parse_mask = _mm512_set1_epi16(0x0F0F); // 4 times 1 then (8 - 4) times 0
        while (num_elements >= 64U) {
            __m512i srcmm0, srcmm1, tmpmm;

            srcmm1 = _mm512_maskz_loadu_epi8(read_mask, src_ptr);
            srcmm0 = _mm512_srli_epi16(srcmm1, 4U);

            // move elements into their places
            // srcmm0: a c e g 0 0 0 0
            // srcmm1: b d f h 0 0 0 0
            tmpmm  = _mm512_unpacklo_epi8(srcmm0, srcmm1);       // ab ef 00 00
            srcmm0 = _mm512_unpackhi_epi8(srcmm0, srcmm1);       // cd gh 00 00
            srcmm0 = _mm512_shuffle_i64x2(tmpmm, srcmm0, 0x44);  // ab ef cd gh
            srcmm0 = _mm512_shuffle_i64x2(srcmm0, srcmm0, 0xD8); // ab cd ef gh

            // turn 4 bit_width into 8 by zeroing 4 of each 8 bits.
            srcmm0 = _mm512_and_si512(srcmm0, parse_mask);

            _mm512_storeu_si512(dst_ptr, srcmm0);

            src_ptr += 8U * 4U;
            dst_ptr += 64U;
            num_elements -= 64U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu8u(src_ptr, num_elements, 0U, bit_width, dst_ptr); }
}

// ********************** 5u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_5u8u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (0U < start_bit) {
        uint32_t align = own_get_align(start_bit, 5U, 8U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu8u(src_ptr, align, start_bit, 5U, dst_ptr);
        src_ptr += ((align * 5U) + start_bit) >> 3U;
        dst_ptr += align;
        num_elements -= align;
    }

    if (num_elements >= 64U) {
        __mmask64 read_mask  = OWN_BIT_MASK(OWN_BITS_2_BYTE(5U * OWN_QWORD_WIDTH));
        __m512i   parse_mask = _mm512_set1_epi8(OWN_BIT_MASK(5U));

        __m512i permutex_idx = _mm512_load_si512(permutex_idx_table_5u);

        __m512i shuffle_idx_ptr[2];
        shuffle_idx_ptr[0] = _mm512_load_si512(shuffle_idx_table_5u_0);
        shuffle_idx_ptr[1] = _mm512_load_si512(shuffle_idx_table_5u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_5u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_5u_1);

        while (num_elements >= 64U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi8(read_mask, src_ptr);
            srcmm = _mm512_permutexvar_epi16(permutex_idx, srcmm);

            // shuffling so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[0]);
            zmm[1] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[1]);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi16(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi16(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi8(zmm[0], 0xAAAAAAAAAAAAAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 8U * 5U;
            dst_ptr += 64U;
            num_elements -= 64U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu8u(src_ptr, num_elements, 0U, 5U, dst_ptr); }
}

// ********************** 6u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_6u8u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (0U < start_bit) {
        uint32_t align = own_get_align(start_bit, 6U, 8U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu8u(src_ptr, align, start_bit, 6U, dst_ptr);
        src_ptr += ((align * 6U) + start_bit) >> 3U;
        dst_ptr += align;
        num_elements -= align;
    }

    if (num_elements >= 64U) {
        __mmask64 read_mask  = OWN_BIT_MASK(OWN_BITS_2_BYTE(6U * OWN_QWORD_WIDTH));
        __m512i   parse_mask = _mm512_set1_epi8(OWN_BIT_MASK(6U));

        __m512i permutex_idx = _mm512_load_si512(permutex_idx_table_6u);

        __m512i shuffle_idx_ptr[2];
        shuffle_idx_ptr[0] = _mm512_load_si512(shuffle_idx_table_6u_0);
        shuffle_idx_ptr[1] = _mm512_load_si512(shuffle_idx_table_6u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_6u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_6u_1);

        while (num_elements >= 64U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi8(read_mask, src_ptr);
            srcmm = _mm512_permutexvar_epi32(permutex_idx, srcmm);

            // shuffling so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[0]);
            zmm[1] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[1]);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi16(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi16(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi8(zmm[0], 0xAAAAAAAAAAAAAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 8U * 6U;
            dst_ptr += 64U;
            num_elements -= 64U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu8u(src_ptr, num_elements, 0U, 6U, dst_ptr); }
}

// ********************** 7u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_7u8u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (0U < start_bit) {
        uint32_t align = own_get_align(start_bit, 7U, 8U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu8u(src_ptr, align, start_bit, 7U, dst_ptr);
        src_ptr += ((align * 7U) + start_bit) >> 3U;
        dst_ptr += align;
        num_elements -= align;
    }

    if (num_elements >= 64U) {
        __mmask64 read_mask  = OWN_BIT_MASK(OWN_BITS_2_BYTE(7U * OWN_QWORD_WIDTH));
        __m512i   parse_mask = _mm512_set1_epi8(OWN_BIT_MASK(7U));

        __m512i permutex_idx = _mm512_load_si512(permutex_idx_table_7u);

        __m512i shuffle_idx_ptr[2];
        shuffle_idx_ptr[0] = _mm512_load_si512(shuffle_idx_table_7u_0);
        shuffle_idx_ptr[1] = _mm512_load_si512(shuffle_idx_table_7u_1);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_7u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_7u_1);

        while (num_elements >= 64U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi8(read_mask, src_ptr);
            srcmm = _mm512_permutexvar_epi16(permutex_idx, srcmm);

            // shuffling so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[0]);
            zmm[1] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[1]);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi16(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi16(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi8(zmm[0], 0xAAAAAAAAAAAAAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 8U * 7U;
            dst_ptr += 64U;
            num_elements -= 64U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu8u(src_ptr, num_elements, 0U, 7U, dst_ptr); }
}

#endif
