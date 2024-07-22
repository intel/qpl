/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "util/checksum.hpp"

#include "compression/inflate/isal_kernels_wrappers.hpp"
#include "igzip_checksums.h"

namespace qpl::ml::util {

/**
 * @brief function to compute Adler-32 checksum.
 *
 * Adler-32 checksum is (B << 16) | A, where:
 * A is the sum of all bytes in the input stream + 1,
 * B is the sum of the individual values of A,
 * and both values are modulo 65521 (largest prime smaller than 2^16).
 *
 * @note if `seed` is non-zero, then the result is the adler32 for the bitstream,
 * consisting of two parts:
 * current input stream starting at `begin` and some other stream which adler32 checksum is `seed`.
 *
 * Otherwise result is simply the adler32 for the input stream starting at `begin`.
*/
auto adler32(const uint8_t* const begin, uint32_t size, uint32_t seed) noexcept -> uint32_t {
    auto old_adler32 = seed;
    auto new_adler32 = seed & least_significant_16_bits;

    new_adler32 = (new_adler32 == adler32_mod - 1) ? 0 : new_adler32 + 1;
    old_adler32 = qpl_isal_adler32((old_adler32 & most_significant_16_bits) | new_adler32, begin, size);
    new_adler32 = (old_adler32 & least_significant_16_bits);
    new_adler32 = (new_adler32 == 0) ? adler32_mod - 1 : new_adler32 - 1;

    return (old_adler32 & most_significant_16_bits) | new_adler32;
}

} // namespace qpl::ml::util
