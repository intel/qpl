/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 12/07/2019
 * @brief Contains an implementation of the parser for PRLE input format
 *
 * @defgroup REFERENCE_RLE RLE parser
 * @ingroup REFERENCE_COMMONS
 * @{
 */

#include "own_ref_defs.h"

#ifndef _QPL_REF_PRLE_H_
#define _QPL_REF_PRLE_H_

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @todo
 * @param source_ptr
 * @param format_ptr
 * @param count_ptr
 * @param available_bytes_ptr
 * @return
 */
REF_INLINE qpl_status ref_get_format_and_count(const uint8_t** source_ptr, uint32_t* format_ptr, uint32_t* count_ptr,
                                               uint32_t* available_bytes_ptr) {

    // Check if we have enough bytes
    REF_BAD_ARG_RET((0 == (*available_bytes_ptr)), QPL_STS_PRLE_FORMAT_ERR);

    // Extract first byte
    uint32_t byte = (uint32_t)(*((*source_ptr)++));

    // PRLE format marker
    (*format_ptr) = byte & REF_LOW_BIT_MASK;

    // Next six are counter
    (*count_ptr) = (byte & REF_7_BIT_MASK) >> 1;

    // Decrement bytes counter
    (*available_bytes_ptr)--;

    // Marker if we need to load next byte
    if (byte & REF_HIGH_BIT_MASK) {
        // Check if we have enough bytes
        REF_BAD_ARG_RET((0 == (*available_bytes_ptr)), QPL_STS_PRLE_FORMAT_ERR);

        // Get next byte
        byte = (uint32_t)(*((*source_ptr)++));

        // Decrement bytes counter
        (*available_bytes_ptr)--;

        // Get bits for counter
        (*count_ptr) |= (byte & REF_7_BIT_MASK) << 6;

        // Marker if we need to load next byte
        if (byte & REF_HIGH_BIT_MASK) {
            // Check if we have enough bytes
            REF_BAD_ARG_RET((0 == (*available_bytes_ptr)), QPL_STS_PRLE_FORMAT_ERR);

            // Get next byte
            byte = (uint32_t)(*((*source_ptr)++));

            // Decrement bytes counter
            (*available_bytes_ptr)--;

            // Get bits for counter
            (*count_ptr) |= (byte & REF_7_BIT_MASK) << 13;

            // Marker if we need to load next byte
            if (byte & REF_HIGH_BIT_MASK) {
                // Check if we have enough bytes
                REF_BAD_ARG_RET((0 == (*available_bytes_ptr)), QPL_STS_PRLE_FORMAT_ERR);

                // Get next byte
                byte = (uint32_t)(*((*source_ptr)++));

                // Decrement bytes counter
                (*available_bytes_ptr)--;

                // Get bits for counter
                (*count_ptr) |= (byte & REF_7_BIT_MASK) << 20;

                // The next byte is var-int overflow
                if (byte & REF_HIGH_BIT_MASK) { return QPL_STS_PRLE_FORMAT_ERR; }
            }
        }
    }

    return QPL_STS_OK;
}

#if defined(__cplusplus)
}
#endif

#endif // _QPL_REF_PRLE_H_

/** @} */
