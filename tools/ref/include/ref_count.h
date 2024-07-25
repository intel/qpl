/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 10/03/2019
 * @brief Contains internal definitions for reference implementation of count elements
 *
 * @defgroup REFERENCE_COUNT Count
 * @ingroup REFERENCE_COMMONS
 * @{
 */

#ifndef _QPL_REF_COUNT_H_
#define _QPL_REF_COUNT_H_

#include "own_ref_defs.h"
#include "ref_prle.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @todo
 * @param source_ptr
 * @param source_end_ptr
 * @param number_of_elements_ptr
 * @param available_bytes
 * @return
 */
qpl_status ref_count_elements_prle(const uint8_t* const source_ptr, const uint8_t* const source_end_ptr,
                                   uint32_t* const number_of_elements_ptr, uint32_t available_bytes);

/**
 * @todo
 * @param source_ptr
 * @param number_of_elements
 * @return
 */
uint32_t ref_count_non_zero_elements_32u(const uint32_t* const source_ptr, uint32_t number_of_elements);

#if defined(__cplusplus)
}
#endif

#endif // _QPL_REF_COUNT_H_

/** @} */
