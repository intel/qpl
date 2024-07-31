/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*------- qplc_pack_16u_k0.h -------*/

/**
 * @brief Contains implementation of functions for vector packing byte integers to 9...16-bit integers
 * @date 18/06/2021
 *
 * @details Function list:
 *          - @ref k0_qplc_pack_16u9u
 *          - @ref k0_qplc_pack_16u10u
 *          - @ref k0_qplc_pack_16u11u
 *          - @ref k0_qplc_pack_16u12u
 *          - @ref k0_qplc_pack_16u13u
 *          - @ref k0_qplc_pack_16u14u
 *          - @ref k0_qplc_pack_16u15u
 *          - @ref k0_qplc_pack_16u32u
 *
 */

#ifndef OWN_PACK_BE_16U_H
#define OWN_PACK_BE_16U_H

#include "own_qplc_defs.h"

OWN_QPLC_INLINE(void, qplc_pack_be_16u_nu,
                (const uint8_t* src_ptr, uint32_t num_elements, uint32_t bit_width, uint8_t* dst_ptr,
                 uint32_t start_bit));

// *********************** Masks  ****************************** //

static uint8_t pshufb_idx_ptr[64] = {
        1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14, 1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14,
        1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14, 1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14};

static uint16_t permute_idx_16u[32] = {0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0};

// ----------------------- 16u9u ------------------------------- //
static uint16_t  permutex_idx_table_9u_0[32] = {0U,  1U,  3U,  5U,  7U,  8U,  10U, 12U, 14U, 16U, 17U,
                                                19U, 21U, 23U, 24U, 26U, 28U, 30U, 32U, 33U, 35U, 37U,
                                                39U, 40U, 42U, 44U, 46U, 0x0, 0x0, 0x0, 0x0, 0x0};
