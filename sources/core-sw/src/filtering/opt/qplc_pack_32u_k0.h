/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*------- qplc_pack_16u_k0.h -------*/

/**
 * @brief Contains implementation of functions for vector packing dword integers to 17...32-bit integers
 * @date 22/03/2021
 *
 * @details Function list:
 *          - @ref qplc_pack_32u17u
 *          - @ref qplc_pack_32u18u
 *          - @ref qplc_pack_32u19u
 *          - @ref qplc_pack_32u20u
 *          - @ref qplc_pack_32u21u
 *          - @ref qplc_pack_32u22u
 *          - @ref qplc_pack_32u23u
 *          - @ref qplc_pack_32u24u
 *          - @ref qplc_pack_32u25u
 *          - @ref qplc_pack_32u26u
 *          - @ref qplc_pack_32u27u
 *          - @ref qplc_pack_32u28u
 *          - @ref qplc_pack_32u29u
 *          - @ref qplc_pack_32u30u
 *          - @ref qplc_pack_32u31u
 *          - @ref qplc_pack_32u32u
 */
#ifndef OWN_PACK_32U_H
#define OWN_PACK_32U_H

#include "own_qplc_defs.h"

// *********************** Masks  ****************************** //

// ----------------------- 32u17u ------------------------------- //
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_17u_0[16]) = {0U,  2U,  4U,  6U,  8U,  10U, 12U, 14U,
                                                                      0x0, 17U, 19U, 21U, 23U, 25U, 27U, 29U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_17u_1[16]) = {1U,  3U,  5U,  7U,  9U,  11U, 13U, 15U,
                                                                      16U, 18U, 20U, 22U, 24U, 26U, 28U, 30U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_17u_2[16]) = {0x0, 1U,  3U,  5U,  7U,  9U,  11U, 13U,
                                                                      15U, 16U, 18U, 20U, 22U, 24U, 26U, 28U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_17u_3[16]) = {15U, 16U, 18U, 20U, 22U, 24U, 26U, 28U,
                                                                      30U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_17u_4[16]) = {0x0, 17U, 19U, 21U, 23U, 25U, 27U, 29U,
                                                                      31U, 32U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_17u_5[16]) = {14U, 0x0, 17U, 19U, 21U, 23U, 25U, 27U,
                                                                      29U, 31U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static __mmask16 permutex_masks_17u_ptr[6]                         = {0xFEFF, 0xFFFF, 0xFFFE, 0x01FF, 0x03FE, 0x03FD};

OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_17u_0[16]) = {0U, 2U, 4U, 6U, 8U, 10U, 12U, 14U,
                                                                    0U, 1U, 3U, 5U, 7U, 9U,  11U, 13U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_17u_1[16]) = {17U, 19U, 21U, 23U, 25U, 27U, 29U, 31U,
                                                                    16U, 18U, 20U, 22U, 24U, 26U, 28U, 30U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_17u_2[16]) = {0U, 15U, 13U, 11U, 9U,  7U, 5U, 3U,
                                                                    1U, 16U, 14U, 12U, 10U, 8U, 6U, 4U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_17u_3[16]) = {15U, 0U, 2U,  4U,  6U,  8U,  10U, 12U,
                                                                    14U, 0U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_17u_4[16]) = {0U,  17U, 19U, 21U, 23U, 25U, 27U, 29U,
                                                                    31U, 16U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_17u_5[16]) = {2U, 0U, 15U, 13U, 11U, 9U,  7U,  5U,
                                                                    3U, 1U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

// ----------------------- 32u18u ------------------------------- //
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_18u_0[16]) = {0U,  2U,  4U,  6U,  0x0, 9U,  11U, 13U,
                                                                      15U, 16U, 18U, 20U, 22U, 0x0, 25U, 27U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_18u_1[16]) = {1U,  3U,  5U,  7U,  8U,  10U, 12U, 14U,
                                                                      0x0, 17U, 19U, 21U, 23U, 24U, 26U, 28U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_18u_2[16]) = {0x0, 1U,  3U,  5U,  7U,  8U,  10U, 12U,
                                                                      14U, 0x0, 17U, 19U, 21U, 23U, 24U, 26U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_18u_3[16]) = {13U, 15U, 16U, 18U, 20U, 22U, 0x0, 25U,
                                                                      27U, 29U, 31U, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_18u_4[16]) = {14U, 0x0, 17U, 19U, 21U, 23U, 24U, 26U,
                                                                      28U, 30U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_18u_5[16]) = {12U, 14U, 0x0, 17U, 19U, 21U, 23U, 24U,
                                                                      26U, 28U, 30U, 0x0, 0x0, 0x0, 0x0, 0x0};
static __mmask16 permutex_masks_18u_ptr[6]                         = {0xDFEF, 0xFEFF, 0xFDFE, 0x07BF, 0x03FD, 0x07FB};

OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_18u_0[16]) = {0U,  4U, 8U, 12U, 0U,  2U, 6U, 10U,
                                                                    14U, 0U, 4U, 8U,  12U, 0U, 2U, 6U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_18u_1[16]) = {18U, 22U, 26U, 30U, 16U, 20U, 24U, 28U,
                                                                    0U,  18U, 22U, 26U, 30U, 16U, 20U, 24U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_18u_2[16]) = {0U, 14U, 10U, 6U,  2U, 16U, 12U, 8U,
                                                                    4U, 0U,  14U, 10U, 6U, 2U,  16U, 12U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_18u_3[16]) = {10U, 14U, 0U,  4U,  8U,  12U, 0U,  2U,
                                                                    6U,  10U, 14U, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_18u_4[16]) = {28U, 0U,  18U, 22U, 26U, 30U, 16U, 20U,
                                                                    24U, 28U, 0U,  0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_18u_5[16]) = {8U,  4U, 0U, 14U, 10U, 6U,  2U,  16U,
                                                                    12U, 8U, 4U, 0x0, 0x0, 0x0, 0x0, 0x0};

// ----------------------- 32u19u ------------------------------- //
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_19u_0[16]) = {0U,  2U,  4U,  0x0, 7U,  9U,  0x0, 12U,
                                                                      14U, 0x0, 17U, 19U, 21U, 22U, 24U, 26U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_19u_1[16]) = {1U,  3U,  5U,  6U,  8U,  10U, 11U, 13U,
                                                                      15U, 16U, 18U, 20U, 0x0, 23U, 25U, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_19u_2[16]) = {0x0, 1U,  3U,  5U,  6U,  8U,  10U, 11U,
                                                                      13U, 15U, 16U, 18U, 20U, 21U, 23U, 25U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_19u_3[16]) = {11U, 13U, 15U, 16U, 18U, 20U, 0x0, 23U,
                                                                      25U, 0x0, 28U, 30U, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_19u_4[16]) = {12U, 14U, 0x0, 17U, 19U, 21U, 22U, 24U,
                                                                      26U, 27U, 29U, 31U, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_19u_5[16]) = {10U, 12U, 14U, 0x0, 17U, 19U, 21U, 22U,
                                                                      24U, 26U, 27U, 29U, 31U, 0x0, 0x0, 0x0};
static __mmask16 permutex_masks_19u_ptr[6]                         = {0xFDB7, 0x6FFF, 0xFFFE, 0x0DBF, 0x0FFB, 0x1FF7};

OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_19u_0[16]) = {0U,  6U, 12U, 0U, 5U,  11U, 0U, 4U,
                                                                    10U, 0U, 3U,  9U, 15U, 2U,  8U, 14U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_19u_1[16]) = {19U, 25U, 31U, 18U, 24U, 30U, 17U, 23U,
                                                                    29U, 16U, 22U, 28U, 0U,  21U, 27U, 0U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_19u_2[16]) = {0U, 13U, 7U,  1U,  14U, 8U,  2U,  15U,
                                                                    9U, 3U,  16U, 10U, 4U,  17U, 11U, 5U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_19u_3[16]) = {1U,  7U, 13U, 0U,  6U, 12U, 0U,  5U,
                                                                    11U, 0U, 4U,  10U, 0U, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_19u_4[16]) = {20U, 26U, 0U,  19U, 25U, 31U, 18U, 24U,
                                                                    30U, 17U, 23U, 29U, 0U,  0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_19u_5[16]) = {18U, 12U, 6U,  0U, 13U, 7U,  1U,  14U,
                                                                    8U,  2U,  15U, 9U, 3U,  0x0, 0x0, 0x0};

// ----------------------- 32u20u ------------------------------- //
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_20u_0[16]) = {0U,  2U,  0x0, 5U,  7U,  8U,  10U, 0x0,
                                                                      13U, 15U, 16U, 18U, 0x0, 21U, 23U, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_20u_1[16]) = {1U,  3U,  4U,  6U,  0x0, 9U,  11U, 12U,
                                                                      14U, 0x0, 17U, 19U, 20U, 22U, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_20u_2[16]) = {0x0, 1U,  3U,  4U,  6U,  0x0, 9U,  11U,
                                                                      12U, 14U, 0x0, 17U, 19U, 20U, 22U, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_20u_3[16]) = {8U,  10U, 0x0, 13U, 15U, 16U, 18U, 0x0,
                                                                      21U, 23U, 24U, 26U, 0x0, 29U, 31U, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_20u_4[16]) = {9U,  11U, 12U, 14U, 0x0, 17U, 19U, 20U,
                                                                      22U, 0x0, 25U, 27U, 28U, 30U, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_20u_5[16]) = {0x0, 9U,  11U, 12U, 14U, 0x0, 17U, 19U,
                                                                      20U, 22U, 0x0, 25U, 27U, 28U, 30U, 0x0};
static __mmask16 permutex_masks_20u_ptr[3]                         = {0x6F7B, 0x3DEF, 0x7BDE};

OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_20u_0[16]) = {0U, 8U,  0U, 4U, 12U, 0U, 8U,  0U,
                                                                    4U, 12U, 0U, 8U, 0U,  4U, 12U, 0U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_20u_1[16]) = {20U, 28U, 16U, 24U, 0U,  20U, 28U, 16U,
                                                                    24U, 0U,  20U, 28U, 16U, 24U, 0U,  0U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_20u_2[16]) = {0U,  12U, 4U, 16U, 8U, 0U,  12U, 4U,
                                                                    16U, 8U,  0U, 12U, 4U, 16U, 8U,  0U};

// ----------------------- 32u21u ------------------------------- //
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_21u_0[16]) = {0U,  2U,  0x0, 5U,  0x0, 8U,  0x0, 11U,
                                                                      0x0, 14U, 0x0, 17U, 19U, 20U, 22U, 23U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_21u_1[16]) = {1U,  3U,  4U,  6U,  7U,  9U,  10U, 12U,
                                                                      13U, 15U, 16U, 18U, 0x0, 21U, 0x0, 24U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_21u_2[16]) = {0x0, 1U,  3U,  4U,  6U,  7U,  9U,  10U,
                                                                      12U, 13U, 15U, 16U, 18U, 19U, 21U, 22U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_21u_3[16]) = {9U,  10U, 12U, 13U, 15U, 16U, 18U, 0x0,
                                                                      21U, 0x0, 24U, 0x0, 27U, 0x0, 30U, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_21u_4[16]) = {0x0, 11U, 0x0, 14U, 0x0, 17U, 19U, 20U,
                                                                      22U, 23U, 25U, 26U, 28U, 29U, 31U, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_21u_5[16]) = {8U,  9U,  11U, 12U, 14U, 0x0, 17U, 19U,
                                                                      20U, 22U, 23U, 25U, 26U, 28U, 29U, 31U};
static __mmask16 permutex_masks_21u_ptr[6]                         = {0xFAAB, 0xAFFF, 0xFFFE, 0x557F, 0x7FEA, 0xFFDF};

OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_21u_0[16]) = {0U, 10U, 0U, 9U, 0U,  8U, 0U,  7U,
                                                                    0U, 6U,  0U, 5U, 15U, 4U, 14U, 3U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_21u_1[16]) = {21U, 31U, 20U, 30U, 19U, 29U, 18U, 28U,
                                                                    17U, 27U, 16U, 26U, 0U,  25U, 0U,  24U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_21u_2[16]) = {0U, 11U, 1U, 12U, 2U, 13U, 3U, 14U,
                                                                    4U, 15U, 5U, 16U, 6U, 17U, 7U, 18U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_21u_3[16]) = {13U, 2U, 12U, 1U, 11U, 0U, 10U, 0U,
                                                                    9U,  0U, 8U,  0U, 7U,  0U, 6U,  0U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_21u_4[16]) = {0U,  23U, 0U,  22U, 0U,  21U, 31U, 20U,
                                                                    30U, 19U, 29U, 18U, 28U, 17U, 27U, 0U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_21u_5[16]) = {8U,  19U, 9U,  20U, 10U, 0U, 11U, 1U,
                                                                    12U, 2U,  13U, 3U,  14U, 4U, 15U, 5U};

