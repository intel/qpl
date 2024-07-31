/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_CORE_SW_SRC_FILTERING_OPT_QPLC_UNPACK_BE_32U_K0_H
#define QPL_SOURCES_CORE_SW_SRC_FILTERING_OPT_QPLC_UNPACK_BE_32U_K0_H

/**
 * @brief Contains implementation of functions for unpacking 17..32-bit BE data to dwords
 * @date 07/06/2020
 *
 * @details Function list:
 *          - @ref qplc_unpack_be_17u32u
 *          - @ref k0_qplc_unpack_be_18u32u
 *          - @ref k0_qplc_unpack_be_19u32u
 *          - @ref k0_qplc_unpack_be_20u32u
 *          - @ref k0_qplc_unpack_be_21u32u
 *          - @ref k0_qplc_unpack_be_22u32u
 *          - @ref k0_qplc_unpack_be_23u32u
 *          - @ref k0_qplc_unpack_be_24u32u
 *          - @ref k0_qplc_unpack_be_25u32u
 *          - @ref k0_qplc_unpack_be_26u32u
 *          - @ref k0_qplc_unpack_be_27u32u
 *          - @ref k0_qplc_unpack_be_28u32u
 *          - @ref k0_qplc_unpack_be_29u32u
 *          - @ref k0_qplc_unpack_be_30u32u
 *          - @ref k0_qplc_unpack_be_31u32u
 *          - @ref k0_qplc_unpack_be_32u32u
 *
 */

#include "own_qplc_defs.h"

// ------------------------------------ 17u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_17u_0[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 3U, 4U,
                                                                      4U, 5U, 5U, 6U, 6U, 7U, 7U, 8U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_17u_1[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 3U, 4U,
                                                                      4U, 5U, 5U, 6U, 6U, 7U, 7U, 8U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_17u_0[8])         = {0U, 2U, 4U, 6U, 8U, 10U, 12U, 14U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_17u_1[8])         = {15U, 13U, 11U, 9U, 7U, 5U, 3U, 1U};

OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_17u_0[64]) = {
        3U, 2U, 1U, 0U, 5U, 4U, 3U, 2U, 7U, 6U, 5U, 4U, 9U, 8U, 7U, 6U, 3U, 2U, 1U, 0U, 5U, 4U,
        3U, 2U, 7U, 6U, 5U, 4U, 9U, 8U, 7U, 6U, 3U, 2U, 1U, 0U, 5U, 4U, 3U, 2U, 7U, 6U, 5U, 4U,
        9U, 8U, 7U, 6U, 3U, 2U, 1U, 0U, 5U, 4U, 3U, 2U, 7U, 6U, 5U, 4U, 9U, 8U, 7U, 6U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_17u_2[16])   = {15U, 14U, 13U, 12U, 11U, 10U, 9U, 8U,
                                                                 15U, 14U, 13U, 12U, 11U, 10U, 9U, 8U};
OWN_ALIGNED_64_ARRAY(static uint64_t gather_idx_table_17u[8]) = {0U, 8U, 8U, 16U, 17U, 25U, 25U, 33U};

// ------------------------------------ 18u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_18u_0[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 3U, 4U,
                                                                      4U, 5U, 5U, 6U, 6U, 7U, 7U, 8U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_18u_1[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 3U, 4U,
                                                                      5U, 6U, 6U, 7U, 7U, 8U, 8U, 9U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_18u_0[8])         = {0U, 4U, 8U, 12U, 16U, 20U, 24U, 28U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_18u_1[8])         = {14U, 10U, 6U, 2U, 30U, 26U, 22U, 18U};

OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_18u_0[64]) = {
        3U, 2U, 1U, 0U, 5U, 4U, 3U, 2U, 7U, 6U, 5U, 4U, 9U, 8U, 7U, 6U, 3U, 2U, 1U, 0U, 5U, 4U,
        3U, 2U, 7U, 6U, 5U, 4U, 9U, 8U, 7U, 6U, 3U, 2U, 1U, 0U, 5U, 4U, 3U, 2U, 7U, 6U, 5U, 4U,
        9U, 8U, 7U, 6U, 3U, 2U, 1U, 0U, 5U, 4U, 3U, 2U, 7U, 6U, 5U, 4U, 9U, 8U, 7U, 6U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_18u_2[16])   = {14U, 12U, 10U, 8U, 14U, 12U, 10U, 8U,
                                                                 14U, 12U, 10U, 8U, 14U, 12U, 10U, 8U};
OWN_ALIGNED_64_ARRAY(static uint64_t gather_idx_table_18u[8]) = {0U, 8U, 9U, 17U, 18U, 26U, 27U, 35U};

// ------------------------------------ 19u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_19u_0[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 3U, 4U,
                                                                      4U, 5U, 5U, 6U, 7U, 8U, 8U, 9U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_19u_1[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 4U, 5U,
                                                                      5U, 6U, 6U, 7U, 7U, 8U, 8U, 9U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_19u_0[8])         = {0U, 6U, 12U, 18U, 24U, 30U, 4U, 10U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_19u_1[8])         = {13U, 7U, 1U, 27U, 21U, 15U, 9U, 3U};

OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_19u_0[64]) = {
        3U,  2U, 1U, 0U, 5U, 4U, 3U,  2U, 7U, 6U, 5U, 4U, 10U, 9U, 8U, 7U, 3U,  2U, 1U, 0U, 5U, 4U,
        3U,  2U, 8U, 7U, 6U, 5U, 10U, 9U, 8U, 7U, 3U, 2U, 1U,  0U, 5U, 4U, 3U,  2U, 7U, 6U, 5U, 4U,
        10U, 9U, 8U, 7U, 3U, 2U, 1U,  0U, 5U, 4U, 3U, 2U, 8U,  7U, 6U, 5U, 10U, 9U, 8U, 7U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_19u_2[16])   = {13U, 10U, 7U, 12U, 9U, 6U, 11U, 8U,
                                                                 13U, 10U, 7U, 12U, 9U, 6U, 11U, 8U};
OWN_ALIGNED_64_ARRAY(static uint64_t gather_idx_table_19u[8]) = {0U, 8U, 9U, 17U, 19U, 27U, 28U, 36U};

// ------------------------------------ 20u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_20u_0[64]) = {
        3U,  2U, 1U, 0U, 5U, 4U, 3U,  2U, 8U, 7U, 6U, 5U, 10U, 9U, 8U, 7U, 3U,  2U, 1U, 0U, 5U, 4U,
        3U,  2U, 8U, 7U, 6U, 5U, 10U, 9U, 8U, 7U, 3U, 2U, 1U,  0U, 5U, 4U, 3U,  2U, 8U, 7U, 6U, 5U,
        10U, 9U, 8U, 7U, 3U, 2U, 1U,  0U, 5U, 4U, 3U, 2U, 8U,  7U, 6U, 5U, 10U, 9U, 8U, 7U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_20u[16])        = {12U, 8U, 12U, 8U, 12U, 8U, 12U, 8U,
                                                                    12U, 8U, 12U, 8U, 12U, 8U, 12U, 8U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_20u[32]) = {
        0U,  1U,  2U,  3U,  4U,  0x0, 0x0, 0x0, 5U,  6U,  7U,  8U,  9U,  0x0, 0x0, 0x0,
        10U, 11U, 12U, 13U, 14U, 0x0, 0x0, 0x0, 15U, 16U, 17U, 18U, 19U, 0x0, 0x0, 0x0};

// ------------------------------------ 21u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_21u_0[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 3U, 4U,
                                                                      5U, 6U, 6U, 7U, 7U, 8U, 9U, 10U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_21u_1[16]) = {0U, 1U, 1U, 2U, 3U, 4U, 4U, 5U,
                                                                      5U, 6U, 7U, 8U, 8U, 9U, 9U, 10U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_21u_0[8])         = {0U, 10U, 20U, 30U, 8U, 18U, 28U, 6U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_21u_1[8])         = {11U, 1U, 23U, 13U, 3U, 25U, 15U, 5U};

OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_21u_0[64]) = {
        3U,  2U, 1U, 0U, 5U, 4U, 3U,  2U,  8U, 7U, 6U, 5U, 10U, 9U, 8U, 7U, 3U,  2U,  1U, 0U, 6U, 5U,
        4U,  3U, 8U, 7U, 6U, 5U, 11U, 10U, 9U, 8U, 3U, 2U, 1U,  0U, 5U, 4U, 3U,  2U,  8U, 7U, 6U, 5U,
        10U, 9U, 8U, 7U, 3U, 2U, 1U,  0U,  6U, 5U, 4U, 3U, 8U,  7U, 6U, 5U, 11U, 10U, 9U, 8U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_21u_2[16])   = {11U, 6U, 9U, 4U, 7U, 10U, 5U, 8U,
                                                                 11U, 6U, 9U, 4U, 7U, 10U, 5U, 8U};
OWN_ALIGNED_64_ARRAY(static uint64_t gather_idx_table_21u[8]) = {0U, 8U, 10U, 18U, 21U, 29U, 31U, 39U};

// ------------------------------------ 22u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_22u_0[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 4U, 5U,
                                                                      5U, 6U, 6U, 7U, 8U, 9U, 9U, 10U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_22u_1[16]) = {0U, 1U, 2U, 3U, 3U, 4U, 4U,  5U,
                                                                      6U, 7U, 7U, 8U, 8U, 9U, 10U, 11U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_22u_0[8])         = {0U, 12U, 24U, 4U, 16U, 28U, 8U, 20U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_22u_1[8])         = {10U, 30U, 18U, 6U, 26U, 14U, 2U, 22U};

OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_22u_0[64]) = {
        3U,  2U,  1U, 0U, 5U, 4U, 3U,  2U,  8U, 7U, 6U, 5U, 11U, 10U, 9U, 8U, 3U,  2U,  1U, 0U, 5U, 4U,
        3U,  2U,  8U, 7U, 6U, 5U, 11U, 10U, 9U, 8U, 3U, 2U, 1U,  0U,  5U, 4U, 3U,  2U,  8U, 7U, 6U, 5U,
        11U, 10U, 9U, 8U, 3U, 2U, 1U,  0U,  5U, 4U, 3U, 2U, 8U,  7U,  6U, 5U, 11U, 10U, 9U, 8U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_22u_2[16])   = {10U, 4U, 6U, 8U, 10U, 4U, 6U, 8U,
                                                                 10U, 4U, 6U, 8U, 10U, 4U, 6U, 8U};
OWN_ALIGNED_64_ARRAY(static uint64_t gather_idx_table_22u[8]) = {0U, 8U, 11U, 19U, 22U, 30U, 33U, 41U};

// ------------------------------------ 23u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_23u_0[16]) = {0U, 1U, 1U, 2U, 2U, 3U, 4U,  5U,
                                                                      5U, 6U, 7U, 8U, 8U, 9U, 10U, 11U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_23u_1[16]) = {0U, 1U, 2U, 3U, 3U, 4U,  5U,  6U,
                                                                      6U, 7U, 7U, 8U, 9U, 10U, 10U, 11U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_23u_0[8])         = {0U, 14U, 28U, 10U, 24U, 6U, 20U, 2U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_23u_1[8])         = {9U, 27U, 13U, 31U, 17U, 3U, 21U, 7U};

OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_23u_0[64]) = {
        3U,  2U,  1U, 0U, 5U, 4U, 3U,  2U,  8U,  7U, 6U, 5U, 11U, 10U, 9U, 8U, 3U,  2U,  1U,  0U, 6U, 5U,
        4U,  3U,  9U, 8U, 7U, 6U, 12U, 11U, 10U, 9U, 3U, 2U, 1U,  0U,  5U, 4U, 3U,  2U,  8U,  7U, 6U, 5U,
        11U, 10U, 9U, 8U, 3U, 2U, 1U,  0U,  6U,  5U, 4U, 3U, 9U,  8U,  7U, 6U, 12U, 11U, 10U, 9U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_23u_2[16])   = {9U, 2U, 3U, 4U, 5U, 6U, 7U, 8U,
                                                                 9U, 2U, 3U, 4U, 5U, 6U, 7U, 8U};
OWN_ALIGNED_64_ARRAY(static uint64_t gather_idx_table_23u[8]) = {0U, 8U, 11U, 19U, 23U, 31U, 34U, 42U};

// ------------------------------------ 24u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_24u_0[64]) = {
        2U, 1U, 0U, 0xFF, 5U, 4U, 3U, 0xFF, 8U, 7U, 6U, 0xFF, 11U, 10U, 9U, 0xFF,
        2U, 1U, 0U, 0xFF, 5U, 4U, 3U, 0xFF, 8U, 7U, 6U, 0xFF, 11U, 10U, 9U, 0xFF,
        2U, 1U, 0U, 0xFF, 5U, 4U, 3U, 0xFF, 8U, 7U, 6U, 0xFF, 11U, 10U, 9U, 0xFF,
        2U, 1U, 0U, 0xFF, 5U, 4U, 3U, 0xFF, 8U, 7U, 6U, 0xFF, 11U, 10U, 9U, 0xFF};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_24u[16]) = {0U, 1U, 2U, 0x0, 3U, 4U,  5U,  0x0,
                                                                    6U, 7U, 8U, 0x0, 9U, 10U, 11U, 0x0};

// ------------------------------------ 25u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_25u_0[16]) = {0U, 1U, 1U, 2U, 3U, 4U,  4U,  5U,
                                                                      6U, 7U, 7U, 8U, 9U, 10U, 10U, 11U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_25u_1[16]) = {0U, 1U, 2U, 3U, 3U,  4U,  5U,  6U,
                                                                      7U, 8U, 8U, 9U, 10U, 11U, 11U, 12U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_25u_0[8])         = {0U, 18U, 4U, 22U, 8U, 26U, 12U, 30U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_25u_1[8])         = {7U, 21U, 3U, 17U, 31U, 13U, 27U, 9U};

OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_25u_0[64]) = {
        3U,  2U,  1U,  0U, 6U, 5U, 4U,  3U,  9U,  8U, 7U, 6U, 12U, 11U, 10U, 9U, 3U,  2U,  1U,  0U, 6U, 5U,
        4U,  3U,  9U,  8U, 7U, 6U, 12U, 11U, 10U, 9U, 3U, 2U, 1U,  0U,  6U,  5U, 4U,  3U,  9U,  8U, 7U, 6U,
        12U, 11U, 10U, 9U, 3U, 2U, 1U,  0U,  6U,  5U, 4U, 3U, 9U,  8U,  7U,  6U, 12U, 11U, 10U, 9U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_25u_2[16])   = {7U, 6U, 5U, 4U, 3U, 2U, 1U, 0U,
                                                                 7U, 6U, 5U, 4U, 3U, 2U, 1U, 0U};
OWN_ALIGNED_64_ARRAY(static uint64_t gather_idx_table_25u[8]) = {0U, 8U, 12U, 20U, 25U, 33U, 37U, 45U};

// ------------------------------------ 26u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_26u_0[16]) = {0U, 1U, 1U, 2U, 3U, 4U,  4U,  5U,
                                                                      6U, 7U, 8U, 9U, 9U, 10U, 11U, 12U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_26u_1[16]) = {0U, 1U, 2U, 3U, 4U,  5U,  5U,  6U,
                                                                      7U, 8U, 8U, 9U, 10U, 11U, 12U, 13U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_26u_0[8])         = {0U, 20U, 8U, 28U, 16U, 4U, 24U, 12U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_26u_1[8])         = {6U, 18U, 30U, 10U, 22U, 2U, 14U, 26U};

OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_26u_0[64]) = {
        3U,  2U,  1U,  0U, 6U, 5U, 4U,  3U,  9U,  8U, 7U, 6U, 12U, 11U, 10U, 9U, 3U,  2U,  1U,  0U, 6U, 5U,
        4U,  3U,  9U,  8U, 7U, 6U, 12U, 11U, 10U, 9U, 3U, 2U, 1U,  0U,  6U,  5U, 4U,  3U,  9U,  8U, 7U, 6U,
        12U, 11U, 10U, 9U, 3U, 2U, 1U,  0U,  6U,  5U, 4U, 3U, 9U,  8U,  7U,  6U, 12U, 11U, 10U, 9U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_26u_2[16])   = {6U, 4U, 2U, 0U, 6U, 4U, 2U, 0U,
                                                                 6U, 4U, 2U, 0U, 6U, 4U, 2U, 0U};
OWN_ALIGNED_64_ARRAY(static uint64_t gather_idx_table_26u[8]) = {0U, 8U, 13U, 21U, 26U, 34U, 39U, 47U};

// ------------------------------------ 27u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_27u_0[16]) = {0U, 1U, 1U, 2U, 3U,  4U,  5U,  6U,
                                                                      6U, 7U, 8U, 9U, 10U, 11U, 11U, 12U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_27u_1[16]) = {0U, 1U, 2U, 3U,  4U,  5U,  5U,  6U,
                                                                      7U, 8U, 9U, 10U, 10U, 11U, 12U, 13U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_27u_0[8])         = {0U, 22U, 12U, 2U, 24U, 14U, 4U, 26U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_27u_1[8])         = {5U, 15U, 25U, 3U, 13U, 23U, 1U, 11U};

OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_27u_0[64]) = {
        0U, 0U, 0U, 4U, 3U, 2U,  1U,  0U, 0U, 0U, 0U, 10U, 9U, 8U, 7U, 6U,  0U,  0U, 0U, 4U, 3U, 2U,
        1U, 0U, 0U, 0U, 0U, 11U, 10U, 9U, 8U, 7U, 0U, 0U,  0U, 4U, 3U, 2U,  1U,  0U, 0U, 0U, 0U, 10U,
        9U, 8U, 7U, 6U, 0U, 0U,  0U,  4U, 3U, 2U, 1U, 0U,  0U, 0U, 0U, 11U, 10U, 9U, 8U, 7U};
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_27u_1[64]) = {
        7U,  6U, 5U,  4U,  3U,  0U,  0U,  0U, 14U, 13U, 12U, 11U, 10U, 0U,  0U,  0U,  7U,  6U, 5U,  4U,  3U,  0U,
        0U,  0U, 14U, 13U, 12U, 11U, 10U, 0U, 0U,  0U,  7U,  6U,  5U,  4U,  3U,  0U,  0U,  0U, 14U, 13U, 12U, 11U,
        10U, 0U, 0U,  0U,  7U,  6U,  5U,  4U, 3U,  0U,  0U,  0U,  14U, 13U, 12U, 11U, 10U, 0U, 0U,  0U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_27u_2[8])    = {37U, 31U, 33U, 35U, 37U, 31U, 33U, 35U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_27u_3[8])    = {22U, 20U, 26U, 24U, 22U, 20U, 26U, 24U};
OWN_ALIGNED_64_ARRAY(static uint64_t gather_idx_table_27u[8]) = {0U, 8U, 13U, 21U, 27U, 35U, 40U, 48U};

// ------------------------------------ 28u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_28u_0[64]) = {
        3U,  2U,  1U,  0U,  6U, 5U, 4U,  3U,  10U, 9U,  8U, 7U, 13U, 12U, 11U, 10U, 3U,  2U,  1U,  0U, 6U, 5U,
        4U,  3U,  10U, 9U,  8U, 7U, 13U, 12U, 11U, 10U, 3U, 2U, 1U,  0U,  6U,  5U,  4U,  3U,  10U, 9U, 8U, 7U,
        13U, 12U, 11U, 10U, 3U, 2U, 1U,  0U,  6U,  5U,  4U, 3U, 10U, 9U,  8U,  7U,  13U, 12U, 11U, 10U};
OWN_ALIGNED_64_ARRAY(static uint32_t shift_table_28u[16])        = {4U, 0U, 4U, 0U, 4U, 0U, 4U, 0U,
                                                                    4U, 0U, 4U, 0U, 4U, 0U, 4U, 0U};
OWN_ALIGNED_64_ARRAY(static uint16_t permutex_idx_table_28u[32]) = {
        0U,  1U,  2U,  3U,  4U,  5U,  6U,  0x0, 7U,  8U,  9U,  10U, 11U, 12U, 13U, 0x0,
        14U, 15U, 16U, 17U, 18U, 19U, 20U, 0x0, 21U, 22U, 23U, 24U, 25U, 26U, 27U, 0x0};

// ------------------------------------ 29u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_29u_0[16]) = {0U, 1U, 1U, 2U,  3U,  4U,  5U,  6U,
                                                                      7U, 8U, 9U, 10U, 10U, 11U, 12U, 13U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_29u_1[16]) = {0U, 1U, 2U, 3U,  4U,  5U,  6U,  7U,
                                                                      8U, 9U, 9U, 10U, 11U, 12U, 13U, 14U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_29u_0[8])         = {0U, 26U, 20U, 14U, 8U, 2U, 28U, 22U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_29u_1[8])         = {3U, 9U, 15U, 21U, 27U, 1U, 7U, 13U};

OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_29u_0[64]) = {
        0U,  0U, 0U, 4U, 3U, 2U,  1U,  0U, 0U, 0U, 0U, 11U, 10U, 9U, 8U, 7U,  0U,  0U, 0U, 4U, 3U, 2U,
        1U,  0U, 0U, 0U, 0U, 11U, 10U, 9U, 8U, 7U, 0U, 0U,  0U,  4U, 3U, 2U,  1U,  0U, 0U, 0U, 0U, 11U,
        10U, 9U, 8U, 7U, 0U, 0U,  0U,  4U, 3U, 2U, 1U, 0U,  0U,  0U, 0U, 11U, 10U, 9U, 8U, 7U};
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_29u_1[64]) = {
        7U,  6U, 5U,  4U,  3U,  0U,  0U,  0U, 14U, 13U, 12U, 11U, 10U, 0U,  0U,  0U,  8U,  7U, 6U,  5U,  4U,  0U,
        0U,  0U, 15U, 14U, 13U, 12U, 11U, 0U, 0U,  0U,  7U,  6U,  5U,  4U,  3U,  0U,  0U,  0U, 14U, 13U, 12U, 11U,
        10U, 0U, 0U,  0U,  8U,  7U,  6U,  5U, 4U,  0U,  0U,  0U,  15U, 14U, 13U, 12U, 11U, 0U, 0U,  0U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_29u_2[8])    = {35U, 33U, 31U, 29U, 35U, 33U, 31U, 29U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_29u_3[8])    = {26U, 28U, 22U, 24U, 26U, 28U, 22U, 24U};
OWN_ALIGNED_64_ARRAY(static uint64_t gather_idx_table_29u[8]) = {0U, 8U, 14U, 22U, 29U, 37U, 43U, 51U};

