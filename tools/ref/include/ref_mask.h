/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/22/2019
 * @brief Internal definitions for reference implementation of mask extracting functionality
 *
 * @defgroup REFERENCE_MASK Mask
 * @ingroup REFERENCE_COMMONS
 * @{
 */

#ifndef _QPL_REF_MASK_H_
#define _QPL_REF_MASK_H_

#include "own_ref_defs.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @todo
 * @param source_ptr
 * @param length
 * @param mask_be
 * @param destination_ptr
 * @return
 */
REF_INLINE qpl_status ref_extract_mask_bits(const uint8_t* const source_ptr, uint32_t length, uint32_t mask_be,
                                            uint32_t* const destination_ptr) {
    REF_BAD_PTR_RET(source_ptr);
    REF_BAD_PTR_RET(destination_ptr);

    const uint8_t* current_ptr = source_ptr;

    // Init mask bit-mask
    uint8_t bit_mask = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);

    for (uint32_t i = 0; i < length; ++i) {
        // Get mask bit
        destination_ptr[i] = (*current_ptr) & bit_mask;

        // Sift mask bit-mask to the next position
        bit_mask = (mask_be) ? bit_mask >> QPL_ONE_8U : bit_mask << QPL_ONE_8U;

        // If bit-mask is beyond byte boundaries
        if (!bit_mask) {
            // Load next mask byte
            ++current_ptr;

            // Init mask bit for the new byte
            bit_mask = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
        }
    }

    return QPL_STS_OK;
}

#if defined(__cplusplus)
}
#endif

#endif // _QPL_REF_MASK_H_

/** @} */