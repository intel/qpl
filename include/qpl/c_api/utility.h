/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Utility API (public C++ API)
 */

#ifndef QPL_UTILITY_H_
#define QPL_UTILITY_H_

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC visibility push(default)
#endif

#include <stdint.h>

#include "qpl/c_api/defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup UTILITY_API Utility API
 * @{
 */

/**
 * @brief Calculate the maximum buffer size for compression, compression output should not exceed this size.
 * 
 * @param[in]  source_size  size of the input buffer
 * 
 * @note This only applies to deflate compressions, Huffman Only mode is not supported.
 * 
 * @note This function does not include overhead for gzip/zlib headers and footers.
 * 
 * @return uint32_t
 */
QPL_API(uint32_t, qpl_get_safe_deflate_compression_buffer_size, (uint32_t source_size))

/** @} */

#ifdef __cplusplus
}
#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC visibility pop
#endif

#endif // QPL_UTILITY_H_