// ------------------------------------ 30u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_30u_0[16]) = {0U, 1U, 1U, 2U,  3U,  4U,  5U,  6U,
                                                                      7U, 8U, 9U, 10U, 11U, 12U, 13U, 14U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_30u_1[16]) = {0U, 1U, 2U,  3U,  4U,  5U,  6U,  7U,
                                                                      8U, 9U, 10U, 11U, 12U, 13U, 14U, 15U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_30u_0[8])         = {0U, 28U, 24U, 20U, 16U, 12U, 8U, 4U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_30u_1[8])         = {2U, 6U, 10U, 14U, 18U, 22U, 26U, 30U};

OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_30u_0[64]) = {
        0U,  0U, 0U, 4U, 3U, 2U,  1U,  0U, 0U, 0U, 0U, 11U, 10U, 9U, 8U, 7U,  0U,  0U, 0U, 4U, 3U, 2U,
        1U,  0U, 0U, 0U, 0U, 11U, 10U, 9U, 8U, 7U, 0U, 0U,  0U,  4U, 3U, 2U,  1U,  0U, 0U, 0U, 0U, 11U,
        10U, 9U, 8U, 7U, 0U, 0U,  0U,  4U, 3U, 2U, 1U, 0U,  0U,  0U, 0U, 11U, 10U, 9U, 8U, 7U};
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_30u_1[64]) = {
        7U,  6U, 5U,  4U,  3U,  0U,  0U,  0U, 15U, 14U, 13U, 12U, 11U, 0U,  0U,  0U,  7U,  6U, 5U,  4U,  3U,  0U,
        0U,  0U, 15U, 14U, 13U, 12U, 11U, 0U, 0U,  0U,  7U,  6U,  5U,  4U,  3U,  0U,  0U,  0U, 15U, 14U, 13U, 12U,
        11U, 0U, 0U,  0U,  7U,  6U,  5U,  4U, 3U,  0U,  0U,  0U,  15U, 14U, 13U, 12U, 11U, 0U, 0U,  0U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_30u_2[8])    = {34U, 30U, 34U, 30U, 34U, 30U, 34U, 30U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_30u_3[8])    = {28U, 24U, 28U, 24U, 28U, 24U, 28U, 24U};
OWN_ALIGNED_64_ARRAY(static uint64_t gather_idx_table_30u[8]) = {0U, 8U, 15U, 23U, 30U, 38U, 45U, 53U};

// ------------------------------------ 31u -----------------------------------------
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_31u_0[16]) = {0U, 1U, 1U, 2U,  3U,  4U,  5U,  6U,
                                                                      7U, 8U, 9U, 10U, 11U, 12U, 13U, 14U};
OWN_ALIGNED_64_ARRAY(static uint32_t permutex_idx_table_31u_1[16]) = {0U, 1U, 2U,  3U,  4U,  5U,  6U,  7U,
                                                                      8U, 9U, 10U, 11U, 12U, 13U, 14U, 15U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_31u_0[8])         = {0U, 30U, 28U, 26U, 24U, 22U, 20U, 18U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_31u_1[8])         = {1U, 3U, 5U, 7U, 9U, 11U, 13U, 15U};

OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_31u_0[64]) = {
        0U,  0U, 0U, 4U, 3U, 2U,  1U,  0U,  0U, 0U, 0U, 11U, 10U, 9U, 8U, 7U,  0U,  0U,  0U, 4U, 3U, 2U,
        1U,  0U, 0U, 0U, 0U, 12U, 11U, 10U, 9U, 8U, 0U, 0U,  0U,  4U, 3U, 2U,  1U,  0U,  0U, 0U, 0U, 11U,
        10U, 9U, 8U, 7U, 0U, 0U,  0U,  4U,  3U, 2U, 1U, 0U,  0U,  0U, 0U, 12U, 11U, 10U, 9U, 8U};
OWN_ALIGNED_64_ARRAY(static uint8_t shuffle_idx_table_31u_1[64]) = {
        7U,  6U, 5U,  4U,  3U,  0U,  0U,  0U, 15U, 14U, 13U, 12U, 11U, 0U,  0U,  0U,  8U,  7U, 6U,  5U,  4U,  0U,
        0U,  0U, 16U, 15U, 14U, 13U, 12U, 0U, 0U,  0U,  7U,  6U,  5U,  4U,  3U,  0U,  0U,  0U, 15U, 14U, 13U, 12U,
        11U, 0U, 0U,  0U,  8U,  7U,  6U,  5U, 4U,  0U,  0U,  0U,  16U, 15U, 14U, 13U, 12U, 0U, 0U,  0U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_31u_2[8])    = {33U, 27U, 29U, 31U, 33U, 27U, 29U, 31U};
OWN_ALIGNED_64_ARRAY(static uint64_t shift_table_31u_3[8])    = {30U, 28U, 26U, 24U, 30U, 28U, 26U, 24U};
OWN_ALIGNED_64_ARRAY(static uint64_t gather_idx_table_31u[8]) = {0U, 8U, 15U, 23U, 31U, 39U, 46U, 54U};

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

OWN_ALIGNED_64_ARRAY(static uint64_t reverse_mask_table_32u[8]) = {
        0x0405060700010203, 0x0C0D0E0F08090A0B, 0x1415161710111213, 0x1C1D1E1F18191A1B,
        0x2425262720212223, 0x2C2D2E2F28292A2B, 0x3435363730313233, 0x3C3D3E3F38393A3B};

OWN_QPLC_INLINE(uint32_t, own_get_align, (uint32_t start_bit, uint32_t base, uint32_t bitsize)) {
    uint32_t remnant = bitsize - start_bit;
    for (uint32_t i = 0U; i < bitsize; ++i) {
        uint32_t test_value = (i * base) % bitsize;
        if (test_value == remnant) { return i; }
    }
    return 0xFFFFFFFF;
}

// For BE start_bit is bit index from the top of a byte
OWN_QPLC_INLINE(void, px_qplc_unpack_be_Nu32u,
                (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint32_t bit_width,
                 uint8_t* dst_ptr)) {
    uint32_t* src32u_ptr    = (uint32_t*)src_ptr;
    uint8_t*  src8u_ptr     = (uint8_t*)src_ptr;
    uint32_t* dst32u_ptr    = (uint32_t*)dst_ptr;
    uint32_t  shift         = OWN_QWORD_WIDTH - bit_width;
    uint32_t  bits_in_buf   = 0U;
    uint64_t  src           = 0U;
    uint64_t  next_dword    = 0U;
    uint32_t  bytes_to_read = OWN_BITS_2_BYTE(num_elements * bit_width + start_bit);

    if (sizeof(uint32_t) <= bytes_to_read) {
        bits_in_buf = OWN_DWORD_WIDTH - start_bit;
        src         = ((uint64_t)qplc_swap_bytes_32u(*src32u_ptr)) << (OWN_QWORD_WIDTH - bits_in_buf);

        src32u_ptr++;

        while (2U < num_elements) {
            if (bit_width > bits_in_buf) {
                next_dword = (uint64_t)qplc_swap_bytes_32u(*src32u_ptr);
                src32u_ptr++;
                next_dword = next_dword << (OWN_DWORD_WIDTH - bits_in_buf);
                src        = src | next_dword;
                bits_in_buf += OWN_DWORD_WIDTH;
            }
            *dst32u_ptr = (uint32_t)(src >> shift);
            src         = src << bit_width;
            bits_in_buf -= bit_width;
            dst32u_ptr++;
            num_elements--;
        }

        bytes_to_read =
                OWN_BITS_2_BYTE(num_elements * bit_width > bits_in_buf ? num_elements * bit_width - bits_in_buf : 0U);

        if (bytes_to_read > 3U) {
            next_dword = (uint64_t)qplc_swap_bytes_32u(*src32u_ptr);
            src32u_ptr++;
            next_dword = next_dword << (OWN_DWORD_WIDTH - bits_in_buf);
            src        = src | next_dword;
            bits_in_buf += OWN_DWORD_WIDTH;
            bytes_to_read -= 4U;
        }

        src8u_ptr = (uint8_t*)src32u_ptr;
    } else {
        next_dword = 0U;
        for (uint32_t byte_to_read = 0U; byte_to_read < bytes_to_read; byte_to_read++) {
            next_dword |= ((uint64_t)(*src8u_ptr)) << (byte_to_read * OWN_BYTE_WIDTH);
            src8u_ptr++;
        }
        next_dword  = (uint64_t)qplc_swap_bytes_32u((uint32_t)next_dword);
        bits_in_buf = OWN_DWORD_WIDTH - start_bit;
        next_dword  = next_dword << (OWN_QWORD_WIDTH - bits_in_buf);
        src         = next_dword;
        *dst32u_ptr = (uint32_t)(src >> shift);
        return;
    }

    while (0U < num_elements) {
        if (bit_width > bits_in_buf) {
            next_dword = 0U;
            for (uint32_t byte_to_read = 0U; byte_to_read < bytes_to_read; byte_to_read++) {
                next_dword |= ((uint64_t)(*src8u_ptr)) << (byte_to_read * OWN_BYTE_WIDTH);
                src8u_ptr++;
            }
            next_dword = (uint64_t)qplc_swap_bytes_32u((uint32_t)next_dword);
            next_dword = next_dword << (OWN_DWORD_WIDTH - bits_in_buf);
            src        = src | next_dword;
            bits_in_buf += OWN_DWORD_WIDTH;
        }
        *dst32u_ptr = (uint32_t)(src >> shift);
        src         = src << bit_width;
        bits_in_buf -= bit_width;
        dst32u_ptr++;
        num_elements--;
    }
}

