/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_CORE_SW_SRC_FILTERING_OPT_QPLC_UNPACK_BE_16U_K0_H
#define QPL_SOURCES_CORE_SW_SRC_FILTERING_OPT_QPLC_UNPACK_BE_16U_K0_H

/**
 * @brief Contains implementation of functions for unpacking 9..16-bit BE data to words
 * @date 07/06/2020
 *
 * @details Function list:
 *          - @ref k0_qplc_unpack_be_9u16u
 *          - @ref k0_qplc_unpack_be_10u16u
 *          - @ref k0_qplc_unpack_be_11u16u
 *          - @ref k0_qplc_unpack_be_12u16u
 *          - @ref k0_qplc_unpack_be_13u16u
 *          - @ref k0_qplc_unpack_be_14u16u
 *          - @ref k0_qplc_unpack_be_15u16u
 *          - @ref k0_qplc_unpack_be_16u16u
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

OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_9u_0[64]) = {
        1U, 0U, 2U, 1U, 3U, 2U, 4U, 3U, 5U, 4U, 6U, 5U, 7U, 6U, 8U, 7U, 1U, 0U, 2U, 1U, 3U, 2U,
        4U, 3U, 5U, 4U, 6U, 5U, 7U, 6U, 8U, 7U, 1U, 0U, 2U, 1U, 3U, 2U, 4U, 3U, 5U, 4U, 6U, 5U,
        7U, 6U, 8U, 7U, 1U, 0U, 2U, 1U, 3U, 2U, 4U, 3U, 5U, 4U, 6U, 5U, 7U, 6U, 8U, 7U};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_table_9u_2[32])   = {7U, 6U, 5U, 4U, 3U, 2U, 1U, 0U, 7U, 6U, 5U,
                                                                4U, 3U, 2U, 1U, 0U, 7U, 6U, 5U, 4U, 3U, 2U,
                                                                1U, 0U, 7U, 6U, 5U, 4U, 3U, 2U, 1U, 0U};
OWN_ALIGNED_64_ARRAY(static uint64_t gather_idx_table_9u[8]) = {0U, 8U, 9U, 17U, 18U, 26U, 27U, 35U};

// ------------------------------------ 10u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_10u_0[64]) = {
        1U, 0U, 2U, 1U, 3U, 2U, 4U, 3U, 6U, 5U, 7U, 6U, 8U, 7U, 9U, 8U, 1U, 0U, 2U, 1U, 3U, 2U,
        4U, 3U, 6U, 5U, 7U, 6U, 8U, 7U, 9U, 8U, 1U, 0U, 2U, 1U, 3U, 2U, 4U, 3U, 6U, 5U, 7U, 6U,
        8U, 7U, 9U, 8U, 1U, 0U, 2U, 1U, 3U, 2U, 4U, 3U, 6U, 5U, 7U, 6U, 8U, 7U, 9U, 8U};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_table_10u[32])        = {6U, 4U, 2U, 0U, 6U, 4U, 2U, 0U, 6U, 4U, 2U,
                                                                    0U, 6U, 4U, 2U, 0U, 6U, 4U, 2U, 0U, 6U, 4U,
                                                                    2U, 0U, 6U, 4U, 2U, 0U, 6U, 4U, 2U, 0U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_10u[32]) = {
        0U,  1U,  2U,  3U,  4U,  0x0, 0x0, 0x0, 5U,  6U,  7U,  8U,  9U,  0x0, 0x0, 0x0,
        10U, 11U, 12U, 13U, 14U, 0x0, 0x0, 0x0, 15U, 16U, 17U, 18U, 19U, 0x0, 0x0, 0x0};

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

OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_11u_0[64]) = {
        3U,  2U,  1U, 0U, 5U, 4U, 3U,  2U,  8U, 7U, 6U, 5U, 11U, 10U, 9U, 8U, 3U,  2U,  1U, 0U, 5U, 4U,
        3U,  2U,  8U, 7U, 6U, 5U, 11U, 10U, 9U, 8U, 3U, 2U, 1U,  0U,  5U, 4U, 3U,  2U,  8U, 7U, 6U, 5U,
        11U, 10U, 9U, 8U, 3U, 2U, 1U,  0U,  5U, 4U, 3U, 2U, 8U,  7U,  6U, 5U, 11U, 10U, 9U, 8U};
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_11u_1[64]) = {
        3U,  2U,  1U, 0U, 6U, 5U, 4U,  0U,  8U, 7U, 6U, 0U, 11U, 10U, 9U, 0U, 3U,  2U,  1U, 0U, 6U, 5U,
        4U,  0U,  8U, 7U, 6U, 0U, 11U, 10U, 9U, 0U, 3U, 2U, 1U,  0U,  6U, 5U, 4U,  0U,  8U, 7U, 6U, 0U,
        11U, 10U, 9U, 0U, 3U, 2U, 1U,  0U,  6U, 5U, 4U, 0U, 8U,  7U,  6U, 0U, 11U, 10U, 9U, 0U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_11u_2[16])   = {21U, 15U, 17U, 19U, 21U, 15U, 17U, 19U,
                                                                 21U, 15U, 17U, 19U, 21U, 15U, 17U, 19U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_11u_3[16])   = {6U, 4U, 10U, 8U, 6U, 4U, 10U, 8U,
                                                                 6U, 4U, 10U, 8U, 6U, 4U, 10U, 8U};
OWN_ALIGNED_64_ARRAY(static uint64_t gather_idx_table_11u[8]) = {0U, 8U, 11U, 19U, 22U, 30U, 33U, 41U};

// ------------------------------------ 12u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_12u_0[64]) = {
        1U,  0U, 2U,  1U,  4U, 3U, 5U,  4U, 7U,  6U,  8U, 7U, 10U, 9U, 11U, 10U, 1U,  0U, 2U,  1U, 4U, 3U,
        5U,  4U, 7U,  6U,  8U, 7U, 10U, 9U, 11U, 10U, 1U, 0U, 2U,  1U, 4U,  3U,  5U,  4U, 7U,  6U, 8U, 7U,
        10U, 9U, 11U, 10U, 1U, 0U, 2U,  1U, 4U,  3U,  5U, 4U, 7U,  6U, 8U,  7U,  10U, 9U, 11U, 10U};
OWN_ALIGNED_64_ARRAY(static uint16_t shift_table_12u[32])        = {4U, 0U, 4U, 0U, 4U, 0U, 4U, 0U, 4U, 0U, 4U,
                                                                    0U, 4U, 0U, 4U, 0U, 4U, 0U, 4U, 0U, 4U, 0U,
                                                                    4U, 0U, 4U, 0U, 4U, 0U, 4U, 0U, 4U, 0U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_12u[16]) = {0U, 1U, 2U, 0x0, 3U, 4U,  5U,  0x0,
                                                                    6U, 7U, 8U, 0x0, 9U, 10U, 11U, 0x0};

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

OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_13u_0[64]) = {
        3U,  2U,  1U,  0U, 6U, 5U, 4U,  3U,  9U,  8U, 7U, 6U, 12U, 11U, 10U, 9U, 3U,  2U,  1U,  0U, 6U, 5U,
        4U,  3U,  9U,  8U, 7U, 6U, 12U, 11U, 10U, 9U, 3U, 2U, 1U,  0U,  6U,  5U, 4U,  3U,  9U,  8U, 7U, 6U,
        12U, 11U, 10U, 9U, 3U, 2U, 1U,  0U,  6U,  5U, 4U, 3U, 9U,  8U,  7U,  6U, 12U, 11U, 10U, 9U};
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_13u_1[64]) = {
        3U,  2U,  1U,  0U, 6U, 5U, 4U,  0U,  10U, 9U, 8U, 0U, 13U, 12U, 11U, 0U, 3U,  2U,  1U,  0U, 6U, 5U,
        4U,  0U,  10U, 9U, 8U, 0U, 13U, 12U, 11U, 0U, 3U, 2U, 1U,  0U,  6U,  5U, 4U,  0U,  10U, 9U, 8U, 0U,
        13U, 12U, 11U, 0U, 3U, 2U, 1U,  0U,  6U,  5U, 4U, 0U, 10U, 9U,  8U,  0U, 13U, 12U, 11U, 0U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_13u_2[16])   = {19U, 17U, 15U, 13U, 19U, 17U, 15U, 13U,
                                                                 19U, 17U, 15U, 13U, 19U, 17U, 15U, 13U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_13u_3[16])   = {10U, 12U, 6U, 8U, 10U, 12U, 6U, 8U,
                                                                 10U, 12U, 6U, 8U, 10U, 12U, 6U, 8U};
OWN_ALIGNED_64_ARRAY(static uint64_t gather_idx_table_13u[8]) = {0U, 8U, 13U, 21U, 26U, 34U, 39U, 47U};

// ------------------------------------ 14u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_14u_0[64]) = {
        3U,  2U,  1U,  0U,  6U, 5U, 4U,  3U,  10U, 9U,  8U, 7U, 13U, 12U, 11U, 10U, 3U,  2U,  1U,  0U, 6U, 5U,
        4U,  3U,  10U, 9U,  8U, 7U, 13U, 12U, 11U, 10U, 3U, 2U, 1U,  0U,  6U,  5U,  4U,  3U,  10U, 9U, 8U, 7U,
        13U, 12U, 11U, 10U, 3U, 2U, 1U,  0U,  6U,  5U,  4U, 3U, 10U, 9U,  8U,  7U,  13U, 12U, 11U, 10U};
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_14u_1[64]) = {
        3U,  2U,  1U,  0U, 7U, 6U, 5U,  0U,  10U, 9U, 8U, 0U, 14U, 13U, 12U, 0U, 3U,  2U,  1U,  0U, 7U, 6U,
        5U,  0U,  10U, 9U, 8U, 0U, 14U, 13U, 12U, 0U, 3U, 2U, 1U,  0U,  7U,  6U, 5U,  0U,  10U, 9U, 8U, 0U,
        14U, 13U, 12U, 0U, 3U, 2U, 1U,  0U,  7U,  6U, 5U, 0U, 10U, 9U,  8U,  0U, 14U, 13U, 12U, 0U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_14u_0[16])      = {18U, 14U, 18U, 14U, 18U, 14U, 18U, 14U,
                                                                    18U, 14U, 18U, 14U, 18U, 14U, 18U, 14U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_14u_1[16])      = {12U, 8U, 12U, 8U, 12U, 8U, 12U, 8U,
                                                                    12U, 8U, 12U, 8U, 12U, 8U, 12U, 8U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_14u[32]) = {
        0U,  1U,  2U,  3U,  4U,  5U,  6U,  0x0, 7U,  8U,  9U,  10U, 11U, 12U, 13U, 0x0,
        14U, 15U, 16U, 17U, 18U, 19U, 20U, 0x0, 21U, 22U, 23U, 24U, 25U, 26U, 27U, 0x0};

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

OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_15u_0[64]) = {
        3U,  2U,  1U,  0U,  6U, 5U, 4U,  3U,  10U, 9U,  8U, 7U, 14U, 13U, 12U, 11U, 3U,  2U,  1U,  0U, 6U, 5U,
        4U,  3U,  10U, 9U,  8U, 7U, 14U, 13U, 12U, 11U, 3U, 2U, 1U,  0U,  6U,  5U,  4U,  3U,  10U, 9U, 8U, 7U,
        14U, 13U, 12U, 11U, 3U, 2U, 1U,  0U,  6U,  5U,  4U, 3U, 10U, 9U,  8U,  7U,  14U, 13U, 12U, 11U};
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_15u_1[64]) = {
        3U,  2U,  1U,  0U,  7U, 6U, 5U,  0U,  11U, 10U, 9U, 0U, 15U, 14U, 13U, 0U, 3U,  2U,  1U,  0U,  7U, 6U,
        5U,  0U,  11U, 10U, 9U, 0U, 15U, 14U, 13U, 0U,  3U, 2U, 1U,  0U,  7U,  6U, 5U,  0U,  11U, 10U, 9U, 0U,
        15U, 14U, 13U, 0U,  3U, 2U, 1U,  0U,  7U,  6U,  5U, 0U, 11U, 10U, 9U,  0U, 15U, 14U, 13U, 0U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_15u_2[16])   = {17U, 11U, 13U, 15U, 17U, 11U, 13U, 15U,
                                                                 17U, 11U, 13U, 15U, 17U, 11U, 13U, 15U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_15u_3[16])   = {14U, 12U, 10U, 8U, 14U, 12U, 10U, 8U,
                                                                 14U, 12U, 10U, 8U, 14U, 12U, 10U, 8U};
OWN_ALIGNED_64_ARRAY(static uint64_t gather_idx_table_15u[8]) = {0U, 8U, 15U, 23U, 30U, 38U, 45U, 53U};

/*
0 -> 00
1 -> 08
2 -> 04
3 -> 0C
4 -> 02
5 -> 0A
6 -> 06
7 -> 0E
8 -> 01
9 -> 09
A -> 05
B -> 0D
C -> 03
D -> 0B
E -> 07
F -> 0F
*/
OWN_ALIGNED_64_ARRAY(static uint64_t nibble_reverse_table[8]) = {
        0x0E060A020C040800, 0x0F070B030D050901, 0x0E060A020C040800, 0x0F070B030D050901,
        0x0E060A020C040800, 0x0F070B030D050901, 0x0E060A020C040800, 0x0F070B030D050901};