// ----------------------- 32u22u ------------------------------- //
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_22u_0[16]) = {0U,  2U,  3U,  5U,  6U,  0x0, 9U,  0x0,
                                                                      12U, 0x0, 15U, 16U, 18U, 19U, 21U, 22U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_22u_1[16]) = {1U,  0x0, 4U,  0x0, 7U,  8U,  10U, 11U,
                                                                      13U, 14U, 0x0, 17U, 0x0, 20U, 0x0, 23U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_22u_2[16]) = {0x0, 1U,  2U,  4U,  5U,  7U,  8U,  10U,
                                                                      11U, 13U, 14U, 0x0, 17U, 18U, 20U, 21U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_22u_3[16]) = {0x0, 9U,  0x0, 12U, 0x0, 15U, 16U, 18U,
                                                                      19U, 21U, 22U, 0x0, 25U, 0x0, 28U, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_22u_4[16]) = {8U,  10U, 11U, 13U, 14U, 0x0, 17U, 0x0,
                                                                      20U, 0x0, 23U, 24U, 26U, 27U, 29U, 30U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_22u_5[16]) = {7U,  8U,  10U, 11U, 13U, 14U, 0x0, 17U,
                                                                      18U, 20U, 21U, 23U, 24U, 26U, 27U, 29U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_22u_6[16]) = {15U, 16U, 18U, 19U, 21U, 22U, 0x0, 25U,
                                                                      0x0, 28U, 0x0, 31U, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_22u_7[16]) = {0x0, 17U, 0x0, 20U, 0x0, 23U, 24U, 26U,
                                                                      27U, 29U, 30U, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_22u_8[16]) = {14U, 0x0, 17U, 18U, 20U, 21U, 23U, 24U,
                                                                      26U, 27U, 29U, 30U, 0x0, 0x0, 0x0, 0x0};
static __mmask16 permutex_masks_22u_ptr[9] = {0xFD5F, 0xABF5, 0xF7FE, 0x57EA, 0xFD5F, 0xFFBF, 0x0ABF, 0x07EA, 0x0FFD};

OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_22u_0[16]) = {0U, 12U, 2U,  14U, 4U,  0U, 6U,  0U,
                                                                    8U, 9U,  10U, 0U,  12U, 2U, 14U, 4U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_22u_1[16]) = {22U, 0U,  24U, 0U,  26U, 16U, 28U, 18U,
                                                                    30U, 20U, 0U,  22U, 0U,  24U, 0U,  26U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_22u_2[16]) = {0U,  10U, 20U, 8U, 18U, 6U,  16U, 4U,
                                                                    14U, 2U,  12U, 0U, 10U, 20U, 8U,  18U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_22u_3[16]) = {0U, 6U,  0U, 8U, 9U, 10U, 0U, 12U,
                                                                    2U, 14U, 4U, 0U, 6U, 0U,  8U, 9U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_22u_4[16]) = {16U, 28U, 18U, 30U, 20U, 0U,  22U, 0U,
                                                                    24U, 0U,  26U, 16U, 28U, 18U, 30U, 20U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_22u_5[16]) = {6U,  16U, 4U,  14U, 2U,  12U, 0U,  10U,
                                                                    20U, 8U,  18U, 6U,  16U, 4U,  14U, 2U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_22u_6[16]) = {10U, 0U, 12U, 2U,  14U, 4U,  0U,  6U,
                                                                    0U,  8U, 9U,  10U, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_22u_7[16]) = {0U,  22U, 0U,  24U, 0U,  26U, 16U, 28U,
                                                                    18U, 30U, 20U, 0U,  0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_22u_8[16]) = {12U, 0U,  10U, 20U, 8U,  18U, 6U,  16U,
                                                                    4U,  14U, 2U,  12U, 0x0, 0x0, 0x0, 0x0};

// ----------------------- 32u23u ------------------------------- //
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_23u_0[16]) = {0U,  2U,  3U,  0x0, 6U,  7U,  9U,  10U,
                                                                      0x0, 13U, 14U, 0x0, 17U, 0x0, 20U, 21U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_23u_1[16]) = {1U,  0x0, 4U,  5U,  0x0, 8U,  0x0, 11U,
                                                                      12U, 0x0, 15U, 16U, 18U, 19U, 0x0, 22U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_23u_2[16]) = {0x0, 1U,  2U,  4U,  5U,  6U,  8U,  9U,
                                                                      11U, 12U, 13U, 15U, 16U, 18U, 19U, 20U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_23u_3[16]) = {0x0, 8U,  0x0, 11U, 12U, 0x0, 15U, 16U,
                                                                      18U, 19U, 0x0, 22U, 23U, 25U, 26U, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_23u_4[16]) = {7U,  9U,  10U, 0x0, 13U, 14U, 0x0, 17U,
                                                                      0x0, 20U, 21U, 0x0, 24U, 0x0, 27U, 28U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_23u_5[16]) = {6U,  7U,  9U,  10U, 11U, 13U, 14U, 0x0,
                                                                      17U, 18U, 20U, 21U, 22U, 24U, 25U, 27U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_23u_6[16]) = {13U, 14U, 0x0, 17U, 0x0, 20U, 21U, 0x0,
                                                                      24U, 0x0, 27U, 28U, 0x0, 31U, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_23u_7[16]) = {0x0, 15U, 16U, 18U, 19U, 0x0, 22U, 23U,
                                                                      25U, 26U, 0x0, 29U, 30U, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_23u_8[16]) = {12U, 13U, 15U, 16U, 18U, 19U, 20U, 22U,
                                                                      23U, 25U, 26U, 27U, 29U, 30U, 0x0, 0x0};
static __mmask16 permutex_masks_23u_ptr[9] = {0xD6F7, 0xBDAD, 0xFFFE, 0x7BDA, 0xD6B7, 0xFF7F, 0x2D6B, 0x1BDE, 0x3FFF};

OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_23u_0[16]) = {0U, 14U, 5U, 0U, 10U, 1U, 15U, 6U,
                                                                    0U, 11U, 2U, 0U, 7U,  0U, 12U, 3U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_23u_1[16]) = {23U, 0U, 28U, 19U, 0U,  24U, 0U, 29U,
                                                                    20U, 0U, 25U, 16U, 30U, 21U, 0U, 26U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_23u_2[16]) = {0U, 9U,  18U, 4U, 13U, 22U, 8U,  17U,
                                                                    3U, 12U, 21U, 7U, 16U, 2U,  11U, 20U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_23u_3[16]) = {0U,  8U, 0U, 13U, 4U, 0U,  9U, 0U,
                                                                    14U, 5U, 0U, 10U, 1U, 15U, 6U, 0U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_23u_4[16]) = {17U, 31U, 22U, 0U, 27U, 18U, 0U,  23U,
                                                                    0U,  28U, 19U, 0U, 24U, 0U,  29U, 20U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_23u_5[16]) = {6U, 15U, 1U, 10U, 19U, 5U, 14U, 0U,
                                                                    9U, 18U, 4U, 13U, 22U, 8U, 17U, 3U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_23u_6[16]) = {11U, 2U, 0U,  7U, 0U, 12U, 3U,  0U,
                                                                    8U,  0U, 13U, 4U, 0U, 9U,  0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_23u_7[16]) = {0U,  25U, 16U, 30U, 21U, 0U, 26U, 17U,
                                                                    31U, 22U, 0U,  27U, 18U, 0U, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_23u_8[16]) = {12U, 21U, 7U,  16U, 2U, 11U, 20U, 6U,
                                                                    15U, 1U,  10U, 19U, 5U, 14U, 0x0, 0x0};

// ----------------------- 32u24u ------------------------------- //
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_24u_0[16]) = {1U,  2U,  3U,  5U,  6U,  7U,  9U,  10U,
                                                                      11U, 13U, 14U, 15U, 17U, 18U, 19U, 21U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_24u_1[16]) = {0U,  1U,  2U,  4U,  5U,  6U,  8U,  9U,
                                                                      10U, 12U, 13U, 14U, 16U, 17U, 18U, 20U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_24u_2[16]) = {6U,  7U,  9U,  10U, 11U, 13U, 14U, 15U,
                                                                      17U, 18U, 19U, 21U, 22U, 23U, 25U, 26U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_24u_3[16]) = {5U,  6U,  8U,  9U,  10U, 12U, 13U, 14U,
                                                                      16U, 17U, 18U, 20U, 21U, 22U, 24U, 25U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_24u_4[16]) = {11U, 13U, 14U, 15U, 17U, 18U, 19U, 21U,
                                                                      22U, 23U, 25U, 26U, 27U, 29U, 30U, 31U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_24u_5[16]) = {10U, 12U, 13U, 14U, 16U, 17U, 18U, 20U,
                                                                      21U, 22U, 24U, 25U, 26U, 28U, 29U, 30U};

OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_24u_0[16]) = {24U, 16U, 8U,  24U, 16U, 8U,  24U, 16U,
                                                                    8U,  24U, 16U, 8U,  24U, 16U, 8U,  24U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_24u_1[16]) = {0U,  8U, 16U, 0U,  8U, 16U, 0U,  8U,
                                                                    16U, 0U, 8U,  16U, 0U, 8U,  16U, 0U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_24u_2[16]) = {16U, 8U,  24U, 16U, 8U,  24U, 16U, 8U,
                                                                    24U, 16U, 8U,  24U, 16U, 8U,  24U, 16U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_24u_3[16]) = {8U, 16U, 0U,  8U, 16U, 0U,  8U, 16U,
                                                                    0U, 8U,  16U, 0U, 8U,  16U, 0U, 8U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_24u_4[16]) = {8U,  24U, 16U, 8U,  24U, 16U, 8U,  24U,
                                                                    16U, 8U,  24U, 16U, 8U,  24U, 16U, 8U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_24u_5[16]) = {16U, 0U,  8U, 16U, 0U,  8U, 16U, 0U,
                                                                    8U,  16U, 0U, 8U,  16U, 0U, 8U,  16U};

// ----------------------- 32u25u ------------------------------- //
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_25u_0[16]) = {0U,  0x0, 3U,  4U,  0x0, 7U,  8U,  9U,
                                                                      0x0, 12U, 13U, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_25u_1[16]) = {1U,  2U,  0x0, 5U,  6U,  0x0, 0x0, 10U,
                                                                      11U, 0x0, 14U, 15U, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_25u_2[16]) = {0x0, 1U,  2U,  3U,  5U,  6U,  7U,  8U,
                                                                      10U, 11U, 12U, 14U, 15U, 0x0, 0x0, 0x0};
static __mmask16 permutex_masks_25u_ptr[3]                         = {0x06ED, 0x0D9B, 0x1FFE};

OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_25u_0[16]) = {0U, 0U,  11U, 4U, 0U, 15U, 8U, 1U,
                                                                    0U, 12U, 5U,  0U, 0U, 0U,  0U, 0U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_25u_1[16]) = {25U, 18U, 0U,  29U, 22U, 0U, 0U, 26U,
                                                                    19U, 0U,  30U, 23U, 0U,  0U, 0U, 0U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_25u_2[16]) = {0U, 7U,  14U, 21U, 3U, 10U, 17U, 24U,
                                                                    6U, 13U, 20U, 2U,  9U, 0U,  0U,  0U};

// ----------------------- 32u26u ------------------------------- //
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_26u_0[16]) = {0U,  0x0, 3U,  4U,  5U,  0x0, 0x0, 9U,
                                                                      10U, 0x0, 0x0, 14U, 15U, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_26u_1[16]) = {1U,  2U,  0x0, 0x0, 6U,  7U,  8U,  0x0,
                                                                      11U, 12U, 13U, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_26u_2[16]) = {0x0, 1U,  2U,  3U,  4U,  6U,  7U,  8U,
                                                                      9U,  11U, 12U, 13U, 14U, 0x0, 0x0, 0x0};
static __mmask16 permutex_masks_26u_ptr[3]                         = {0x199D, 0x0773, 0x1FFE};

OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_26u_0[16]) = {0U, 0U, 14U, 8U,  2U, 0U,  0U,  10U,
                                                                    4U, 0U, 0U,  12U, 6U, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_26u_1[16]) = {26U, 20U, 0U,  0U, 28U, 22U, 16U, 0U,
                                                                    30U, 24U, 18U, 0U, 0U,  0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_26u_2[16]) = {0U,  6U, 12U, 18U, 24U, 4U,  10U, 16U,
                                                                    22U, 2U, 8U,  14U, 20U, 0x0, 0x0, 0x0};

// ----------------------- 32u27u ------------------------------- //
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_27u_0[16]) = {0U,  0x0, 0x0, 4U,  5U,  6U,  0x0, 0x0,
                                                                      10U, 11U, 12U, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_27u_1[16]) = {1U,  2U,  3U,  0x0, 0x0, 7U,  8U,  9U,
                                                                      0x0, 0x0, 13U, 14U, 15U, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_27u_2[16]) = {0x0, 1U,  2U,  3U,  4U,  5U,  7U,  8U,
                                                                      9U,  10U, 11U, 13U, 14U, 15U, 0x0, 0x0};
static __mmask16 permutex_masks_27u_ptr[3]                         = {0x0739, 0x1CE7, 0x3FFE};

OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_27u_0[16]) = {0U,  0U, 0U, 12U, 7U, 2U, 0U,  0U,
                                                                    14U, 9U, 4U, 0U,  0U, 0U, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_27u_1[16]) = {27U, 22U, 17U, 0U,  0U,  29U, 24U, 19U,
                                                                    0U,  0U,  31U, 26U, 21U, 0U,  0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_27u_2[16]) = {0U,  5U,  10U, 15U, 20U, 25U, 3U,  8U,
                                                                    13U, 18U, 23U, 1U,  6U,  11U, 0x0, 0x0};

// ----------------------- 32u28u ------------------------------- //
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_28u_0[16]) = {1U,  2U,  3U,  4U,  5U,  6U,  7U,  9U,
                                                                      10U, 11U, 12U, 13U, 14U, 15U, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_28u_1[16]) = {0U, 1U,  2U,  3U,  4U,  5U,  6U,  8U,
                                                                      9U, 10U, 11U, 12U, 13U, 14U, 0x0, 0x0};
static __mmask16 permutex_masks_28u_ptr[2]                         = {0x3FFF, 0x3FFF};

OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_28u_0[16]) = {28U, 24U, 20U, 16U, 12U, 8U, 4U,  28U,
                                                                    24U, 20U, 16U, 12U, 8U,  4U, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_28u_1[16]) = {0U, 4U, 8U,  12U, 16U, 20U, 24U, 0U,
                                                                    4U, 8U, 12U, 16U, 20U, 24U, 0x0, 0x0};

// ----------------------- 32u29u ------------------------------- //
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_29u_0[16]) = {0U, 0x0, 0x0, 0x0, 0x0, 6U,  7U,  8U,
                                                                      9U, 10U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_29u_1[16]) = {1U,  2U,  3U,  4U,  5U,  0x0, 0x0, 0x0,
                                                                      0x0, 11U, 12U, 13U, 14U, 15U, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_29u_2[16]) = {0x0, 1U, 2U,  3U,  4U,  5U,  6U,  7U,
                                                                      8U,  9U, 11U, 12U, 13U, 14U, 15U, 0x0};
static __mmask16 permutex_masks_29u_ptr[3]                         = {0x03E1, 0x3E1F, 0x7FFE};

OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_29u_0[16]) = {0U, 0U, 0U, 0U, 0U, 14U, 11U, 8U,
                                                                    5U, 2U, 0U, 0U, 0U, 0U,  0U,  0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_29u_1[16]) = {29U, 26U, 23U, 20U, 17U, 0U,  0U, 0U,
                                                                    0U,  31U, 28U, 25U, 22U, 19U, 0U, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_29u_2[16]) = {0U,  3U,  6U, 9U, 12U, 15U, 18U, 21U,
                                                                    24U, 27U, 1U, 4U, 7U,  10U, 13U, 0x0};

// ----------------------- 32u30u ------------------------------- //
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_30u_0[16]) = {1U, 2U,  3U,  4U,  5U,  6U,  7U,  8U,
                                                                      9U, 10U, 11U, 12U, 13U, 14U, 15U, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_30u_1[16]) = {0U, 1U, 2U,  3U,  4U,  5U,  6U,  7U,
                                                                      8U, 9U, 10U, 11U, 12U, 13U, 14U, 0x0};
static __mmask16 permutex_masks_30u_ptr[2]                         = {0x7FFF, 0x7FFF};

OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_30u_0[16]) = {30U, 28U, 26U, 24U, 22U, 20U, 18U, 16U,
                                                                    14U, 12U, 10U, 8U,  6U,  4U,  2U,  0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_30u_1[16]) = {0U,  2U,  4U,  6U,  8U,  10U, 12U, 14U,
                                                                    16U, 18U, 20U, 22U, 24U, 26U, 28U, 0x0};

// ----------------------- 32u31u ------------------------------- //
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_31u_0[16]) = {1U, 2U,  3U,  4U,  5U,  6U,  7U,  8U,
                                                                      9U, 10U, 11U, 12U, 13U, 14U, 15U, 0x0};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_31u_1[16]) = {0U, 1U, 2U,  3U,  4U,  5U,  6U,  7U,
                                                                      8U, 9U, 10U, 11U, 12U, 13U, 14U, 15U};
static __mmask16 permutex_masks_31u_ptr[2]                         = {0x7FFF, 0xFFFF};

OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_31u_0[16]) = {31U, 30U, 29U, 28U, 27U, 26U, 25U, 24U,
                                                                    23U, 22U, 21U, 20U, 19U, 18U, 17U, 0U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_mask_table_31u_1[16]) = {0U, 1U, 2U,  3U,  4U,  5U,  6U,  7U,
                                                                    8U, 9U, 10U, 11U, 12U, 13U, 14U, 15U};

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

// ********************** 32u17u ****************************** //

OWN_QPLC_INLINE(void, px_qplc_pack_32u17u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 17U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;
    uint64_t  src         = (uint64_t)(*dst_32u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint64_t)(*src_32u_ptr)) << start_bit;
    src_32u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_DWORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_32u_ptr = (uint32_t)(src);
            dst_32u_ptr++;
            src = src >> OWN_DWORD_WIDTH;
            bits_in_buf -= OWN_DWORD_WIDTH;
        }
        src = src | (((uint64_t)(*src_32u_ptr)) << bits_in_buf);
        src_32u_ptr++;
        num_elements--;
        bits_in_buf += (int32_t)bit_width;
    }
    dst_ptr = (uint8_t*)dst_32u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_QPLC_INLINE(void, k0_qplc_pack_32u17u_tail, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    __m512i srcmm0;
    __m512i zmm0, zmm1, zmm2;

    __mmask16 tail_mask  = OWN_BIT_MASK(num_elements);
    __mmask64 store_mask = OWN_BIT_MASK(OWN_BITS_2_BYTE(num_elements * 17U));

    __m512i permutex_idx_ptr[3];
    permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_17u_0);
    permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_17u_1);
    permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_17u_2);

    __m512i shift_masks_ptr[3];
    shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_17u_0);
    shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_17u_1);
    shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_17u_2);

    srcmm0 = _mm512_maskz_loadu_epi32(tail_mask, src_ptr);
    zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_17u_ptr[0], permutex_idx_ptr[0], srcmm0);
    zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_17u_ptr[1], permutex_idx_ptr[1], srcmm0);
    zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_17u_ptr[2], permutex_idx_ptr[2], srcmm0);

    zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
    zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
    zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

    zmm0 = _mm512_or_si512(zmm0, zmm1);
    zmm0 = _mm512_or_si512(zmm0, zmm2);
    _mm512_mask_storeu_epi8(dst_ptr, store_mask, zmm0);
}

OWN_OPT_FUN(void, k0_qplc_pack_32u17u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 17U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_pack_32u17u(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 4;
        dst_ptr += ((align * 17U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 16U;
    if (num_elements >= 16U) {
        uint32_t num_elements_48 = num_elements / 48U;
        uint32_t num_elements_16 = (num_elements % 48U) / 16U;
        __m512i  srcmm0, srcmm1, srcmm2;
        __m512i  zmm0, zmm1, zmm2, zmm3, zmm4, zmm5;

        __m512i permutex_idx_ptr[6];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_17u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_17u_1);
        permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_17u_2);
        permutex_idx_ptr[3] = _mm512_load_si512(permutex_idx_table_17u_3);
        permutex_idx_ptr[4] = _mm512_load_si512(permutex_idx_table_17u_4);
        permutex_idx_ptr[5] = _mm512_load_si512(permutex_idx_table_17u_5);

        __m512i shift_masks_ptr[6];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_17u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_17u_1);
        shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_17u_2);
        shift_masks_ptr[3] = _mm512_load_si512(shift_mask_table_17u_3);
        shift_masks_ptr[4] = _mm512_load_si512(shift_mask_table_17u_4);
        shift_masks_ptr[5] = _mm512_load_si512(shift_mask_table_17u_5);

        for (uint32_t idx = 0; idx < num_elements_48; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            srcmm1 = _mm512_loadu_si512(src_ptr + 64U);
            srcmm2 = _mm512_loadu_si512(src_ptr + 128U);

            zmm0 = _mm512_maskz_permutex2var_epi32(permutex_masks_17u_ptr[0], srcmm0, permutex_idx_ptr[0], srcmm1);
            zmm1 = _mm512_maskz_permutex2var_epi32(permutex_masks_17u_ptr[1], srcmm0, permutex_idx_ptr[1], srcmm1);
            zmm2 = _mm512_maskz_permutex2var_epi32(permutex_masks_17u_ptr[2], srcmm0, permutex_idx_ptr[2], srcmm1);
            zmm3 = _mm512_maskz_permutex2var_epi32(permutex_masks_17u_ptr[3], srcmm1, permutex_idx_ptr[3], srcmm2);
            zmm4 = _mm512_maskz_permutex2var_epi32(permutex_masks_17u_ptr[4], srcmm1, permutex_idx_ptr[4], srcmm2);
            zmm5 = _mm512_maskz_permutex2var_epi32(permutex_masks_17u_ptr[5], srcmm1, permutex_idx_ptr[5], srcmm2);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);
            zmm3 = _mm512_sllv_epi32(zmm3, shift_masks_ptr[3]);
            zmm4 = _mm512_sllv_epi32(zmm4, shift_masks_ptr[4]);
            zmm5 = _mm512_srlv_epi32(zmm5, shift_masks_ptr[5]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            zmm3 = _mm512_or_si512(zmm3, zmm4);
            zmm3 = _mm512_or_si512(zmm3, zmm5);

            _mm512_storeu_si512(dst_ptr, zmm0);
            _mm512_mask_storeu_epi16(dst_ptr + 64U, 0x0007FFFF, zmm3);

            src_ptr += 192U;
            dst_ptr += 17U * 6U;
        }

        for (uint32_t idx = 0; idx < num_elements_16; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_17u_ptr[0], permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_17u_ptr[1], permutex_idx_ptr[1], srcmm0);
            zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_17u_ptr[2], permutex_idx_ptr[2], srcmm0);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            _mm512_mask_storeu_epi16(dst_ptr, 0x0001FFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 17U * 2U;
        }
    }

    if (tail > 0) { k0_qplc_pack_32u17u_tail(src_ptr, tail, dst_ptr); }
}

// ********************** 32u18u ****************************** //

OWN_QPLC_INLINE(void, px_qplc_pack_32u18u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 18U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;
    uint64_t  src         = (uint64_t)(*dst_32u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint64_t)(*src_32u_ptr)) << start_bit;
    src_32u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_DWORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_32u_ptr = (uint32_t)(src);
            dst_32u_ptr++;
            src = src >> OWN_DWORD_WIDTH;
            bits_in_buf -= OWN_DWORD_WIDTH;
        }
        src = src | (((uint64_t)(*src_32u_ptr)) << bits_in_buf);
        src_32u_ptr++;
        num_elements--;
        bits_in_buf += (int32_t)bit_width;
    }
    dst_ptr = (uint8_t*)dst_32u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_QPLC_INLINE(void, k0_qplc_pack_32u18u_tail, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    __m512i srcmm0;
    __m512i zmm0, zmm1, zmm2;

    __mmask16 tail_mask  = OWN_BIT_MASK(num_elements);
    __mmask64 store_mask = OWN_BIT_MASK(OWN_BITS_2_BYTE(num_elements * 18U));

    __m512i permutex_idx_ptr[3];
    permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_18u_0);
    permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_18u_1);
    permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_18u_2);

    __m512i shift_masks_ptr[3];
    shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_18u_0);
    shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_18u_1);
    shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_18u_2);

    srcmm0 = _mm512_maskz_loadu_epi32(tail_mask, src_ptr);
    zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_18u_ptr[0], permutex_idx_ptr[0], srcmm0);
    zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_18u_ptr[1], permutex_idx_ptr[1], srcmm0);
    zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_18u_ptr[2], permutex_idx_ptr[2], srcmm0);

    zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
    zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
    zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

    zmm0 = _mm512_or_si512(zmm0, zmm1);
    zmm0 = _mm512_or_si512(zmm0, zmm2);
    _mm512_mask_storeu_epi8(dst_ptr, store_mask, zmm0);
}

