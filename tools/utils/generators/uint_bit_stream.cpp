/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Generators
 */

#include <algorithm>

#include "format_generator.hpp"
#include "random_generator.h"

namespace qpl::test {

namespace details {

constexpr uint32_t BYTE_BIT_LENGTH  = 8U;
constexpr uint32_t BIT_BUF_LEN      = (sizeof(uint64_t) * BYTE_BIT_LENGTH);
constexpr uint32_t BIT_BUF_LEN_HALF = (BIT_BUF_LEN >> 1);

static inline auto bit_reverse32(uint32_t input) -> uint32_t {
    union {
        uint32_t uint;
        uint8_t  ubyte[4];
    } buffer, output;

    buffer.uint     = input;
    output.ubyte[0] = buffer.ubyte[3];
    output.ubyte[1] = buffer.ubyte[2];
    output.ubyte[2] = buffer.ubyte[1];
    output.ubyte[3] = buffer.ubyte[0];

    return output.uint;
}

} // namespace details

auto format_generator::push_back_uint_vector(std::vector<uint8_t>& vector, std::vector<uint32_t>& values_vector,
                                             uint32_t bit_vector_element_width, bool is_little_endian)
        -> std::vector<uint8_t> {
    using namespace details;
    const uint32_t bytes_in_element = (bit_vector_element_width + 7) / BYTE_BIT_LENGTH;
    const uint32_t vector_size      = static_cast<uint32_t>(vector.size());

    const uint64_t mask           = (1ULL << bit_vector_element_width) - 1;
    uint64_t       buffer         = 0U;
    uint32_t       bits_in_buffer = 0U;

    // Prepare vector
    vector.resize(vector_size + values_vector.size() * 4U);

    auto vector_it = vector.begin() + vector_size;
    std::fill(vector_it, vector.end(), 0U);

    for (auto source_it = values_vector.begin(); source_it < values_vector.end(); source_it++) {
        auto value_to_load = static_cast<uint64_t>(*source_it & mask);
        buffer |= value_to_load << bits_in_buffer;
        bits_in_buffer += bit_vector_element_width;

        if (BIT_BUF_LEN_HALF <= bits_in_buffer) {
            auto destination_ptr = reinterpret_cast<uint32_t*>(&(*vector_it));

            if (is_little_endian) {
                *destination_ptr = static_cast<uint32_t>(buffer);
            } else {
                *destination_ptr = bit_reverse32(static_cast<uint32_t>(buffer));
            }

            vector_it += sizeof(uint32_t);
            buffer >>= BIT_BUF_LEN_HALF;
            bits_in_buffer -= BIT_BUF_LEN_HALF;
        }
    }

    while (0 < bits_in_buffer) {
        if (is_little_endian) {
            *vector_it = static_cast<uint8_t>(buffer);
        } else {
            if (bits_in_buffer > 24) {
                auto fourth_byte = static_cast<uint8_t>(buffer);
                buffer >>= BYTE_BIT_LENGTH;
                auto third_byte = static_cast<uint8_t>(buffer);
                buffer >>= BYTE_BIT_LENGTH;

                auto second_byte = static_cast<uint8_t>(buffer);
                buffer >>= BYTE_BIT_LENGTH;

                auto first_byte = static_cast<uint8_t>(buffer);
                buffer >>= BYTE_BIT_LENGTH;
                *vector_it = first_byte;
                vector_it++;
                *vector_it = second_byte;
                vector_it++;
                *vector_it = third_byte;
                vector_it++;
                *vector_it = fourth_byte;
            } else if (bits_in_buffer > 16) {
                auto third_byte = static_cast<uint8_t>(buffer);
                buffer >>= BYTE_BIT_LENGTH;

                auto second_byte = static_cast<uint8_t>(buffer);
                buffer >>= BYTE_BIT_LENGTH;

                auto first_byte = static_cast<uint8_t>(buffer);
                buffer >>= BYTE_BIT_LENGTH;
                *vector_it = first_byte;
                vector_it++;
                *vector_it = second_byte;
                vector_it++;
                *vector_it = third_byte;
            } else if (bits_in_buffer > 8) {
                auto third_byte = static_cast<uint8_t>(buffer);
                buffer >>= BYTE_BIT_LENGTH;

                auto second_byte = static_cast<uint8_t>(buffer);
                buffer >>= BYTE_BIT_LENGTH;
                *vector_it = second_byte;
                vector_it++;
                *vector_it = third_byte;
            } else {
                auto third_byte = static_cast<uint8_t>(buffer);
                buffer >>= BYTE_BIT_LENGTH;
                *vector_it = third_byte;
            }

            bits_in_buffer = 0;
        }

        vector_it++;
        buffer >>= BYTE_BIT_LENGTH;

        if (bits_in_buffer < BYTE_BIT_LENGTH) { break; }

        bits_in_buffer -= BYTE_BIT_LENGTH;
    }

    if (vector.end() - vector_it >= bytes_in_element) { vector.erase(vector_it, vector.end()); }

    return vector;
}

auto format_generator::generate_uint_bit_sequence(uint32_t length, uint32_t bit_width, uint32_t seed,
                                                  bool is_little_endian, uint32_t prologue_bytes)
        -> std::vector<uint8_t> {
    std::vector<uint8_t> vector(prologue_bytes);

    const uint64_t    max_input_value = (1ULL << bit_width) - 1U;
    qpl::test::random num_generator(0, static_cast<double>(max_input_value), seed);

    std::vector<uint32_t> values_vector(length, 0U);
    std::generate(values_vector.begin(), values_vector.end(),
                  [&num_generator]() { return static_cast<uint32_t>(num_generator); });

    vector = push_back_uint_vector(vector, values_vector, bit_width, is_little_endian);

    return vector;
}

auto format_generator::generate_length_sequence() -> std::vector<uint32_t> {
    std::vector<uint32_t> result;

    for (uint32_t i = 1; i < 128; i++) {
        result.push_back(i);
    }

    for (size_t i = 7; i <= 13; i++) {
        auto leftIndex  = 1 << i;
        auto rightIndex = leftIndex << 1;
        result.push_back((rightIndex + leftIndex) >> 1);
    }

    return result;
}

} // namespace qpl::test