// ********************** 17u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_17u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 17U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu32u(src_ptr, align, start_bit, 17U, dst_ptr);
        src_ptr += ((align * 17U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask32 read_mask        = OWN_BIT_MASK(17U);
        __m512i   parse_mask0      = _mm512_set1_epi32(OWN_BIT_MASK(17U));
        __m512i   nibble_reversemm = _mm512_load_si512(nibble_reverse_table);
        __m512i   reverse_mask_32u = _mm512_load_si512(reverse_mask_table_32u);
        __m512i   maskmm           = _mm512_set1_epi8(0x0F);

        __m512i shuffle_idx_ptr = _mm512_load_si512(shuffle_idx_table_17u_0);

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_17u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_17u_1);

        __m512i shift_mask_ptr[3];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_17u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_17u_1);
        shift_mask_ptr[2] = _mm512_load_si512(shift_table_17u_2);

        __m512i gather_idxmm = _mm512_load_si512(gather_idx_table_17u);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_i64gather_epi64(gather_idxmm, src_ptr, 1U);

            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[2]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 17U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }

        if (num_elements >= 16U) {
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
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            zmm[0]        = _mm512_slli_epi32(zmm[0], 15U);
            low_nibblemm  = _mm512_and_si512(zmm[0], maskmm);
            high_nibblemm = _mm512_srli_epi16(zmm[0], 4U);
            high_nibblemm = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            zmm[0] = _mm512_or_si512(low_nibblemm, high_nibblemm);
            zmm[0] = _mm512_shuffle_epi8(zmm[0], reverse_mask_32u);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 17U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu32u(src_ptr, num_elements, 0U, 17U, dst_ptr); }
}

// ********************** 18u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_18u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 18U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu32u(src_ptr, align, start_bit, 18U, dst_ptr);
        src_ptr += ((align * 18U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask16 read_mask        = OWN_BIT_MASK(OWN_BITS_2_DWORD(18U * OWN_WORD_WIDTH));
        __m512i   parse_mask0      = _mm512_set1_epi32(OWN_BIT_MASK(18U));
        __m512i   nibble_reversemm = _mm512_load_si512(nibble_reverse_table);
        __m512i   reverse_mask_32u = _mm512_load_si512(reverse_mask_table_32u);
        __m512i   maskmm           = _mm512_set1_epi8(0x0F);

        __m512i shuffle_idx_ptr = _mm512_load_si512(shuffle_idx_table_18u_0);

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_18u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_18u_1);

        __m512i shift_mask_ptr[3];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_18u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_18u_1);
        shift_mask_ptr[2] = _mm512_load_si512(shift_table_18u_2);

        __m512i gather_idxmm = _mm512_load_si512(gather_idx_table_18u);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_i64gather_epi64(gather_idxmm, src_ptr, 1U);

            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[2]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 18U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }

        if (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi32(read_mask, src_ptr);

            __m512i low_nibblemm  = _mm512_and_si512(srcmm, maskmm);
            __m512i high_nibblemm = _mm512_srli_epi16(srcmm, 4U);
            high_nibblemm         = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            srcmm = _mm512_or_si512(low_nibblemm, high_nibblemm);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            zmm[0]        = _mm512_slli_epi32(zmm[0], 14U);
            low_nibblemm  = _mm512_and_si512(zmm[0], maskmm);
            high_nibblemm = _mm512_srli_epi16(zmm[0], 4U);
            high_nibblemm = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            zmm[0] = _mm512_or_si512(low_nibblemm, high_nibblemm);
            zmm[0] = _mm512_shuffle_epi8(zmm[0], reverse_mask_32u);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 18U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu32u(src_ptr, num_elements, 0U, 18U, dst_ptr); }
}

// ********************** 19u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_19u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 19U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu32u(src_ptr, align, start_bit, 19U, dst_ptr);
        src_ptr += ((align * 19U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask32 read_mask        = OWN_BIT_MASK(19U);
        __m512i   parse_mask0      = _mm512_set1_epi32(OWN_BIT_MASK(19U));
        __m512i   nibble_reversemm = _mm512_load_si512(nibble_reverse_table);
        __m512i   reverse_mask_32u = _mm512_load_si512(reverse_mask_table_32u);
        __m512i   maskmm           = _mm512_set1_epi8(0x0F);

        __m512i shuffle_idx_ptr = _mm512_load_si512(shuffle_idx_table_19u_0);

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_19u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_19u_1);

        __m512i shift_mask_ptr[3];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_19u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_19u_1);
        shift_mask_ptr[2] = _mm512_load_si512(shift_table_19u_2);

        __m512i gather_idxmm = _mm512_load_si512(gather_idx_table_19u);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_i64gather_epi64(gather_idxmm, src_ptr, 1U);

            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[2]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 19U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }

        if (num_elements >= 16U) {
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
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            zmm[0]        = _mm512_slli_epi32(zmm[0], 13U);
            low_nibblemm  = _mm512_and_si512(zmm[0], maskmm);
            high_nibblemm = _mm512_srli_epi16(zmm[0], 4U);
            high_nibblemm = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            zmm[0] = _mm512_or_si512(low_nibblemm, high_nibblemm);
            zmm[0] = _mm512_shuffle_epi8(zmm[0], reverse_mask_32u);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 19U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu32u(src_ptr, num_elements, 0U, 19U, dst_ptr); }
}