OWN_OPT_FUN(void, k0_qplc_pack_32u18u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 18U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_pack_32u18u(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 4;
        dst_ptr += ((align * 18U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 16U;
    if (num_elements >= 16U) {
        uint32_t num_elements_48 = num_elements / 48U;
        uint32_t num_elements_16 = (num_elements % 48U) / 16U;
        __m512i  srcmm0, srcmm1, srcmm2;
        __m512i  zmm0, zmm1, zmm2, zmm3, zmm4, zmm5;

        __m512i permutex_idx_ptr[6];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_18u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_18u_1);
        permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_18u_2);
        permutex_idx_ptr[3] = _mm512_load_si512(permutex_idx_table_18u_3);
        permutex_idx_ptr[4] = _mm512_load_si512(permutex_idx_table_18u_4);
        permutex_idx_ptr[5] = _mm512_load_si512(permutex_idx_table_18u_5);

        __m512i shift_masks_ptr[6];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_18u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_18u_1);
        shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_18u_2);
        shift_masks_ptr[3] = _mm512_load_si512(shift_mask_table_18u_3);
        shift_masks_ptr[4] = _mm512_load_si512(shift_mask_table_18u_4);
        shift_masks_ptr[5] = _mm512_load_si512(shift_mask_table_18u_5);

        for (uint32_t idx = 0; idx < num_elements_48; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            srcmm1 = _mm512_loadu_si512(src_ptr + 64U);
            srcmm2 = _mm512_loadu_si512(src_ptr + 128U);

            zmm0 = _mm512_maskz_permutex2var_epi32(permutex_masks_18u_ptr[0], srcmm0, permutex_idx_ptr[0], srcmm1);
            zmm1 = _mm512_maskz_permutex2var_epi32(permutex_masks_18u_ptr[1], srcmm0, permutex_idx_ptr[1], srcmm1);
            zmm2 = _mm512_maskz_permutex2var_epi32(permutex_masks_18u_ptr[2], srcmm0, permutex_idx_ptr[2], srcmm1);
            zmm3 = _mm512_maskz_permutex2var_epi32(permutex_masks_18u_ptr[3], srcmm1, permutex_idx_ptr[3], srcmm2);
            zmm4 = _mm512_maskz_permutex2var_epi32(permutex_masks_18u_ptr[4], srcmm1, permutex_idx_ptr[4], srcmm2);
            zmm5 = _mm512_maskz_permutex2var_epi32(permutex_masks_18u_ptr[5], srcmm1, permutex_idx_ptr[5], srcmm2);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);
            zmm3 = _mm512_sllv_epi32(zmm3, shift_masks_ptr[3]);
            zmm4 = _mm512_sllv_epi32(zmm4, shift_masks_ptr[4]);
            zmm5 = _mm512_srlv_epi32(zmm5, shift_masks_ptr[5]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            zmm3 = _mm512_or_si512(zmm3, zmm4);
            zmm3 = _mm512_or_si512(zmm3, zmm5);

            _mm512_storeu_si512(dst_ptr, zmm0);
            _mm512_mask_storeu_epi16(dst_ptr + 64U, 0x003FFFFF, zmm3);

            src_ptr += 192U;
            dst_ptr += 18U * 6U;
        }

        for (uint32_t idx = 0; idx < num_elements_16; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_18u_ptr[0], permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_18u_ptr[1], permutex_idx_ptr[1], srcmm0);
            zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_18u_ptr[2], permutex_idx_ptr[2], srcmm0);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            _mm512_mask_storeu_epi16(dst_ptr, 0x0003FFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 18U * 2U;
        }
    }

    if (tail > 0) { k0_qplc_pack_32u18u_tail(src_ptr, tail, dst_ptr); }
}

// ********************** 32u19u ****************************** //

OWN_QPLC_INLINE(void, px_qplc_pack_32u19u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 19U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;
    uint64_t  src         = (uint64_t)(*dst_32u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint64_t)(*src_32u_ptr)) << start_bit;
    src_32u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_DWORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_32u_ptr = (uint32_t)(src);
            dst_32u_ptr++;
            src = src >> OWN_DWORD_WIDTH;
            bits_in_buf -= OWN_DWORD_WIDTH;
        }
        src = src | (((uint64_t)(*src_32u_ptr)) << bits_in_buf);
        src_32u_ptr++;
        num_elements--;
        bits_in_buf += (int32_t)bit_width;
    }
    dst_ptr = (uint8_t*)dst_32u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_QPLC_INLINE(void, k0_qplc_pack_32u19u_tail, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    __m512i srcmm0;
    __m512i zmm0, zmm1, zmm2;

    __mmask16 tail_mask  = OWN_BIT_MASK(num_elements);
    __mmask64 store_mask = OWN_BIT_MASK(OWN_BITS_2_BYTE(num_elements * 19U));

    __m512i permutex_idx_ptr[3];
    permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_19u_0);
    permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_19u_1);
    permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_19u_2);

    __m512i shift_masks_ptr[3];
    shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_19u_0);
    shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_19u_1);
    shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_19u_2);

    srcmm0 = _mm512_maskz_loadu_epi32(tail_mask, src_ptr);
    zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_19u_ptr[0], permutex_idx_ptr[0], srcmm0);
    zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_19u_ptr[1], permutex_idx_ptr[1], srcmm0);
    zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_19u_ptr[2], permutex_idx_ptr[2], srcmm0);

    zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
    zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
    zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

    zmm0 = _mm512_or_si512(zmm0, zmm1);
    zmm0 = _mm512_or_si512(zmm0, zmm2);
    _mm512_mask_storeu_epi8(dst_ptr, store_mask, zmm0);
}

OWN_OPT_FUN(void, k0_qplc_pack_32u19u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 19U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_pack_32u19u(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 4;
        dst_ptr += ((align * 19U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 16U;
    if (num_elements >= 16U) {
        uint32_t num_elements_48 = num_elements / 48U;
        uint32_t num_elements_16 = (num_elements % 48U) / 16U;
        __m512i  srcmm0, srcmm1, srcmm2;
        __m512i  zmm0, zmm1, zmm2, zmm3, zmm4, zmm5;

        __m512i permutex_idx_ptr[6];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_19u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_19u_1);
        permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_19u_2);
        permutex_idx_ptr[3] = _mm512_load_si512(permutex_idx_table_19u_3);
        permutex_idx_ptr[4] = _mm512_load_si512(permutex_idx_table_19u_4);
        permutex_idx_ptr[5] = _mm512_load_si512(permutex_idx_table_19u_5);

        __m512i shift_masks_ptr[6];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_19u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_19u_1);
        shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_19u_2);
        shift_masks_ptr[3] = _mm512_load_si512(shift_mask_table_19u_3);
        shift_masks_ptr[4] = _mm512_load_si512(shift_mask_table_19u_4);
        shift_masks_ptr[5] = _mm512_load_si512(shift_mask_table_19u_5);

        for (uint32_t idx = 0; idx < num_elements_48; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            srcmm1 = _mm512_loadu_si512(src_ptr + 64U);
            srcmm2 = _mm512_loadu_si512(src_ptr + 128U);

            zmm0 = _mm512_maskz_permutex2var_epi32(permutex_masks_19u_ptr[0], srcmm0, permutex_idx_ptr[0], srcmm1);
            zmm1 = _mm512_maskz_permutex2var_epi32(permutex_masks_19u_ptr[1], srcmm0, permutex_idx_ptr[1], srcmm1);
            zmm2 = _mm512_maskz_permutex2var_epi32(permutex_masks_19u_ptr[2], srcmm0, permutex_idx_ptr[2], srcmm1);
            zmm3 = _mm512_maskz_permutex2var_epi32(permutex_masks_19u_ptr[3], srcmm1, permutex_idx_ptr[3], srcmm2);
            zmm4 = _mm512_maskz_permutex2var_epi32(permutex_masks_19u_ptr[4], srcmm1, permutex_idx_ptr[4], srcmm2);
            zmm5 = _mm512_maskz_permutex2var_epi32(permutex_masks_19u_ptr[5], srcmm1, permutex_idx_ptr[5], srcmm2);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);
            zmm3 = _mm512_sllv_epi32(zmm3, shift_masks_ptr[3]);
            zmm4 = _mm512_sllv_epi32(zmm4, shift_masks_ptr[4]);
            zmm5 = _mm512_srlv_epi32(zmm5, shift_masks_ptr[5]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            zmm3 = _mm512_or_si512(zmm3, zmm4);
            zmm3 = _mm512_or_si512(zmm3, zmm5);

            _mm512_storeu_si512(dst_ptr, zmm0);
            _mm512_mask_storeu_epi16(dst_ptr + 64U, 0x01FFFFFF, zmm3);

            src_ptr += 192U;
            dst_ptr += 19U * 6U;
        }

        for (uint32_t idx = 0; idx < num_elements_16; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_19u_ptr[0], permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_19u_ptr[1], permutex_idx_ptr[1], srcmm0);
            zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_19u_ptr[2], permutex_idx_ptr[2], srcmm0);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            _mm512_mask_storeu_epi16(dst_ptr, 0x0007FFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 19U * 2U;
        }
    }

    if (tail > 0) { k0_qplc_pack_32u19u_tail(src_ptr, tail, dst_ptr); }
}

// ********************** 32u20u ****************************** //

OWN_QPLC_INLINE(void, px_qplc_pack_32u20u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 20U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;
    uint64_t  src         = (uint64_t)(*dst_32u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint64_t)(*src_32u_ptr)) << start_bit;
    src_32u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_DWORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_32u_ptr = (uint32_t)(src);
            dst_32u_ptr++;
            src = src >> OWN_DWORD_WIDTH;
            bits_in_buf -= OWN_DWORD_WIDTH;
        }
        src = src | (((uint64_t)(*src_32u_ptr)) << bits_in_buf);
        src_32u_ptr++;
        num_elements--;
        bits_in_buf += (int32_t)bit_width;
    }
    dst_ptr = (uint8_t*)dst_32u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_QPLC_INLINE(void, k0_qplc_pack_32u20u_tail, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    __m512i srcmm0;
    __m512i zmm0, zmm1, zmm2;

    __mmask16 tail_mask  = OWN_BIT_MASK(num_elements);
    __mmask64 store_mask = OWN_BIT_MASK(OWN_BITS_2_BYTE(num_elements * 20U));

    __m512i permutex_idx_ptr[3];
    permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_20u_0);
    permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_20u_1);
    permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_20u_2);

    __m512i shift_masks_ptr[3];
    shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_20u_0);
    shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_20u_1);
    shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_20u_2);

    srcmm0 = _mm512_maskz_loadu_epi32(tail_mask, src_ptr);
    zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_20u_ptr[0], permutex_idx_ptr[0], srcmm0);
    zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_20u_ptr[1], permutex_idx_ptr[1], srcmm0);
    zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_20u_ptr[2], permutex_idx_ptr[2], srcmm0);

    zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
    zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
    zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

    zmm0 = _mm512_or_si512(zmm0, zmm1);
    zmm0 = _mm512_or_si512(zmm0, zmm2);
    _mm512_mask_storeu_epi8(dst_ptr, store_mask, zmm0);
}

