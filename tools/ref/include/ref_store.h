/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/22/2019
 * @brief Contains internal definitions for reference implementation of storing value
 *
 * @defgroup REFERENCE_STORE Store
 * @ingroup REFERENCE_COMMONS
 * @{
 */

#ifndef _QPL_REF_STORE_H_
#define _QPL_REF_STORE_H_

#include "own_ref_defs.h"
#include "ref_copy.h"
#include "stdbool.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @todo
 */
#define REF_SIZE_IN_BYTES(nBit) (((nBit) >> 3) + (((nBit) & 7) ? 1 : 0))

/**
 * @todo
 */
#define REF_CHECK_BIT_IN_VECTOR(vector, nBit) (((vector)[(nBit) >> 3]) & (1 << ((nBit) & 7)))

/**
 * @todo
 */
#define REF_OVERVALUE_8U_RET(a, b)                                                             \
    {                                                                                          \
        if (UINT8_MAX < (uint16_t)(a) + (uint16_t)(b)) { return QPL_STS_OUTPUT_OVERFLOW_ERR; } \
    }

/**
 * @todo
 */
#define REF_OVERVALUE_16U_RET(a, b)                                                             \
    {                                                                                           \
        if (UINT16_MAX < (uint32_t)(a) + (uint32_t)(b)) { return QPL_STS_OUTPUT_OVERFLOW_ERR; } \
    }

/**
 * @todo
 */
#define REF_OVERVALUE_32U_RET(a, b)                                                             \
    {                                                                                           \
        if (UINT32_MAX < (uint64_t)(b) + (uint64_t)(b)) { return QPL_STS_OUTPUT_OVERFLOW_ERR; } \
    }

/**
 * @todo
 * @param last_bit_offset_ptr
 * @param number_of_elements
 * @param source_bit_width
 * @param avail_bytes
 * @param format
 * @param out_bytes_ptr
 * @return
 */
REF_INLINE qpl_status ref_get_output_bytes(uint32_t* const last_bit_offset_ptr, uint32_t number_of_elements,
                                           uint32_t source_bit_width, uint32_t avail_bytes, uint32_t format,
                                           uint32_t* const out_bytes_ptr) {
    uint64_t source_bit_length = (uint64_t)number_of_elements * (uint64_t)source_bit_width;

    // Init last bit offset
    (*last_bit_offset_ptr) = 0;

    switch (format) {
        case qpl_ow_nom: {
            // Convert number of bits to number of bytes
            (*out_bytes_ptr)       = (uint32_t)REF_BIT_2_BYTE(source_bit_length);
            (*last_bit_offset_ptr) = source_bit_length & REF_MAX_BIT_IDX;
            break;
        }
        case qpl_ow_8: {
            (*out_bytes_ptr) = sizeof(uint8_t) * number_of_elements;
            break;
        }
        case qpl_ow_16: {
            (*out_bytes_ptr) = sizeof(uint16_t) * number_of_elements;
            break;
        }
        case qpl_ow_32: {
            (*out_bytes_ptr) = sizeof(uint32_t) * number_of_elements;
            break;
        }
        default: {
            return QPL_STS_BIT_WIDTH_ERR;
        }
    }

    // Nor nominal bit-vector and output modification we can't estimate required output
    if (!((QPL_ONE_32U == source_bit_width) && (qpl_ow_nom < format))) {
        // Not enough space in destination buffer
        REF_BAD_ARG_RET((avail_bytes < (*out_bytes_ptr)), QPL_STS_DST_IS_SHORT_ERR);
    }

    return QPL_STS_OK;
}

/**
 * @todo ref_common
 * @param bit_index
 * @param vector_ptr
 * @param o_format
 * @return
 */
qpl_status ref_set_bit_by_index(uint32_t bit_index, uint8_t* vector_ptr, uint32_t o_format);

/**
 * @todo
 * @param source_ptr
 * @param number_of_elements
 * @param source_bit_width
 * @param destination_ptr
 * @param destination_end_ptr
 * @param output_be
 * @param output_format
 * @param element_index_ptr
 * @return
 */
qpl_status ref_store_values(const uint32_t* const source_ptr, uint32_t number_of_elements, uint32_t source_bit_width,
                            uint8_t* const destination_ptr, const uint8_t* const destination_end_ptr, bool output_be,
                            qpl_out_format output_format, uint32_t* element_index_ptr);

#if defined(__cplusplus)
}
#endif

#endif // _QPL_REF_STORE_H_

/** @} */