// ********************** 20u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_20u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 20U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu32u(src_ptr, align, start_bit, 20U, dst_ptr);
        src_ptr += ((align * 20U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask16 read_mask   = OWN_BIT_MASK(OWN_BITS_2_DWORD(20U * OWN_WORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi32(OWN_BIT_MASK(20U));

        __m512i shuffle_idx_ptr = _mm512_load_si512(shuffle_idx_table_20u_0);
        __m512i permutex_idx    = _mm512_load_si512(permutex_idx_table_20u);
        __m512i shift_mask      = _mm512_load_si512(shift_table_20u);

        while (num_elements >= 16U) {
            __m512i srcmm, zmm;

            srcmm = _mm512_maskz_loadu_epi32(read_mask, src_ptr);

            zmm = _mm512_permutexvar_epi16(permutex_idx, srcmm);
            zmm = _mm512_shuffle_epi8(zmm, shuffle_idx_ptr);

            // shifting elements so they start from the start of the word
            zmm = _mm512_srlv_epi32(zmm, shift_mask);
            zmm = _mm512_and_si512(zmm, parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm);

            src_ptr += 2U * 20U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu32u(src_ptr, num_elements, 0U, 20U, dst_ptr); }
}

// ********************** 21u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_21u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 21U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu32u(src_ptr, align, start_bit, 21U, dst_ptr);
        src_ptr += ((align * 21U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask32 read_mask        = OWN_BIT_MASK(21U);
        __m512i   parse_mask0      = _mm512_set1_epi32(OWN_BIT_MASK(21U));
        __m512i   nibble_reversemm = _mm512_load_si512(nibble_reverse_table);
        __m512i   reverse_mask_32u = _mm512_load_si512(reverse_mask_table_32u);
        __m512i   maskmm           = _mm512_set1_epi8(0x0F);

        __m512i shuffle_idx_ptr = _mm512_load_si512(shuffle_idx_table_21u_0);

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_21u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_21u_1);

        __m512i shift_mask_ptr[3];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_21u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_21u_1);
        shift_mask_ptr[2] = _mm512_load_si512(shift_table_21u_2);

        __m512i gather_idxmm = _mm512_load_si512(gather_idx_table_21u);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_i64gather_epi64(gather_idxmm, src_ptr, 1U);

            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[2]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 21U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }

        if (num_elements >= 16U) {
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
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            zmm[0]        = _mm512_slli_epi32(zmm[0], 11U);
            low_nibblemm  = _mm512_and_si512(zmm[0], maskmm);
            high_nibblemm = _mm512_srli_epi16(zmm[0], 4U);
            high_nibblemm = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            zmm[0] = _mm512_or_si512(low_nibblemm, high_nibblemm);
            zmm[0] = _mm512_shuffle_epi8(zmm[0], reverse_mask_32u);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 21U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu32u(src_ptr, num_elements, 0U, 21U, dst_ptr); }
}

// ********************** 22u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_22u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 22U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu32u(src_ptr, align, start_bit, 22U, dst_ptr);
        src_ptr += ((align * 22U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask16 read_mask        = OWN_BIT_MASK(OWN_BITS_2_DWORD(22U * OWN_WORD_WIDTH));
        __m512i   parse_mask0      = _mm512_set1_epi32(OWN_BIT_MASK(22U));
        __m512i   nibble_reversemm = _mm512_load_si512(nibble_reverse_table);
        __m512i   reverse_mask_32u = _mm512_load_si512(reverse_mask_table_32u);
        __m512i   maskmm           = _mm512_set1_epi8(0x0F);

        __m512i shuffle_idx_ptr = _mm512_load_si512(shuffle_idx_table_22u_0);

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_22u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_22u_1);

        __m512i shift_mask_ptr[3];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_22u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_22u_1);
        shift_mask_ptr[2] = _mm512_load_si512(shift_table_22u_2);

        __m512i gather_idxmm = _mm512_load_si512(gather_idx_table_22u);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_i64gather_epi64(gather_idxmm, src_ptr, 1U);

            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[2]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 22U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }

        if (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi32(read_mask, src_ptr);

            __m512i low_nibblemm  = _mm512_and_si512(srcmm, maskmm);
            __m512i high_nibblemm = _mm512_srli_epi16(srcmm, 4U);
            high_nibblemm         = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            srcmm = _mm512_or_si512(low_nibblemm, high_nibblemm);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            zmm[0]        = _mm512_slli_epi32(zmm[0], 10U);
            low_nibblemm  = _mm512_and_si512(zmm[0], maskmm);
            high_nibblemm = _mm512_srli_epi16(zmm[0], 4U);
            high_nibblemm = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            zmm[0] = _mm512_or_si512(low_nibblemm, high_nibblemm);
            zmm[0] = _mm512_shuffle_epi8(zmm[0], reverse_mask_32u);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 22U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu32u(src_ptr, num_elements, 0U, 22U, dst_ptr); }
}

// ********************** 23u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_23u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 23U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu32u(src_ptr, align, start_bit, 23U, dst_ptr);
        src_ptr += ((align * 23U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask32 read_mask        = OWN_BIT_MASK(23U);
        __m512i   parse_mask0      = _mm512_set1_epi32(OWN_BIT_MASK(23U));
        __m512i   nibble_reversemm = _mm512_load_si512(nibble_reverse_table);
        __m512i   reverse_mask_32u = _mm512_load_si512(reverse_mask_table_32u);
        __m512i   maskmm           = _mm512_set1_epi8(0x0F);

        __m512i shuffle_idx_ptr = _mm512_load_si512(shuffle_idx_table_23u_0);

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_23u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_23u_1);

        __m512i shift_mask_ptr[3];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_23u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_23u_1);
        shift_mask_ptr[2] = _mm512_load_si512(shift_table_23u_2);

        __m512i gather_idxmm = _mm512_load_si512(gather_idx_table_23u);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_i64gather_epi64(gather_idxmm, src_ptr, 1U);

            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[2]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 23U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }

        if (num_elements >= 16U) {
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
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            zmm[0]        = _mm512_slli_epi32(zmm[0], 9U);
            low_nibblemm  = _mm512_and_si512(zmm[0], maskmm);
            high_nibblemm = _mm512_srli_epi16(zmm[0], 4U);
            high_nibblemm = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            zmm[0] = _mm512_or_si512(low_nibblemm, high_nibblemm);
            zmm[0] = _mm512_shuffle_epi8(zmm[0], reverse_mask_32u);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 23U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu32u(src_ptr, num_elements, 0U, 23U, dst_ptr); }
}

// ********************** 24u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_24u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 24U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu32u(src_ptr, align, start_bit, 24U, dst_ptr);
        src_ptr += ((align * 24U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask16 read_mask = OWN_BIT_MASK(OWN_BITS_2_DWORD(24U * OWN_WORD_WIDTH));

        __m512i shuffle_idx  = _mm512_load_si512(shuffle_idx_table_24u_0);
        __m512i permutex_idx = _mm512_load_si512(permutex_idx_table_24u);

        while (num_elements >= 16U) {
            __m512i srcmm, zmm;

            srcmm = _mm512_maskz_loadu_epi32(read_mask, src_ptr);

            zmm = _mm512_permutexvar_epi32(permutex_idx, srcmm);
            zmm = _mm512_shuffle_epi8(zmm, shuffle_idx);

            _mm512_storeu_si512(dst_ptr, zmm);

            src_ptr += 2U * 24U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu32u(src_ptr, num_elements, 0U, 24U, dst_ptr); }
}

// ********************** 25u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_25u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 25U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu32u(src_ptr, align, start_bit, 25U, dst_ptr);
        src_ptr += ((align * 25U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask32 read_mask        = OWN_BIT_MASK(25U);
        __m512i   parse_mask0      = _mm512_set1_epi32(OWN_BIT_MASK(25U));
        __m512i   nibble_reversemm = _mm512_load_si512(nibble_reverse_table);
        __m512i   reverse_mask_32u = _mm512_load_si512(reverse_mask_table_32u);
        __m512i   maskmm           = _mm512_set1_epi8(0x0F);

        __m512i shuffle_idx_ptr = _mm512_load_si512(shuffle_idx_table_25u_0);

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_25u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_25u_1);

        __m512i shift_mask_ptr[3];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_25u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_25u_1);
        shift_mask_ptr[2] = _mm512_load_si512(shift_table_25u_2);

        __m512i gather_idxmm = _mm512_load_si512(gather_idx_table_25u);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_i64gather_epi64(gather_idxmm, src_ptr, 1U);

            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[2]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 25U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }

        if (num_elements >= 16U) {
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
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            zmm[0]        = _mm512_slli_epi32(zmm[0], 7U);
            low_nibblemm  = _mm512_and_si512(zmm[0], maskmm);
            high_nibblemm = _mm512_srli_epi16(zmm[0], 4U);
            high_nibblemm = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            zmm[0] = _mm512_or_si512(low_nibblemm, high_nibblemm);
            zmm[0] = _mm512_shuffle_epi8(zmm[0], reverse_mask_32u);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 25U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu32u(src_ptr, num_elements, 0U, 25U, dst_ptr); }
}

