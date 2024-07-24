/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*------- qplc_pack_16u_k0.h -------*/

/**
 * @brief Contains implementation of functions for vector packing byte integers to 9...16-bit integers
 * @date 16/02/2021
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
#ifndef OWN_PACK_16U_H
#define OWN_PACK_16U_H

#include "own_qplc_defs.h"

// *********************** Masks  ****************************** //

// ----------------------- 16u9u ------------------------------- //
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_9u_0[32]) = {
        0U,  2U,  4U,  6U,  0x0, 9U,  11U, 13U, 15U, 16U, 18U, 20U, 22U, 0x0, 25U, 27U,
        29U, 31U, 32U, 34U, 36U, 38U, 0x0, 41U, 43U, 45U, 47U, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_9u_1[32]) = {
        1U,  3U,  5U,  7U,  8U,  10U, 12U, 14U, 0x0, 17U, 19U, 21U, 23U, 24U, 26U, 28U,
        30U, 0x0, 33U, 35U, 37U, 39U, 40U, 42U, 44U, 46U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_9u_2[32]) = {
        0x0, 1U,  3U,  5U,  7U,  8U,  10U, 12U, 14U, 0x0, 17U, 19U, 21U, 23U, 24U, 26U,
        28U, 30U, 0x0, 33U, 35U, 37U, 39U, 40U, 42U, 44U, 46U, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_9u_3[32]) = {
        16U, 18U, 20U, 22U, 0x0, 25U, 27U, 29U, 31U, 32U, 34U, 36U, 38U, 0x0, 41U, 43U,
        45U, 47U, 48U, 50U, 52U, 54U, 0x0, 57U, 59U, 61U, 63U, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_9u_4[32]) = {
        17U, 19U, 21U, 23U, 24U, 26U, 28U, 30U, 0x0, 33U, 35U, 37U, 39U, 40U, 42U, 44U,
        46U, 0x0, 49U, 51U, 53U, 55U, 56U, 58U, 60U, 62U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_9u_5[32]) = {
        0x0, 17U, 19U, 21U, 23U, 24U, 26U, 28U, 30U, 0x0, 33U, 35U, 37U, 39U, 40U, 42U,
        44U, 46U, 0x0, 49U, 51U, 53U, 55U, 56U, 58U, 60U, 62U, 0x0, 0x0, 0x0, 0x0, 0x0};
static __mmask32 permutex_masks_9u_ptr[3] = {0x07BFDFEF, 0x03FDFEFF, 0x07FBFDFE};

OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_9u_0[32]) = {0U, 2U, 4U, 6U, 0U, 1U,  3U,  5U,  7U,  0U, 2U,
                                                                   4U, 6U, 0U, 1U, 3U, 5U,  7U,  0U,  2U,  4U, 6U,
                                                                   0U, 1U, 3U, 5U, 7U, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_9u_1[32]) = {
        9U,  11U, 13U, 15U, 8U,  10U, 12U, 14U, 8U,  9U,  11U, 13U, 15U, 8U,  10U, 12U,
        14U, 8U,  9U,  11U, 13U, 15U, 8U,  10U, 12U, 14U, 8U,  0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_9u_2[32]) = {0U, 7U, 5U, 3U, 1U, 8U,  6U,  4U,  2U,  0U, 7U,
                                                                   5U, 3U, 1U, 8U, 6U, 4U,  2U,  0U,  7U,  5U, 3U,
                                                                   1U, 8U, 6U, 4U, 2U, 0x0, 0x0, 0x0, 0x0, 0x0};

// ----------------------- 16u10u ------------------------------ //
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_10u_0[32]) = {
        0U,  2U,  0x0, 5U,  7U,  8U,  10U, 0x0, 13U, 15U, 16U, 18U, 0x0, 21U, 23U, 24U,
        26U, 0x0, 29U, 31U, 32U, 34U, 0x0, 37U, 39U, 40U, 42U, 0x0, 45U, 47U, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_10u_1[32]) = {
        1U,  3U,  4U,  6U,  0x0, 9U,  11U, 12U, 14U, 0x0, 17U, 19U, 20U, 22U, 0x0, 25U,
        27U, 28U, 30U, 0x0, 33U, 35U, 36U, 38U, 0x0, 41U, 43U, 44U, 46U, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_10u_2[32]) = {
        0x0, 1U,  3U,  4U,  6U,  0x0, 9U,  11U, 12U, 14U, 0x0, 17U, 19U, 20U, 22U, 0x0,
        25U, 27U, 28U, 30U, 0x0, 33U, 35U, 36U, 38U, 0x0, 41U, 43U, 44U, 46U, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_10u_3[32]) = {
        16U, 18U, 0x0, 21U, 23U, 24U, 26U, 0x0, 29U, 31U, 32U, 34U, 0x0, 37U, 39U, 40U,
        42U, 0x0, 45U, 47U, 48U, 50U, 0x0, 53U, 55U, 56U, 58U, 0x0, 61U, 63U, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_10u_4[32]) = {
        17U, 19U, 20U, 22U, 0x0, 25U, 27U, 28U, 30U, 0x0, 33U, 35U, 36U, 38U, 0x0, 41U,
        43U, 44U, 46U, 0x0, 49U, 51U, 52U, 54U, 0x0, 57U, 59U, 60U, 62U, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_10u_5[32]) = {
        0x0, 17U, 19U, 20U, 22U, 0x0, 25U, 27U, 28U, 30U, 0x0, 33U, 35U, 36U, 38U, 0x0,
        41U, 43U, 44U, 46U, 0x0, 49U, 51U, 52U, 54U, 0x0, 57U, 59U, 60U, 62U, 0x0, 0x0};
static __mmask32 permutex_masks_10u_ptr[3] = {0x37BDEF7B, 0x1EF7BDEF, 0x3DEF7BDE};

OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_10u_0[32]) = {0U, 4U, 0U, 2U, 6U, 0U, 4U, 0U, 2U,  6U, 0U,
                                                                    4U, 0U, 2U, 6U, 0U, 4U, 0U, 2U, 6U,  0U, 4U,
                                                                    0U, 2U, 6U, 0U, 4U, 0U, 2U, 6U, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_10u_1[32]) = {
        10U, 14U, 8U,  12U, 0U,  10U, 14U, 8U,  12U, 0U,  10U, 14U, 8U,  12U, 0U,  10U,
        14U, 8U,  12U, 0U,  10U, 14U, 8U,  12U, 0U,  10U, 14U, 8U,  12U, 0U,  0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_10u_2[32]) = {0U, 6U, 2U, 8U, 4U, 0U, 6U, 2U, 8U,  4U, 0U,
                                                                    6U, 2U, 8U, 4U, 0U, 6U, 2U, 8U, 4U,  0U, 6U,
                                                                    2U, 8U, 4U, 0U, 6U, 2U, 8U, 4U, 0x0, 0x0};

// ----------------------- 16u11u ------------------------------ //
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_11u_0[32]) = {
        0U,  2U,  3U,  5U,  6U,  0x0, 9U,  0x0, 12U, 0x0, 15U, 16U, 18U, 19U, 21U, 22U,
        0x0, 25U, 0x0, 28U, 0x0, 31U, 32U, 34U, 35U, 37U, 38U, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_11u_1[32]) = {
        1U,  0x0, 4U,  0x0, 7U,  8U,  10U, 11U, 13U, 14U, 0x0, 17U, 0x0, 20U, 0x0, 23U,
        24U, 26U, 27U, 29U, 30U, 0x0, 33U, 0x0, 36U, 0x0, 39U, 40U, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_11u_2[32]) = {
        0x0, 1U,  2U,  4U,  5U,  7U,  8U,  10U, 11U, 13U, 14U, 0x0, 17U, 18U, 20U, 21U,
        23U, 24U, 26U, 27U, 29U, 30U, 0x0, 33U, 34U, 36U, 37U, 39U, 0x0, 0x0, 0x0, 0x0};

OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_11u_3[32]) = {
        9U,  0x0, 12U, 0x0, 15U, 16U, 18U, 19U, 21U, 22U, 0x0, 25U, 0x0, 28U, 0x0, 31U,
        32U, 34U, 35U, 37U, 38U, 0x0, 41U, 0x0, 44U, 0x0, 47U, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_11u_4[32]) = {
        10U, 11U, 13U, 14U, 0x0, 17U, 0x0, 20U, 0x0, 23U, 24U, 26U, 27U, 29U, 30U, 0x0,
        33U, 0x0, 36U, 0x0, 39U, 40U, 42U, 43U, 45U, 46U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_11u_5[32]) = {
        8U,  10U, 11U, 13U, 14U, 0x0, 17U, 18U, 20U, 21U, 23U, 24U, 26U, 27U, 29U, 30U,
        0x0, 33U, 34U, 36U, 37U, 39U, 40U, 42U, 43U, 45U, 46U, 0x0, 0x0, 0x0, 0x0, 0x0};

OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_11u_6[32]) = {
        16U, 18U, 19U, 21U, 22U, 0x0, 25U, 0x0, 28U, 0x0, 31U, 32U, 34U, 35U, 37U, 38U,
        0x0, 41U, 0x0, 44U, 0x0, 47U, 48U, 50U, 51U, 53U, 54U, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_11u_7[32]) = {
        17U, 0x0, 20U, 0x0, 23U, 24U, 26U, 27U, 29U, 30U, 0x0, 33U, 0x0, 36U, 0x0, 39U,
        40U, 42U, 43U, 45U, 46U, 0x0, 49U, 0x0, 52U, 0x0, 55U, 56U, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_11u_8[32]) = {
        0x0, 17U, 18U, 20U, 21U, 23U, 24U, 26U, 27U, 29U, 30U, 0x0, 33U, 34U, 36U, 37U,
        39U, 40U, 42U, 43U, 45U, 46U, 0x0, 49U, 50U, 52U, 53U, 55U, 0x0, 0x0, 0x0, 0x0};

OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_11u_9[32]) = {
        25U, 0x0, 28U, 0x0, 31U, 32U, 34U, 35U, 37U, 38U, 0x0, 41U, 0x0, 44U, 0x0, 47U,
        48U, 50U, 51U, 53U, 54U, 0x0, 57U, 0x0, 60U, 0x0, 63U, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_11u_10[32]) = {
        26U, 27U, 29U, 30U, 0x0, 33U, 0x0, 36U, 0x0, 39U, 40U, 42U, 43U, 45U, 46U, 0x0,
        49U, 0x0, 52U, 0x0, 55U, 56U, 58U, 59U, 61U, 62U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_11u_11[32]) = {
        24U, 26U, 27U, 29U, 30U, 0x0, 33U, 34U, 36U, 37U, 39U, 40U, 42U, 43U, 45U, 46U,
        0x0, 49U, 50U, 52U, 53U, 55U, 56U, 58U, 59U, 61U, 62U, 0x0, 0x0, 0x0, 0x0, 0x0};
static __mmask32 permutex_masks_11u_ptr[6] = {0x07EAFD5F, 0x0D5FABF5, 0x0FBFF7FE, 0x055FABF5, 0x03F57EAF, 0x07FEFFDF};

OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_11u_0[32]) = {0U, 6U, 1U, 7U, 2U, 0U, 3U,  0U,  4U,  0U, 5U,
                                                                    0U, 6U, 1U, 7U, 2U, 0U, 3U,  0U,  4U,  0U, 5U,
                                                                    0U, 6U, 1U, 7U, 2U, 0U, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_11u_1[32]) = {11U, 8U, 12U, 8U, 13U, 8U, 14U, 9U,  15U, 10U, 8U,
                                                                    11U, 8U, 12U, 8U, 13U, 8U, 14U, 9U,  15U, 10U, 8U,
                                                                    11U, 8U, 12U, 8U, 13U, 8U, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_11u_2[32]) = {0U, 5U, 10U, 4U, 9U, 3U, 8U,  2U,  7U,  1U, 6U,
                                                                    0U, 5U, 10U, 4U, 9U, 3U, 8U,  2U,  7U,  1U, 6U,
                                                                    0U, 5U, 10U, 4U, 9U, 3U, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_11u_3[32]) = {3U, 0U, 4U, 0U, 5U, 0U,  6U,  1U,  7U,  2U, 0U,
                                                                    3U, 0U, 4U, 0U, 5U, 0U,  6U,  1U,  7U,  2U, 0U,
                                                                    3U, 0U, 4U, 0U, 5U, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_11u_4[32]) = {14U, 9U, 15U, 10U, 8U, 11U, 8U,  12U, 8U,  13U, 8U,
                                                                    14U, 9U, 15U, 10U, 8U, 11U, 8U,  12U, 8U,  13U, 8U,
                                                                    14U, 9U, 15U, 10U, 8U, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_11u_5[32]) = {8U, 2U, 7U, 1U, 6U, 0U,  5U,  10U, 4U,  9U, 3U,
                                                                    8U, 2U, 7U, 1U, 6U, 0U,  5U,  10U, 4U,  9U, 3U,
                                                                    8U, 2U, 7U, 1U, 6U, 0x0, 0x0, 0x0, 0x0, 0x0};

// ----------------------- 16u12u ------------------------------ //
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_12u_0[32]) = {
        1U,  2U,  3U,  5U,  6U,  7U,  9U,  10U, 11U, 13U, 14U, 15U, 17U, 18U, 19U, 21U,
        22U, 23U, 25U, 26U, 27U, 29U, 30U, 31U, 33U, 34U, 35U, 37U, 38U, 39U, 41U, 42U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_12u_1[32]) = {
        0U,  1U,  2U,  4U,  5U,  6U,  8U,  9U,  10U, 12U, 13U, 14U, 16U, 17U, 18U, 20U,
        21U, 22U, 24U, 25U, 26U, 28U, 29U, 30U, 32U, 33U, 34U, 36U, 37U, 38U, 40U, 41U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_12u_2[32]) = {
        11U, 13U, 14U, 15U, 17U, 18U, 19U, 21U, 22U, 23U, 25U, 26U, 27U, 29U, 30U, 31U,
        33U, 34U, 35U, 37U, 38U, 39U, 41U, 42U, 43U, 45U, 46U, 47U, 49U, 50U, 51U, 53U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_12u_3[32]) = {
        10U, 12U, 13U, 14U, 16U, 17U, 18U, 20U, 21U, 22U, 24U, 25U, 26U, 28U, 29U, 30U,
        32U, 33U, 34U, 36U, 37U, 38U, 40U, 41U, 42U, 44U, 45U, 46U, 48U, 49U, 50U, 52U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_12u_4[32]) = {
        22U, 23U, 25U, 26U, 27U, 29U, 30U, 31U, 33U, 34U, 35U, 37U, 38U, 39U, 41U, 42U,
        43U, 45U, 46U, 47U, 49U, 50U, 51U, 53U, 54U, 55U, 57U, 58U, 59U, 61U, 62U, 63U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_12u_5[32]) = {
        21U, 22U, 24U, 25U, 26U, 28U, 29U, 30U, 32U, 33U, 34U, 36U, 37U, 38U, 40U, 41U,
        42U, 44U, 45U, 46U, 48U, 49U, 50U, 52U, 53U, 54U, 56U, 57U, 58U, 60U, 61U, 62U};

OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_12u_0[32]) = {12U, 8U, 4U, 12U, 8U, 4U, 12U, 8U, 4U, 12U, 8U, 4U,
                                                                    12U, 8U, 4U, 12U, 8U, 4U, 12U, 8U, 4U, 12U, 8U, 4U,
                                                                    12U, 8U, 4U, 12U, 8U, 4U, 12U, 8U};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_12u_1[32]) = {0U, 4U, 8U, 0U, 4U, 8U, 0U, 4U, 8U, 0U, 4U,
                                                                    8U, 0U, 4U, 8U, 0U, 4U, 8U, 0U, 4U, 8U, 0U,
                                                                    4U, 8U, 0U, 4U, 8U, 0U, 4U, 8U, 0U, 4U};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_12u_2[32]) = {4U, 12U, 8U, 4U, 12U, 8U, 4U, 12U, 8U, 4U, 12U, 8U,
                                                                    4U, 12U, 8U, 4U, 12U, 8U, 4U, 12U, 8U, 4U, 12U, 8U,
                                                                    4U, 12U, 8U, 4U, 12U, 8U, 4U, 12U};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_12u_3[32]) = {8U, 0U, 4U, 8U, 0U, 4U, 8U, 0U, 4U, 8U, 0U,
                                                                    4U, 8U, 0U, 4U, 8U, 0U, 4U, 8U, 0U, 4U, 8U,
                                                                    0U, 4U, 8U, 0U, 4U, 8U, 0U, 4U, 8U, 0U};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_12u_4[32]) = {8U, 4U, 12U, 8U, 4U, 12U, 8U, 4U, 12U, 8U, 4U, 12U,
                                                                    8U, 4U, 12U, 8U, 4U, 12U, 8U, 4U, 12U, 8U, 4U, 12U,
                                                                    8U, 4U, 12U, 8U, 4U, 12U, 8U, 4U};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_12u_5[32]) = {4U, 8U, 0U, 4U, 8U, 0U, 4U, 8U, 0U, 4U, 8U,
                                                                    0U, 4U, 8U, 0U, 4U, 8U, 0U, 4U, 8U, 0U, 4U,
                                                                    8U, 0U, 4U, 8U, 0U, 4U, 8U, 0U, 4U, 8U};

// ----------------------- 16u13u ------------------------------ //
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_13u_0[32]) = {
        0U,  0x0, 3U,  4U,  5U,  0x0, 0x0, 9U,  10U, 0x0, 0x0, 14U, 15U, 16U, 0x0, 19U,
        20U, 21U, 0x0, 0x0, 25U, 26U, 0x0, 0x0, 30U, 31U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_13u_1[32]) = {
        1U,  2U,  0x0, 0x0, 6U,  7U,  8U,  0x0, 11U, 12U, 13U, 0x0, 0x0, 17U, 18U, 0x0,
        0x0, 22U, 23U, 24U, 0x0, 27U, 28U, 29U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_13u_2[32]) = {
        0x0, 1U,  2U,  3U,  4U,  6U,  7U,  8U,  9U,  11U, 12U, 13U, 14U, 0x0, 17U, 18U,
        19U, 20U, 22U, 23U, 24U, 25U, 27U, 28U, 29U, 30U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static __mmask32 permutex_masks_13u_ptr[3] = {0x0333B99D, 0x00EE6773, 0x03FFDFFE};

OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_13u_0[32]) = {0U, 0U, 7U, 4U, 1U,  0U,  0U,  5U,  2U,  0U, 0U,
                                                                    6U, 3U, 0U, 0U, 7U,  4U,  1U,  0U,  0U,  5U, 2U,
                                                                    0U, 0U, 6U, 3U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_13u_1[32]) = {
        13U, 10U, 0U,  0U, 14U, 11U, 8U,  0U, 15U, 12U, 9U,  0U,  0U,  13U, 10U, 0U,
        0U,  14U, 11U, 8U, 0U,  15U, 12U, 9U, 0U,  0U,  0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_13u_2[32]) = {0U, 3U,  6U, 9U,  12U, 2U,  5U,  8U,  11U, 1U, 4U,
                                                                    7U, 10U, 0U, 3U,  6U,  9U,  12U, 2U,  5U,  8U, 11U,
                                                                    1U, 4U,  7U, 10U, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

// ----------------------- 16u14u ------------------------------ //
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_14u_0[32]) = {
        1U,  2U,  3U,  4U,  5U,  6U,  7U,  9U,  10U, 11U, 12U, 13U, 14U, 15U, 17U, 18U,
        19U, 20U, 21U, 22U, 23U, 25U, 26U, 27U, 28U, 29U, 30U, 31U, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_14u_1[32]) = {
        0U,  1U,  2U,  3U,  4U,  5U,  6U,  8U,  9U,  10U, 11U, 12U, 13U, 14U, 16U, 17U,
        18U, 19U, 20U, 21U, 22U, 24U, 25U, 26U, 27U, 28U, 29U, 30U, 0x0, 0x0, 0x0, 0x0};

OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_14u_0[32]) = {
        14U, 12U, 10U, 8U, 6U, 4U,  2U,  14U, 12U, 10U, 8U, 6U, 4U,  2U,  14U, 12U,
        10U, 8U,  6U,  4U, 2U, 14U, 12U, 10U, 8U,  6U,  4U, 2U, 0x0, 0x0, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_14u_1[32]) = {0U, 2U,  4U,  6U, 8U,  10U, 12U, 0U,  2U,  4U,  6U,
                                                                    8U, 10U, 12U, 0U, 2U,  4U,  6U,  8U,  10U, 12U, 0U,
                                                                    2U, 4U,  6U,  8U, 10U, 12,  0x0, 0x0, 0x0, 0x0};

// ----------------------- 16u15u ------------------------------ //
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_15u_0[32]) = {
        1U,  2U,  3U,  4U,  5U,  6U,  7U,  8U,  9U,  10U, 11U, 12U, 13U, 14U, 15U, 17U,
        18U, 19U, 20U, 21U, 22U, 23U, 24U, 25U, 26U, 27U, 28U, 29U, 30U, 31U, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_15u_1[32]) = {
        0U,  1U,  2U,  3U,  4U,  5U,  6U,  7U,  8U,  9U,  10U, 11U, 12U, 13U, 14U, 16U,
        17U, 18U, 19U, 20U, 21U, 22U, 23U, 24U, 25U, 26U, 27U, 28U, 29U, 30U, 0x0, 0x0};

OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_15u_0[32]) = {
        15U, 14U, 13U, 12U, 11U, 10U, 9U, 8U, 7U, 6U, 5U, 4U, 3U, 2U, 1U,  15U,
        14U, 13U, 12U, 11U, 10U, 9U,  8U, 7U, 6U, 5U, 4U, 3U, 2U, 1U, 0x0, 0x0};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_mask_table_15u_1[32]) = {
        0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U,  10U, 11U, 12U, 13U, 14U, 0U,
        1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U, 12U, 13U, 14U, 0x0, 0x0};

// ********************** 16u32u ****************************** //

OWN_QPLC_INLINE(void, k0_qplc_pack_16u32u_tail, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    __m256i srcmm;
    __m512i dstmm;

    __mmask16 tail_mask = OWN_BIT_MASK(num_elements);
    srcmm               = _mm256_maskz_loadu_epi16(tail_mask, (const __m256i*)src_ptr);
    dstmm               = _mm512_maskz_cvtepu16_epi32(tail_mask, srcmm);
    _mm512_mask_storeu_epi32(dst_ptr, tail_mask, dstmm);
}

OWN_OPT_FUN(void, k0_qplc_pack_16u32u, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    __m256i srcmm;
    __m512i dstmm;

    while (num_elements > 16U) {
        srcmm = _mm256_loadu_si256((const __m256i*)src_ptr);
        dstmm = _mm512_cvtepu16_epi32(srcmm);
        _mm512_storeu_si512(dst_ptr, dstmm);

        num_elements -= 16U;
        src_ptr += 32U;
        dst_ptr += 64U;
    }

    k0_qplc_pack_16u32u_tail(src_ptr, num_elements, dst_ptr);
}

// ********************** 16u9u ****************************** //

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

OWN_QPLC_INLINE(void, k0_qplc_pack_16u9u_tail,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 9U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint16_t* src_16u_ptr = (uint16_t*)src_ptr;
    uint16_t* dst_16u_ptr = (uint16_t*)dst_ptr;
    uint32_t  src         = (uint32_t)(*dst_16u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint32_t)(*src_16u_ptr)) << start_bit;
    src_16u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_WORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_16u_ptr = (uint16_t)(src);
            dst_16u_ptr++;
            src = src >> OWN_WORD_WIDTH;
            bits_in_buf -= OWN_WORD_WIDTH;
        }
        src = src | (((uint32_t)(*src_16u_ptr)) << bits_in_buf);
        src_16u_ptr++;
        num_elements--;
        bits_in_buf += bit_width;
    }
    dst_ptr = (uint8_t*)dst_16u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_OPT_FUN(void, k0_qplc_pack_16u9u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 9U, 16U);
        if (align > num_elements) { align = num_elements; }
        k0_qplc_pack_16u9u_tail(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 2;
        dst_ptr += ((align * 9U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 32U;
    if (num_elements >= 32U) {
        uint32_t num_elements_96 = num_elements / 96U;
        uint32_t num_elements_32 = (num_elements % 96U) / 32U;
        __m512i  srcmm0, srcmm1, srcmm2;
        __m512i  zmm0, zmm1, zmm2, zmm3, zmm4, zmm5;

        __m512i permutex_idx_ptr[6];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_9u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_9u_1);
        permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_9u_2);
        permutex_idx_ptr[3] = _mm512_load_si512(permutex_idx_table_9u_3);
        permutex_idx_ptr[4] = _mm512_load_si512(permutex_idx_table_9u_4);
        permutex_idx_ptr[5] = _mm512_load_si512(permutex_idx_table_9u_5);

        __m512i shift_masks_ptr[3];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_9u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_9u_1);
        shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_9u_2);

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

            _mm512_mask_storeu_epi16(dst_ptr, 0x07FFFFFF, zmm0);
            _mm512_mask_storeu_epi16((dst_ptr + 54U), 0x07FFFFFF, zmm3);

            src_ptr += 192U;
            dst_ptr += 9U * 12U;
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
            _mm512_mask_storeu_epi16(dst_ptr, 0x0003FFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 9U * 4U;
        }
    }

    if (tail > 0) { k0_qplc_pack_16u9u_tail(src_ptr, tail, dst_ptr, 0U); }
}

OWN_QPLC_INLINE(void, k0_qplc_pack_16u10u_tail,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 10U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint16_t* src_16u_ptr = (uint16_t*)src_ptr;
    uint16_t* dst_16u_ptr = (uint16_t*)dst_ptr;
    uint32_t  src         = (uint32_t)(*dst_16u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint32_t)(*src_16u_ptr)) << start_bit;
    src_16u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_WORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_16u_ptr = (uint16_t)(src);
            dst_16u_ptr++;
            src = src >> OWN_WORD_WIDTH;
            bits_in_buf -= OWN_WORD_WIDTH;
        }
        src = src | (((uint32_t)(*src_16u_ptr)) << bits_in_buf);
        src_16u_ptr++;
        num_elements--;
        bits_in_buf += bit_width;
    }
    dst_ptr = (uint8_t*)dst_16u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_OPT_FUN(void, k0_qplc_pack_16u10u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 10U, 16U);
        if (align > num_elements) { align = num_elements; }
        k0_qplc_pack_16u10u_tail(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 2;
        dst_ptr += ((align * 10U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 32U;
    if (num_elements >= 32U) {
        uint32_t num_elements_96 = num_elements / 96U;
        uint32_t num_elements_32 = (num_elements % 96U) / 32U;
        __m512i  srcmm0, srcmm1, srcmm2;
        __m512i  zmm0, zmm1, zmm2, zmm3, zmm4, zmm5;

        __m512i permutex_idx_ptr[6];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_10u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_10u_1);
        permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_10u_2);
        permutex_idx_ptr[3] = _mm512_load_si512(permutex_idx_table_10u_3);
        permutex_idx_ptr[4] = _mm512_load_si512(permutex_idx_table_10u_4);
        permutex_idx_ptr[5] = _mm512_load_si512(permutex_idx_table_10u_5);

        __m512i shift_masks_ptr[3];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_10u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_10u_1);
        shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_10u_2);

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

            _mm512_mask_storeu_epi16(dst_ptr, 0x3FFFFFFF, zmm0);
            _mm512_mask_storeu_epi16((dst_ptr + 60U), 0x3FFFFFFF, zmm3);

            src_ptr += 192U;
            dst_ptr += 10U * 12U;
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
            _mm512_mask_storeu_epi16(dst_ptr, 0x000FFFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 10U * 4U;
        }
    }

    if (tail > 0) { k0_qplc_pack_16u10u_tail(src_ptr, tail, dst_ptr, 0U); }
}

OWN_QPLC_INLINE(void, k0_qplc_pack_16u11u_tail,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 11U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint16_t* src_16u_ptr = (uint16_t*)src_ptr;
    uint16_t* dst_16u_ptr = (uint16_t*)dst_ptr;
    uint32_t  src         = (uint32_t)(*dst_16u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint32_t)(*src_16u_ptr)) << start_bit;
    src_16u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_WORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_16u_ptr = (uint16_t)(src);
            dst_16u_ptr++;
            src = src >> OWN_WORD_WIDTH;
            bits_in_buf -= OWN_WORD_WIDTH;
        }
        src = src | (((uint32_t)(*src_16u_ptr)) << bits_in_buf);
        src_16u_ptr++;
        num_elements--;
        bits_in_buf += bit_width;
    }
    dst_ptr = (uint8_t*)dst_16u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_OPT_FUN(void, k0_qplc_pack_16u11u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 11U, 16U);
        if (align > num_elements) { align = num_elements; }
        k0_qplc_pack_16u11u_tail(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 2;
        dst_ptr += ((align * 11U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 32U;
    if (num_elements >= 32U) {
        uint32_t num_elements_160 = num_elements / 160U;
        uint32_t num_elements_32  = (num_elements % 160U) / 32U;
        __m512i  srcmm0, srcmm1, srcmm2, srcmm3, srcmm4;
        __m512i  zmm0, zmm1, zmm2, zmm3, zmm4, zmm5, zmm6, zmm7, zmm8, zmm9, zmm10, zmm11;

        __m512i permutex_idx_ptr[12];
        permutex_idx_ptr[0]  = _mm512_loadu_si512(permutex_idx_table_11u_0);
        permutex_idx_ptr[1]  = _mm512_loadu_si512(permutex_idx_table_11u_1);
        permutex_idx_ptr[2]  = _mm512_loadu_si512(permutex_idx_table_11u_2);
        permutex_idx_ptr[3]  = _mm512_loadu_si512(permutex_idx_table_11u_3);
        permutex_idx_ptr[4]  = _mm512_loadu_si512(permutex_idx_table_11u_4);
        permutex_idx_ptr[5]  = _mm512_loadu_si512(permutex_idx_table_11u_5);
        permutex_idx_ptr[6]  = _mm512_loadu_si512(permutex_idx_table_11u_6);
        permutex_idx_ptr[7]  = _mm512_loadu_si512(permutex_idx_table_11u_7);
        permutex_idx_ptr[8]  = _mm512_loadu_si512(permutex_idx_table_11u_8);
        permutex_idx_ptr[9]  = _mm512_loadu_si512(permutex_idx_table_11u_9);
        permutex_idx_ptr[10] = _mm512_loadu_si512(permutex_idx_table_11u_10);
        permutex_idx_ptr[11] = _mm512_loadu_si512(permutex_idx_table_11u_11);

        __m512i shift_masks_ptr[6];
        shift_masks_ptr[0] = _mm512_loadu_si512(shift_mask_table_11u_0);
        shift_masks_ptr[1] = _mm512_loadu_si512(shift_mask_table_11u_1);
        shift_masks_ptr[2] = _mm512_loadu_si512(shift_mask_table_11u_2);
        shift_masks_ptr[3] = _mm512_loadu_si512(shift_mask_table_11u_3);
        shift_masks_ptr[4] = _mm512_loadu_si512(shift_mask_table_11u_4);
        shift_masks_ptr[5] = _mm512_loadu_si512(shift_mask_table_11u_5);

        for (uint32_t idx = 0; idx < num_elements_160; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            srcmm1 = _mm512_loadu_si512((src_ptr + 64U));
            srcmm2 = _mm512_loadu_si512((src_ptr + 128U));
            srcmm3 = _mm512_loadu_si512((src_ptr + 192U));
            srcmm4 = _mm512_loadu_si512((src_ptr + 256U));

            zmm0  = _mm512_maskz_permutex2var_epi16(permutex_masks_11u_ptr[0], srcmm0, permutex_idx_ptr[0], srcmm1);
            zmm1  = _mm512_maskz_permutex2var_epi16(permutex_masks_11u_ptr[1], srcmm0, permutex_idx_ptr[1], srcmm1);
            zmm2  = _mm512_maskz_permutex2var_epi16(permutex_masks_11u_ptr[2], srcmm0, permutex_idx_ptr[2], srcmm1);
            zmm3  = _mm512_maskz_permutex2var_epi16(permutex_masks_11u_ptr[3], srcmm1, permutex_idx_ptr[3], srcmm2);
            zmm4  = _mm512_maskz_permutex2var_epi16(permutex_masks_11u_ptr[4], srcmm1, permutex_idx_ptr[4], srcmm2);
            zmm5  = _mm512_maskz_permutex2var_epi16(permutex_masks_11u_ptr[5], srcmm1, permutex_idx_ptr[5], srcmm2);
            zmm6  = _mm512_maskz_permutex2var_epi16(permutex_masks_11u_ptr[0], srcmm2, permutex_idx_ptr[6], srcmm3);
            zmm7  = _mm512_maskz_permutex2var_epi16(permutex_masks_11u_ptr[1], srcmm2, permutex_idx_ptr[7], srcmm3);
            zmm8  = _mm512_maskz_permutex2var_epi16(permutex_masks_11u_ptr[2], srcmm2, permutex_idx_ptr[8], srcmm3);
            zmm9  = _mm512_maskz_permutex2var_epi16(permutex_masks_11u_ptr[3], srcmm3, permutex_idx_ptr[9], srcmm4);
            zmm10 = _mm512_maskz_permutex2var_epi16(permutex_masks_11u_ptr[4], srcmm3, permutex_idx_ptr[10], srcmm4);
            zmm11 = _mm512_maskz_permutex2var_epi16(permutex_masks_11u_ptr[5], srcmm3, permutex_idx_ptr[11], srcmm4);

            zmm0  = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
            zmm1  = _mm512_sllv_epi16(zmm1, shift_masks_ptr[1]);
            zmm2  = _mm512_srlv_epi16(zmm2, shift_masks_ptr[2]);
            zmm3  = _mm512_sllv_epi16(zmm3, shift_masks_ptr[3]);
            zmm4  = _mm512_sllv_epi16(zmm4, shift_masks_ptr[4]);
            zmm5  = _mm512_srlv_epi16(zmm5, shift_masks_ptr[5]);
            zmm6  = _mm512_sllv_epi16(zmm6, shift_masks_ptr[0]);
            zmm7  = _mm512_sllv_epi16(zmm7, shift_masks_ptr[1]);
            zmm8  = _mm512_srlv_epi16(zmm8, shift_masks_ptr[2]);
            zmm9  = _mm512_sllv_epi16(zmm9, shift_masks_ptr[3]);
            zmm10 = _mm512_sllv_epi16(zmm10, shift_masks_ptr[4]);
            zmm11 = _mm512_srlv_epi16(zmm11, shift_masks_ptr[5]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            zmm3 = _mm512_or_si512(zmm3, zmm4);
            zmm3 = _mm512_or_si512(zmm3, zmm5);
            zmm6 = _mm512_or_si512(zmm6, zmm7);
            zmm6 = _mm512_or_si512(zmm6, zmm8);
            zmm9 = _mm512_or_si512(zmm9, zmm10);
            zmm9 = _mm512_or_si512(zmm9, zmm11);

            _mm512_mask_storeu_epi16(dst_ptr, 0x0FFFFFFF, zmm0);
            _mm512_mask_storeu_epi16((dst_ptr + 56U), 0x07FFFFFF, zmm3);
            _mm512_mask_storeu_epi16((dst_ptr + 110U), 0x0FFFFFFF, zmm6);
            _mm512_mask_storeu_epi16((dst_ptr + 166U), 0x07FFFFFF, zmm9);

            src_ptr += 320U;
            dst_ptr += 11U * 20U;
        }

        for (uint32_t idx = 0; idx < num_elements_32; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_maskz_permutexvar_epi16(permutex_masks_11u_ptr[0], permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_maskz_permutexvar_epi16(permutex_masks_11u_ptr[1], permutex_idx_ptr[1], srcmm0);
            zmm2   = _mm512_maskz_permutexvar_epi16(permutex_masks_11u_ptr[2], permutex_idx_ptr[2], srcmm0);

            zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_sllv_epi16(zmm1, shift_masks_ptr[1]);
            zmm2 = _mm512_srlv_epi16(zmm2, shift_masks_ptr[2]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            zmm0 = _mm512_or_si512(zmm0, zmm2);
            _mm512_mask_storeu_epi16(dst_ptr, 0x003FFFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 11U * 4U;
        }
    }

    if (tail > 0) { k0_qplc_pack_16u11u_tail(src_ptr, tail, dst_ptr, 0U); }
}

OWN_QPLC_INLINE(void, k0_qplc_pack_16u12u_tail,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 12U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint16_t* src_16u_ptr = (uint16_t*)src_ptr;
    uint16_t* dst_16u_ptr = (uint16_t*)dst_ptr;
    uint32_t  src         = (uint32_t)(*dst_16u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint32_t)(*src_16u_ptr)) << start_bit;
    src_16u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_WORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_16u_ptr = (uint16_t)(src);
            dst_16u_ptr++;
            src = src >> OWN_WORD_WIDTH;
            bits_in_buf -= OWN_WORD_WIDTH;
        }
        src = src | (((uint32_t)(*src_16u_ptr)) << bits_in_buf);
        src_16u_ptr++;
        num_elements--;
        bits_in_buf += bit_width;
    }
    dst_ptr = (uint8_t*)dst_16u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_OPT_FUN(void, k0_qplc_pack_16u12u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 12U, 16U);
        if (align > num_elements) { align = num_elements; }
        k0_qplc_pack_16u12u_tail(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 2;
        dst_ptr += ((align * 12U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 32U;
    if (num_elements >= 32U) {
        uint32_t num_elements_128 = num_elements / 128U;
        uint32_t num_elements_32  = (num_elements % 128U) / 32U;
        __m512i  srcmm0, srcmm1, srcmm2, srcmm3;
        __m512i  zmm0, zmm1, zmm2, zmm3, zmm4, zmm5;

        __m512i permutex_idx_ptr[6];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_12u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_12u_1);
        permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_12u_2);
        permutex_idx_ptr[3] = _mm512_load_si512(permutex_idx_table_12u_3);
        permutex_idx_ptr[4] = _mm512_load_si512(permutex_idx_table_12u_4);
        permutex_idx_ptr[5] = _mm512_load_si512(permutex_idx_table_12u_5);

        __m512i shift_masks_ptr[6];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_12u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_12u_1);
        shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_12u_2);
        shift_masks_ptr[3] = _mm512_load_si512(shift_mask_table_12u_3);
        shift_masks_ptr[4] = _mm512_load_si512(shift_mask_table_12u_4);
        shift_masks_ptr[5] = _mm512_load_si512(shift_mask_table_12u_5);

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

            _mm512_storeu_si512(dst_ptr, zmm0);
            _mm512_storeu_si512((dst_ptr + 64U), zmm2);
            _mm512_storeu_si512((dst_ptr + 128U), zmm4);

            src_ptr += 256;
            dst_ptr += 12U * 16U;
        }

        for (uint32_t idx = 0; idx < num_elements_32; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_permutexvar_epi16(permutex_idx_ptr[1], srcmm0);

            zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_srlv_epi16(zmm1, shift_masks_ptr[1]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            _mm512_mask_storeu_epi16(dst_ptr, 0x00FFFFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 12U * 4U;
        }
    }

    if (tail > 0) { k0_qplc_pack_16u12u_tail(src_ptr, tail, dst_ptr, 0U); }
}

OWN_QPLC_INLINE(void, k0_qplc_pack_16u13u_tail,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 13U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint16_t* src_16u_ptr = (uint16_t*)src_ptr;
    uint16_t* dst_16u_ptr = (uint16_t*)dst_ptr;
    uint32_t  src         = (uint32_t)(*dst_16u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint32_t)(*src_16u_ptr)) << start_bit;
    src_16u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_WORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_16u_ptr = (uint16_t)(src);
            dst_16u_ptr++;
            src = src >> OWN_WORD_WIDTH;
            bits_in_buf -= OWN_WORD_WIDTH;
        }
        src = src | (((uint32_t)(*src_16u_ptr)) << bits_in_buf);
        src_16u_ptr++;
        num_elements--;
        bits_in_buf += bit_width;
    }
    dst_ptr = (uint8_t*)dst_16u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_OPT_FUN(void, k0_qplc_pack_16u13u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 13U, 16U);
        if (align > num_elements) { align = num_elements; }
        k0_qplc_pack_16u13u_tail(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 2;
        dst_ptr += ((align * 13U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 32U;
    if (num_elements >= 32U) {
        uint32_t num_elements_32 = num_elements / 32U;
        __m512i  srcmm0;
        __m512i  zmm0, zmm1, zmm2;

        __m512i permutex_idx_ptr[3];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_13u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_13u_1);
        permutex_idx_ptr[2] = _mm512_load_si512(permutex_idx_table_13u_2);

        __m512i shift_masks_ptr[3];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_13u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_13u_1);
        shift_masks_ptr[2] = _mm512_load_si512(shift_mask_table_13u_2);

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
            _mm512_mask_storeu_epi16(dst_ptr, 0x03FFFFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 13U * 4U;
        }
    }

    if (tail > 0) { k0_qplc_pack_16u13u_tail(src_ptr, tail, dst_ptr, 0U); }
}

OWN_QPLC_INLINE(void, k0_qplc_pack_16u14u_tail,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 14U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint16_t* src_16u_ptr = (uint16_t*)src_ptr;
    uint16_t* dst_16u_ptr = (uint16_t*)dst_ptr;
    uint32_t  src         = (uint32_t)(*dst_16u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint32_t)(*src_16u_ptr)) << start_bit;
    src_16u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_WORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_16u_ptr = (uint16_t)(src);
            dst_16u_ptr++;
            src = src >> OWN_WORD_WIDTH;
            bits_in_buf -= OWN_WORD_WIDTH;
        }
        src = src | (((uint32_t)(*src_16u_ptr)) << bits_in_buf);
        src_16u_ptr++;
        num_elements--;
        bits_in_buf += bit_width;
    }
    dst_ptr = (uint8_t*)dst_16u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_OPT_FUN(void, k0_qplc_pack_16u14u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 14U, 16U);
        if (align > num_elements) { align = num_elements; }
        k0_qplc_pack_16u14u_tail(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 2;
        dst_ptr += ((align * 14U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 32U;
    if (num_elements >= 32U) {
        uint32_t num_elements_32 = num_elements / 32U;
        __m512i  srcmm0;
        __m512i  zmm0, zmm1;

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_14u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_14u_1);

        __m512i shift_masks_ptr[2];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_14u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_14u_1);

        for (uint32_t idx = 0; idx < num_elements_32; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_permutexvar_epi16(permutex_idx_ptr[1], srcmm0);

            zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_srlv_epi16(zmm1, shift_masks_ptr[1]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            _mm512_mask_storeu_epi16(dst_ptr, 0x0FFFFFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 14U * 4U;
        }
    }

    if (tail > 0) { k0_qplc_pack_16u14u_tail(src_ptr, tail, dst_ptr, 0U); }
}

OWN_QPLC_INLINE(void, k0_qplc_pack_16u15u_tail,
                (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    uint32_t  bit_width   = 15U;
    int32_t   bits_in_buf = (int32_t)(bit_width + start_bit);
    uint16_t* src_16u_ptr = (uint16_t*)src_ptr;
    uint16_t* dst_16u_ptr = (uint16_t*)dst_ptr;
    uint32_t  src         = (uint32_t)(*dst_16u_ptr) & OWN_BIT_MASK(start_bit);

    src |= ((uint32_t)(*src_16u_ptr)) << start_bit;
    src_16u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_WORD_WIDTH <= (uint32_t)bits_in_buf) {
            *dst_16u_ptr = (uint16_t)(src);
            dst_16u_ptr++;
            src = src >> OWN_WORD_WIDTH;
            bits_in_buf -= OWN_WORD_WIDTH;
        }
        src = src | (((uint32_t)(*src_16u_ptr)) << bits_in_buf);
        src_16u_ptr++;
        num_elements--;
        bits_in_buf += bit_width;
    }
    dst_ptr = (uint8_t*)dst_16u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

OWN_OPT_FUN(void, k0_qplc_pack_16u15u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 15U, 16U);
        if (align > num_elements) { align = num_elements; }
        k0_qplc_pack_16u15u_tail(src_ptr, align, dst_ptr, start_bit);
        src_ptr += align * 2;
        dst_ptr += ((align * 15U) + start_bit) >> 3U;
        num_elements -= align;
    }

    uint32_t tail = num_elements % 32U;
    if (num_elements >= 32U) {
        uint32_t num_elements_32 = num_elements / 32U;
        __m512i  srcmm0;
        __m512i  zmm0, zmm1;

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_15u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_15u_1);

        __m512i shift_masks_ptr[2];
        shift_masks_ptr[0] = _mm512_load_si512(shift_mask_table_15u_0);
        shift_masks_ptr[1] = _mm512_load_si512(shift_mask_table_15u_1);

        for (uint32_t idx = 0; idx < num_elements_32; ++idx) {
            srcmm0 = _mm512_loadu_si512(src_ptr);
            zmm0   = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm0);
            zmm1   = _mm512_permutexvar_epi16(permutex_idx_ptr[1], srcmm0);

            zmm0 = _mm512_sllv_epi16(zmm0, shift_masks_ptr[0]);
            zmm1 = _mm512_srlv_epi16(zmm1, shift_masks_ptr[1]);

            zmm0 = _mm512_or_si512(zmm0, zmm1);
            _mm512_mask_storeu_epi16(dst_ptr, 0x3FFFFFFF, zmm0);

            src_ptr += 64U;
            dst_ptr += 15U * 4U;
        }
    }

    if (tail > 0) { k0_qplc_pack_16u15u_tail(src_ptr, tail, dst_ptr, 0U); }
}
#endif // OWN_PACK_8U_H
