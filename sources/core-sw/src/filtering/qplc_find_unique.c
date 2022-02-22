/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @brief Contains implementation of all functions for find_unique analytics operation
 * @date 07/27/2020
 *
 * @details Function list:
 *          - @ref qplc_find_unique_8u
 *          - @ref qplc_find_unique_16u8u
 *          - @ref qplc_find_unique_32u8u
 */

#include "own_qplc_defs.h"

#if PLATFORM >= K0

#include "opt/qplc_find_unique_k0.h"

#endif


/******** out-of-place find unique functions ********/

OWN_QPLC_FUN(void, qplc_find_unique_8u, (const uint8_t *src_ptr,
        uint8_t *dst_ptr,
        uint32_t length,
        uint32_t shift,
        uint32_t mask)) {
#if PLATFORM >= K0
    CALL_OPT_FUNCTION(k0_qplc_find_unique_8u)(src_ptr, dst_ptr, length, shift, mask);
#else
    uint32_t index;

    for (uint32_t idx = 0u; idx < length; idx++) {
        index = (src_ptr[idx] >> shift) & mask;
        dst_ptr[index] = 1u;
    }
#endif
}

OWN_QPLC_FUN(void, qplc_find_unique_16u8u, (const uint8_t *src_ptr,
        uint8_t *dst_ptr,
        uint32_t length,
        uint32_t shift,
        uint32_t mask)) {
#if PLATFORM >= K0
    CALL_OPT_FUNCTION(k0_qplc_find_unique_16u8u)(src_ptr, dst_ptr, length, shift, mask);
#else
    uint16_t *src_16u_ptr = (uint16_t *) src_ptr;
    uint32_t index;

    for (uint32_t idx = 0u; idx < length; idx++) {
        index = (src_16u_ptr[idx] >> shift) & mask;
        dst_ptr[index] = 1u;
    }
#endif
}

OWN_QPLC_FUN(void, qplc_find_unique_32u8u, (const uint8_t *src_ptr,
        uint8_t *dst_ptr,
        uint32_t length,
        uint32_t shift,
        uint32_t mask)) {
#if PLATFORM >= K0
    CALL_OPT_FUNCTION(k0_qplc_find_unique_32u8u)(src_ptr, dst_ptr, length, shift, mask);
#else
    uint32_t *src_32u_ptr = (uint32_t *) src_ptr;
    uint32_t index;

    for (uint32_t idx = 0u; idx < length; idx++) {
        index = (src_32u_ptr[idx] >> shift) & mask;
        dst_ptr[index] = 1u;
    }
#endif
}