static uint16_t  permutex_idx_table_9u_1[32] = {0x0, 2U,  4U,  6U,  0x0, 9U,  11U, 13U, 0x0, 0x0, 18U,
                                                20U, 22U, 0x0, 25U, 27U, 29U, 0x0, 0x0, 34U, 36U, 38U,
                                                0x0, 41U, 43U, 45U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static uint16_t  permutex_idx_table_9u_2[32] = {1U,  3U,  5U,  7U,  8U,  10U, 12U, 14U, 15U, 17U, 19U,
                                                21U, 23U, 24U, 26U, 28U, 30U, 31U, 33U, 35U, 37U, 39U,
                                                40U, 42U, 44U, 46U, 47U, 0x0, 0x0, 0x0, 0x0, 0x0};
static uint16_t  permutex_idx_table_9u_3[32] = {16U, 17U, 19U, 21U, 23U, 24U, 26U, 28U, 30U, 32U, 33U,
                                                35U, 37U, 39U, 40U, 42U, 44U, 46U, 48U, 49U, 51U, 53U,
                                                55U, 56U, 58U, 60U, 62U, 0x0, 0x0, 0x0, 0x0, 0x0};
static uint16_t  permutex_idx_table_9u_4[32] = {0x0, 18U, 20U, 22U, 0x0, 25U, 27U, 29U, 0x0, 0x0, 34U,
                                                36U, 38U, 0x0, 41U, 43U, 45U, 0x0, 0x0, 50U, 52U, 54U,
                                                0x0, 57U, 59U, 61U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static uint16_t  permutex_idx_table_9u_5[32] = {17U, 19U, 21U, 23U, 24U, 26U, 28U, 30U, 31U, 33U, 35U,
                                                37U, 39U, 40U, 42U, 44U, 46U, 47U, 49U, 51U, 53U, 55U,
                                                56U, 58U, 60U, 62U, 63U, 0x0, 0x0, 0x0, 0x0, 0x0};
static __mmask32 permutex_masks_9u_ptr[3]    = {0x07FFFFFF, 0x03B9DCEE, 0x07FFFFFF};

static uint16_t shift_mask_table_9u_0[32] = {7U,  14U, 12U, 10U, 8U,  15U, 13U, 11U, 9U,  7U,  14U,
                                             12U, 10U, 8U,  15U, 13U, 11U, 9U,  7U,  14U, 12U, 10U,
                                             8U,  15U, 13U, 11U, 9U,  0x0, 0x0, 0x0, 0x0, 0x0};
static uint16_t shift_mask_table_9u_1[32] = {0x0, 5U, 3U,  1U, 0x0, 6U,  4U,  2U,  0x0, 0x0, 5U,
                                             3U,  1U, 0x0, 6U, 4U,  2U,  0x0, 0x0, 5U,  3U,  1U,
                                             0x0, 6U, 4U,  2U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static uint16_t shift_mask_table_9u_2[32] = {2U, 4U, 6U, 8U, 1U, 3U, 5U, 7U, 0U, 2U, 4U, 6U,  8U,  1U,  3U,  5U,
                                             7U, 0U, 2U, 4U, 6U, 8U, 1U, 3U, 5U, 7U, 0U, 0x0, 0x0, 0x0, 0x0, 0x0};

static uint32_t table_align_16u9u[16] = {0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9};

OWN_OPT_FUN(void, k0_qplc_pack_be_16u9u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t align = table_align_16u9u[start_bit & 15];

    if (align > num_elements) {
        align = num_elements;
        qplc_pack_be_16u_nu(src_ptr, align, 9U, dst_ptr, start_bit);
        return;
    }

    {
        uint32_t tail            = 0U;
        uint32_t num_elements_96 = 0U;
        uint32_t num_elements_32 = 0U;
        __m512i  srcmm0, srcmm1, srcmm2;
        __m512i  zmm0, zmm1, zmm2, zmm3, zmm4, zmm5;
        __m512i  permutex_idx_ptr[6];
        __m512i  shift_masks_ptr[3];
        __m512i  pshufb_idx = _mm512_loadu_si512(pshufb_idx_ptr);
        permutex_idx_ptr[0] = _mm512_loadu_si512(permutex_idx_table_9u_0);
        permutex_idx_ptr[1] = _mm512_loadu_si512(permutex_idx_table_9u_1);
        permutex_idx_ptr[2] = _mm512_loadu_si512(permutex_idx_table_9u_2);
        permutex_idx_ptr[3] = _mm512_loadu_si512(permutex_idx_table_9u_3);
        permutex_idx_ptr[4] = _mm512_loadu_si512(permutex_idx_table_9u_4);
        permutex_idx_ptr[5] = _mm512_loadu_si512(permutex_idx_table_9u_5);
        shift_masks_ptr[0]  = _mm512_loadu_si512(shift_mask_table_9u_0);
        shift_masks_ptr[1]  = _mm512_loadu_si512(shift_mask_table_9u_1);
        shift_masks_ptr[2]  = _mm512_loadu_si512(shift_mask_table_9u_2);

        if (align) {
            __m512i   permute_idx   = _mm512_loadu_si512(permute_idx_16u);
            __mmask32 mask32_load   = (1 << align) - 1;
            uint64_t  num_bytes_out = ((uint64_t)align * 9 + start_bit) / OWN_BYTE_WIDTH;
            __mmask64 mask64_store  = ((uint64_t)1 << num_bytes_out) - (uint64_t)1;
            uint32_t  src =
                    ((uint32_t)qplc_swap_bytes_16u(*(uint16_t*)dst_ptr)) & (0xffff << (OWN_WORD_WIDTH - start_bit));

            srcmm0 = _mm512_mask_loadu_epi16(_mm512_setzero_si512(), mask32_load, (void const*)src_ptr);

            zmm0 = _mm512_maskz_permutexvar_epi16(permutex_masks_9u_ptr[0], permutex_idx_ptr[0], srcmm0);
            zmm1 = _mm512_maskz_permutexvar_epi16(permutex_masks_9u_ptr[1], permutex_idx_ptr[1], srcmm0);
            zmm2 = _mm512_maskz_permutexvar_epi16(permutex_masks_9u_ptr[2], permutex_idx_ptr[2], srcmm0);

            zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi16(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi16(zmm2, shift_masks_ptr[2]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);

            zmm1 = _mm512_maskz_permutexvar_epi16(0x0000FFFE, permute_idx, zmm0);
            zmm0 = _mm512_srl_epi16(zmm0, _mm_cvtsi32_si128(start_bit));
            zmm1 = _mm512_sll_epi16(zmm1, _mm_cvtsi32_si128(OWN_WORD_WIDTH - start_bit));
            zmm1 = _mm512_castsi256_si512(_mm256_insert_epi16(_mm512_castsi512_si256(zmm1), (short)src, 0));
            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
            _mm512_mask_storeu_epi8(dst_ptr, mask64_store, zmm0);

            src_ptr += align * 2;
            dst_ptr += ((align * 9U) + start_bit) >> 3U;
            num_elements -= align;
        }

        {
            tail            = num_elements % 32U;
            num_elements_96 = num_elements / 96U;
            num_elements_32 = (num_elements % 96U) / 32U;

            for (uint32_t idx = 0; idx < num_elements_96; ++idx) {
                srcmm0 = _mm512_loadu_si512(src_ptr);
                srcmm1 = _mm512_loadu_si512((src_ptr + 64U));
                srcmm2 = _mm512_loadu_si512((src_ptr + 128U));

                zmm0 = _mm512_maskz_permutex2var_epi16(permutex_masks_9u_ptr[0], srcmm0, permutex_idx_ptr[0], srcmm1);
                zmm1 = _mm512_maskz_permutex2var_epi16(permutex_masks_9u_ptr[1], srcmm0, permutex_idx_ptr[1], srcmm1);
                zmm2 = _mm512_maskz_permutex2var_epi16(permutex_masks_9u_ptr[2], srcmm0, permutex_idx_ptr[2], srcmm1);
                zmm3 = _mm512_maskz_permutex2var_epi16(permutex_masks_9u_ptr[0], srcmm1, permutex_idx_ptr[3], srcmm2);
                zmm4 = _mm512_maskz_permutex2var_epi16(permutex_masks_9u_ptr[1], srcmm1, permutex_idx_ptr[4], srcmm2);
                zmm5 = _mm512_maskz_permutex2var_epi16(permutex_masks_9u_ptr[2], srcmm1, permutex_idx_ptr[5], srcmm2);

                zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
                zmm1 = _mm512_sllv_epi16(zmm1, shift_masks_ptr[1]);
                zmm2 = _mm512_srlv_epi16(zmm2, shift_masks_ptr[2]);
                zmm3 = _mm512_sllv_epi16(zmm3, shift_masks_ptr[0]);
                zmm4 = _mm512_sllv_epi16(zmm4, shift_masks_ptr[1]);
                zmm5 = _mm512_srlv_epi16(zmm5, shift_masks_ptr[2]);

                zmm0 = _mm512_or_si512(zmm0, zmm1);
                zmm0 = _mm512_or_si512(zmm0, zmm2);
                zmm3 = _mm512_or_si512(zmm3, zmm4);
                zmm3 = _mm512_or_si512(zmm3, zmm5);

                zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
                zmm3 = _mm512_shuffle_epi8(zmm3, pshufb_idx);

                _mm512_mask_storeu_epi16(dst_ptr, 0x07FFFFFF, zmm0);
                _mm512_mask_storeu_epi16((dst_ptr + 54U), 0x07FFFFFF, zmm3);

                src_ptr += 96U * sizeof(uint16_t);
                dst_ptr += 54U * sizeof(uint16_t);
            }

            for (uint32_t idx = 0; idx < num_elements_32; ++idx) {
                srcmm0 = _mm512_loadu_si512(src_ptr);
                zmm0   = _mm512_maskz_permutexvar_epi16(permutex_masks_9u_ptr[0], permutex_idx_ptr[0], srcmm0);
                zmm1   = _mm512_maskz_permutexvar_epi16(permutex_masks_9u_ptr[1], permutex_idx_ptr[1], srcmm0);
                zmm2   = _mm512_maskz_permutexvar_epi16(permutex_masks_9u_ptr[2], permutex_idx_ptr[2], srcmm0);

                zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
                zmm1 = _mm512_sllv_epi16(zmm1, shift_masks_ptr[1]);
                zmm2 = _mm512_srlv_epi16(zmm2, shift_masks_ptr[2]);

                zmm0 = _mm512_or_si512(zmm0, zmm1);
                zmm0 = _mm512_or_si512(zmm0, zmm2);
                zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
                _mm512_mask_storeu_epi16(dst_ptr, 0x0003FFFF, zmm0);

                src_ptr += 32U * sizeof(uint16_t);
                dst_ptr += 18U * sizeof(uint16_t);
            }
            if (tail > 0) {
                uint64_t  num_bytes_out = ((uint64_t)tail * 9 + 7) / OWN_BYTE_WIDTH;
                __mmask32 mask32_load   = (1 << tail) - 1;
                __mmask64 mask64_store  = ((uint64_t)1 << num_bytes_out) - (uint64_t)1;

                srcmm0 = _mm512_mask_loadu_epi16(_mm512_setzero_si512(), mask32_load, (void const*)src_ptr);
                zmm0   = _mm512_maskz_permutexvar_epi16(permutex_masks_9u_ptr[0], permutex_idx_ptr[0], srcmm0);
                zmm1   = _mm512_maskz_permutexvar_epi16(permutex_masks_9u_ptr[1], permutex_idx_ptr[1], srcmm0);
                zmm2   = _mm512_maskz_permutexvar_epi16(permutex_masks_9u_ptr[2], permutex_idx_ptr[2], srcmm0);
                zmm0   = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
                zmm1   = _mm512_sllv_epi16(zmm1, shift_masks_ptr[1]);
                zmm2   = _mm512_srlv_epi16(zmm2, shift_masks_ptr[2]);
                zmm0   = _mm512_or_si512(zmm0, zmm1);
                zmm0   = _mm512_or_si512(zmm0, zmm2);
                zmm0   = _mm512_shuffle_epi8(zmm0, pshufb_idx);
                _mm512_mask_storeu_epi8(dst_ptr, mask64_store, zmm0);
            }
        }
    }
}

// ----------------------- 16u10u ------------------------------- //
static uint16_t  permutex_idx_table_10u_0[32] = {0U,  1U,  3U,  4U,  6U,  8U,  9U,  11U, 12U, 14U, 16U,
                                                 17U, 19U, 20U, 22U, 24U, 25U, 27U, 28U, 30U, 32U, 33U,
                                                 35U, 36U, 38U, 40U, 41U, 43U, 44U, 46U, 0x0, 0x0};
static uint16_t  permutex_idx_table_10u_1[32] = {0x0, 2U,  0x0, 5U,  0x0, 0x0, 10U, 0x0, 13U, 0x0, 0x0,
                                                 18U, 0x0, 21U, 0x0, 0x0, 26U, 0x0, 29U, 0x0, 0x0, 34U,
                                                 0x0, 37U, 0x0, 0x0, 42U, 0x0, 45U, 0x0, 0x0, 0x0};
static uint16_t  permutex_idx_table_10u_2[32] = {1U,  3U,  4U,  6U,  7U,  9U,  11U, 12U, 14U, 15U, 17U,
                                                 19U, 20U, 22U, 23U, 25U, 27U, 28U, 30U, 31U, 33U, 35U,
                                                 36U, 38U, 39U, 41U, 43U, 44U, 46U, 47U, 0x0, 0x0};
static uint16_t  permutex_idx_table_10u_3[32] = {16U, 17U, 19U, 20U, 22U, 24U, 25U, 27U, 28U, 30U, 32U,
                                                 33U, 35U, 36U, 38U, 40U, 41U, 43U, 44U, 46U, 48U, 49U,
                                                 51U, 52U, 54U, 56U, 57U, 59U, 60U, 62U, 0x0, 0x0};
static uint16_t  permutex_idx_table_10u_4[32] = {0x0, 18U, 0x0, 21U, 0x0, 0x0, 26U, 0x0, 29U, 0x0, 0x0,
                                                 34U, 0x0, 37U, 0x0, 0x0, 42U, 0x0, 45U, 0x0, 0x0, 50U,
                                                 0x0, 53U, 0x0, 0x0, 58U, 0x0, 61U, 0x0, 0x0, 0x0};
static uint16_t  permutex_idx_table_10u_5[32] = {17U, 19U, 20U, 22U, 23U, 25U, 27U, 28U, 30U, 31U, 33U,
                                                 35U, 36U, 38U, 39U, 41U, 43U, 44U, 46U, 47U, 49U, 51U,
                                                 52U, 54U, 55U, 57U, 59U, 60U, 62U, 63U, 0x0, 0x0};
static __mmask32 permutex_masks_10u_ptr[3]    = {0x3FFFFFFF, 0x14A5294A, 0x3FFFFFFF};

static uint16_t shift_mask_table_10u_0[32] = {6U,  12U, 8U,  14U, 10U, 6U,  12U, 8U,  14U, 10U, 6U,
                                              12U, 8U,  14U, 10U, 6U,  12U, 8U,  14U, 10U, 6U,  12U,
                                              8U,  14U, 10U, 6U,  12U, 8U,  14U, 10U, 0x0, 0x0};
static uint16_t shift_mask_table_10u_1[32] = {0x0, 2U,  0x0, 4U,  0x0, 0x0, 2U,  0x0, 4U,  0x0, 0x0,
                                              2U,  0x0, 4U,  0x0, 0x0, 2U,  0x0, 4U,  0x0, 0x0, 2U,
                                              0x0, 4U,  0x0, 0x0, 2U,  0x0, 4U,  0x0, 0x0, 0x0};
static uint16_t shift_mask_table_10u_2[32] = {4U, 8U, 2U, 6U, 0U, 4U, 8U, 2U, 6U, 0U, 4U, 8U, 2U, 6U, 0U,  4U,
                                              8U, 2U, 6U, 0U, 4U, 8U, 2U, 6U, 0U, 4U, 8U, 2U, 6U, 0U, 0x0, 0x0};

static uint32_t table_align_16u10u[16] = {0, 0xffffffff, 3, 0xffffffff, 6, 0xffffffff, 1, 0xffffffff,
                                          4, 0xffffffff, 7, 0xffffffff, 2, 0xffffffff, 5, 0xffffffff};

OWN_OPT_FUN(void, k0_qplc_pack_be_16u10u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t align = table_align_16u10u[start_bit & 15];

    if (align > num_elements) {
        align = num_elements;
        qplc_pack_be_16u_nu(src_ptr, align, 10U, dst_ptr, start_bit);
        return;
    }

    {
        uint32_t tail            = 0U;
        uint32_t num_elements_96 = 0U;
        uint32_t num_elements_32 = 0U;
        __m512i  srcmm0, srcmm1, srcmm2;
        __m512i  zmm0, zmm1, zmm2, zmm3, zmm4, zmm5;
        __m512i  permutex_idx_ptr[6];
        __m512i  shift_masks_ptr[3];
        __m512i  pshufb_idx = _mm512_loadu_si512(pshufb_idx_ptr);
        permutex_idx_ptr[0] = _mm512_loadu_si512(permutex_idx_table_10u_0);
        permutex_idx_ptr[1] = _mm512_loadu_si512(permutex_idx_table_10u_1);
        permutex_idx_ptr[2] = _mm512_loadu_si512(permutex_idx_table_10u_2);
        permutex_idx_ptr[3] = _mm512_loadu_si512(permutex_idx_table_10u_3);
        permutex_idx_ptr[4] = _mm512_loadu_si512(permutex_idx_table_10u_4);
        permutex_idx_ptr[5] = _mm512_loadu_si512(permutex_idx_table_10u_5);
        shift_masks_ptr[0]  = _mm512_loadu_si512(shift_mask_table_10u_0);
        shift_masks_ptr[1]  = _mm512_loadu_si512(shift_mask_table_10u_1);
        shift_masks_ptr[2]  = _mm512_loadu_si512(shift_mask_table_10u_2);

        if (align) {
            __m512i   permute_idx   = _mm512_loadu_si512(permute_idx_16u);
            __mmask32 mask32_load   = (1 << align) - 1;
            uint64_t  num_bytes_out = ((uint64_t)align * 10 + start_bit) / OWN_BYTE_WIDTH;
            __mmask64 mask64_store  = ((uint64_t)1 << num_bytes_out) - (uint64_t)1;
            uint32_t  src =
                    ((uint32_t)qplc_swap_bytes_16u(*(uint16_t*)dst_ptr)) & (0xffff << (OWN_WORD_WIDTH - start_bit));

            srcmm0 = _mm512_mask_loadu_epi16(_mm512_setzero_si512(), mask32_load, (void const*)src_ptr);

            zmm0 = _mm512_maskz_permutexvar_epi16(permutex_masks_10u_ptr[0], permutex_idx_ptr[0], srcmm0);
            zmm1 = _mm512_maskz_permutexvar_epi16(permutex_masks_10u_ptr[1], permutex_idx_ptr[1], srcmm0);
            zmm2 = _mm512_maskz_permutexvar_epi16(permutex_masks_10u_ptr[2], permutex_idx_ptr[2], srcmm0);

            zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi16(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi16(zmm2, shift_masks_ptr[2]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);

            zmm1 = _mm512_maskz_permutexvar_epi16(0x0000FFFE, permute_idx, zmm0);
            zmm0 = _mm512_srl_epi16(zmm0, _mm_cvtsi32_si128(start_bit));
            zmm1 = _mm512_sll_epi16(zmm1, _mm_cvtsi32_si128(OWN_WORD_WIDTH - start_bit));
            zmm1 = _mm512_castsi256_si512(_mm256_insert_epi16(_mm512_castsi512_si256(zmm1), (short)src, 0));
            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
            _mm512_mask_storeu_epi8(dst_ptr, mask64_store, zmm0);

            src_ptr += align * 2;
            dst_ptr += ((align * 10U) + start_bit) >> 3U;
            num_elements -= align;
        }

        {
            tail            = num_elements % 32U;
            num_elements_96 = num_elements / 96U;
            num_elements_32 = (num_elements % 96U) / 32U;

            for (uint32_t idx = 0; idx < num_elements_96; ++idx) {
                srcmm0 = _mm512_loadu_si512(src_ptr);
                srcmm1 = _mm512_loadu_si512((src_ptr + 64U));
                srcmm2 = _mm512_loadu_si512((src_ptr + 128U));

                zmm0 = _mm512_maskz_permutex2var_epi16(permutex_masks_10u_ptr[0], srcmm0, permutex_idx_ptr[0], srcmm1);
                zmm1 = _mm512_maskz_permutex2var_epi16(permutex_masks_10u_ptr[1], srcmm0, permutex_idx_ptr[1], srcmm1);
                zmm2 = _mm512_maskz_permutex2var_epi16(permutex_masks_10u_ptr[2], srcmm0, permutex_idx_ptr[2], srcmm1);
                zmm3 = _mm512_maskz_permutex2var_epi16(permutex_masks_10u_ptr[0], srcmm1, permutex_idx_ptr[3], srcmm2);
                zmm4 = _mm512_maskz_permutex2var_epi16(permutex_masks_10u_ptr[1], srcmm1, permutex_idx_ptr[4], srcmm2);
                zmm5 = _mm512_maskz_permutex2var_epi16(permutex_masks_10u_ptr[2], srcmm1, permutex_idx_ptr[5], srcmm2);

                zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
                zmm1 = _mm512_sllv_epi16(zmm1, shift_masks_ptr[1]);
                zmm2 = _mm512_srlv_epi16(zmm2, shift_masks_ptr[2]);
                zmm3 = _mm512_sllv_epi16(zmm3, shift_masks_ptr[0]);
                zmm4 = _mm512_sllv_epi16(zmm4, shift_masks_ptr[1]);
                zmm5 = _mm512_srlv_epi16(zmm5, shift_masks_ptr[2]);

                zmm0 = _mm512_or_si512(zmm0, zmm1);
                zmm0 = _mm512_or_si512(zmm0, zmm2);
                zmm3 = _mm512_or_si512(zmm3, zmm4);
                zmm3 = _mm512_or_si512(zmm3, zmm5);

                zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
                zmm3 = _mm512_shuffle_epi8(zmm3, pshufb_idx);

                _mm512_mask_storeu_epi16(dst_ptr, 0x3FFFFFFF, zmm0);
                _mm512_mask_storeu_epi16((dst_ptr + 60U), 0x3FFFFFFF, zmm3);

                src_ptr += 96U * 2U;
                dst_ptr += 60U * 2U;
            }

            for (uint32_t idx = 0; idx < num_elements_32; ++idx) {
                srcmm0 = _mm512_loadu_si512(src_ptr);
                zmm0   = _mm512_maskz_permutexvar_epi16(permutex_masks_10u_ptr[0], permutex_idx_ptr[0], srcmm0);
                zmm1   = _mm512_maskz_permutexvar_epi16(permutex_masks_10u_ptr[1], permutex_idx_ptr[1], srcmm0);
                zmm2   = _mm512_maskz_permutexvar_epi16(permutex_masks_10u_ptr[2], permutex_idx_ptr[2], srcmm0);

                zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
                zmm1 = _mm512_sllv_epi16(zmm1, shift_masks_ptr[1]);
                zmm2 = _mm512_srlv_epi16(zmm2, shift_masks_ptr[2]);

                zmm0 = _mm512_or_si512(zmm0, zmm1);
                zmm0 = _mm512_or_si512(zmm0, zmm2);
                zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
                _mm512_mask_storeu_epi16(dst_ptr, 0x000FFFFF, zmm0);

                src_ptr += 32U * 2U;
                dst_ptr += 20U * 2U;
            }
            if (tail > 0) {
                uint64_t  num_bytes_out = ((uint64_t)tail * 10 + 7) / OWN_BYTE_WIDTH;
                __mmask32 mask32_load   = (1 << tail) - 1;
                __mmask64 mask64_store  = ((uint64_t)1 << num_bytes_out) - (uint64_t)1;

                srcmm0 = _mm512_mask_loadu_epi16(_mm512_setzero_si512(), mask32_load, (void const*)src_ptr);
                zmm0   = _mm512_maskz_permutexvar_epi16(permutex_masks_10u_ptr[0], permutex_idx_ptr[0], srcmm0);
                zmm1   = _mm512_maskz_permutexvar_epi16(permutex_masks_10u_ptr[1], permutex_idx_ptr[1], srcmm0);
                zmm2   = _mm512_maskz_permutexvar_epi16(permutex_masks_10u_ptr[2], permutex_idx_ptr[2], srcmm0);
                zmm0   = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
                zmm1   = _mm512_sllv_epi16(zmm1, shift_masks_ptr[1]);
                zmm2   = _mm512_srlv_epi16(zmm2, shift_masks_ptr[2]);
                zmm0   = _mm512_or_si512(zmm0, zmm1);
                zmm0   = _mm512_or_si512(zmm0, zmm2);
                zmm0   = _mm512_shuffle_epi8(zmm0, pshufb_idx);
                _mm512_mask_storeu_epi8(dst_ptr, mask64_store, zmm0);
            }
        }
    }
}

// ----------------------- 16u11u ------------------------------- //
static uint16_t permutex_idx_table_11u_0[32] = {0U,  1U,  2U,  4U,  5U,  7U,  8U,  10U, 11U, 13U, 14U,
                                                16U, 17U, 18U, 20U, 21U, 23U, 24U, 26U, 27U, 29U, 30U,
                                                32U, 33U, 34U, 36U, 37U, 39U, 40U, 42U, 43U, 45U};
static uint16_t permutex_idx_table_11u_1[32] = {0x0, 0x0, 3U,  0x0, 6U,  0x0, 9U,  0x0, 12U, 0x0, 0x0,
                                                0x0, 0x0, 19U, 0x0, 22U, 0x0, 25U, 0x0, 28U, 0x0, 0x0,
                                                0x0, 0x0, 35U, 0x0, 38U, 0x0, 41U, 0x0, 44U, 0x0};
static uint16_t permutex_idx_table_11u_2[32] = {1U,  2U,  4U,  5U,  7U,  8U,  10U, 11U, 13U, 14U, 15U,
                                                17U, 18U, 20U, 21U, 23U, 24U, 26U, 27U, 29U, 30U, 31U,
                                                33U, 34U, 36U, 37U, 39U, 40U, 42U, 43U, 45U, 46U};
static uint16_t permutex_idx_table_11u_3[32] = {14U, 16U, 17U, 18U, 20U, 21U, 23U, 24U, 26U, 27U, 29U,
                                                30U, 32U, 33U, 34U, 36U, 37U, 39U, 40U, 42U, 43U, 45U,
                                                46U, 48U, 49U, 50U, 52U, 53U, 55U, 56U, 58U, 59U};
static uint16_t permutex_idx_table_11u_4[32] = {0x0, 0x0, 0x0, 19U, 0x0, 22U, 0x0, 25U, 0x0, 28U, 0x0,
                                                0x0, 0x0, 0x0, 35U, 0x0, 38U, 0x0, 41U, 0x0, 44U, 0x0,
                                                0x0, 0x0, 0x0, 51U, 0x0, 54U, 0x0, 57U, 0x0, 60U};
static uint16_t permutex_idx_table_11u_5[32] = {15U, 17U, 18U, 20U, 21U, 23U, 24U, 26U, 27U, 29U, 30U,
                                                31U, 33U, 34U, 36U, 37U, 39U, 40U, 42U, 43U, 45U, 46U,
                                                47U, 49U, 50U, 52U, 53U, 55U, 56U, 58U, 59U, 61U};
static uint16_t permutex_idx_table_11u_6[32] = {29U, 30U, 32U, 33U, 34U, 36U, 37U, 39U, 40U, 42U, 43U,
                                                45U, 46U, 48U, 49U, 50U, 52U, 53U, 55U, 56U, 58U, 59U,
                                                61U, 62U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static uint16_t permutex_idx_table_11u_7[32] = {0x0, 0x0, 0x0, 0x0, 35U, 0x0, 38U, 0x0, 41U, 0x0, 44U,
                                                0x0, 0x0, 0x0, 0x0, 51U, 0x0, 54U, 0x0, 57U, 0x0, 60U,
                                                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static uint16_t permutex_idx_table_11u_8[32] = {30U, 31U, 33U, 34U, 36U, 37U, 39U, 40U, 42U, 43U, 45U,
                                                46U, 47U, 49U, 50U, 52U, 53U, 55U, 56U, 58U, 59U, 61U,
                                                62U, 63U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

static uint16_t shift_mask_table_11u_0[32] = {5U, 10U, 15U, 9U, 14U, 8U, 13U, 7U, 12U, 6U, 11U,
                                              5U, 10U, 15U, 9U, 14U, 8U, 13U, 7U, 12U, 6U, 11U,
                                              5U, 10U, 15U, 9U, 14U, 8U, 13U, 7U, 12U, 6U};
static uint16_t shift_mask_table_11u_1[32] = {0x0, 0x0, 4U, 0x0, 3U, 0x0, 2U, 0x0, 1U, 0x0, 0x0,
                                              0x0, 0x0, 4U, 0x0, 3U, 0x0, 2U, 0x0, 1U, 0x0, 0x0,
                                              0x0, 0x0, 4U, 0x0, 3U, 0x0, 2U, 0x0, 1U, 0x0};
static uint16_t shift_mask_table_11u_2[32] = {6U, 1U, 7U, 2U,  8U, 3U, 9U, 4U, 10U, 5U, 0U, 6U, 1U, 7U, 2U,  8U,
                                              3U, 9U, 4U, 10U, 5U, 0U, 6U, 1U, 7U,  2U, 8U, 3U, 9U, 4U, 10U, 5U};
static uint16_t shift_mask_table_11u_3[32] = {11U, 5U, 10U, 15U, 9U, 14U, 8U, 13U, 7U, 12U, 6U,
                                              11U, 5U, 10U, 15U, 9U, 14U, 8U, 13U, 7U, 12U, 6U,
                                              11U, 5U, 10U, 15U, 9U, 14U, 8U, 13U, 7U, 12U};
static uint16_t shift_mask_table_11u_4[32] = {0x0, 0x0, 0x0, 4U, 0x0, 3U, 0x0, 2U, 0x0, 1U, 0x0,
                                              0x0, 0x0, 0x0, 4U, 0x0, 3U, 0x0, 2U, 0x0, 1U, 0x0,
                                              0x0, 0x0, 0x0, 4U, 0x0, 3U, 0x0, 2U, 0x0, 1U};
static uint16_t shift_mask_table_11u_5[32] = {0U, 6U, 1U, 7U, 2U,  8U, 3U, 9U, 4U, 10U, 5U, 0U, 6U, 1U, 7U, 2U,
                                              8U, 3U, 9U, 4U, 10U, 5U, 0U, 6U, 1U, 7U,  2U, 8U, 3U, 9U, 4U, 10U};
static uint16_t shift_mask_table_11u_6[32] = {6U, 11U, 5U,  10U, 15U, 9U,  14U, 8U,  13U, 7U, 12U,
                                              6U, 11U, 5U,  10U, 15U, 9U,  14U, 8U,  13U, 7U, 12U,
                                              6U, 11U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static uint16_t shift_mask_table_11u_7[32] = {0x0, 0x0, 0x0, 0x0, 4U,  0x0, 3U,  0x0, 2U,  0x0, 1U,
                                              0x0, 0x0, 0x0, 0x0, 4U,  0x0, 3U,  0x0, 2U,  0x0, 1U,
                                              0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static uint16_t shift_mask_table_11u_8[32] = {5U, 0U, 6U, 1U, 7U, 2U,  8U, 3U, 9U,  4U,  10U, 5U,  0U,  6U,  1U,  7U,
                                              2U, 8U, 3U, 9U, 4U, 10U, 5U, 0U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

static uint32_t table_align_16u11u[16] = {0, 13, 10, 7, 4, 1, 14, 11, 8, 5, 2, 15, 12, 9, 6, 3};

OWN_OPT_FUN(void, k0_qplc_pack_be_16u11u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t align = table_align_16u11u[start_bit & 15];

    if (align > num_elements) {
        align = num_elements;
        qplc_pack_be_16u_nu(src_ptr, align, 11U, dst_ptr, start_bit);
        return;
    }

    {
        uint32_t tail             = 0U;
        uint32_t num_elements_128 = 0U;
        uint32_t num_elements_32  = 0U;
        __m512i  srcmm0, srcmm1, srcmm2, srcmm3;
        __m512i  zmm00, zmm01, zmm02, zmm10, zmm11, zmm12, zmm20, zmm21, zmm22;
        __m512i  permutex_idx_ptr[9];
        __m512i  shift_masks_ptr[9];
        __m512i  pshufb_idx = _mm512_loadu_si512(pshufb_idx_ptr);
        permutex_idx_ptr[0] = _mm512_loadu_si512(permutex_idx_table_11u_0);
        permutex_idx_ptr[1] = _mm512_loadu_si512(permutex_idx_table_11u_1);
        permutex_idx_ptr[2] = _mm512_loadu_si512(permutex_idx_table_11u_2);
        permutex_idx_ptr[3] = _mm512_loadu_si512(permutex_idx_table_11u_3);
        permutex_idx_ptr[4] = _mm512_loadu_si512(permutex_idx_table_11u_4);
        permutex_idx_ptr[5] = _mm512_loadu_si512(permutex_idx_table_11u_5);
        permutex_idx_ptr[6] = _mm512_loadu_si512(permutex_idx_table_11u_6);
        permutex_idx_ptr[7] = _mm512_loadu_si512(permutex_idx_table_11u_7);
        permutex_idx_ptr[8] = _mm512_loadu_si512(permutex_idx_table_11u_8);
        shift_masks_ptr[0]  = _mm512_loadu_si512(shift_mask_table_11u_0);
        shift_masks_ptr[1]  = _mm512_loadu_si512(shift_mask_table_11u_1);
        shift_masks_ptr[2]  = _mm512_loadu_si512(shift_mask_table_11u_2);
        shift_masks_ptr[3]  = _mm512_loadu_si512(shift_mask_table_11u_3);
        shift_masks_ptr[4]  = _mm512_loadu_si512(shift_mask_table_11u_4);
        shift_masks_ptr[5]  = _mm512_loadu_si512(shift_mask_table_11u_5);
        shift_masks_ptr[6]  = _mm512_loadu_si512(shift_mask_table_11u_6);
        shift_masks_ptr[7]  = _mm512_loadu_si512(shift_mask_table_11u_7);
        shift_masks_ptr[8]  = _mm512_loadu_si512(shift_mask_table_11u_8);

        if (align) {
            __m512i   permute_idx   = _mm512_loadu_si512(permute_idx_16u);
            __mmask32 mask32_load   = (1 << align) - 1;
            uint64_t  num_bytes_out = ((uint64_t)align * 11 + start_bit) / OWN_BYTE_WIDTH;
            __mmask64 mask64_store  = ((uint64_t)1 << num_bytes_out) - (uint64_t)1;
            uint32_t  src =
                    ((uint32_t)qplc_swap_bytes_16u(*(uint16_t*)dst_ptr)) & (0xffff << (OWN_WORD_WIDTH - start_bit));

            srcmm0 = _mm512_mask_loadu_epi16(_mm512_setzero_si512(), mask32_load, (void const*)src_ptr);

            zmm00 = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm0);
            zmm01 = _mm512_maskz_permutexvar_epi16(0x550aa154, permutex_idx_ptr[1], srcmm0);
            zmm02 = _mm512_permutexvar_epi16(permutex_idx_ptr[2], srcmm0);

            zmm00 = _mm512_sllv_epi16(zmm00, shift_masks_ptr[0]);
            zmm01 = _mm512_sllv_epi16(zmm01, shift_masks_ptr[1]);
            zmm02 = _mm512_srlv_epi16(zmm02, shift_masks_ptr[2]);

            zmm00 = _mm512_or_si512(zmm00, zmm01);
            zmm00 = _mm512_or_si512(zmm00, zmm02);

            zmm01 = _mm512_maskz_permutexvar_epi16(0x0000FFFE, permute_idx, zmm00);
            zmm00 = _mm512_srl_epi16(zmm00, _mm_cvtsi32_si128(start_bit));
            zmm01 = _mm512_sll_epi16(zmm01, _mm_cvtsi32_si128(OWN_WORD_WIDTH - start_bit));
            zmm01 = _mm512_castsi256_si512(_mm256_insert_epi16(_mm512_castsi512_si256(zmm01), (short)src, 0));
            zmm00 = _mm512_or_si512(zmm00, zmm01);
            zmm00 = _mm512_shuffle_epi8(zmm00, pshufb_idx);
            _mm512_mask_storeu_epi8(dst_ptr, mask64_store, zmm00);

            src_ptr += align * 2;
            dst_ptr += ((align * 11U) + start_bit) >> 3U;
            num_elements -= align;
        }

        {
            tail             = num_elements & 0x1fU;
            num_elements_128 = num_elements >> 7U;
            num_elements_32  = (num_elements >> 5U) & 3U;

            for (uint32_t idx = 0; idx < num_elements_128; ++idx) {
                srcmm0 = _mm512_loadu_si512(src_ptr);
                srcmm1 = _mm512_loadu_si512((src_ptr + 64U));
                srcmm2 = _mm512_loadu_si512((src_ptr + 128U));
                srcmm3 = _mm512_loadu_si512((src_ptr + 192U));

                zmm00 = _mm512_permutex2var_epi16(srcmm0, permutex_idx_ptr[0], srcmm1);
                zmm01 = _mm512_maskz_permutex2var_epi16(0x550aa154, srcmm0, permutex_idx_ptr[1], srcmm1);
                zmm02 = _mm512_permutex2var_epi16(srcmm0, permutex_idx_ptr[2], srcmm1);
                zmm10 = _mm512_permutex2var_epi16(srcmm1, permutex_idx_ptr[3], srcmm2);
                zmm11 = _mm512_maskz_permutex2var_epi16(0xaa1542a8, srcmm1, permutex_idx_ptr[4], srcmm2);
                zmm12 = _mm512_permutex2var_epi16(srcmm1, permutex_idx_ptr[5], srcmm2);
                zmm20 = _mm512_maskz_permutex2var_epi16(0x00ffffff, srcmm2, permutex_idx_ptr[6], srcmm3);
                zmm21 = _mm512_maskz_permutex2var_epi16(0x002a8550, srcmm2, permutex_idx_ptr[7], srcmm3);
                zmm22 = _mm512_maskz_permutex2var_epi16(0x00ffffff, srcmm2, permutex_idx_ptr[8], srcmm3);

                zmm00 = _mm512_sllv_epi16(zmm00, shift_masks_ptr[0]);
                zmm01 = _mm512_sllv_epi16(zmm01, shift_masks_ptr[1]);
                zmm02 = _mm512_srlv_epi16(zmm02, shift_masks_ptr[2]);
                zmm10 = _mm512_sllv_epi16(zmm10, shift_masks_ptr[3]);
                zmm11 = _mm512_sllv_epi16(zmm11, shift_masks_ptr[4]);
                zmm12 = _mm512_srlv_epi16(zmm12, shift_masks_ptr[5]);
                zmm20 = _mm512_sllv_epi16(zmm20, shift_masks_ptr[6]);
                zmm21 = _mm512_sllv_epi16(zmm21, shift_masks_ptr[7]);
                zmm22 = _mm512_srlv_epi16(zmm22, shift_masks_ptr[8]);

                zmm00 = _mm512_or_si512(zmm00, zmm01);
                zmm10 = _mm512_or_si512(zmm10, zmm11);
                zmm20 = _mm512_or_si512(zmm20, zmm21);
                zmm00 = _mm512_or_si512(zmm00, zmm02);
                zmm10 = _mm512_or_si512(zmm10, zmm12);
                zmm20 = _mm512_or_si512(zmm20, zmm22);

                zmm00 = _mm512_shuffle_epi8(zmm00, pshufb_idx);
                zmm10 = _mm512_shuffle_epi8(zmm10, pshufb_idx);
                zmm20 = _mm512_shuffle_epi8(zmm20, pshufb_idx);

                _mm512_storeu_si512(dst_ptr, zmm00);
                _mm512_storeu_si512((dst_ptr + 64U), zmm10);
                _mm512_mask_storeu_epi16(dst_ptr + 128U, 0x00FFFFFF, zmm20);

                src_ptr += 128U * 2U;
                dst_ptr += 88U * 2U;
            }

            for (uint32_t idx = 0; idx < num_elements_32; ++idx) {
                srcmm0 = _mm512_loadu_si512(src_ptr);
                zmm00  = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm0);
                zmm01  = _mm512_maskz_permutexvar_epi16(0x550aa154, permutex_idx_ptr[1], srcmm0);
                zmm02  = _mm512_permutexvar_epi16(permutex_idx_ptr[2], srcmm0);

                zmm00 = _mm512_sllv_epi16(zmm00, shift_masks_ptr[0]);
                zmm01 = _mm512_sllv_epi16(zmm01, shift_masks_ptr[1]);
                zmm02 = _mm512_srlv_epi16(zmm02, shift_masks_ptr[2]);

                zmm00 = _mm512_or_si512(zmm00, zmm01);
                zmm00 = _mm512_or_si512(zmm00, zmm02);
                zmm00 = _mm512_shuffle_epi8(zmm00, pshufb_idx);
                _mm512_mask_storeu_epi16(dst_ptr, 0x003FFFFF, zmm00);

                src_ptr += 32U * 2U;
                dst_ptr += 22U * 2U;
            }
            if (tail > 0) {
                uint64_t  num_bytes_out = ((uint64_t)tail * 11 + 7) / OWN_BYTE_WIDTH;
                __mmask32 mask32_load   = (1 << tail) - 1;
                __mmask64 mask64_store  = ((uint64_t)1 << num_bytes_out) - (uint64_t)1;

                srcmm0 = _mm512_mask_loadu_epi16(_mm512_setzero_si512(), mask32_load, (void const*)src_ptr);
                zmm00  = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm0);
                zmm01  = _mm512_maskz_permutexvar_epi16(0x550aa154, permutex_idx_ptr[1], srcmm0);
                zmm02  = _mm512_permutexvar_epi16(permutex_idx_ptr[2], srcmm0);

                zmm00 = _mm512_sllv_epi16(zmm00, shift_masks_ptr[0]);
                zmm01 = _mm512_sllv_epi16(zmm01, shift_masks_ptr[1]);
                zmm02 = _mm512_srlv_epi16(zmm02, shift_masks_ptr[2]);

                zmm00 = _mm512_or_si512(zmm00, zmm01);
                zmm00 = _mm512_or_si512(zmm00, zmm02);
                zmm00 = _mm512_shuffle_epi8(zmm00, pshufb_idx);
                _mm512_mask_storeu_epi8(dst_ptr, mask64_store, zmm00);
            }
        }
    }
}

// ----------------------- 16u12u ------------------------------- //
static uint16_t permutex_idx_table_12u_0[32] = {0U,  1U,  2U,  4U,  5U,  6U,  8U,  9U,  10U, 12U, 13U,
                                                14U, 16U, 17U, 18U, 20U, 21U, 22U, 24U, 25U, 26U, 28U,
                                                29U, 30U, 32U, 33U, 34U, 36U, 37U, 38U, 40U, 41U};
static uint16_t permutex_idx_table_12u_1[32] = {1U,  2U,  3U,  5U,  6U,  7U,  9U,  10U, 11U, 13U, 14U,
                                                15U, 17U, 18U, 19U, 21U, 22U, 23U, 25U, 26U, 27U, 29U,
                                                30U, 31U, 33U, 34U, 35U, 37U, 38U, 39U, 41U, 42U};
static uint16_t permutex_idx_table_12u_2[32] = {10U, 12U, 13U, 14U, 16U, 17U, 18U, 20U, 21U, 22U, 24U,
                                                25U, 26U, 28U, 29U, 30U, 32U, 33U, 34U, 36U, 37U, 38U,
                                                40U, 41U, 42U, 44U, 45U, 46U, 48U, 49U, 50U, 52U};
static uint16_t permutex_idx_table_12u_3[32] = {11U, 13U, 14U, 15U, 17U, 18U, 19U, 21U, 22U, 23U, 25U,
                                                26U, 27U, 29U, 30U, 31U, 33U, 34U, 35U, 37U, 38U, 39U,
                                                41U, 42U, 43U, 45U, 46U, 47U, 49U, 50U, 51U, 53U};
static uint16_t permutex_idx_table_12u_4[32] = {21U, 22U, 24U, 25U, 26U, 28U, 29U, 30U, 32U, 33U, 34U,
                                                36U, 37U, 38U, 40U, 41U, 42U, 44U, 45U, 46U, 48U, 49U,
                                                50U, 52U, 53U, 54U, 56U, 57U, 58U, 60U, 61U, 62U};
static uint16_t permutex_idx_table_12u_5[32] = {22U, 23U, 25U, 26U, 27U, 29U, 30U, 31U, 33U, 34U, 35U,
                                                37U, 38U, 39U, 41U, 42U, 43U, 45U, 46U, 47U, 49U, 50U,
                                                51U, 53U, 54U, 55U, 57U, 58U, 59U, 61U, 62U, 63U};

static uint16_t shift_mask_table_12u_0[32] = {4U, 8U,  12U, 4U, 8U,  12U, 4U, 8U,  12U, 4U, 8U,  12U, 4U, 8U,  12U, 4U,
                                              8U, 12U, 4U,  8U, 12U, 4U,  8U, 12U, 4U,  8U, 12U, 4U,  8U, 12U, 4U,  8U};
static uint16_t shift_mask_table_12u_2[32] = {12U, 4U, 8U,  12U, 4U, 8U,  12U, 4U, 8U,  12U, 4U, 8U,  12U, 4U, 8U,  12U,
                                              4U,  8U, 12U, 4U,  8U, 12U, 4U,  8U, 12U, 4U,  8U, 12U, 4U,  8U, 12U, 4U};
static uint16_t shift_mask_table_12u_4[32] = {8U,  12U, 4U,  8U,  12U, 4U,  8U,  12U, 4U,  8U, 12U,
                                              4U,  8U,  12U, 4U,  8U,  12U, 4U,  8U,  12U, 4U, 8U,
                                              12U, 4U,  8U,  12U, 4U,  8U,  12U, 4U,  8U,  12U};
static uint16_t shift_mask_table_12u_1[32] = {8U, 4U, 0U, 8U, 4U, 0U, 8U, 4U, 0U, 8U, 4U, 0U, 8U, 4U, 0U, 8U,
                                              4U, 0U, 8U, 4U, 0U, 8U, 4U, 0U, 8U, 4U, 0U, 8U, 4U, 0U, 8U, 4U};
static uint16_t shift_mask_table_12u_3[32] = {0U, 8U, 4U, 0U, 8U, 4U, 0U, 8U, 4U, 0U, 8U, 4U, 0U, 8U, 4U, 0U,
                                              8U, 4U, 0U, 8U, 4U, 0U, 8U, 4U, 0U, 8U, 4U, 0U, 8U, 4U, 0U, 8U};
static uint16_t shift_mask_table_12u_5[32] = {4U, 0U, 8U, 4U, 0U, 8U, 4U, 0U, 8U, 4U, 0U, 8U, 4U, 0U, 8U, 4U,
                                              0U, 8U, 4U, 0U, 8U, 4U, 0U, 8U, 4U, 0U, 8U, 4U, 0U, 8U, 4U, 0U};

static uint32_t table_align_16u12u[16] = {0, 0xffffffff, 0xffffffff, 0xffffffff, 1, 0xffffffff, 0xffffffff, 0xffffffff,
                                          2, 0xffffffff, 0xffffffff, 0xffffffff, 3, 0xffffffff, 0xffffffff, 0xffffffff};

OWN_OPT_FUN(void, k0_qplc_pack_be_16u12u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t align = table_align_16u12u[start_bit & 15];

    if (align > num_elements) {
        align = num_elements;
        qplc_pack_be_16u_nu(src_ptr, align, 12U, dst_ptr, start_bit);
        return;
    }

    {
        uint32_t tail             = 0;
        uint32_t num_elements_128 = 0;
        uint32_t num_elements_32  = 0;
        __m512i  srcmm0, srcmm1, srcmm2, srcmm3;
        __m512i  zmm0, zmm1, zmm2, zmm3, zmm4, zmm5;
        __m512i  permutex_idx_ptr[6];
        __m512i  shift_masks_ptr[6];
        __m512i  pshufb_idx = _mm512_loadu_si512(pshufb_idx_ptr);
        permutex_idx_ptr[0] = _mm512_loadu_si512(permutex_idx_table_12u_0);
        permutex_idx_ptr[1] = _mm512_loadu_si512(permutex_idx_table_12u_1);
        permutex_idx_ptr[2] = _mm512_loadu_si512(permutex_idx_table_12u_2);
        permutex_idx_ptr[3] = _mm512_loadu_si512(permutex_idx_table_12u_3);
        permutex_idx_ptr[4] = _mm512_loadu_si512(permutex_idx_table_12u_4);
        permutex_idx_ptr[5] = _mm512_loadu_si512(permutex_idx_table_12u_5);
        shift_masks_ptr[0]  = _mm512_loadu_si512(shift_mask_table_12u_0);
        shift_masks_ptr[1]  = _mm512_loadu_si512(shift_mask_table_12u_1);
        shift_masks_ptr[2]  = _mm512_loadu_si512(shift_mask_table_12u_2);
        shift_masks_ptr[3]  = _mm512_loadu_si512(shift_mask_table_12u_3);
        shift_masks_ptr[4]  = _mm512_loadu_si512(shift_mask_table_12u_4);
        shift_masks_ptr[5]  = _mm512_loadu_si512(shift_mask_table_12u_5);

        if (align) {
            __m512i   permute_idx   = _mm512_loadu_si512(permute_idx_16u);
            __mmask32 mask32_load   = (1 << align) - 1;
            uint64_t  num_bytes_out = ((uint64_t)align * 12 + start_bit) / OWN_BYTE_WIDTH;
            __mmask64 mask64_store  = ((uint64_t)1 << num_bytes_out) - (uint64_t)1;
            uint32_t  src =
                    ((uint32_t)qplc_swap_bytes_16u(*(uint16_t*)dst_ptr)) & (0xffff << (OWN_WORD_WIDTH - start_bit));

            srcmm0 = _mm512_mask_loadu_epi16(_mm512_setzero_si512(), mask32_load, (void const*)src_ptr);
            zmm0   = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_permutexvar_epi16(permutex_idx_ptr[1], srcmm0);

            zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_srlv_epi16(zmm1, shift_masks_ptr[1]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm1 = _mm512_maskz_permutexvar_epi16(0x0000FFFE, permute_idx, zmm0);
            zmm0 = _mm512_srl_epi16(zmm0, _mm_cvtsi32_si128(start_bit));
            zmm1 = _mm512_sll_epi16(zmm1, _mm_cvtsi32_si128(OWN_WORD_WIDTH - start_bit));
            zmm1 = _mm512_castsi256_si512(_mm256_insert_epi16(_mm512_castsi512_si256(zmm1), (short)src, 0));
            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
            _mm512_mask_storeu_epi8(dst_ptr, mask64_store, zmm0);

            src_ptr += align * 2;
            dst_ptr += ((align * 12U) + start_bit) >> 3U;
            num_elements -= align;
        }

        {
            tail             = num_elements & 31U;
            num_elements_128 = num_elements >> 7U;
            num_elements_32  = (num_elements & 127U) >> 5U;

            for (uint32_t idx = 0; idx < num_elements_128; ++idx) {
                srcmm0 = _mm512_loadu_si512(src_ptr);
                srcmm1 = _mm512_loadu_si512((src_ptr + 64U));
                srcmm2 = _mm512_loadu_si512((src_ptr + 128U));
                srcmm3 = _mm512_loadu_si512((src_ptr + 192U));

                zmm0 = _mm512_permutex2var_epi16(srcmm0, permutex_idx_ptr[0], srcmm1);
                zmm1 = _mm512_permutex2var_epi16(srcmm0, permutex_idx_ptr[1], srcmm1);
                zmm2 = _mm512_permutex2var_epi16(srcmm1, permutex_idx_ptr[2], srcmm2);
                zmm3 = _mm512_permutex2var_epi16(srcmm1, permutex_idx_ptr[3], srcmm2);
                zmm4 = _mm512_permutex2var_epi16(srcmm2, permutex_idx_ptr[4], srcmm3);
                zmm5 = _mm512_permutex2var_epi16(srcmm2, permutex_idx_ptr[5], srcmm3);

                zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
                zmm1 = _mm512_srlv_epi16(zmm1, shift_masks_ptr[1]);
                zmm2 = _mm512_sllv_epi16(zmm2, shift_masks_ptr[2]);
                zmm3 = _mm512_srlv_epi16(zmm3, shift_masks_ptr[3]);
                zmm4 = _mm512_sllv_epi16(zmm4, shift_masks_ptr[4]);
                zmm5 = _mm512_srlv_epi16(zmm5, shift_masks_ptr[5]);

                zmm0 = _mm512_or_si512(zmm0, zmm1);
                zmm2 = _mm512_or_si512(zmm2, zmm3);
                zmm4 = _mm512_or_si512(zmm4, zmm5);

                zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
                zmm2 = _mm512_shuffle_epi8(zmm2, pshufb_idx);
                zmm4 = _mm512_shuffle_epi8(zmm4, pshufb_idx);

                _mm512_storeu_si512(dst_ptr, zmm0);
                _mm512_storeu_si512((dst_ptr + 64U), zmm2);
                _mm512_storeu_si512((dst_ptr + 128U), zmm4);

                src_ptr += 128U * sizeof(uint16_t);
                dst_ptr += 96U * sizeof(uint16_t);
            }

            for (uint32_t idx = 0; idx < num_elements_32; ++idx) {
                srcmm0 = _mm512_loadu_si512(src_ptr);
                zmm0   = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm0);
                zmm1   = _mm512_permutexvar_epi16(permutex_idx_ptr[1], srcmm0);

                zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
                zmm1 = _mm512_srlv_epi16(zmm1, shift_masks_ptr[1]);

                zmm0 = _mm512_or_si512(zmm0, zmm1);
                zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
                _mm512_mask_storeu_epi16(dst_ptr, 0x00FFFFFF, zmm0);

                src_ptr += 32U * sizeof(uint16_t);
                dst_ptr += 24U * sizeof(uint16_t);
            }
            if (tail > 0) {
                uint64_t  num_bytes_out = ((uint64_t)tail * 12 + 7) / OWN_BYTE_WIDTH;
                __mmask32 mask32_load   = (1 << tail) - 1;
                __mmask64 mask64_store  = ((uint64_t)1 << num_bytes_out) - (uint64_t)1;

                srcmm0 = _mm512_mask_loadu_epi16(_mm512_setzero_si512(), mask32_load, (void const*)src_ptr);
                zmm0   = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm0);
                zmm1   = _mm512_permutexvar_epi16(permutex_idx_ptr[1], srcmm0);

                zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
                zmm1 = _mm512_srlv_epi16(zmm1, shift_masks_ptr[1]);

                zmm0 = _mm512_or_si512(zmm0, zmm1);
                zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
                _mm512_mask_storeu_epi8(dst_ptr, mask64_store, zmm0);
            }
        }
    }
}

// ----------------------- 16u13u ------------------------------- //
static uint16_t permutex_idx_table_13u_0[32] = {0U,  1U,  2U,  3U,  4U,  6U,  7U,  8U,  9U,  11U, 12U,
                                                13U, 14U, 16U, 17U, 18U, 19U, 20U, 22U, 23U, 24U, 25U,
                                                27U, 28U, 29U, 30U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static uint16_t permutex_idx_table_13u_1[32] = {0x0, 0x0, 0x0, 0x0, 5U,  0x0, 0x0, 0x0, 10U, 0x0, 0x0,
                                                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 21U, 0x0, 0x0, 0x0, 26U,
                                                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static uint16_t permutex_idx_table_13u_2[32] = {1U,  2U,  3U,  4U,  6U,  7U,  8U,  9U,  11U, 12U, 13U,
                                                14U, 15U, 17U, 18U, 19U, 20U, 22U, 23U, 24U, 25U, 27U,
                                                28U, 29U, 30U, 31U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

static __mmask32 permutex_masks_13u_ptr[3] = {0x03FFFFFF, 0x00220110, 0x03FFFFFF};

static uint16_t shift_mask_table_13u_0[32] = {3U,  6U,  9U,  12U, 15U, 5U,  8U,  11U, 14U, 4U,  7U,
                                              10U, 13U, 3U,  6U,  9U,  12U, 15U, 5U,  8U,  11U, 14U,
                                              4U,  7U,  10U, 13U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static uint16_t shift_mask_table_13u_1[32] = {0x0, 0x0, 0x0, 0x0, 2U,  0x0, 0x0, 0x0, 1U,  0x0, 0x0,
                                              0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 2U,  0x0, 0x0, 0x0, 1U,
                                              0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static uint16_t shift_mask_table_13u_2[32] = {10U, 7U,  4U, 1U, 11U, 8U,  5U, 2U, 12U, 9U, 6U, 3U,  0U,  10U, 7U,  4U,
                                              1U,  11U, 8U, 5U, 2U,  12U, 9U, 6U, 3U,  0U, 7U, 0x0, 0x0, 0x0, 0x0, 0x0};

static uint32_t table_align_16u13u[16] = {0, 11, 6, 1, 12, 7, 2, 13, 8, 3, 14, 9, 4, 15, 10, 5};

OWN_OPT_FUN(void, k0_qplc_pack_be_16u13u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t align = table_align_16u13u[start_bit & 15];

    if (align > num_elements) {
        align = num_elements;
        qplc_pack_be_16u_nu(src_ptr, align, 13U, dst_ptr, start_bit);
        return;
    }

    {
        uint32_t tail            = 0;
        uint32_t num_elements_32 = 0;
        __m512i  srcmm0;
        __m512i  zmm0, zmm1, zmm2;
        __m512i  permutex_idx_ptr[3];
        __m512i  shift_masks_ptr[3];
        __m512i  pshufb_idx = _mm512_loadu_si512(pshufb_idx_ptr);
        permutex_idx_ptr[0] = _mm512_loadu_si512(permutex_idx_table_13u_0);
        permutex_idx_ptr[1] = _mm512_loadu_si512(permutex_idx_table_13u_1);
        permutex_idx_ptr[2] = _mm512_loadu_si512(permutex_idx_table_13u_2);
        shift_masks_ptr[0]  = _mm512_loadu_si512(shift_mask_table_13u_0);
        shift_masks_ptr[1]  = _mm512_loadu_si512(shift_mask_table_13u_1);
        shift_masks_ptr[2]  = _mm512_loadu_si512(shift_mask_table_13u_2);

        if (align) {
            __m512i   permute_idx   = _mm512_loadu_si512(permute_idx_16u);
            __mmask32 mask32_load   = (1 << align) - 1;
            uint64_t  num_bytes_out = ((uint64_t)align * 13 + start_bit) / OWN_BYTE_WIDTH;
            __mmask64 mask64_store  = ((uint64_t)1 << num_bytes_out) - (uint64_t)1;
            uint32_t  src =
                    ((uint32_t)qplc_swap_bytes_16u(*(uint16_t*)dst_ptr)) & (0xffff << (OWN_WORD_WIDTH - start_bit));

            srcmm0 = _mm512_mask_loadu_epi16(_mm512_setzero_si512(), mask32_load, (void const*)src_ptr);
            zmm0   = _mm512_maskz_permutexvar_epi16(permutex_masks_13u_ptr[0], permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_maskz_permutexvar_epi16(permutex_masks_13u_ptr[1], permutex_idx_ptr[1], srcmm0);
            zmm2   = _mm512_maskz_permutexvar_epi16(permutex_masks_13u_ptr[2], permutex_idx_ptr[2], srcmm0);

            zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi16(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi16(zmm2, shift_masks_ptr[2]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);

            zmm1 = _mm512_maskz_permutexvar_epi16(0x0000FFFE, permute_idx, zmm0);
            zmm0 = _mm512_srl_epi16(zmm0, _mm_cvtsi32_si128(start_bit));
            zmm1 = _mm512_sll_epi16(zmm1, _mm_cvtsi32_si128(OWN_WORD_WIDTH - start_bit));
            zmm1 = _mm512_castsi256_si512(_mm256_insert_epi16(_mm512_castsi512_si256(zmm1), (short)src, 0));
            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
            _mm512_mask_storeu_epi8(dst_ptr, mask64_store, zmm0);

            src_ptr += align * 2;
            dst_ptr += ((align * 13U) + start_bit) >> 3U;
            num_elements -= align;
        }

        {
            tail            = num_elements & 31U;
            num_elements_32 = num_elements >> 5U;

            for (uint32_t idx = 0; idx < num_elements_32; ++idx) {
                srcmm0 = _mm512_loadu_si512(src_ptr);
                zmm0   = _mm512_maskz_permutexvar_epi16(permutex_masks_13u_ptr[0], permutex_idx_ptr[0], srcmm0);
                zmm1   = _mm512_maskz_permutexvar_epi16(permutex_masks_13u_ptr[1], permutex_idx_ptr[1], srcmm0);
                zmm2   = _mm512_maskz_permutexvar_epi16(permutex_masks_13u_ptr[2], permutex_idx_ptr[2], srcmm0);

                zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
                zmm1 = _mm512_sllv_epi16(zmm1, shift_masks_ptr[1]);
                zmm2 = _mm512_srlv_epi16(zmm2, shift_masks_ptr[2]);

                zmm0 = _mm512_or_si512(zmm0, zmm1);
                zmm0 = _mm512_or_si512(zmm0, zmm2);
                zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
                _mm512_mask_storeu_epi16(dst_ptr, 0x03FFFFFF, zmm0);

                src_ptr += 32U * sizeof(uint16_t);
                dst_ptr += 26U * sizeof(uint16_t);
            }

            if (tail > 0) {
                uint64_t  num_bytes_out = ((uint64_t)tail * 13 + 7) / OWN_BYTE_WIDTH;
                __mmask32 mask32_load   = (1 << tail) - 1;
                __mmask64 mask64_store  = ((uint64_t)1 << num_bytes_out) - (uint64_t)1;

                srcmm0 = _mm512_mask_loadu_epi16(_mm512_setzero_si512(), mask32_load, (void const*)src_ptr);
                zmm0   = _mm512_maskz_permutexvar_epi16(permutex_masks_13u_ptr[0], permutex_idx_ptr[0], srcmm0);
                zmm1   = _mm512_maskz_permutexvar_epi16(permutex_masks_13u_ptr[1], permutex_idx_ptr[1], srcmm0);
                zmm2   = _mm512_maskz_permutexvar_epi16(permutex_masks_13u_ptr[2], permutex_idx_ptr[2], srcmm0);

                zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
                zmm1 = _mm512_sllv_epi16(zmm1, shift_masks_ptr[1]);
                zmm2 = _mm512_srlv_epi16(zmm2, shift_masks_ptr[2]);

                zmm0 = _mm512_or_si512(zmm0, zmm1);
                zmm0 = _mm512_or_si512(zmm0, zmm2);
                zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
                _mm512_mask_storeu_epi8(dst_ptr, mask64_store, zmm0);
            }
        }
    }
}

// ----------------------- 16u14u ------------------------------- //
static uint16_t permutex_idx_table_14u_0[32] = {0U,  1U,  2U,  3U,  4U,  5U,  6U,  8U,  9U,  10U, 11U,
                                                12U, 13U, 14U, 16U, 17U, 18U, 19U, 20U, 21U, 22U, 24U,
                                                25U, 26U, 27U, 28U, 29U, 30U, 0x0, 0x0, 0x0, 0x0};
static uint16_t permutex_idx_table_14u_1[32] = {1U,  2U,  3U,  4U,  5U,  6U,  7U,  9U,  10U, 11U, 12U,
                                                13U, 14U, 15U, 17U, 18U, 19U, 20U, 21U, 22U, 23U, 25U,
                                                26U, 27U, 28U, 29U, 30U, 31U, 0x0, 0x0, 0x0, 0x0};

static uint16_t shift_mask_table_14u_0[32] = {2U,  4U,  6U,  8U,  10U, 12U, 14U, 2U,  4U,  6U,  8U,
                                              10U, 12U, 14U, 2U,  4U,  6U,  8U,  10U, 12U, 14U, 2U,
                                              4U,  6U,  8U,  10U, 12U, 14U, 0x0, 0x0, 0x0, 0x0};
static uint16_t shift_mask_table_14u_1[32] = {12U, 10U, 8U, 6U, 4U, 2U,  0U,  12U, 10U, 8U, 6U, 4U, 2U,  0U,  12U, 10U,
                                              8U,  6U,  4U, 2U, 0U, 12U, 10U, 8U,  6U,  4U, 2U, 0U, 0x0, 0x0, 0x0, 0x0};

OWN_OPT_FUN(void, k0_qplc_pack_be_16u14u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t align = (start_bit & 1) ? 0xffffffff : (start_bit & 15) >> 1;

    if (align > num_elements) {
        align = num_elements;
        qplc_pack_be_16u_nu(src_ptr, align, 14U, dst_ptr, start_bit);
        return;
    }

    {
        uint32_t tail            = 0U;
        uint32_t num_elements_32 = 0U;
        __m512i  srcmm0;
        __m512i  zmm0, zmm1;
        __m512i  permutex_idx_ptr[2];
        __m512i  shift_masks_ptr[2];
        __m512i  pshufb_idx = _mm512_loadu_si512(pshufb_idx_ptr);
        permutex_idx_ptr[0] = _mm512_loadu_si512(permutex_idx_table_14u_0);
        permutex_idx_ptr[1] = _mm512_loadu_si512(permutex_idx_table_14u_1);
        shift_masks_ptr[0]  = _mm512_loadu_si512(shift_mask_table_14u_0);
        shift_masks_ptr[1]  = _mm512_loadu_si512(shift_mask_table_14u_1);
        pshufb_idx          = _mm512_loadu_si512(pshufb_idx_ptr);

        if (align) {
            __m512i   permute_idx   = _mm512_loadu_si512(permute_idx_16u);
            __mmask32 mask32_load   = (1 << align) - 1;
            uint64_t  num_bytes_out = ((uint64_t)align * 14 + start_bit) / OWN_BYTE_WIDTH;
            __mmask64 mask64_store  = ((uint64_t)1 << num_bytes_out) - (uint64_t)1;
            uint32_t  src =
                    ((uint32_t)qplc_swap_bytes_16u(*(uint16_t*)dst_ptr)) & (0xffff << (OWN_WORD_WIDTH - start_bit));

            srcmm0 = _mm512_mask_loadu_epi16(_mm512_setzero_si512(), mask32_load, (void const*)src_ptr);
            zmm0   = _mm512_maskz_permutexvar_epi16(0x0FFFFFFF, permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_maskz_permutexvar_epi16(0x0FFFFFFF, permutex_idx_ptr[1], srcmm0);

            zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_srlv_epi16(zmm1, shift_masks_ptr[1]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);

            zmm1 = _mm512_maskz_permutexvar_epi16(0x0000FFFE, permute_idx, zmm0);
            zmm0 = _mm512_srl_epi16(zmm0, _mm_cvtsi32_si128(start_bit));
            zmm1 = _mm512_sll_epi16(zmm1, _mm_cvtsi32_si128(OWN_WORD_WIDTH - start_bit));
            zmm1 = _mm512_castsi256_si512(_mm256_insert_epi16(_mm512_castsi512_si256(zmm1), (short)src, 0));
            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
            _mm512_mask_storeu_epi8(dst_ptr, mask64_store, zmm0);

            src_ptr += align * 2;
            dst_ptr += ((align * 14U) + start_bit) >> 3U;
            num_elements -= align;
        }

        {
            tail            = num_elements & 31U;
            num_elements_32 = num_elements >> 5U;

            for (uint32_t idx = 0; idx < num_elements_32; ++idx) {
                srcmm0 = _mm512_loadu_si512(src_ptr);
                zmm0   = _mm512_maskz_permutexvar_epi16(0x0FFFFFFF, permutex_idx_ptr[0], srcmm0);
                zmm1   = _mm512_maskz_permutexvar_epi16(0x0FFFFFFF, permutex_idx_ptr[1], srcmm0);

                zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
                zmm1 = _mm512_srlv_epi16(zmm1, shift_masks_ptr[1]);

                zmm0 = _mm512_or_si512(zmm0, zmm1);
                zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
                _mm512_mask_storeu_epi16(dst_ptr, 0x0FFFFFFF, zmm0);

                src_ptr += 32U * sizeof(uint16_t);
                dst_ptr += 28U * sizeof(uint16_t);
            }

            if (tail > 0) {
                uint64_t  num_bytes_out = ((uint64_t)tail * 14 + 7) / OWN_BYTE_WIDTH;
                __mmask32 mask32_load   = (1 << tail) - 1;
                __mmask64 mask64_store  = ((uint64_t)1 << num_bytes_out) - (uint64_t)1;

                srcmm0 = _mm512_mask_loadu_epi16(_mm512_setzero_si512(), mask32_load, (void const*)src_ptr);
                zmm0   = _mm512_maskz_permutexvar_epi16(0x0FFFFFFF, permutex_idx_ptr[0], srcmm0);
                zmm1   = _mm512_maskz_permutexvar_epi16(0x0FFFFFFF, permutex_idx_ptr[1], srcmm0);

                zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
                zmm1 = _mm512_srlv_epi16(zmm1, shift_masks_ptr[1]);

                zmm0 = _mm512_or_si512(zmm0, zmm1);
                zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
                _mm512_mask_storeu_epi8(dst_ptr, mask64_store, zmm0);
            }
        }
    }
}

// ----------------------- 16u15u ------------------------------- //

static uint16_t permutex_idx_table_15u_0[32] = {0U,  1U,  2U,  3U,  4U,  5U,  6U,  7U,  8U,  9U,  10U,
                                                11U, 12U, 13U, 14U, 16U, 17U, 18U, 19U, 20U, 21U, 22U,
                                                23U, 24U, 25U, 26U, 27U, 28U, 29U, 30U, 0x0, 0x0};
static uint16_t permutex_idx_table_15u_1[32] = {1U,  2U,  3U,  4U,  5U,  6U,  7U,  8U,  9U,  10U, 11U,
                                                12U, 13U, 14U, 15U, 17U, 18U, 19U, 20U, 21U, 22U, 23U,
                                                24U, 25U, 26U, 27U, 28U, 29U, 30U, 31U, 0x0};

static uint16_t shift_mask_table_15u_0[32] = {1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U,  10U, 11U, 12U, 13U, 14U, 15U, 1U,
                                              2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U, 12U, 13U, 14U, 15U, 0x0, 0x0};
static uint16_t shift_mask_table_15u_1[32] = {14U, 13U, 12U, 11U, 10U, 9U, 8U, 7U, 6U, 5U, 4U, 3U, 2U, 1U, 0U,  14U,
                                              13U, 12U, 11U, 10U, 9U,  8U, 7U, 6U, 5U, 4U, 3U, 2U, 1U, 0U, 0x0, 0x0};

OWN_OPT_FUN(void, k0_qplc_pack_be_16u15u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t align = start_bit & 15;

    if (align > num_elements) {
        align = num_elements;
        qplc_pack_be_16u_nu(src_ptr, align, 15U, dst_ptr, start_bit);
        return;
    }

    {
        uint32_t tail            = 0U;
        uint32_t num_elements_32 = 0U;
        __m512i  srcmm0;
        __m512i  zmm0, zmm1;
        __m512i  permutex_idx_ptr[2];
        __m512i  shift_masks_ptr[2];
        __m512i  pshufb_idx = _mm512_loadu_si512(pshufb_idx_ptr);
        permutex_idx_ptr[0] = _mm512_loadu_si512(permutex_idx_table_15u_0);
        permutex_idx_ptr[1] = _mm512_loadu_si512(permutex_idx_table_15u_1);
        shift_masks_ptr[0]  = _mm512_loadu_si512(shift_mask_table_15u_0);
        shift_masks_ptr[1]  = _mm512_loadu_si512(shift_mask_table_15u_1);

        if (align) {
            __m512i   permute_idx   = _mm512_loadu_si512(permute_idx_16u);
            __mmask32 mask32_load   = (1 << align) - 1;
            uint64_t  num_bytes_out = ((uint64_t)align * 15 + start_bit) / OWN_BYTE_WIDTH;
            __mmask64 mask64_store  = ((uint64_t)1 << num_bytes_out) - (uint64_t)1;
            uint32_t  src =
                    ((uint32_t)qplc_swap_bytes_16u(*(uint16_t*)dst_ptr)) & (0xffff << (OWN_WORD_WIDTH - start_bit));

            srcmm0 = _mm512_mask_loadu_epi16(_mm512_setzero_si512(), mask32_load, (void const*)src_ptr);
            zmm0   = _mm512_maskz_permutexvar_epi16(0x3FFFFFFF, permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_maskz_permutexvar_epi16(0x3FFFFFFF, permutex_idx_ptr[1], srcmm0);

            zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_srlv_epi16(zmm1, shift_masks_ptr[1]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);

            zmm1 = _mm512_maskz_permutexvar_epi16(0x0000FFFE, permute_idx, zmm0);
            zmm0 = _mm512_srl_epi16(zmm0, _mm_cvtsi32_si128(start_bit));
            zmm1 = _mm512_sll_epi16(zmm1, _mm_cvtsi32_si128(OWN_WORD_WIDTH - start_bit));
            zmm1 = _mm512_castsi256_si512(_mm256_insert_epi16(_mm512_castsi512_si256(zmm1), (short)src, 0));
            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
            _mm512_mask_storeu_epi8(dst_ptr, mask64_store, zmm0);

            src_ptr += align * 2;
            dst_ptr += ((align * 15U) + start_bit) >> 3U;
            num_elements -= align;
        }

        {
            tail            = num_elements & 31U;
            num_elements_32 = num_elements >> 5U;

            for (uint32_t idx = 0; idx < num_elements_32; ++idx) {
                srcmm0 = _mm512_loadu_si512(src_ptr);
                zmm0   = _mm512_maskz_permutexvar_epi16(0x3FFFFFFF, permutex_idx_ptr[0], srcmm0);
                zmm1   = _mm512_maskz_permutexvar_epi16(0x3FFFFFFF, permutex_idx_ptr[1], srcmm0);

                zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
                zmm1 = _mm512_srlv_epi16(zmm1, shift_masks_ptr[1]);

                zmm0 = _mm512_or_si512(zmm0, zmm1);
                zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
                _mm512_mask_storeu_epi16(dst_ptr, 0x3FFFFFFF, zmm0);

                src_ptr += 32U * sizeof(uint16_t);
                dst_ptr += 30U * sizeof(uint16_t);
            }

            if (tail > 0) {
                uint64_t  num_bytes_out = ((uint64_t)tail * 15 + 7) / OWN_BYTE_WIDTH;
                __mmask32 mask32_load   = (1 << tail) - 1;
                __mmask64 mask64_store  = ((uint64_t)1 << num_bytes_out) - (uint64_t)1;

                srcmm0 = _mm512_mask_loadu_epi16(_mm512_setzero_si512(), mask32_load, (void const*)src_ptr);
                zmm0   = _mm512_maskz_permutexvar_epi16(0x3FFFFFFF, permutex_idx_ptr[0], srcmm0);
                zmm1   = _mm512_maskz_permutexvar_epi16(0x3FFFFFFF, permutex_idx_ptr[1], srcmm0);

                zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
                zmm1 = _mm512_srlv_epi16(zmm1, shift_masks_ptr[1]);

                zmm0 = _mm512_or_si512(zmm0, zmm1);
                zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
                _mm512_mask_storeu_epi8(dst_ptr, mask64_store, zmm0);
            }
        }
    }
}

// ----------------------- 16u16u ------------------------------- //

OWN_OPT_FUN(void, k0_qplc_pack_be_16u16u, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    uint16_t* dst_16u_ptr     = (uint16_t*)dst_ptr;
    uint16_t* src_16u_ptr     = (uint16_t*)src_ptr;
    uint32_t  num_elements_32 = num_elements >> 5U;
    ;
    uint32_t tail = num_elements & 31U;
    __m512i  zmm0;

    __m512i pshufb_idx = _mm512_loadu_si512(pshufb_idx_ptr);

    for (uint32_t i = 0U; i < num_elements_32; i++) {
        zmm0 = _mm512_shuffle_epi8(_mm512_loadu_si512(src_16u_ptr), pshufb_idx);
        _mm512_storeu_si512((void*)dst_16u_ptr, zmm0);
        src_16u_ptr += 32;
        dst_16u_ptr += 32;
    }

    if (tail) {
        __mmask32 mask32_loadstore = (1 << tail) - 1;
        zmm0 = _mm512_shuffle_epi8(_mm512_maskz_loadu_epi16(mask32_loadstore, src_16u_ptr), pshufb_idx);
        _mm512_mask_storeu_epi16((void*)dst_16u_ptr, mask32_loadstore, zmm0);
    }
}

// ----------------------- 16u16u ------------------------------- //

static uint8_t pshufb16u32u_idx_ptr[64] = {
        3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12, 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12,
        3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12, 3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12};

OWN_OPT_FUN(void, k0_qplc_pack_be_16u32u, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    uint32_t* dst_32u_ptr     = (uint32_t*)dst_ptr;
    uint16_t* src_16u_ptr     = (uint16_t*)src_ptr;
    uint32_t  num_elements_16 = num_elements >> 4U;
    ;
    uint32_t tail = num_elements & 15U;
    __m512i  zmm0;
    __m512i  pshufb_idx = _mm512_loadu_si512(pshufb16u32u_idx_ptr);

    for (uint32_t i = 0U; i < num_elements_16; i++) {
        zmm0 = _mm512_cvtepu16_epi32(_mm256_loadu_si256((const __m256i*)src_16u_ptr));
        zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
        _mm512_storeu_si512((void*)dst_32u_ptr, zmm0);
        src_16u_ptr += 16;
        dst_32u_ptr += 16;
    }

    if (tail) {
        __mmask16 mask16_loadstore = (1 << tail) - 1;
        zmm0 = _mm512_cvtepu16_epi32(_mm256_maskz_loadu_epi16(mask16_loadstore, (const __m256i*)src_16u_ptr));
        zmm0 = _mm512_shuffle_epi8(zmm0, pshufb_idx);
        _mm512_mask_storeu_epi32((void*)dst_32u_ptr, mask16_loadstore, zmm0);
    }
}

#endif // OWN_PACK_BE_16U_H
