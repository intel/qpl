/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Middle Layer API (private C++ API)
 */

#include "format_generator.hpp"
#include "random_generator.h"

namespace qpl::test {
    auto format_generator::get_random_crc64_poly(uint32_t poly_shift, uint32_t seed) -> uint64_t
    {
        qpl::test::random poly_gen(0, UINT16_MAX, seed);
        uint64_t      result_poly;
        result_poly = (uint16_t) poly_gen;
        result_poly = (result_poly << 16u) ^ (uint16_t) poly_gen;
        result_poly = (result_poly << 16u) ^ (uint16_t) poly_gen;
        result_poly = (result_poly << 16u) ^ (uint16_t) poly_gen;
        result_poly |= 1u;
        result_poly <<= poly_shift;

        return result_poly;
    }
}
