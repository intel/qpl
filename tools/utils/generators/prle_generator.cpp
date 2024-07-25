/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "prle_generator.hpp"

#include <algorithm>

constexpr uint32_t seven_low_bits_mask            = 0x7F;
constexpr uint32_t six_low_bits_mask              = seven_low_bits_mask >> 1;
constexpr uint32_t prle_count_following_byte_bits = 7;
constexpr uint32_t prle_count_first_byte_bits     = 6;
constexpr uint32_t byte_bits_length               = 8;
constexpr uint32_t bit_buffer_length              = sizeof(uint64_t) * byte_bits_length;
constexpr uint32_t bit_half_buffer_length         = bit_buffer_length >> 1;

namespace qpl::test {
auto create_prle_header(prle_encoding_t prle_encoding, uint32_t prle_count) -> std::vector<uint8_t> {
    uint32_t             header_bytes = get_prle_header_size_bytes(prle_count);
    std::vector<uint8_t> result_vector(header_bytes);
    auto                 source_it = result_vector.begin();

    uint64_t buffer         = 0;
    int32_t  bits_in_buffer = 0;

    if (prle_encoding_t::parquet == prle_encoding) // store encoding type
    {
        buffer = (1ULL << 0);
    }

    uint64_t mask = six_low_bits_mask;

    // store first 6 bits of count with 1 bit shift, because 1st byte contains encoding type information
    buffer |= (uint64_t(prle_count) & mask) << 1;

    if (header_bytes > 1) // check if additional bytes required to store count
    {
        // set last bit to indicate that the following byte is still header
        buffer |= (1UL << prle_count_following_byte_bits);
    }

    *source_it = uint8_t(buffer); // store first header byte
    header_bytes--;
    source_it++;

    buffer         = 0;
    bits_in_buffer = 0;
    mask           = seven_low_bits_mask;
    prle_count >>= prle_count_first_byte_bits;

    while (header_bytes > 0) // store the rest of prle_count bits
    {
        buffer |= (uint64_t(prle_count) & mask) << bits_in_buffer;

        prle_count >>= prle_count_following_byte_bits;
        bits_in_buffer += prle_count_following_byte_bits;
        header_bytes--;

        if (header_bytes > 0) {
            buffer |= (1ULL << bits_in_buffer);
            bits_in_buffer++;
        }

        if (bits_in_buffer >= bit_half_buffer_length) {
            auto source32_ptr = reinterpret_cast<uint32_t*>(&(*source_it));
            *source32_ptr     = (uint32_t)buffer;
            source_it += sizeof(uint32_t);
            buffer >>= bit_half_buffer_length;
            bits_in_buffer -= bit_half_buffer_length;
        }
    }

    while (0 < bits_in_buffer) {
        *source_it = (uint8_t)buffer;
        source_it++;
        buffer >>= byte_bits_length;
        bits_in_buffer -= byte_bits_length;
    }

    auto actual_vector_size = std::distance(result_vector.begin(), source_it);
    result_vector.resize(actual_vector_size);

    return result_vector;
}

auto create_rle_group(rle_element_t rle_element) -> std::vector<uint8_t> {
    // Get prle stream header size
    const uint32_t header_size = get_prle_header_size_bytes(rle_element.repeat_count);

    // Expect to have the following maximum result RLE vector size
    const uint32_t       expected_vector_size = sizeof(uint32_t) + header_size;
    std::vector<uint8_t> result_vector(expected_vector_size);
    auto                 source_it = result_vector.begin();

    // Create prle header for this rle group
    auto prle_header = create_prle_header(prle_encoding_t::run_length_encoding, rle_element.repeat_count);

    // Store RLE group header into result vector
    for (size_t i = 0; i < prle_header.size() && source_it != result_vector.end(); i++) {
        *source_it = prle_header.at(i);
        source_it++;
    }

    const uint64_t mask           = (1ULL << rle_element.bit_width) - 1;
    uint64_t       buffer         = 0;
    int32_t        bits_in_buffer = 0;

    // Store RLE-value to buffer
    buffer |= (uint64_t(rle_element.element_value) & mask) << bits_in_buffer;
    bits_in_buffer += rle_element.bit_width;

    // Pack RLE-value to result vector
    while (0 < bits_in_buffer && source_it != result_vector.end()) {
        *source_it = (uint8_t)buffer;
        source_it++;
        buffer >>= byte_bits_length;
        bits_in_buffer -= byte_bits_length;
    }

    auto actual_vector_size = std::distance(result_vector.begin(), source_it);
    result_vector.resize(actual_vector_size);

    return result_vector;
}

auto create_parquet_group(parquet_element_t parquet_element) -> std::vector<uint8_t> {
    const uint64_t mask           = (1ULL << parquet_element.bit_width) - 1;
    uint64_t       buffer         = 0;
    int32_t        bits_in_buffer = 0;

    const uint32_t expected_result_size =
            get_prle_header_size_bytes(parquet_element.repeat_count) + sizeof(uint32_t) * 8;

    std::vector<uint8_t> result_vector(expected_result_size, 0U);

    auto source_it     = result_vector.begin();
    auto source_end_it = result_vector.end();

    // Create prle header for this rle group
    auto prle_header = create_prle_header(prle_encoding_t::parquet, parquet_element.repeat_count);

    // Store RLE group header into result vector
    for (auto prle_header_element : prle_header) {
        *source_it = prle_header_element;
        source_it++;
    }

    for (auto element : parquet_element.parquet_group) {
        buffer |= (uint64_t(element) & mask) << bits_in_buffer;
        bits_in_buffer += parquet_element.bit_width;

        if (bits_in_buffer >= bit_half_buffer_length) {
            auto source32_ptr = reinterpret_cast<uint32_t*>(&(*source_it));

            if (source_it >= source_end_it) {
                throw std::out_of_range("Not enough elements in the temporary buffer while generating PRLE stream");
            }

            *source32_ptr = (uint32_t)buffer;
            source_it += sizeof(uint32_t);
            buffer >>= bit_half_buffer_length;
            bits_in_buffer -= bit_half_buffer_length;
        }
    }

    while (0 < bits_in_buffer) {
        if (source_it >= source_end_it) {
            throw std::out_of_range("Not enough elements in the temporary buffer while generating PRLE stream");
        }

        *source_it = (uint8_t)buffer;
        source_it++;
        buffer >>= byte_bits_length;
        bits_in_buffer -= byte_bits_length;
    }

    auto actual_vector_size = std::distance(result_vector.begin(), source_it);
    result_vector.resize(actual_vector_size);

    return result_vector;
}

auto get_prle_header_size_bytes(uint32_t count) -> uint32_t {
    uint32_t bit_count = 0; // number of bits required to store count value

    for (; count != 0; bit_count++) {
        count >>= 1;
    }

    uint32_t header_bytes = 1;

    header_bytes += (bit_count > 6) ? 1 : 0;
    header_bytes += (bit_count > 13) ? 1 : 0;
    header_bytes += (bit_count > 20) ? 1 : 0;

    return header_bytes;
}

} // namespace qpl::test