OWN_OPT_FUN(void, k0_qplc_pack_32u20u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 20U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_pack_32u20u(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 4;
        dst_ptr += ((align * 20U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 16U;
    if (num_elements >= 16U) {
        uint32_t num_elements_48 = num_elements / 48U;
        uint32_t num_elements_16 = (num_elements % 48U) / 16U;
        __m512i  srcmm0, srcmm1, srcmm2;
        __m512i  zmm0, zmm1, zmm2, zmm3, zmm4, zmm5;

        __m512i permutex_idx_ptr[6];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_20u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_20u_1);
        permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_20u_2);
        permutex_idx_ptr[3] = _mm512_load_si512(permutex_idx_table_20u_3);
        permutex_idx_ptr[4] = _mm512_load_si512(permutex_idx_table_20u_4);
        permutex_idx_ptr[5] = _mm512_load_si512(permutex_idx_table_20u_5);

        __m512i shift_masks_ptr[3];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_20u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_20u_1);
        shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_20u_2);

        for (uint32_t idx = 0; idx < num_elements_48; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            srcmm1 = _mm512_loadu_si512(src_ptr + 64U);
            srcmm2 = _mm512_loadu_si512(src_ptr + 128U);

            zmm0 = _mm512_maskz_permutex2var_epi32(permutex_masks_20u_ptr[0], srcmm0, permutex_idx_ptr[0], srcmm1);
            zmm1 = _mm512_maskz_permutex2var_epi32(permutex_masks_20u_ptr[1], srcmm0, permutex_idx_ptr[1], srcmm1);
            zmm2 = _mm512_maskz_permutex2var_epi32(permutex_masks_20u_ptr[2], srcmm0, permutex_idx_ptr[2], srcmm1);
            zmm3 = _mm512_maskz_permutex2var_epi32(permutex_masks_20u_ptr[0], srcmm1, permutex_idx_ptr[3], srcmm2);
            zmm4 = _mm512_maskz_permutex2var_epi32(permutex_masks_20u_ptr[1], srcmm1, permutex_idx_ptr[4], srcmm2);
            zmm5 = _mm512_maskz_permutex2var_epi32(permutex_masks_20u_ptr[2], srcmm1, permutex_idx_ptr[5], srcmm2);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);
            zmm3 = _mm512_sllv_epi32(zmm3, shift_masks_ptr[0]);
            zmm4 = _mm512_sllv_epi32(zmm4, shift_masks_ptr[1]);
            zmm5 = _mm512_srlv_epi32(zmm5, shift_masks_ptr[2]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            zmm3 = _mm512_or_si512(zmm3, zmm4);
            zmm3 = _mm512_or_si512(zmm3, zmm5);

            _mm512_mask_storeu_epi16(dst_ptr, 0x3FFFFFFF, zmm0);
            _mm512_mask_storeu_epi16(dst_ptr + 60U, 0x3FFFFFFF, zmm3);

            src_ptr += 192U;
            dst_ptr += 20U * 6U;
        }

        for (uint32_t idx = 0; idx < num_elements_16; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_20u_ptr[0], permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_20u_ptr[1], permutex_idx_ptr[1], srcmm0);
            zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_20u_ptr[2], permutex_idx_ptr[2], srcmm0);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            _mm512_mask_storeu_epi16(dst_ptr, 0x000FFFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 20U * 2U;
        }
    }

    if (tail > 0) { k0_qplc_pack_32u20u_tail(src_ptr, tail, dst_ptr); }
}

// ********************** 32u21u ****************************** //

OWN_QPLC_INLINE(void, px_qplc_pack_32u21u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 21U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;
    uint64_t  src         = (uint64_t)(*dst_32u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint64_t)(*src_32u_ptr)) << start_bit;
    src_32u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_DWORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_32u_ptr = (uint32_t)(src);
            dst_32u_ptr++;
            src = src >> OWN_DWORD_WIDTH;
            bits_in_buf -= OWN_DWORD_WIDTH;
        }
        src = src | (((uint64_t)(*src_32u_ptr)) << bits_in_buf);
        src_32u_ptr++;
        num_elements--;
        bits_in_buf += (int32_t)bit_width;
    }
    dst_ptr = (uint8_t*)dst_32u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_QPLC_INLINE(void, k0_qplc_pack_32u21u_tail, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    __m512i srcmm0;
    __m512i zmm0, zmm1, zmm2;

    __mmask16 tail_mask  = OWN_BIT_MASK(num_elements);
    __mmask64 store_mask = OWN_BIT_MASK(OWN_BITS_2_BYTE(num_elements * 21U));

    __m512i permutex_idx_ptr[3];
    permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_21u_0);
    permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_21u_1);
    permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_21u_2);

    __m512i shift_masks_ptr[3];
    shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_21u_0);
    shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_21u_1);
    shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_21u_2);

    srcmm0 = _mm512_maskz_loadu_epi32(tail_mask, src_ptr);
    zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_21u_ptr[0], permutex_idx_ptr[0], srcmm0);
    zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_21u_ptr[1], permutex_idx_ptr[1], srcmm0);
    zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_21u_ptr[2], permutex_idx_ptr[2], srcmm0);

    zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
    zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
    zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

    zmm0 = _mm512_or_si512(zmm0, zmm1);
    zmm0 = _mm512_or_si512(zmm0, zmm2);
    _mm512_mask_storeu_epi8(dst_ptr, store_mask, zmm0);
}

OWN_OPT_FUN(void, k0_qplc_pack_32u21u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 21U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_pack_32u21u(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 4;
        dst_ptr += ((align * 21U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 16U;
    if (num_elements >= 16U) {
        uint32_t num_elements_48 = num_elements / 48U;
        uint32_t num_elements_16 = (num_elements % 48U) / 16U;
        __m512i  srcmm0, srcmm1, srcmm2;
        __m512i  zmm0, zmm1, zmm2, zmm3, zmm4, zmm5;

        __m512i permutex_idx_ptr[6];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_21u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_21u_1);
        permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_21u_2);
        permutex_idx_ptr[3] = _mm512_load_si512(permutex_idx_table_21u_3);
        permutex_idx_ptr[4] = _mm512_load_si512(permutex_idx_table_21u_4);
        permutex_idx_ptr[5] = _mm512_load_si512(permutex_idx_table_21u_5);

        __m512i shift_masks_ptr[6];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_21u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_21u_1);
        shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_21u_2);
        shift_masks_ptr[3] = _mm512_load_si512(shift_mask_table_21u_3);
        shift_masks_ptr[4] = _mm512_load_si512(shift_mask_table_21u_4);
        shift_masks_ptr[5] = _mm512_load_si512(shift_mask_table_21u_5);

        for (uint32_t idx = 0; idx < num_elements_48; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            srcmm1 = _mm512_loadu_si512(src_ptr + 64U);
            srcmm2 = _mm512_loadu_si512(src_ptr + 128U);

            zmm0 = _mm512_maskz_permutex2var_epi32(permutex_masks_21u_ptr[0], srcmm0, permutex_idx_ptr[0], srcmm1);
            zmm1 = _mm512_maskz_permutex2var_epi32(permutex_masks_21u_ptr[1], srcmm0, permutex_idx_ptr[1], srcmm1);
            zmm2 = _mm512_maskz_permutex2var_epi32(permutex_masks_21u_ptr[2], srcmm0, permutex_idx_ptr[2], srcmm1);
            zmm3 = _mm512_maskz_permutex2var_epi32(permutex_masks_21u_ptr[3], srcmm1, permutex_idx_ptr[3], srcmm2);
            zmm4 = _mm512_maskz_permutex2var_epi32(permutex_masks_21u_ptr[4], srcmm1, permutex_idx_ptr[4], srcmm2);
            zmm5 = _mm512_maskz_permutex2var_epi32(permutex_masks_21u_ptr[5], srcmm1, permutex_idx_ptr[5], srcmm2);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);
            zmm3 = _mm512_sllv_epi32(zmm3, shift_masks_ptr[3]);
            zmm4 = _mm512_sllv_epi32(zmm4, shift_masks_ptr[4]);
            zmm5 = _mm512_srlv_epi32(zmm5, shift_masks_ptr[5]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            zmm3 = _mm512_or_si512(zmm3, zmm4);
            zmm3 = _mm512_or_si512(zmm3, zmm5);

            _mm512_storeu_si512(dst_ptr, zmm0);
            _mm512_mask_storeu_epi16(dst_ptr + 64U, 0x7FFFFFFF, zmm3);

            src_ptr += 192U;
            dst_ptr += 21U * 6U;
        }

        for (uint32_t idx = 0; idx < num_elements_16; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_21u_ptr[0], permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_21u_ptr[1], permutex_idx_ptr[1], srcmm0);
            zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_21u_ptr[2], permutex_idx_ptr[2], srcmm0);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            _mm512_mask_storeu_epi16(dst_ptr, 0x001FFFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 21U * 2U;
        }
    }

    if (tail > 0) { k0_qplc_pack_32u21u_tail(src_ptr, tail, dst_ptr); }
}

// ********************** 32u22u ****************************** //

OWN_QPLC_INLINE(void, px_qplc_pack_32u22u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 22U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;
    uint64_t  src         = (uint64_t)(*dst_32u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint64_t)(*src_32u_ptr)) << start_bit;
    src_32u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_DWORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_32u_ptr = (uint32_t)(src);
            dst_32u_ptr++;
            src = src >> OWN_DWORD_WIDTH;
            bits_in_buf -= OWN_DWORD_WIDTH;
        }
        src = src | (((uint64_t)(*src_32u_ptr)) << bits_in_buf);
        src_32u_ptr++;
        num_elements--;
        bits_in_buf += (int32_t)bit_width;
    }
    dst_ptr = (uint8_t*)dst_32u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_QPLC_INLINE(void, k0_qplc_pack_32u22u_tail, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    __m512i srcmm0;
    __m512i zmm0, zmm1, zmm2;

    __mmask16 tail_mask  = OWN_BIT_MASK(num_elements);
    __mmask64 store_mask = OWN_BIT_MASK(OWN_BITS_2_BYTE(num_elements * 22U));

    __m512i permutex_idx_ptr[3];
    permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_22u_0);
    permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_22u_1);
    permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_22u_2);

    __m512i shift_masks_ptr[3];
    shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_22u_0);
    shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_22u_1);
    shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_22u_2);

    srcmm0 = _mm512_maskz_loadu_epi32(tail_mask, src_ptr);
    zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_22u_ptr[0], permutex_idx_ptr[0], srcmm0);
    zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_22u_ptr[1], permutex_idx_ptr[1], srcmm0);
    zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_22u_ptr[2], permutex_idx_ptr[2], srcmm0);

    zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
    zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
    zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

    zmm0 = _mm512_or_si512(zmm0, zmm1);
    zmm0 = _mm512_or_si512(zmm0, zmm2);
    _mm512_mask_storeu_epi8(dst_ptr, store_mask, zmm0);
}

