/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Generators
 */

#ifndef QPL_TESTS_GENERATORS_INCLUDE_FORMAT_GENERATOR_HPP_
#define QPL_TESTS_GENERATORS_INCLUDE_FORMAT_GENERATOR_HPP_

#include <vector>

#include "stdint.h"

namespace qpl::test {
class format_generator {
public:
    static auto generate_uint_bit_sequence(uint32_t length, uint32_t bit_width, uint32_t seed, bool is_little_endian,
                                           uint32_t prologue_bytes = 0U) -> std::vector<uint8_t>;

    static auto push_back_uint_vector(std::vector<uint8_t>& vector, std::vector<uint32_t>& values_vector,
                                      uint32_t bit_vector_element_width, bool is_little_endian) -> std::vector<uint8_t>;

    static auto get_random_crc64_poly(uint32_t poly_shift, uint32_t seed) -> uint64_t;

    static auto generate_length_sequence() -> std::vector<uint32_t>;
};
} // namespace qpl::test

#endif //QPL_TESTS_GENERATORS_INCLUDE_FORMAT_GENERATOR_HPP_