// ********************** 26u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_26u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 26U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu32u(src_ptr, align, start_bit, 26U, dst_ptr);
        src_ptr += ((align * 26U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask16 read_mask        = OWN_BIT_MASK(OWN_BITS_2_DWORD(26U * OWN_WORD_WIDTH));
        __m512i   parse_mask0      = _mm512_set1_epi32(OWN_BIT_MASK(26U));
        __m512i   nibble_reversemm = _mm512_load_si512(nibble_reverse_table);
        __m512i   reverse_mask_32u = _mm512_load_si512(reverse_mask_table_32u);
        __m512i   maskmm           = _mm512_set1_epi8(0x0F);

        __m512i shuffle_idx_ptr = _mm512_load_si512(shuffle_idx_table_26u_0);

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_26u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_26u_1);

        __m512i shift_mask_ptr[3];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_26u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_26u_1);
        shift_mask_ptr[2] = _mm512_load_si512(shift_table_26u_2);

        __m512i gather_idxmm = _mm512_load_si512(gather_idx_table_26u);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_i64gather_epi64(gather_idxmm, src_ptr, 1U);

            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi32(zmm[0], shift_mask_ptr[2]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 26U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }

        if (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi32(read_mask, src_ptr);

            __m512i low_nibblemm  = _mm512_and_si512(srcmm, maskmm);
            __m512i high_nibblemm = _mm512_srli_epi16(srcmm, 4U);
            high_nibblemm         = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            srcmm = _mm512_or_si512(low_nibblemm, high_nibblemm);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            zmm[0]        = _mm512_slli_epi32(zmm[0], 6U);
            low_nibblemm  = _mm512_and_si512(zmm[0], maskmm);
            high_nibblemm = _mm512_srli_epi16(zmm[0], 4U);
            high_nibblemm = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            zmm[0] = _mm512_or_si512(low_nibblemm, high_nibblemm);
            zmm[0] = _mm512_shuffle_epi8(zmm[0], reverse_mask_32u);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 26U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu32u(src_ptr, num_elements, 0U, 26U, dst_ptr); }
}

// ********************** 27u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_27u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 27U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu32u(src_ptr, align, start_bit, 27U, dst_ptr);
        src_ptr += ((align * 27U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask32 read_mask        = OWN_BIT_MASK(27U);
        __m512i   parse_mask0      = _mm512_set1_epi32(OWN_BIT_MASK(27U));
        __m512i   nibble_reversemm = _mm512_load_si512(nibble_reverse_table);
        __m512i   reverse_mask_32u = _mm512_load_si512(reverse_mask_table_32u);
        __m512i   maskmm           = _mm512_set1_epi8(0x0F);

        __m512i shuffle_idx_ptr[2];
        shuffle_idx_ptr[0] = _mm512_load_si512(shuffle_idx_table_27u_0);
        shuffle_idx_ptr[1] = _mm512_load_si512(shuffle_idx_table_27u_1);

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_27u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_27u_1);

        __m512i shift_mask_ptr[4];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_27u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_27u_1);
        shift_mask_ptr[2] = _mm512_load_si512(shift_table_27u_2);
        shift_mask_ptr[3] = _mm512_load_si512(shift_table_27u_3);

        __m512i gather_idxmm = _mm512_load_si512(gather_idx_table_27u);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_i64gather_epi64(gather_idxmm, src_ptr, 1U);

            // shuffling so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[0]);
            zmm[1] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[1]);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[2]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[3]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 27U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
        if (num_elements >= 16U) {
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
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            zmm[0]        = _mm512_slli_epi32(zmm[0], 5U);
            low_nibblemm  = _mm512_and_si512(zmm[0], maskmm);
            high_nibblemm = _mm512_srli_epi16(zmm[0], 4U);
            high_nibblemm = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            zmm[0] = _mm512_or_si512(low_nibblemm, high_nibblemm);
            zmm[0] = _mm512_shuffle_epi8(zmm[0], reverse_mask_32u);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 27U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu32u(src_ptr, num_elements, 0U, 27U, dst_ptr); }
}

// ********************** 28u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_28u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 28U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu32u(src_ptr, align, start_bit, 28U, dst_ptr);
        src_ptr += ((align * 28U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask16 read_mask   = OWN_BIT_MASK(OWN_BITS_2_DWORD(28U * OWN_WORD_WIDTH));
        __m512i   parse_mask0 = _mm512_set1_epi32(OWN_BIT_MASK(28U));

        __m512i shuffle_idx_ptr = _mm512_load_si512(shuffle_idx_table_28u_0);
        __m512i permutex_idx    = _mm512_load_si512(permutex_idx_table_28u);
        __m512i shift_mask      = _mm512_load_si512(shift_table_28u);

        while (num_elements >= 16U) {
            __m512i srcmm, zmm;

            srcmm = _mm512_maskz_loadu_epi32(read_mask, src_ptr);

            zmm = _mm512_permutexvar_epi16(permutex_idx, srcmm);
            zmm = _mm512_shuffle_epi8(zmm, shuffle_idx_ptr);

            // shifting elements so they start from the start of the word
            zmm = _mm512_srlv_epi32(zmm, shift_mask);
            zmm = _mm512_and_si512(zmm, parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm);

            src_ptr += 2U * 28U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu32u(src_ptr, num_elements, 0U, 28U, dst_ptr); }
}

// ********************** 29u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_29u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 29U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu32u(src_ptr, align, start_bit, 29U, dst_ptr);
        src_ptr += ((align * 29U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask32 read_mask        = OWN_BIT_MASK(29U);
        __m512i   parse_mask0      = _mm512_set1_epi32(OWN_BIT_MASK(29U));
        __m512i   nibble_reversemm = _mm512_load_si512(nibble_reverse_table);
        __m512i   reverse_mask_32u = _mm512_load_si512(reverse_mask_table_32u);
        __m512i   maskmm           = _mm512_set1_epi8(0x0F);

        __m512i shuffle_idx_ptr[2];
        shuffle_idx_ptr[0] = _mm512_load_si512(shuffle_idx_table_29u_0);
        shuffle_idx_ptr[1] = _mm512_load_si512(shuffle_idx_table_29u_1);

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_29u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_29u_1);

        __m512i shift_mask_ptr[4];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_29u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_29u_1);
        shift_mask_ptr[2] = _mm512_load_si512(shift_table_29u_2);
        shift_mask_ptr[3] = _mm512_load_si512(shift_table_29u_3);

        __m512i gather_idxmm = _mm512_load_si512(gather_idx_table_29u);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_i64gather_epi64(gather_idxmm, src_ptr, 1U);

            // shuffling so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[0]);
            zmm[1] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[1]);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[2]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[3]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 29U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
        if (num_elements >= 16U) {
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
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            zmm[0]        = _mm512_slli_epi32(zmm[0], 3U);
            low_nibblemm  = _mm512_and_si512(zmm[0], maskmm);
            high_nibblemm = _mm512_srli_epi16(zmm[0], 4U);
            high_nibblemm = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            zmm[0] = _mm512_or_si512(low_nibblemm, high_nibblemm);
            zmm[0] = _mm512_shuffle_epi8(zmm[0], reverse_mask_32u);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 29U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu32u(src_ptr, num_elements, 0U, 29U, dst_ptr); }
}