OWN_OPT_FUN(void, k0_qplc_pack_32u22u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 22U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_pack_32u22u(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 4;
        dst_ptr += ((align * 22U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 16U;
    if (num_elements >= 16U) {
        uint32_t num_elements_64 = num_elements / 64U;
        uint32_t num_elements_16 = (num_elements % 64U) / 16U;
        __m512i  srcmm0, srcmm1, srcmm2, srcmm3;
        __m512i  zmm0, zmm1, zmm2, zmm3, zmm4, zmm5, zmm6, zmm7, zmm8;

        __m512i permutex_idx_ptr[9];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_22u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_22u_1);
        permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_22u_2);
        permutex_idx_ptr[3] = _mm512_load_si512(permutex_idx_table_22u_3);
        permutex_idx_ptr[4] = _mm512_load_si512(permutex_idx_table_22u_4);
        permutex_idx_ptr[5] = _mm512_load_si512(permutex_idx_table_22u_5);
        permutex_idx_ptr[6] = _mm512_load_si512(permutex_idx_table_22u_6);
        permutex_idx_ptr[7] = _mm512_load_si512(permutex_idx_table_22u_7);
        permutex_idx_ptr[8] = _mm512_load_si512(permutex_idx_table_22u_8);

        __m512i shift_masks_ptr[9];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_22u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_22u_1);
        shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_22u_2);
        shift_masks_ptr[3] = _mm512_load_si512(shift_mask_table_22u_3);
        shift_masks_ptr[4] = _mm512_load_si512(shift_mask_table_22u_4);
        shift_masks_ptr[5] = _mm512_load_si512(shift_mask_table_22u_5);
        shift_masks_ptr[6] = _mm512_load_si512(shift_mask_table_22u_6);
        shift_masks_ptr[7] = _mm512_load_si512(shift_mask_table_22u_7);
        shift_masks_ptr[8] = _mm512_load_si512(shift_mask_table_22u_8);

        for (uint32_t idx = 0; idx < num_elements_64; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            srcmm1 = _mm512_loadu_si512(src_ptr + 64U);
            srcmm2 = _mm512_loadu_si512(src_ptr + 128U);
            srcmm3 = _mm512_loadu_si512(src_ptr + 192U);

            zmm0 = _mm512_maskz_permutex2var_epi32(permutex_masks_22u_ptr[0], srcmm0, permutex_idx_ptr[0], srcmm1);
            zmm1 = _mm512_maskz_permutex2var_epi32(permutex_masks_22u_ptr[1], srcmm0, permutex_idx_ptr[1], srcmm1);
            zmm2 = _mm512_maskz_permutex2var_epi32(permutex_masks_22u_ptr[2], srcmm0, permutex_idx_ptr[2], srcmm1);
            zmm3 = _mm512_maskz_permutex2var_epi32(permutex_masks_22u_ptr[3], srcmm1, permutex_idx_ptr[3], srcmm2);
            zmm4 = _mm512_maskz_permutex2var_epi32(permutex_masks_22u_ptr[4], srcmm1, permutex_idx_ptr[4], srcmm2);
            zmm5 = _mm512_maskz_permutex2var_epi32(permutex_masks_22u_ptr[5], srcmm1, permutex_idx_ptr[5], srcmm2);
            zmm6 = _mm512_maskz_permutex2var_epi32(permutex_masks_22u_ptr[6], srcmm2, permutex_idx_ptr[6], srcmm3);
            zmm7 = _mm512_maskz_permutex2var_epi32(permutex_masks_22u_ptr[7], srcmm2, permutex_idx_ptr[7], srcmm3);
            zmm8 = _mm512_maskz_permutex2var_epi32(permutex_masks_22u_ptr[8], srcmm2, permutex_idx_ptr[8], srcmm3);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);
            zmm3 = _mm512_sllv_epi32(zmm3, shift_masks_ptr[3]);
            zmm4 = _mm512_sllv_epi32(zmm4, shift_masks_ptr[4]);
            zmm5 = _mm512_srlv_epi32(zmm5, shift_masks_ptr[5]);
            zmm6 = _mm512_sllv_epi32(zmm6, shift_masks_ptr[6]);
            zmm7 = _mm512_sllv_epi32(zmm7, shift_masks_ptr[7]);
            zmm8 = _mm512_srlv_epi32(zmm8, shift_masks_ptr[8]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            zmm3 = _mm512_or_si512(zmm3, zmm4);
            zmm3 = _mm512_or_si512(zmm3, zmm5);
            zmm6 = _mm512_or_si512(zmm6, zmm7);
            zmm6 = _mm512_or_si512(zmm6, zmm8);

            _mm512_storeu_si512(dst_ptr, zmm0);
            _mm512_storeu_si512(dst_ptr + 64U, zmm3);
            _mm512_mask_storeu_epi16(dst_ptr + 128U, 0x00FFFFFF, zmm6);

            src_ptr += 256U;
            dst_ptr += 22U * 8U;
        }

        for (uint32_t idx = 0; idx < num_elements_16; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_22u_ptr[0], permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_22u_ptr[1], permutex_idx_ptr[1], srcmm0);
            zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_22u_ptr[2], permutex_idx_ptr[2], srcmm0);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            _mm512_mask_storeu_epi16(dst_ptr, 0x003FFFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 22U * 2U;
        }
    }

    if (tail > 0) { k0_qplc_pack_32u22u_tail(src_ptr, tail, dst_ptr); }
}

// ********************** 32u23u ****************************** //

OWN_QPLC_INLINE(void, px_qplc_pack_32u23u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 23U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;
    uint64_t  src         = (uint64_t)(*dst_32u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint64_t)(*src_32u_ptr)) << start_bit;
    src_32u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_DWORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_32u_ptr = (uint32_t)(src);
            dst_32u_ptr++;
            src = src >> OWN_DWORD_WIDTH;
            bits_in_buf -= OWN_DWORD_WIDTH;
        }
        src = src | (((uint64_t)(*src_32u_ptr)) << bits_in_buf);
        src_32u_ptr++;
        num_elements--;
        bits_in_buf += (int32_t)bit_width;
    }
    dst_ptr = (uint8_t*)dst_32u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_QPLC_INLINE(void, k0_qplc_pack_32u23u_tail, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    __m512i srcmm0;
    __m512i zmm0, zmm1, zmm2;

    __mmask16 tail_mask  = OWN_BIT_MASK(num_elements);
    __mmask64 store_mask = OWN_BIT_MASK(OWN_BITS_2_BYTE(num_elements * 23U));

    __m512i permutex_idx_ptr[3];
    permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_23u_0);
    permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_23u_1);
    permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_23u_2);

    __m512i shift_masks_ptr[3];
    shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_23u_0);
    shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_23u_1);
    shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_23u_2);

    srcmm0 = _mm512_maskz_loadu_epi32(tail_mask, src_ptr);
    zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_23u_ptr[0], permutex_idx_ptr[0], srcmm0);
    zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_23u_ptr[1], permutex_idx_ptr[1], srcmm0);
    zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_23u_ptr[2], permutex_idx_ptr[2], srcmm0);

    zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
    zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
    zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

    zmm0 = _mm512_or_si512(zmm0, zmm1);
    zmm0 = _mm512_or_si512(zmm0, zmm2);
    _mm512_mask_storeu_epi8(dst_ptr, store_mask, zmm0);
}

OWN_OPT_FUN(void, k0_qplc_pack_32u23u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 23U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_pack_32u23u(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 4;
        dst_ptr += ((align * 23U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 16U;
    if (num_elements >= 16U) {
        uint32_t num_elements_64 = num_elements / 64U;
        uint32_t num_elements_16 = (num_elements % 64U) / 16U;
        __m512i  srcmm0, srcmm1, srcmm2, srcmm3;
        __m512i  zmm0, zmm1, zmm2, zmm3, zmm4, zmm5, zmm6, zmm7, zmm8;

        __m512i permutex_idx_ptr[9];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_23u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_23u_1);
        permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_23u_2);
        permutex_idx_ptr[3] = _mm512_load_si512(permutex_idx_table_23u_3);
        permutex_idx_ptr[4] = _mm512_load_si512(permutex_idx_table_23u_4);
        permutex_idx_ptr[5] = _mm512_load_si512(permutex_idx_table_23u_5);
        permutex_idx_ptr[6] = _mm512_load_si512(permutex_idx_table_23u_6);
        permutex_idx_ptr[7] = _mm512_load_si512(permutex_idx_table_23u_7);
        permutex_idx_ptr[8] = _mm512_load_si512(permutex_idx_table_23u_8);

        __m512i shift_masks_ptr[9];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_23u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_23u_1);
        shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_23u_2);
        shift_masks_ptr[3] = _mm512_load_si512(shift_mask_table_23u_3);
        shift_masks_ptr[4] = _mm512_load_si512(shift_mask_table_23u_4);
        shift_masks_ptr[5] = _mm512_load_si512(shift_mask_table_23u_5);
        shift_masks_ptr[6] = _mm512_load_si512(shift_mask_table_23u_6);
        shift_masks_ptr[7] = _mm512_load_si512(shift_mask_table_23u_7);
        shift_masks_ptr[8] = _mm512_load_si512(shift_mask_table_23u_8);

        for (uint32_t idx = 0; idx < num_elements_64; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            srcmm1 = _mm512_loadu_si512(src_ptr + 64U);
            srcmm2 = _mm512_loadu_si512(src_ptr + 128U);
            srcmm3 = _mm512_loadu_si512(src_ptr + 192U);

            zmm0 = _mm512_maskz_permutex2var_epi32(permutex_masks_23u_ptr[0], srcmm0, permutex_idx_ptr[0], srcmm1);
            zmm1 = _mm512_maskz_permutex2var_epi32(permutex_masks_23u_ptr[1], srcmm0, permutex_idx_ptr[1], srcmm1);
            zmm2 = _mm512_maskz_permutex2var_epi32(permutex_masks_23u_ptr[2], srcmm0, permutex_idx_ptr[2], srcmm1);
            zmm3 = _mm512_maskz_permutex2var_epi32(permutex_masks_23u_ptr[3], srcmm1, permutex_idx_ptr[3], srcmm2);
            zmm4 = _mm512_maskz_permutex2var_epi32(permutex_masks_23u_ptr[4], srcmm1, permutex_idx_ptr[4], srcmm2);
            zmm5 = _mm512_maskz_permutex2var_epi32(permutex_masks_23u_ptr[5], srcmm1, permutex_idx_ptr[5], srcmm2);
            zmm6 = _mm512_maskz_permutex2var_epi32(permutex_masks_23u_ptr[6], srcmm2, permutex_idx_ptr[6], srcmm3);
            zmm7 = _mm512_maskz_permutex2var_epi32(permutex_masks_23u_ptr[7], srcmm2, permutex_idx_ptr[7], srcmm3);
            zmm8 = _mm512_maskz_permutex2var_epi32(permutex_masks_23u_ptr[8], srcmm2, permutex_idx_ptr[8], srcmm3);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);
            zmm3 = _mm512_sllv_epi32(zmm3, shift_masks_ptr[3]);
            zmm4 = _mm512_sllv_epi32(zmm4, shift_masks_ptr[4]);
            zmm5 = _mm512_srlv_epi32(zmm5, shift_masks_ptr[5]);
            zmm6 = _mm512_sllv_epi32(zmm6, shift_masks_ptr[6]);
            zmm7 = _mm512_sllv_epi32(zmm7, shift_masks_ptr[7]);
            zmm8 = _mm512_srlv_epi32(zmm8, shift_masks_ptr[8]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            zmm3 = _mm512_or_si512(zmm3, zmm4);
            zmm3 = _mm512_or_si512(zmm3, zmm5);
            zmm6 = _mm512_or_si512(zmm6, zmm7);
            zmm6 = _mm512_or_si512(zmm6, zmm8);

            _mm512_storeu_si512(dst_ptr, zmm0);
            _mm512_storeu_si512(dst_ptr + 64U, zmm3);
            _mm512_mask_storeu_epi16(dst_ptr + 128U, 0x0FFFFFFF, zmm6);

            src_ptr += 256U;
            dst_ptr += 23U * 8U;
        }

        for (uint32_t idx = 0; idx < num_elements_16; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_23u_ptr[0], permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_23u_ptr[1], permutex_idx_ptr[1], srcmm0);
            zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_23u_ptr[2], permutex_idx_ptr[2], srcmm0);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            _mm512_mask_storeu_epi16(dst_ptr, 0x007FFFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 23U * 2U;
        }
    }

    if (tail > 0) { k0_qplc_pack_32u23u_tail(src_ptr, tail, dst_ptr); }
}

// ********************** 32u24u ****************************** //

OWN_QPLC_INLINE(void, px_qplc_pack_32u24u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 24U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;
    uint64_t  src         = (uint64_t)(*dst_32u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint64_t)(*src_32u_ptr)) << start_bit;
    src_32u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_DWORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_32u_ptr = (uint32_t)(src);
            dst_32u_ptr++;
            src = src >> OWN_DWORD_WIDTH;
            bits_in_buf -= OWN_DWORD_WIDTH;
        }
        src = src | (((uint64_t)(*src_32u_ptr)) << bits_in_buf);
        src_32u_ptr++;
        num_elements--;
        bits_in_buf += (int32_t)bit_width;
    }
    dst_ptr = (uint8_t*)dst_32u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_QPLC_INLINE(void, k0_qplc_pack_32u24u_tail, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    __m512i srcmm0;
    __m512i zmm0, zmm1;

    __mmask16 tail_mask  = OWN_BIT_MASK(num_elements);
    __mmask64 store_mask = OWN_BIT_MASK(OWN_BITS_2_BYTE(num_elements * 24U));

    __m512i permutex_idx_ptr[2];
    permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_24u_0);
    permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_24u_1);

    __m512i shift_masks_ptr[2];
    shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_24u_0);
    shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_24u_1);

    srcmm0 = _mm512_maskz_loadu_epi32(tail_mask, src_ptr);
    zmm0   = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm0);
    zmm1   = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm0);

    zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
    zmm1 = _mm512_srlv_epi32(zmm1, shift_masks_ptr[1]);

    zmm0 = _mm512_or_si512(zmm0, zmm1);
    _mm512_mask_storeu_epi8(dst_ptr, store_mask, zmm0);
}

