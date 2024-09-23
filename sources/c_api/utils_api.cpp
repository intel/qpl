/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Utility API (public C API)
 */

#include <cstdint>

#include "qpl/qpl.h"

#include "own_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

// Calculate maximum size needed for compression operations
QPL_FUN(uint32_t, qpl_get_safe_deflate_compression_buffer_size, (uint32_t source_size)) {

    // Define constants
    const uint32_t BLOCK_HEADER_SIZE       = 5U;
    const uint32_t FIRST_BLOCK_HEADER_SIZE = 1U;
    const uint32_t OUTPUT_ACCUMULATOR_SIZE = 32U;
    const uint32_t END_OF_BLOCK_SIZE       = 2U;

    // Check if source_size + overhead is >= 0xFFFFFFFF
    if (source_size >= (UINT32_MAX - FIRST_BLOCK_HEADER_SIZE - OUTPUT_ACCUMULATOR_SIZE - END_OF_BLOCK_SIZE)) {
        return 0;
    }

    // Efficient divide by 65535, works for all values of x except for 0xFFFFFFFF
    auto div_65535 = [](uint64_t x) -> uint32_t {
        const uint32_t BITS_PER_BYTE          = 8U;
        const uint32_t DIV_ROUND_UP_INCREMENT = 1U;

        const uint64_t y = (x >> (BITS_PER_BYTE * 2)) + x + DIV_ROUND_UP_INCREMENT;
        return static_cast<uint32_t>(y >> (BITS_PER_BYTE * 2));
    };

// Calculating stored block size, does not include first block header
#ifdef _WIN32
    auto stored_block_size = [&div_65535, BLOCK_HEADER_SIZE, FIRST_BLOCK_HEADER_SIZE](uint32_t osize) -> uint32_t {
#else
    auto stored_block_size = [&div_65535](uint32_t osize) -> uint32_t {
#endif
        const uint32_t ROUND_UP_OFFSET = UINT16_MAX - 1U;

        if (osize == 0) return BLOCK_HEADER_SIZE - FIRST_BLOCK_HEADER_SIZE;
        const uint32_t num_blks = div_65535(osize + ROUND_UP_OFFSET); // round up
        return osize + (num_blks * BLOCK_HEADER_SIZE) - FIRST_BLOCK_HEADER_SIZE;
    };

    // Estimate the maximum size needed for compression operations
    // Add 1 byte for the first block header, 32 bytes for the output accumulator, and 2 bytes for the EOB
    uint32_t num_bytes =
            stored_block_size(source_size) + FIRST_BLOCK_HEADER_SIZE + OUTPUT_ACCUMULATOR_SIZE + END_OF_BLOCK_SIZE;

    // Round up to even number of bytes for BE16 encoding
    if (num_bytes % 2 != 0) { num_bytes++; }

    return num_bytes;
}

#ifdef __cplusplus
}
#endif