OWN_ALIGNED_64_ARRAY(static uint64_t reverse_mask_table_16u[8]) = {
        0x0607040502030001, 0x0E0F0C0D0A0B0809, 0x1617141512131011, 0x1E1F1C1D1A1B1819,
        0x2627242522232021, 0x2E2F2C2D2A2B2829, 0x3637343532333031, 0x3E3F3C3D3A3B3839};

OWN_QPLC_INLINE(uint32_t, own_get_align, (uint32_t start_bit, uint32_t base, uint32_t bitsize)) {
    uint32_t remnant = bitsize - start_bit;
    for (uint32_t i = 0U; i < bitsize; ++i) {
        uint32_t test_value = (i * base) % bitsize;
        if (test_value == remnant) { return i; }
    }
    return 0xFFFFFFFF;
}

// For BE start_bit is bit index from the top of a byte
OWN_QPLC_INLINE(void, px_qplc_unpack_be_Nu16u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint32_t bit_width,
                 uint8_t* dst_ptr)) {
    uint16_t* src16u_ptr  = (uint16_t*)src_ptr;
    uint16_t* dst16u_ptr  = (uint16_t*)dst_ptr;
    uint32_t  bits_in_buf = OWN_WORD_WIDTH - start_bit;
    uint32_t  shift       = OWN_DWORD_WIDTH - bit_width;
    uint32_t  src         = ((uint32_t)qplc_swap_bytes_16u(*src16u_ptr)) << (OWN_DWORD_WIDTH - bits_in_buf);
    uint32_t  next_word   = 0U;

    src16u_ptr++;

    while (1U < num_elements) {
        if (bit_width > bits_in_buf) {
            next_word = (uint32_t)qplc_swap_bytes_16u(*src16u_ptr);
            src16u_ptr++;
            next_word = next_word << (OWN_WORD_WIDTH - bits_in_buf);
            src       = src | next_word;
            bits_in_buf += OWN_WORD_WIDTH;
        }
        *dst16u_ptr = (uint16_t)(src >> shift);
        src         = src << bit_width;
        bits_in_buf -= bit_width;
        dst16u_ptr++;
        num_elements--;
    }

    uint8_t* src8u_ptr = (uint8_t*)src16u_ptr;
    if (bit_width > bits_in_buf) {
        uint32_t bytes_to_read = OWN_BITS_2_BYTE(bit_width - bits_in_buf);
        if (bytes_to_read == 2U) {
            next_word = *((uint16_t*)src8u_ptr);
            src8u_ptr += 2;
        } else {
            next_word = *src8u_ptr;
            src8u_ptr++;
        }
        next_word = (uint32_t)qplc_swap_bytes_16u(next_word);
        next_word = next_word << (OWN_WORD_WIDTH - bits_in_buf);
        src       = src | next_word;
        bits_in_buf += OWN_WORD_WIDTH;
    }
    *dst16u_ptr = (uint16_t)(src >> shift);
    src         = src << bit_width;
    bits_in_buf -= bit_width;
    dst16u_ptr++;
    num_elements--;
}

