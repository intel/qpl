/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <cstdint>

#include "qpl/cpp_api/operations/compression/inflate_stateful_operation.hpp"
#include "qpl/cpp_api/util/deflate_block_utils.hpp"
#include "qpl/cpp_api/util/status_handler.hpp"

namespace qpl::util {

static inline auto get_start_bit_offset(const uint32_t first_bit_index) noexcept -> uint32_t {
    return first_bit_index & (byte_bit_length - 1u);
}

static inline auto get_end_bit_offset(const uint32_t last_bit_index) noexcept -> uint32_t {
    return (byte_bit_length - 1u) & (0u - last_bit_index);
}

static inline auto get_byte_block_size(const uint32_t first_bit_index,
                                       const uint32_t last_bit_index) noexcept -> uint32_t {
    return ((last_bit_index + (byte_bit_length - 1u)) >> byte_bit_length_power) -
           (first_bit_index >> byte_bit_length_power);
}

static inline auto get_bytes_offset(const uint32_t first_bit_index) noexcept -> uint32_t {
    return first_bit_index >> byte_bit_length_power;
}

auto decompress_block(const uint8_t *source,
                      const uint8_t *destination,
                      const uint32_t destination_size,
                      const uint32_t first_bit_index,
                      const uint32_t last_bit_index,
                      random_access_mode mode,
                      internal::inflate_stateful_operation &operation) -> uint32_t {
    uint32_t start_bit_offset = get_start_bit_offset(first_bit_index);
    uint32_t end_bit_offset   = get_end_bit_offset(last_bit_index);
    uint32_t byte_block_size  = get_byte_block_size(first_bit_index, last_bit_index);
    uint32_t bytes_offset     = get_bytes_offset(first_bit_index);

    operation.enable_random_access(mode);
    operation.set_start_bit_offset(start_bit_offset);
    operation.set_end_bit_offset(end_bit_offset);

    auto result = execute(operation,
                          source + bytes_offset,
                          source + bytes_offset + byte_block_size,
                          destination,
                          destination + destination_size);

    uint32_t result_size = 0;
    result.handle([&result_size](uint32_t value) -> void {
                      result_size = value;
                  },
                  [](uint32_t status) -> void {
                      util::handle_status(status);
                  });

    return result_size;
}

void read_header(const uint8_t *source,
                 const uint8_t *destination,
                 const uint32_t destination_size,
                 internal::index *index_array,
                 internal::inflate_stateful_operation &operation) {
    auto first_bit_index = index_array[0].bit_offset;
    auto last_bit_index  = index_array[1].bit_offset;

    decompress_block(source,
                     destination,
                     destination_size,
                     first_bit_index,
                     last_bit_index,
                     util::header,
                     operation);
}

void read_mini_block(const uint8_t *source,
                     const uint8_t *destination,
                     const uint32_t destination_size,
                     const uint32_t mini_block_index,
                     const internal::index *index_array,
                     internal::inflate_stateful_operation &operation) {
    auto index           = mini_block_index + 1;
    auto first_bit_index = index_array[index].bit_offset;
    auto last_bit_index  = index_array[index + 1].bit_offset;

    decompress_block(source,
                     destination,
                     destination_size,
                     first_bit_index,
                     last_bit_index,
                     util::mini_block,
                     operation);
}

} // namespace qpl::util
