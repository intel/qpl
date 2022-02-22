/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Generators
 */

#include "format_generator.hpp"
#include "random_generator.h"

namespace qpl::test {

namespace details {

template <typename header_type,
          typename elements_type>
static auto generate_zero_compressed_block(qpl::test::random &gen_num) -> std::vector<uint8_t> {
    header_type bit_tags = (std::is_same_v<header_type, uint64_t>)? UINT64_MAX : UINT32_MAX;

    std::vector<elements_type> values; // Temporary vector for generated elements

    const uint32_t block_byte_length = 128u;
    const uint32_t elements_in_block = block_byte_length / sizeof(elements_type);

    for (int i = 0; i < elements_in_block; i++) {
        if ((elements_type) gen_num & 1u) // If true, we suppose that current element is zero
        {
            bit_tags &= ~(1ULL << i); // Set corresponding bit in header to '0'
        } else {
            values.push_back((elements_type) gen_num); // Current element doesn't equal to zero, so we store it
        }
    }

    uint32_t elements_bytes = static_cast<uint32_t>(values.size()) * sizeof(elements_type);
    uint32_t total_bytes    = sizeof(header_type) + elements_bytes;

    std::vector<uint8_t> destination(total_bytes, 0);

    auto header_ptr = reinterpret_cast<header_type *>(destination.data());

    *header_ptr = bit_tags; // Write header to destination
    header_ptr++;

    auto element_ptr = reinterpret_cast<elements_type *>(header_ptr);

    for (auto value: values) {
        *element_ptr = value; // Store element to destination
        element_ptr++;
    }

    return destination;
}
}

auto format_generator::generate_zero_compressed_data(uint32_t bit_width,
                                                     uint32_t byte_length,
                                                     uint32_t seed) -> std::vector<uint8_t> {
    std::vector<uint8_t> destination;
    std::vector<uint8_t> compressed_block;

    if (32u != bit_width && 16u != bit_width) {
        throw std::exception();
    }

    uint32_t max_value = (1ULL << bit_width) - 1u;

    qpl::test::random gen_num(1, max_value, seed);

    for (uint32_t i = 0u; i < byte_length; i++) {
        if (bit_width == 16u) {
            compressed_block = details::generate_zero_compressed_block<uint64_t, uint16_t>(gen_num);
        } else {
            compressed_block = details::generate_zero_compressed_block<uint32_t, uint32_t>(gen_num);
        }

        if (compressed_block.size() + destination.size() > byte_length) {
            break;
        }

        destination.insert(destination.end(), compressed_block.begin(), compressed_block.end());
    }

    return destination;
}
}