OWN_OPT_FUN(void, k0_qplc_pack_32u24u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 24U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_pack_32u24u(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 4;
        dst_ptr += ((align * 24U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 16U;
    if (num_elements >= 16U) {
        uint32_t num_elements_64 = num_elements / 64U;
        uint32_t num_elements_16 = (num_elements % 64U) / 16U;
        __m512i  srcmm0, srcmm1, srcmm2, srcmm3;
        __m512i  zmm0, zmm1, zmm2, zmm3, zmm4, zmm5;

        __m512i permutex_idx_ptr[6];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_24u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_24u_1);
        permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_24u_2);
        permutex_idx_ptr[3] = _mm512_load_si512(permutex_idx_table_24u_3);
        permutex_idx_ptr[4] = _mm512_load_si512(permutex_idx_table_24u_4);
        permutex_idx_ptr[5] = _mm512_load_si512(permutex_idx_table_24u_5);

        __m512i shift_masks_ptr[6];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_24u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_24u_1);
        shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_24u_2);
        shift_masks_ptr[3] = _mm512_load_si512(shift_mask_table_24u_3);
        shift_masks_ptr[4] = _mm512_load_si512(shift_mask_table_24u_4);
        shift_masks_ptr[5] = _mm512_load_si512(shift_mask_table_24u_5);

        for (uint32_t idx = 0; idx < num_elements_64; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            srcmm1 = _mm512_loadu_si512(src_ptr + 64U);
            srcmm2 = _mm512_loadu_si512(src_ptr + 128U);
            srcmm3 = _mm512_loadu_si512(src_ptr + 192U);

            zmm0 = _mm512_permutex2var_epi32(srcmm0, permutex_idx_ptr[0], srcmm1);
            zmm1 = _mm512_permutex2var_epi32(srcmm0, permutex_idx_ptr[1], srcmm1);
            zmm2 = _mm512_permutex2var_epi32(srcmm1, permutex_idx_ptr[2], srcmm2);
            zmm3 = _mm512_permutex2var_epi32(srcmm1, permutex_idx_ptr[3], srcmm2);
            zmm4 = _mm512_permutex2var_epi32(srcmm2, permutex_idx_ptr[4], srcmm3);
            zmm5 = _mm512_permutex2var_epi32(srcmm2, permutex_idx_ptr[5], srcmm3);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_srlv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_sllv_epi32(zmm2, shift_masks_ptr[2]);
            zmm3 = _mm512_srlv_epi32(zmm3, shift_masks_ptr[3]);
            zmm4 = _mm512_sllv_epi32(zmm4, shift_masks_ptr[4]);
            zmm5 = _mm512_srlv_epi32(zmm5, shift_masks_ptr[5]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm2 = _mm512_or_si512(zmm2, zmm3);
            zmm4 = _mm512_or_si512(zmm4, zmm5);

            _mm512_storeu_si512(dst_ptr, zmm0);
            _mm512_storeu_si512(dst_ptr + 64U, zmm2);
            _mm512_storeu_si512(dst_ptr + 128U, zmm4);

            src_ptr += 256U;
            dst_ptr += 24U * 8U;
        }

        for (uint32_t idx = 0; idx < num_elements_16; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm0);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_srlv_epi32(zmm1, shift_masks_ptr[1]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            _mm512_mask_storeu_epi16(dst_ptr, 0x00FFFFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 24U * 2U;
        }
    }

    if (tail > 0) { k0_qplc_pack_32u24u_tail(src_ptr, tail, dst_ptr); }
}

// ********************** 32u25u ****************************** //

OWN_QPLC_INLINE(void, px_qplc_pack_32u25u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 25U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;
    uint64_t  src         = (uint64_t)(*dst_32u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint64_t)(*src_32u_ptr)) << start_bit;
    src_32u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_DWORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_32u_ptr = (uint32_t)(src);
            dst_32u_ptr++;
            src = src >> OWN_DWORD_WIDTH;
            bits_in_buf -= OWN_DWORD_WIDTH;
        }
        src = src | (((uint64_t)(*src_32u_ptr)) << bits_in_buf);
        src_32u_ptr++;
        num_elements--;
        bits_in_buf += (int32_t)bit_width;
    }
    dst_ptr = (uint8_t*)dst_32u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_QPLC_INLINE(void, k0_qplc_pack_32u25u_tail, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    __m512i srcmm0;
    __m512i zmm0, zmm1, zmm2;

    __mmask16 tail_mask  = OWN_BIT_MASK(num_elements);
    __mmask64 store_mask = OWN_BIT_MASK(OWN_BITS_2_BYTE(num_elements * 25U));

    __m512i permutex_idx_ptr[3];
    permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_25u_0);
    permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_25u_1);
    permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_25u_2);

    __m512i shift_masks_ptr[3];
    shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_25u_0);
    shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_25u_1);
    shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_25u_2);

    srcmm0 = _mm512_maskz_loadu_epi32(tail_mask, src_ptr);
    zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_25u_ptr[0], permutex_idx_ptr[0], srcmm0);
    zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_25u_ptr[1], permutex_idx_ptr[1], srcmm0);
    zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_25u_ptr[2], permutex_idx_ptr[2], srcmm0);

    zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
    zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
    zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

    zmm0 = _mm512_or_si512(zmm0, zmm1);
    zmm0 = _mm512_or_si512(zmm0, zmm2);
    _mm512_mask_storeu_epi8(dst_ptr, store_mask, zmm0);
}

OWN_OPT_FUN(void, k0_qplc_pack_32u25u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 25U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_pack_32u25u(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 4;
        dst_ptr += ((align * 25U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 16U;
    if (num_elements >= 16U) {
        uint32_t num_elements_16 = num_elements / 16U;
        __m512i  srcmm0;
        __m512i  zmm0, zmm1, zmm2;

        __m512i permutex_idx_ptr[3];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_25u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_25u_1);
        permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_25u_2);

        __m512i shift_masks_ptr[3];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_25u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_25u_1);
        shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_25u_2);

        for (uint32_t idx = 0; idx < num_elements_16; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_25u_ptr[0], permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_25u_ptr[1], permutex_idx_ptr[1], srcmm0);
            zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_25u_ptr[2], permutex_idx_ptr[2], srcmm0);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            _mm512_mask_storeu_epi16(dst_ptr, 0x01FFFFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 25U * 2U;
        }
    }

    if (tail > 0) { k0_qplc_pack_32u25u_tail(src_ptr, tail, dst_ptr); }
}

// ********************** 32u26u ****************************** //

OWN_QPLC_INLINE(void, px_qplc_pack_32u26u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 26U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;
    uint64_t  src         = (uint64_t)(*dst_32u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint64_t)(*src_32u_ptr)) << start_bit;
    src_32u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_DWORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_32u_ptr = (uint32_t)(src);
            dst_32u_ptr++;
            src = src >> OWN_DWORD_WIDTH;
            bits_in_buf -= OWN_DWORD_WIDTH;
        }
        src = src | (((uint64_t)(*src_32u_ptr)) << bits_in_buf);
        src_32u_ptr++;
        num_elements--;
        bits_in_buf += (int32_t)bit_width;
    }
    dst_ptr = (uint8_t*)dst_32u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_QPLC_INLINE(void, k0_qplc_pack_32u26u_tail, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    __m512i srcmm0;
    __m512i zmm0, zmm1, zmm2;

    __mmask16 tail_mask  = OWN_BIT_MASK(num_elements);
    __mmask64 store_mask = OWN_BIT_MASK(OWN_BITS_2_BYTE(num_elements * 26U));

    __m512i permutex_idx_ptr[3];
    permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_26u_0);
    permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_26u_1);
    permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_26u_2);

    __m512i shift_masks_ptr[3];
    shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_26u_0);
    shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_26u_1);
    shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_26u_2);

    srcmm0 = _mm512_maskz_loadu_epi32(tail_mask, src_ptr);
    zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_26u_ptr[0], permutex_idx_ptr[0], srcmm0);
    zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_26u_ptr[1], permutex_idx_ptr[1], srcmm0);
    zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_26u_ptr[2], permutex_idx_ptr[2], srcmm0);

    zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
    zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
    zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

    zmm0 = _mm512_or_si512(zmm0, zmm1);
    zmm0 = _mm512_or_si512(zmm0, zmm2);
    _mm512_mask_storeu_epi8(dst_ptr, store_mask, zmm0);
}

OWN_OPT_FUN(void, k0_qplc_pack_32u26u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 26U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_pack_32u26u(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 4;
        dst_ptr += ((align * 26U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 16U;
    if (num_elements >= 16U) {
        uint32_t num_elements_16 = num_elements / 16U;
        __m512i  srcmm0;
        __m512i  zmm0, zmm1, zmm2;

        __m512i permutex_idx_ptr[3];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_26u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_26u_1);
        permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_26u_2);

        __m512i shift_masks_ptr[3];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_26u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_26u_1);
        shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_26u_2);

        for (uint32_t idx = 0; idx < num_elements_16; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_26u_ptr[0], permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_26u_ptr[1], permutex_idx_ptr[1], srcmm0);
            zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_26u_ptr[2], permutex_idx_ptr[2], srcmm0);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            _mm512_mask_storeu_epi16(dst_ptr, 0x03FFFFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 26U * 2U;
        }
    }

    if (tail > 0) { k0_qplc_pack_32u26u_tail(src_ptr, tail, dst_ptr); }
}

// ********************** 32u27u ****************************** //

OWN_QPLC_INLINE(void, px_qplc_pack_32u27u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 27U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;
    uint64_t  src         = (uint64_t)(*dst_32u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint64_t)(*src_32u_ptr)) << start_bit;
    src_32u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_DWORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_32u_ptr = (uint32_t)(src);
            dst_32u_ptr++;
            src = src >> OWN_DWORD_WIDTH;
            bits_in_buf -= OWN_DWORD_WIDTH;
        }
        src = src | (((uint64_t)(*src_32u_ptr)) << bits_in_buf);
        src_32u_ptr++;
        num_elements--;
        bits_in_buf += (int32_t)bit_width;
    }
    dst_ptr = (uint8_t*)dst_32u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_QPLC_INLINE(void, k0_qplc_pack_32u27u_tail, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    __m512i srcmm0;
    __m512i zmm0, zmm1, zmm2;

    __mmask16 tail_mask  = OWN_BIT_MASK(num_elements);
    __mmask64 store_mask = OWN_BIT_MASK(OWN_BITS_2_BYTE(num_elements * 27U));

    __m512i permutex_idx_ptr[3];
    permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_27u_0);
    permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_27u_1);
    permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_27u_2);

    __m512i shift_masks_ptr[3];
    shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_27u_0);
    shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_27u_1);
    shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_27u_2);

    srcmm0 = _mm512_maskz_loadu_epi32(tail_mask, src_ptr);
    zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_27u_ptr[0], permutex_idx_ptr[0], srcmm0);
    zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_27u_ptr[1], permutex_idx_ptr[1], srcmm0);
    zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_27u_ptr[2], permutex_idx_ptr[2], srcmm0);

    zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
    zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
    zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

    zmm0 = _mm512_or_si512(zmm0, zmm1);
    zmm0 = _mm512_or_si512(zmm0, zmm2);
    _mm512_mask_storeu_epi8(dst_ptr, store_mask, zmm0);
}

OWN_OPT_FUN(void, k0_qplc_pack_32u27u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 27U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_pack_32u27u(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 4;
        dst_ptr += ((align * 27U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 16U;
    if (num_elements >= 16U) {
        uint32_t num_elements_16 = num_elements / 16U;
        __m512i  srcmm0;
        __m512i  zmm0, zmm1, zmm2;

        __m512i permutex_idx_ptr[3];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_27u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_27u_1);
        permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_27u_2);

        __m512i shift_masks_ptr[3];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_27u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_27u_1);
        shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_27u_2);

        for (uint32_t idx = 0; idx < num_elements_16; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_27u_ptr[0], permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_27u_ptr[1], permutex_idx_ptr[1], srcmm0);
            zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_27u_ptr[2], permutex_idx_ptr[2], srcmm0);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            _mm512_mask_storeu_epi16(dst_ptr, 0x07FFFFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 27U * 2U;
        }
    }

    if (tail > 0) { k0_qplc_pack_32u27u_tail(src_ptr, tail, dst_ptr); }
}

// ********************** 32u28u ****************************** //

