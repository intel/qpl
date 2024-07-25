/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_PRLE_GENERATOR_HPP
#define QPL_PRLE_GENERATOR_HPP

#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>

constexpr uint32_t parquet_group_size = 8;

namespace qpl::test {

enum class prle_encoding_t { run_length_encoding, parquet };

struct rle_element_t {
    uint32_t repeat_count  = 0;
    uint32_t element_value = 0;
    uint32_t bit_width     = 0;
};

struct parquet_element_t {
    std::array<uint32_t, 0> parquet_group = {};
    uint32_t                bit_width     = 0;
    uint32_t                repeat_count  = 0;
};

auto get_prle_header_size_bytes(uint32_t count) -> uint32_t;

auto create_prle_header(prle_encoding_t prle_encoding, uint32_t prle_count) -> std::vector<uint8_t>;

auto create_rle_group(rle_element_t rle_element) -> std::vector<uint8_t>;

auto create_parquet_group(parquet_element_t parquet_element) -> std::vector<uint8_t>;
} // namespace qpl::test
#endif // QPL_PRLE_GENERATOR_HPP
