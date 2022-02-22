/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @brief Contains implementation of all functions for set_membership analytics operation
 * @date 07/28/2020
 *
 * @details Function list:
 *          - @ref qplc_set_membership_8u_i
 *          - @ref qplc_set_membership_16u8u_i
 *          - @ref qplc_set_membership_32u8u_i
 *
 */

#include "own_qplc_defs.h"

#if PLATFORM >= K0

#include "opt/qplc_set_membership_k0.h"

#endif

/******** in-place set membership functions ********/

OWN_QPLC_FUN(void, qplc_set_membership_8u_i, (uint8_t * src_dst_ptr,
        const uint8_t *src2_ptr,
        uint32_t      length,
        uint32_t      shift,
        uint32_t      mask)) {

#if PLATFORM >= K0
    CALL_OPT_FUNCTION(k0_qplc_set_membership_8u_i)(src_dst_ptr, src2_ptr, length, shift, mask);
#else
    uint32_t index;
    for (uint32_t idx = 0u; idx < length; idx++) {
        index = (src_dst_ptr[idx] >> shift) & mask;
        src_dst_ptr[idx] = src2_ptr[index];
    }
#endif
}

OWN_QPLC_FUN(void, qplc_set_membership_16u8u_i, (uint8_t * src_dst_ptr,
        const uint8_t *src2_ptr,
        uint32_t      length,
        uint32_t      shift,
        uint32_t      mask)) {

#if PLATFORM >= K0
    CALL_OPT_FUNCTION(k0_qplc_set_membership_16u8u_i)(src_dst_ptr, src2_ptr, length, shift, mask);
#else
    uint16_t *src_16u_ptr = (uint16_t *) src_dst_ptr;
    uint8_t  *dst_ptr     = src_dst_ptr;
    uint32_t index;

    for (uint32_t idx = 0u; idx < length; idx++) {
        index = (src_16u_ptr[idx] >> shift) & mask;
        dst_ptr[idx] = src2_ptr[index];
    }
#endif
}

OWN_QPLC_FUN(void, qplc_set_membership_32u8u_i, (uint8_t * src_dst_ptr,
        const uint8_t *src2_ptr,
        uint32_t      length,
        uint32_t      shift,
        uint32_t      mask)) {

#if PLATFORM >= K0
    CALL_OPT_FUNCTION(k0_qplc_set_membership_32u8u_i)(src_dst_ptr, src2_ptr, length, shift, mask);
#else
    uint32_t *src_32u_ptr = (uint32_t *) src_dst_ptr;
    uint8_t  *dst_ptr     = src_dst_ptr;
    uint32_t index;

    for (uint32_t idx = 0u; idx < length; idx++) {
        index = (src_32u_ptr[idx] >> shift) & mask;
        dst_ptr[idx] = src2_ptr[index];
    }
#endif
}