// ********************** 9u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_9u16u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 9U, 16U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu16u(src_ptr, align, start_bit, 9U, dst_ptr);
        src_ptr += ((align * 9U) + start_bit) >> 3;
        dst_ptr += align * 2;
        num_elements -= align;
    }

    if (num_elements >= 32U) {
        __mmask32 read_mask        = OWN_BIT_MASK(OWN_BITS_2_WORD(9U * OWN_DWORD_WIDTH));
        __m512i   parse_mask0      = _mm512_set1_epi16(OWN_BIT_MASK(9U));
        __m512i   nibble_reversemm = _mm512_load_si512(nibble_reverse_table);
        __m512i   reverse_mask_16u = _mm512_load_si512(reverse_mask_table_16u);
        __m512i   maskmm           = _mm512_set1_epi8(0x0F);

        __m512i shuffle_idx_ptr = _mm512_load_si512(shuffle_idx_table_9u_0);

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_9u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_9u_1);

        __m512i shift_mask_ptr[3];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_9u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_9u_1);
        shift_mask_ptr[2] = _mm512_load_si512(shift_table_9u_2);

        __m512i gather_idxmm = _mm512_load_si512(gather_idx_table_9u);

        while (num_elements >= 64U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_i64gather_epi64(gather_idxmm, src_ptr, 1U);

            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi16(zmm[0], shift_mask_ptr[2]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 4U * 9U;
            dst_ptr += 64U;
            num_elements -= 32U;
        }
        if (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            __m512i low_nibblemm  = _mm512_and_si512(srcmm, maskmm);
            __m512i high_nibblemm = _mm512_srli_epi16(srcmm, 4U);
            high_nibblemm         = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            srcmm = _mm512_or_si512(low_nibblemm, high_nibblemm);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi16(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi32(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi16(zmm[0], 0xAAAAAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            zmm[0] = _mm512_slli_epi16(zmm[0], 7U);

            low_nibblemm  = _mm512_and_si512(zmm[0], maskmm);
            high_nibblemm = _mm512_srli_epi16(zmm[0], 4U);
            high_nibblemm = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            zmm[0] = _mm512_or_si512(low_nibblemm, high_nibblemm);
            zmm[0] = _mm512_shuffle_epi8(zmm[0], reverse_mask_16u);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 4U * 9U;
            dst_ptr += 64U;
            num_elements -= 32U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu16u(src_ptr, num_elements, 0U, 9U, dst_ptr); }
}

// ********************** 10u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_10u16u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 10U, 16U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu16u(src_ptr, align, start_bit, 10U, dst_ptr);
        src_ptr += ((align * 10U) + start_bit) >> 3;
        dst_ptr += align * 2;
        num_elements -= align;
    }

    if (num_elements >= 32U) {
        __mmask32 read_mask   = OWN_BIT_MASK(OWN_BITS_2_WORD(10U * OWN_DWORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi16(OWN_BIT_MASK(10U));

        __m512i shuffle_idx_ptr = _mm512_load_si512(shuffle_idx_table_10u_0);
        __m512i permutex_idx    = _mm512_load_si512(permutex_idx_table_10u);
        __m512i shift_mask      = _mm512_load_si512(shift_table_10u);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm;

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            zmm = _mm512_permutexvar_epi16(permutex_idx, srcmm);
            zmm = _mm512_shuffle_epi8(zmm, shuffle_idx_ptr);

            // shifting elements so they start from the start of the word
            zmm = _mm512_srlv_epi16(zmm, shift_mask);
            zmm = _mm512_and_si512(zmm, parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm);

            src_ptr += 4U * 10U;
            dst_ptr += 64U;
            num_elements -= 32U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu16u(src_ptr, num_elements, 0U, 10U, dst_ptr); }
}

// ********************** 11u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_11u16u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 11U, 16U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu16u(src_ptr, align, start_bit, 11U, dst_ptr);
        src_ptr += ((align * 11U) + start_bit) >> 3;
        dst_ptr += align * 2;
        num_elements -= align;
    }

    if (num_elements >= 32U) {
        __mmask32 read_mask        = OWN_BIT_MASK(OWN_BITS_2_WORD(11U * OWN_DWORD_WIDTH));
        __m512i   parse_mask0      = _mm512_set1_epi16(OWN_BIT_MASK(11U));
        __m512i   nibble_reversemm = _mm512_load_si512(nibble_reverse_table);
        __m512i   reverse_mask_16u = _mm512_load_si512(reverse_mask_table_16u);
        __m512i   maskmm           = _mm512_set1_epi8(0x0F);

        __m512i shuffle_idx_ptr[2];
        shuffle_idx_ptr[0] = _mm512_load_si512(shuffle_idx_table_11u_0);
        shuffle_idx_ptr[1] = _mm512_load_si512(shuffle_idx_table_11u_1);

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_11u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_11u_1);

        __m512i shift_mask_ptr[4];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_11u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_11u_1);
        shift_mask_ptr[2] = _mm512_load_si512(shift_table_11u_2);
        shift_mask_ptr[3] = _mm512_load_si512(shift_table_11u_3);

        __m512i gather_idxmm = _mm512_load_si512(gather_idx_table_11u);

        while (num_elements >= 64U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_i64gather_epi64(gather_idxmm, src_ptr, 1U);

            // shuffling so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[0]);
            zmm[1] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[1]);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[2]);
            zmm[1] = _mm512_sllv_epi32(zmm[1], shift_mask_ptr[3]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi16(zmm[0], 0xAAAAAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 4U * 11U;
            dst_ptr += 64U;
            num_elements -= 32U;
        }
        if (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            __m512i low_nibblemm  = _mm512_and_si512(srcmm, maskmm);
            __m512i high_nibblemm = _mm512_srli_epi16(srcmm, 4U);
            high_nibblemm         = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            srcmm = _mm512_or_si512(low_nibblemm, high_nibblemm);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi16(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi32(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi16(zmm[0], 0xAAAAAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            zmm[0] = _mm512_slli_epi16(zmm[0], 5U);

            low_nibblemm  = _mm512_and_si512(zmm[0], maskmm);
            high_nibblemm = _mm512_srli_epi16(zmm[0], 4U);
            high_nibblemm = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            zmm[0] = _mm512_or_si512(low_nibblemm, high_nibblemm);
            zmm[0] = _mm512_shuffle_epi8(zmm[0], reverse_mask_16u);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 4U * 11U;
            dst_ptr += 64U;
            num_elements -= 32U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu16u(src_ptr, num_elements, 0U, 11U, dst_ptr); }
}

// ********************** 12u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_12u16u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 12U, 16U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu16u(src_ptr, align, start_bit, 12U, dst_ptr);
        src_ptr += ((align * 12U) + start_bit) >> 3;
        dst_ptr += align * 2;
        num_elements -= align;
    }

    if (num_elements >= 32U) {
        __mmask32 read_mask   = OWN_BIT_MASK(OWN_BITS_2_WORD(12U * OWN_DWORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi16(OWN_BIT_MASK(12U));

        __m512i shuffle_idx_ptr = _mm512_load_si512(shuffle_idx_table_12u_0);
        __m512i permutex_idx    = _mm512_load_si512(permutex_idx_table_12u);
        __m512i shift_mask      = _mm512_load_si512(shift_table_12u);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm;

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            zmm = _mm512_permutexvar_epi32(permutex_idx, srcmm);
            zmm = _mm512_shuffle_epi8(zmm, shuffle_idx_ptr);

            // shifting elements so they start from the start of the word
            zmm = _mm512_srlv_epi16(zmm, shift_mask);
            zmm = _mm512_and_si512(zmm, parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm);

            src_ptr += 4U * 12U;
            dst_ptr += 64U;
            num_elements -= 32U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu16u(src_ptr, num_elements, 0U, 12U, dst_ptr); }
}

// ********************** 13u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_13u16u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 13U, 16U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu16u(src_ptr, align, start_bit, 13U, dst_ptr);
        src_ptr += ((align * 13U) + start_bit) >> 3;
        dst_ptr += align * 2;
        num_elements -= align;
    }

    if (num_elements >= 32U) {
        __mmask32 read_mask        = OWN_BIT_MASK(OWN_BITS_2_WORD(13U * OWN_DWORD_WIDTH));
        __m512i   parse_mask0      = _mm512_set1_epi16(OWN_BIT_MASK(13U));
        __m512i   nibble_reversemm = _mm512_load_si512(nibble_reverse_table);
        __m512i   reverse_mask_16u = _mm512_load_si512(reverse_mask_table_16u);
        __m512i   maskmm           = _mm512_set1_epi8(0x0F);

        __m512i shuffle_idx_ptr[2];
        shuffle_idx_ptr[0] = _mm512_load_si512(shuffle_idx_table_13u_0);
        shuffle_idx_ptr[1] = _mm512_load_si512(shuffle_idx_table_13u_1);

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_13u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_13u_1);

        __m512i shift_mask_ptr[4];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_13u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_13u_1);
        shift_mask_ptr[2] = _mm512_load_si512(shift_table_13u_2);
        shift_mask_ptr[3] = _mm512_load_si512(shift_table_13u_3);

        __m512i gather_idxmm = _mm512_load_si512(gather_idx_table_13u);

        while (num_elements >= 64U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_i64gather_epi64(gather_idxmm, src_ptr, 1U);

            // shuffling so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[0]);
            zmm[1] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[1]);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[2]);
            zmm[1] = _mm512_sllv_epi32(zmm[1], shift_mask_ptr[3]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi16(zmm[0], 0xAAAAAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 4U * 13U;
            dst_ptr += 64U;
            num_elements -= 32U;
        }
        if (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            __m512i low_nibblemm  = _mm512_and_si512(srcmm, maskmm);
            __m512i high_nibblemm = _mm512_srli_epi16(srcmm, 4U);
            high_nibblemm         = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            srcmm = _mm512_or_si512(low_nibblemm, high_nibblemm);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi16(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi32(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi16(zmm[0], 0xAAAAAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            zmm[0] = _mm512_slli_epi16(zmm[0], 3U);

            low_nibblemm  = _mm512_and_si512(zmm[0], maskmm);
            high_nibblemm = _mm512_srli_epi16(zmm[0], 4U);
            high_nibblemm = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            zmm[0] = _mm512_or_si512(low_nibblemm, high_nibblemm);
            zmm[0] = _mm512_shuffle_epi8(zmm[0], reverse_mask_16u);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 4U * 13U;
            dst_ptr += 64U;
            num_elements -= 32U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu16u(src_ptr, num_elements, 0U, 13U, dst_ptr); }
}

// ********************** 14u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_14u16u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 14U, 16U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu16u(src_ptr, align, start_bit, 14U, dst_ptr);
        src_ptr += ((align * 14U) + start_bit) >> 3;
        dst_ptr += align * 2;
        num_elements -= align;
    }

    if (num_elements >= 32U) {
        __mmask32 read_mask   = OWN_BIT_MASK(OWN_BITS_2_WORD(14U * OWN_DWORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi16(OWN_BIT_MASK(14U));

        __m512i shuffle_idx_ptr[2];
        shuffle_idx_ptr[0] = _mm512_load_si512(shuffle_idx_table_14u_0);
        shuffle_idx_ptr[1] = _mm512_load_si512(shuffle_idx_table_14u_1);

        __m512i permutex_idx = _mm512_load_si512(permutex_idx_table_14u);

        __m512i shift_mask_ptr[2];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_14u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_14u_1);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);
            srcmm = _mm512_permutexvar_epi16(permutex_idx, srcmm);

            // shuffling so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[0]);
            zmm[1] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[1]);

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

    if (num_elements > 0) { px_qplc_unpack_be_Nu16u(src_ptr, num_elements, 0U, 14U, dst_ptr); }
}

// ********************** 15u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_15u16u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 15U, 16U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu16u(src_ptr, align, start_bit, 15U, dst_ptr);
        src_ptr += ((align * 15U) + start_bit) >> 3;
        dst_ptr += align * 2;
        num_elements -= align;
    }

    if (num_elements >= 32U) {
        __mmask32 read_mask        = OWN_BIT_MASK(OWN_BITS_2_WORD(15U * OWN_DWORD_WIDTH));
        __m512i   parse_mask0      = _mm512_set1_epi16(OWN_BIT_MASK(15U));
        __m512i   nibble_reversemm = _mm512_load_si512(nibble_reverse_table);
        __m512i   reverse_mask_16u = _mm512_load_si512(reverse_mask_table_16u);
        __m512i   maskmm           = _mm512_set1_epi8(0x0F);

        __m512i shuffle_idx_ptr[2];
        shuffle_idx_ptr[0] = _mm512_load_si512(shuffle_idx_table_15u_0);
        shuffle_idx_ptr[1] = _mm512_load_si512(shuffle_idx_table_15u_1);

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_15u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_15u_1);

        __m512i shift_mask_ptr[4];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_15u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_15u_1);
        shift_mask_ptr[2] = _mm512_load_si512(shift_table_15u_2);
        shift_mask_ptr[3] = _mm512_load_si512(shift_table_15u_3);

        __m512i gather_idxmm = _mm512_load_si512(gather_idx_table_15u);

        while (num_elements >= 64U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_i64gather_epi64(gather_idxmm, src_ptr, 1U);

            // shuffling so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[0]);
            zmm[1] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[1]);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[2]);
            zmm[1] = _mm512_sllv_epi32(zmm[1], shift_mask_ptr[3]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi16(zmm[0], 0xAAAAAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 4U * 15U;
            dst_ptr += 64U;
            num_elements -= 32U;
        }
        if (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi16(read_mask, src_ptr);

            __m512i low_nibblemm  = _mm512_and_si512(srcmm, maskmm);
            __m512i high_nibblemm = _mm512_srli_epi16(srcmm, 4U);
            high_nibblemm         = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            srcmm = _mm512_or_si512(low_nibblemm, high_nibblemm);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi16(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi16(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi32(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi16(zmm[0], 0xAAAAAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            zmm[0] = _mm512_slli_epi16(zmm[0], 1U);

            low_nibblemm  = _mm512_and_si512(zmm[0], maskmm);
            high_nibblemm = _mm512_srli_epi16(zmm[0], 4U);
            high_nibblemm = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            zmm[0] = _mm512_or_si512(low_nibblemm, high_nibblemm);
            zmm[0] = _mm512_shuffle_epi8(zmm[0], reverse_mask_16u);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 4U * 15U;
            dst_ptr += 64U;
            num_elements -= 32U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu16u(src_ptr, num_elements, 0U, 15U, dst_ptr); }
}

// ********************** 16u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_16u16u, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    if (num_elements >= 32U) {
        __m512i reverse_mask_16u = _mm512_load_si512(reverse_mask_table_16u);
        while (num_elements >= 32U) {
            __m512i srcmm = _mm512_loadu_si512(src_ptr);
            srcmm         = _mm512_shuffle_epi8(srcmm, reverse_mask_16u);
            _mm512_storeu_si512(dst_ptr, srcmm);

            src_ptr += 4U * 16U;
            dst_ptr += 64U;
            num_elements -= 32U;
        }
    }

    uint16_t* src16u_ptr = (uint16_t*)src_ptr;
    uint16_t* dst16u_ptr = (uint16_t*)dst_ptr;

    for (uint32_t i = 0U; i < num_elements; i++) {
        dst16u_ptr[i] = qplc_swap_bytes_16u(src16u_ptr[i]);
    }
}

#endif
