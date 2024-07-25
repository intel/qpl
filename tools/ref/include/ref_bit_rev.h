/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/13/2018
 * @brief Contains internal helpers for data transformation from LE to BE and vice versa
 *
 * @defgroup REFERENCE_COMMONS Common Functions
 * @ingroup REFERENCE_PRIVATE
 *
 * @defgroup REFERENCE_REVERSE Reversing
 * @ingroup REFERENCE_COMMONS
 * @{
 */

#ifndef REF_BIT_REV_H__
#define REF_BIT_REV_H__

#include "own_ref_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 * @todo describe one.
 */
extern uint8_t bit_reverse_table_8u[256];

/**
 * @todo ref_common
 * @param vector_ptr
 * @param vector_size
 */
void ref_reverse_bits_8u(uint8_t* vector_ptr, uint32_t vector_size);

/**
 * @brief Helper for converting 8u data from LE to BE and vice versa format
 * @todo describe function
 */
REF_INLINE uint8_t bit_reverse8(uint8_t x) {
    return bit_reverse_table_8u[x];
}

/**
 * @brief Helper for converting 16u data from LE to BE and vice versa format
 * @todo describe function
 */
REF_INLINE uint16_t bit_reverse16(uint16_t x) {
    union {
        uint16_t uint;
        uint8_t  ubyte[2];
    } y, z;

    y.uint     = x;
    z.ubyte[0] = bit_reverse8(y.ubyte[1]);
    z.ubyte[1] = bit_reverse8(y.ubyte[0]);

    return z.uint;
}

/**
 * @brief Helper for converting 32u data from LE to BE and vice versa format
 * @todo describe function
 */
REF_INLINE uint32_t bit_reverse32(uint32_t x) {
    union {
        uint32_t uint;
        uint8_t  ubyte[4];
    } y, z;

    y.uint     = x;
    z.ubyte[0] = bit_reverse8(y.ubyte[3]);
    z.ubyte[1] = bit_reverse8(y.ubyte[2]);
    z.ubyte[2] = bit_reverse8(y.ubyte[1]);
    z.ubyte[3] = bit_reverse8(y.ubyte[0]);

    return z.uint;
}

/**
 * @brief Helper for converting 16u data from LE to BE and vice versa format
 * @todo describe function
 */
REF_INLINE uint16_t swap_bytes16(uint16_t x) {
    union {
        uint16_t uint;
        uint8_t  ubyte[2];
    } y, z;

    y.uint     = x;
    z.ubyte[0] = y.ubyte[1];
    z.ubyte[1] = y.ubyte[0];

    return z.uint;
}

/**
 * @brief Helper for converting 32u data from LE to BE and vice versa format
 * @todo describe function
 */
REF_INLINE uint32_t swap_bytes32(uint32_t x) {
    union {
        uint32_t uint;
        uint8_t  ubyte[4];
    } y, z;

    y.uint     = x;
    z.ubyte[0] = y.ubyte[3];
    z.ubyte[1] = y.ubyte[2];
    z.ubyte[2] = y.ubyte[1];
    z.ubyte[3] = y.ubyte[0];

    return z.uint;
}

#ifdef __cplusplus
}
#endif

#endif /* //REF_BIT_REV_H__ */

/** @} */
