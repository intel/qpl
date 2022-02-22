/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*------- qplc_find_unique.h -------*/

/**
 * @date 07/27/2020
 *
 * @defgroup SW_KERNELS_FIND_UNIQUE_API Find Unique API
 * @ingroup  SW_KERNELS_PRIVATE_API
 * @{
 * @brief Contains Intel® Query Processing Library (Intel® QPL) Core API for `Find Unique` operation
 *
 * @details Core APIs implement the following functionalities:
 *      -   Find unique analytics operation out-of-place kernels for 8u, 16u and 32u input data and 8u output.
 *
 */

#include "qplc_defines.h"

#ifndef QPLC_FIND_UNIQUE_H__
#define QPLC_FIND_UNIQUE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*qplc_find_unique_t_ptr)(const uint8_t *src_ptr,
                                       uint8_t *dst_ptr,
                                       uint32_t length,
                                       uint32_t shift,
                                       uint32_t mask);

/**
 * @name qplc_find_unique_<input bit-width><output bit-width>
 *
 * @brief Find Unique analytics operation out-of-place kernels for 8u, 16u and 32u input data
 *
 * @param[in]   src_ptr  pointer to source vector
 * @param[out]  dst_ptr  pointer to destination vector
 * @param[in]   length   length of source vector in elements
 * @param[in]   shift    how many low bits to drop
 * @param[in]   mask     number of bits that shall remain for analysis from src_ptr[x] value
 *
 * @note Find unique operation sets 1 in dst for values which indexes are defined by src_ptr[x] values
 *
 * @return
 *      - n/a (void).
 * @{
 */
OWN_QPLC_API(void, qplc_find_unique_8u, (const uint8_t *src_ptr,
        uint8_t *dst_ptr,
        uint32_t length,
        uint32_t shift,
        uint32_t mask))

OWN_QPLC_API(void, qplc_find_unique_16u8u, (const uint8_t *src_ptr,
        uint8_t *dst_ptr,
        uint32_t length,
        uint32_t shift,
        uint32_t mask))

OWN_QPLC_API(void, qplc_find_unique_32u8u, (const uint8_t *src_ptr,
        uint8_t *dst_ptr,
        uint32_t length,
        uint32_t shift,
        uint32_t mask))
/** @} */

#ifdef __cplusplus
}
#endif

#endif // QPLC_FIND_UNIQUE_H__
/** @} */