// ********************** 30u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_30u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 30U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu32u(src_ptr, align, start_bit, 30U, dst_ptr);
        src_ptr += ((align * 30U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask16 read_mask        = OWN_BIT_MASK(OWN_BITS_2_DWORD(30U * OWN_WORD_WIDTH));
        __m512i   parse_mask0      = _mm512_set1_epi32(OWN_BIT_MASK(30U));
        __m512i   nibble_reversemm = _mm512_load_si512(nibble_reverse_table);
        __m512i   reverse_mask_32u = _mm512_load_si512(reverse_mask_table_32u);
        __m512i   maskmm           = _mm512_set1_epi8(0x0F);

        __m512i shuffle_idx_ptr[2];
        shuffle_idx_ptr[0] = _mm512_load_si512(shuffle_idx_table_30u_0);
        shuffle_idx_ptr[1] = _mm512_load_si512(shuffle_idx_table_30u_1);

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_30u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_30u_1);

        __m512i shift_mask_ptr[4];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_30u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_30u_1);
        shift_mask_ptr[2] = _mm512_load_si512(shift_table_30u_2);
        shift_mask_ptr[3] = _mm512_load_si512(shift_table_30u_3);

        __m512i gather_idxmm = _mm512_load_si512(gather_idx_table_30u);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_i64gather_epi64(gather_idxmm, src_ptr, 1U);

            // shuffling so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[0]);
            zmm[1] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[1]);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[2]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[3]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 30U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
        if (num_elements >= 16U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_maskz_loadu_epi32(read_mask, src_ptr);

            __m512i low_nibblemm  = _mm512_and_si512(srcmm, maskmm);
            __m512i high_nibblemm = _mm512_srli_epi16(srcmm, 4U);
            high_nibblemm         = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            srcmm = _mm512_or_si512(low_nibblemm, high_nibblemm);

            // permuting so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            zmm[0]        = _mm512_slli_epi32(zmm[0], 2U);
            low_nibblemm  = _mm512_and_si512(zmm[0], maskmm);
            high_nibblemm = _mm512_srli_epi16(zmm[0], 4U);
            high_nibblemm = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            zmm[0] = _mm512_or_si512(low_nibblemm, high_nibblemm);
            zmm[0] = _mm512_shuffle_epi8(zmm[0], reverse_mask_32u);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 30U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu32u(src_ptr, num_elements, 0U, 30U, dst_ptr); }
}

// ********************** 31u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_31u32u,
            (const uint8_t* src_ptr, uint32_t num_elements, uint32_t start_bit, uint8_t* dst_ptr)) {
    if (start_bit > 0U) {
        uint32_t align = own_get_align(start_bit, 31U, 32U);
        if (align > num_elements) { align = num_elements; }
        px_qplc_unpack_be_Nu32u(src_ptr, align, start_bit, 31U, dst_ptr);
        src_ptr += ((align * 31U) + start_bit) >> 3;
        dst_ptr += align * 4;
        num_elements -= align;
    }

    if (num_elements >= 16U) {
        __mmask32 read_mask        = OWN_BIT_MASK(31U);
        __m512i   parse_mask0      = _mm512_set1_epi32(OWN_BIT_MASK(31U));
        __m512i   nibble_reversemm = _mm512_load_si512(nibble_reverse_table);
        __m512i   reverse_mask_32u = _mm512_load_si512(reverse_mask_table_32u);
        __m512i   maskmm           = _mm512_set1_epi8(0x0F);

        __m512i shuffle_idx_ptr[2];
        shuffle_idx_ptr[0] = _mm512_load_si512(shuffle_idx_table_31u_0);
        shuffle_idx_ptr[1] = _mm512_load_si512(shuffle_idx_table_31u_1);

        __m512i permutex_idx_ptr[2];
        permutex_idx_ptr[0] = _mm512_load_si512(permutex_idx_table_31u_0);
        permutex_idx_ptr[1] = _mm512_load_si512(permutex_idx_table_31u_1);

        __m512i shift_mask_ptr[4];
        shift_mask_ptr[0] = _mm512_load_si512(shift_table_31u_0);
        shift_mask_ptr[1] = _mm512_load_si512(shift_table_31u_1);
        shift_mask_ptr[2] = _mm512_load_si512(shift_table_31u_2);
        shift_mask_ptr[3] = _mm512_load_si512(shift_table_31u_3);

        __m512i gather_idxmm = _mm512_load_si512(gather_idx_table_31u);

        while (num_elements >= 32U) {
            __m512i srcmm, zmm[2];

            srcmm = _mm512_i64gather_epi64(gather_idxmm, src_ptr, 1U);

            // shuffling so in zmm[0] will be elements with even indexes and in zmm[1] - with odd ones
            zmm[0] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[0]);
            zmm[1] = _mm512_shuffle_epi8(srcmm, shuffle_idx_ptr[1]);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[2]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[3]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 31U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
        if (num_elements >= 16U) {
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
            zmm[0] = _mm512_permutexvar_epi32(permutex_idx_ptr[0], srcmm);
            zmm[1] = _mm512_permutexvar_epi32(permutex_idx_ptr[1], srcmm);

            // shifting elements so they start from the start of the word
            zmm[0] = _mm512_srlv_epi64(zmm[0], shift_mask_ptr[0]);
            zmm[1] = _mm512_sllv_epi64(zmm[1], shift_mask_ptr[1]);

            // gathering even and odd elements together
            zmm[0] = _mm512_mask_mov_epi32(zmm[0], 0xAAAA, zmm[1]);
            zmm[0] = _mm512_and_si512(zmm[0], parse_mask0);

            zmm[0]        = _mm512_slli_epi32(zmm[0], 1U);
            low_nibblemm  = _mm512_and_si512(zmm[0], maskmm);
            high_nibblemm = _mm512_srli_epi16(zmm[0], 4U);
            high_nibblemm = _mm512_and_si512(high_nibblemm, maskmm);

            low_nibblemm  = _mm512_shuffle_epi8(nibble_reversemm, low_nibblemm);
            high_nibblemm = _mm512_shuffle_epi8(nibble_reversemm, high_nibblemm);
            low_nibblemm  = _mm512_slli_epi16(low_nibblemm, 4U);

            zmm[0] = _mm512_or_si512(low_nibblemm, high_nibblemm);
            zmm[0] = _mm512_shuffle_epi8(zmm[0], reverse_mask_32u);

            _mm512_storeu_si512(dst_ptr, zmm[0]);

            src_ptr += 2U * 31U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }

    if (num_elements > 0) { px_qplc_unpack_be_Nu32u(src_ptr, num_elements, 0U, 31U, dst_ptr); }
}

// ********************** 32u ****************************** //

OWN_OPT_FUN(void, k0_qplc_unpack_be_32u32u, (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr)) {
    if (num_elements >= 16U) {
        __m512i reverse_mask_32u = _mm512_load_si512(reverse_mask_table_32u);
        while (num_elements >= 16U) {
            __m512i srcmm = _mm512_loadu_si512(src_ptr);
            srcmm         = _mm512_shuffle_epi8(srcmm, reverse_mask_32u);
            _mm512_storeu_si512(dst_ptr, srcmm);

            src_ptr += 2U * 32U;
            dst_ptr += 64U;
            num_elements -= 16U;
        }
    }
    uint32_t* src32u_ptr = (uint32_t*)src_ptr;
    uint32_t* dst32u_ptr = (uint32_t*)dst_ptr;

    for (uint32_t i = 0U; i < num_elements; i++) {
        dst32u_ptr[i] = qplc_swap_bytes_32u(src32u_ptr[i]);
    }
}

#endif