OWN_QPLC_INLINE(void, px_qplc_pack_32u28u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 28U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;
    uint64_t  src         = (uint64_t)(*dst_32u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint64_t)(*src_32u_ptr)) << start_bit;
    src_32u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_DWORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_32u_ptr = (uint32_t)(src);
            dst_32u_ptr++;
            src = src >> OWN_DWORD_WIDTH;
            bits_in_buf -= OWN_DWORD_WIDTH;
        }
        src = src | (((uint64_t)(*src_32u_ptr)) << bits_in_buf);
        src_32u_ptr++;
        num_elements--;
        bits_in_buf += (int32_t)bit_width;
    }
    dst_ptr = (uint8_t*)dst_32u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

#if defined _MSC_VER
#if _MSC_VER <= 1916
// if MSVC <= MSVC2017
// There is the problem with compiler of MSVC2017.
#pragma optimize("", off)
#endif
#endif

OWN_QPLC_INLINE(void, k0_qplc_pack_32u28u_tail, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    __m512i srcmm0;
    __m512i zmm0, zmm1;

    __mmask16 tail_mask  = OWN_BIT_MASK(num_elements);
    __mmask64 store_mask = OWN_BIT_MASK(OWN_BITS_2_BYTE(num_elements * 28U));

    __m512i permutex_idx_ptr[2];
    permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_28u_0);
    permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_28u_1);

    __m512i shift_masks_ptr[2];
    shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_28u_0);
    shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_28u_1);

    srcmm0 = _mm512_maskz_loadu_epi32(tail_mask, src_ptr);
    zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_28u_ptr[0], permutex_idx_ptr[0], srcmm0);
    zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_28u_ptr[1], permutex_idx_ptr[1], srcmm0);

    zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
    zmm1 = _mm512_srlv_epi32(zmm1, shift_masks_ptr[1]);

    zmm0 = _mm512_or_si512(zmm0, zmm1);
    _mm512_mask_storeu_epi8(dst_ptr, store_mask, zmm0);
}

#if defined _MSC_VER
#if _MSC_VER <= 1916
// if MSVC <= MSVC2017
#pragma optimize("", on)
#endif
#endif

OWN_OPT_FUN(void, k0_qplc_pack_32u28u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 28U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_pack_32u28u(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 4;
        dst_ptr += ((align * 28U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 16U;
    if (num_elements >= 16U) {
        uint32_t num_elements_16 = num_elements / 16U;
        __m512i  srcmm0;
        __m512i  zmm0, zmm1;

        __m512i permutex_idx_ptr[3];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_28u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_28u_1);

        __m512i shift_masks_ptr[3];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_28u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_28u_1);

        for (uint32_t idx = 0; idx < num_elements_16; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_28u_ptr[0], permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_28u_ptr[1], permutex_idx_ptr[1], srcmm0);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_srlv_epi32(zmm1, shift_masks_ptr[1]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            _mm512_mask_storeu_epi16(dst_ptr, 0x0FFFFFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 28U * 2U;
        }
    }

    if (tail > 0) { k0_qplc_pack_32u28u_tail(src_ptr, tail, dst_ptr); }
}

// ********************** 32u29u ****************************** //

OWN_QPLC_INLINE(void, px_qplc_pack_32u29u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 29U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;
    uint64_t  src         = (uint64_t)(*dst_32u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint64_t)(*src_32u_ptr)) << start_bit;
    src_32u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_DWORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_32u_ptr = (uint32_t)(src);
            dst_32u_ptr++;
            src = src >> OWN_DWORD_WIDTH;
            bits_in_buf -= OWN_DWORD_WIDTH;
        }
        src = src | (((uint64_t)(*src_32u_ptr)) << bits_in_buf);
        src_32u_ptr++;
        num_elements--;
        bits_in_buf += (int32_t)bit_width;
    }
    dst_ptr = (uint8_t*)dst_32u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_QPLC_INLINE(void, k0_qplc_pack_32u29u_tail, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    __m512i srcmm0;
    __m512i zmm0, zmm1, zmm2;

    __mmask16 tail_mask  = OWN_BIT_MASK(num_elements);
    __mmask64 store_mask = OWN_BIT_MASK(OWN_BITS_2_BYTE(num_elements * 29U));

    __m512i permutex_idx_ptr[3];
    permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_29u_0);
    permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_29u_1);
    permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_29u_2);

    __m512i shift_masks_ptr[3];
    shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_29u_0);
    shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_29u_1);
    shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_29u_2);

    srcmm0 = _mm512_maskz_loadu_epi32(tail_mask, src_ptr);
    zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_29u_ptr[0], permutex_idx_ptr[0], srcmm0);
    zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_29u_ptr[1], permutex_idx_ptr[1], srcmm0);
    zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_29u_ptr[2], permutex_idx_ptr[2], srcmm0);

    zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
    zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
    zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

    zmm0 = _mm512_or_si512(zmm0, zmm1);
    zmm0 = _mm512_or_si512(zmm0, zmm2);
    _mm512_mask_storeu_epi8(dst_ptr, store_mask, zmm0);
}

OWN_OPT_FUN(void, k0_qplc_pack_32u29u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 29U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_pack_32u29u(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 4;
        dst_ptr += ((align * 29U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 16U;
    if (num_elements >= 16U) {
        uint32_t num_elements_16 = num_elements / 16U;
        __m512i  srcmm0;
        __m512i  zmm0, zmm1, zmm2;

        __m512i permutex_idx_ptr[3];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_29u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_29u_1);
        permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_29u_2);

        __m512i shift_masks_ptr[3];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_29u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_29u_1);
        shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_29u_2);

        for (uint32_t idx = 0; idx < num_elements_16; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_29u_ptr[0], permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_29u_ptr[1], permutex_idx_ptr[1], srcmm0);
            zmm2   = _mm512_maskz_permutexvar_epi32(permutex_masks_29u_ptr[2], permutex_idx_ptr[2], srcmm0);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi32(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi32(zmm2, shift_masks_ptr[2]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            _mm512_mask_storeu_epi16(dst_ptr, 0x1FFFFFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 29U * 2U;
        }
    }

    if (tail > 0) { k0_qplc_pack_32u29u_tail(src_ptr, tail, dst_ptr); }
}

// ********************** 32u30u ****************************** //

OWN_QPLC_INLINE(void, px_qplc_pack_32u30u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 30U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;
    uint64_t  src         = (uint64_t)(*dst_32u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint64_t)(*src_32u_ptr)) << start_bit;
    src_32u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_DWORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_32u_ptr = (uint32_t)(src);
            dst_32u_ptr++;
            src = src >> OWN_DWORD_WIDTH;
            bits_in_buf -= OWN_DWORD_WIDTH;
        }
        src = src | (((uint64_t)(*src_32u_ptr)) << bits_in_buf);
        src_32u_ptr++;
        num_elements--;
        bits_in_buf += (int32_t)bit_width;
    }
    dst_ptr = (uint8_t*)dst_32u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

#if defined _MSC_VER
#if _MSC_VER <= 1916
// if MSVC <= MSVC2017
// There is the problem with compiler of MSVC2017.
#pragma optimize("", off)
#endif
#endif

OWN_QPLC_INLINE(void, k0_qplc_pack_32u30u_tail, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    __m512i srcmm0;
    __m512i zmm0, zmm1;

    __mmask16 tail_mask  = OWN_BIT_MASK(num_elements);
    __mmask64 store_mask = OWN_BIT_MASK(OWN_BITS_2_BYTE(num_elements * 30U));

    __m512i permutex_idx_ptr[2];
    permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_30u_0);
    permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_30u_1);

    __m512i shift_masks_ptr[2];
    shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_30u_0);
    shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_30u_1);

    srcmm0 = _mm512_maskz_loadu_epi32(tail_mask, src_ptr);
    zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_30u_ptr[0], permutex_idx_ptr[0], srcmm0);
    zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_30u_ptr[1], permutex_idx_ptr[1], srcmm0);

    zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
    zmm1 = _mm512_srlv_epi32(zmm1, shift_masks_ptr[1]);

    zmm0 = _mm512_or_si512(zmm0, zmm1);
    _mm512_mask_storeu_epi8(dst_ptr, store_mask, zmm0);
}

#if defined _MSC_VER
#if _MSC_VER <= 1916
// if MSVC <= MSVC2017
#pragma optimize("", on)
#endif
#endif

OWN_OPT_FUN(void, k0_qplc_pack_32u30u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 30U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_pack_32u30u(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 4;
        dst_ptr += ((align * 30U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 16U;
    if (num_elements >= 16U) {
        uint32_t num_elements_16 = num_elements / 16U;
        __m512i  srcmm0;
        __m512i  zmm0, zmm1;

        __m512i permutex_idx_ptr[3];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_30u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_30u_1);

        __m512i shift_masks_ptr[3];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_30u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_30u_1);

        for (uint32_t idx = 0; idx < num_elements_16; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_30u_ptr[0], permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_maskz_permutexvar_epi32(permutex_masks_30u_ptr[1], permutex_idx_ptr[1], srcmm0);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_srlv_epi32(zmm1, shift_masks_ptr[1]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            _mm512_mask_storeu_epi16(dst_ptr, 0x3FFFFFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 30U * 2U;
        }
    }

    if (tail > 0) { k0_qplc_pack_32u30u_tail(src_ptr, tail, dst_ptr); }
}

// ********************** 32u31u ****************************** //

OWN_QPLC_INLINE(void, px_qplc_pack_32u31u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 31U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;
    uint64_t  src         = (uint64_t)(*dst_32u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint64_t)(*src_32u_ptr)) << start_bit;
    src_32u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_DWORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_32u_ptr = (uint32_t)(src);
            dst_32u_ptr++;
            src = src >> OWN_DWORD_WIDTH;
            bits_in_buf -= OWN_DWORD_WIDTH;
        }
        src = src | (((uint64_t)(*src_32u_ptr)) << bits_in_buf);
        src_32u_ptr++;
        num_elements--;
        bits_in_buf += (int32_t)bit_width;
    }
    dst_ptr = (uint8_t*)dst_32u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

#if defined _MSC_VER
#if _MSC_VER <= 1916
// if MSVC <= MSVC2017
// There is the problem with compiler of MSVC2017.
#pragma optimize("", off)
#endif
#endif

OWN_QPLC_INLINE(void, k0_qplc_pack_32u31u_tail, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    __m512i srcmm0;
    __m512i zmm0, zmm1;

    __mmask16 tail_mask  = OWN_BIT_MASK(num_elements);
    __mmask64 store_mask = OWN_BIT_MASK(OWN_BITS_2_BYTE(num_elements * 31U));

    __m512i permutex_idx_ptr[2];
    permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_31u_0);
    permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_31u_1);

    __m512i shift_masks_ptr[2];
    shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_31u_0);
    shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_31u_1);

    srcmm0 = _mm512_maskz_loadu_epi32(tail_mask, src_ptr);
    zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_31u_ptr[0], permutex_idx_ptr[0], srcmm0);
    zmm1   = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm0);

    zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
    zmm1 = _mm512_srlv_epi32(zmm1, shift_masks_ptr[1]);

    zmm0 = _mm512_or_si512(zmm0, zmm1);
    _mm512_mask_storeu_epi8(dst_ptr, store_mask, zmm0);
}

#if defined _MSC_VER
#if _MSC_VER <= 1916
// if MSVC <= MSVC2017
#pragma optimize("", on)
#endif
#endif

OWN_OPT_FUN(void, k0_qplc_pack_32u31u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 31U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_pack_32u31u(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 4;
        dst_ptr += ((align * 31U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 16U;
    if (num_elements >= 16U) {
        uint32_t num_elements_16 = num_elements / 16U;
        __m512i  srcmm0;
        __m512i  zmm0, zmm1;

        __m512i permutex_idx_ptr[3];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_31u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_31u_1);

        __m512i shift_masks_ptr[3];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_31u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_31u_1);

        for (uint32_t idx = 0; idx < num_elements_16; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_maskz_permutexvar_epi32(permutex_masks_31u_ptr[0], permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm0);

            zmm0 = _mm512_sllv_epi32(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_srlv_epi32(zmm1, shift_masks_ptr[1]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            _mm512_mask_storeu_epi16(dst_ptr, 0x7FFFFFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 31U * 2U;
        }
    }

    if (tail > 0) { k0_qplc_pack_32u31u_tail(src_ptr, tail, dst_ptr); }
}
#endif // OWN_PACK_32U_H
