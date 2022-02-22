/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*------- qplc_set_membership.h -------*/

/**
 * @date 07/27/2020
 *
 * @defgroup SW_KERNELS_SET_MEMBERSHIP_API Set Membership API
 * @ingroup  SW_KERNELS_PRIVATE_API
 * @{
 * @brief Contains Intel® Query Processing Library (Intel® QPL) Core API for `Set Membership` operation
 *
 * @details Core APIs implement the following functionalities:
 *      -   Set membership analytics operation out-of-place kernels for 8u, 16u and 32u input data and 8u output.
 *
 */

#include "qplc_defines.h"

#ifndef QPLC_SET_MEMBERSHIP_H__
#define QPLC_SET_MEMBERSHIP_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*qplc_set_membership_i_t_ptr)(uint8_t *src_dst_ptr,
                                            const uint8_t *src2_ptr,
                                            uint32_t length,
                                            uint32_t shift,
                                            uint32_t mask);

/**
 * @name qplc_set_membership_<input bit-width><output bit-width>_i
 *
 * @brief Set Membership analytics operation in-place kernels for 8u, 16u and 32u input data and 8u output
 *
 * @param[in,out]  src_dst_ptr  pointer to source vector #1 that is also a destination vector
 * @param[in]      src2_ptr     pointer to source vector #2
 * @param[in]      length       length of source vector #1 and destination in elements
 * @param[in]      shift        number of low bits to drop
 * @param[in]      mask         number of bits that shall remain for analysis from src1_ptr[x] value
 *
 * @note Set membership operation sets 1 in dst for values corresponding to src1_ptr[x] if src2_ptr[src1_ptr[x]] == 1
 *
 * @return
 *      - n/a (void).
 * @{
 */
OWN_QPLC_API(void, qplc_set_membership_8u_i, (uint8_t * src_dst_ptr,
        const uint8_t *src2_ptr,
        uint32_t      length,
        uint32_t      shift,
        uint32_t      mask))

OWN_QPLC_API(void, qplc_set_membership_16u8u_i, (uint8_t * src_dst_ptr,
        const uint8_t *src2_ptr,
        uint32_t      length,
        uint32_t      shift,
        uint32_t      mask))

OWN_QPLC_API(void, qplc_set_membership_32u8u_i, (uint8_t * src_dst_ptr,
        const uint8_t *src2_ptr,
        uint32_t      length,
        uint32_t      shift,
        uint32_t      mask))
/** @} */

#ifdef __cplusplus
}
#endif

#endif // QPLC_SET_MEMBERSHIP_H__
/** @} */
