/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file own_deflate_statistics.h
 * @brief internal helper functions for deflate statistics gathering
 */

#ifndef OWN_QPL_DEFLATE_STATISTICS_H
#define OWN_QPL_DEFLATE_STATISTICS_H

#include "own_deflate.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Fills @link isal_histogram @endlink from given deflate statistics
 *
 * @note According to rfc1951 literals/lengths and offsets histograms should have 286 and 30 elements.
 *
 * @param[out]  isal_histogram_ptr            Pointer to ISA-L histogram
 * @param[in]   literal_length_histogram_ptr  Pointer to filled literals/lengths histogram
 * @param[in]   offsets_histogram_ptr         Pointer to filled offsets histogram
 *
 * @return this function doen't return anything
 */
OWN_FUN(void, isal_histogram_set_statistics, (isal_histogram * isal_histogram_ptr,
        const uint32_t *literal_length_histogram_ptr,
        const uint32_t *offsets_histogram_ptr));

/**
 * @brief Fills deflate statistics histogram from given @link isal_histogram @endlink
 *
 * @note According to rfc1951 literals/lengths and offsets histograms should have 286 and 30 elements.
 *
 * @param[out]  literal_length_histogram_ptr  Pointer to literals/lengths histogram
 * @param[out]  offsets_histogram_ptr         Pointer to offsets histogram
 * @param[in]   isal_histogram_ptr            Pointer to filled ISA-L histogram
 *
 * @return this function doen't return anything
 */
OWN_FUN(void, isal_histogram_get_statistics, (const isal_histogram *isal_histogram_ptr,
        uint32_t * literal_length_histogram_ptr,
        uint32_t *offsets_histogram_ptr));

/**
 * @brief Ensures that all histogram values have non-zero values
 *
 * @param[in,out]  isal_histogram_ptr  Pointer to deflate histogram
 *
 * @return this function doen't return anything
 */
OWN_FUN(void, isal_histogram_make_complete, (isal_histogram * isal_histogram_ptr));

#ifdef __cplusplus
}
#endif

#endif // OWN_QPL_DEFLATE_STATISTICS_H
