/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/13/2018
 * @brief Internal definitions for reference implementation of conversion uint8_t vector to uint32_t vector
 *
 * @ingroup REFERENCE_PRIVATE
 * @{
 *
 */

#ifndef QPL_REF_CONVERT_H_
#define QPL_REF_CONVERT_H_

#include <stdlib.h>

#include "own_ref_defs.h"
#include "ref_prle.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @todo describe
 * @param source_ptr
 * @param source_bit_offset
 * @param source_bit_width
 * @param number_of_elements
 * @param destination_ptr
 * @param parser
 * @return
 */
qpl_status ref_convert_to_32u_le_be(const uint8_t* const source_ptr, uint32_t source_bit_offset,
                                    uint32_t source_bit_width, uint32_t number_of_elements,
                                    uint32_t* const destination_ptr, qpl_parser parser);

/**
 * @todo describe
 * @param source_ptr
 * @param source_end_ptr
 * @param destination_ptr
 * @param available_bytes_ptr
 * @return
 */
qpl_status ref_convert_to_32u_prle(const uint8_t* const source_ptr, const uint8_t* const source_end_ptr,
                                   uint32_t* const destination_ptr, uint32_t* const available_bytes_ptr);

#if defined(__cplusplus)
}
#endif

#endif // QPL_REF_CONVERT_H_

/** @} */
