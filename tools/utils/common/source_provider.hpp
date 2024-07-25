/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TEST_SOURCE_PROVIDER_HPP
#define QPL_TEST_SOURCE_PROVIDER_HPP

/* Must provide with test_data */

#include <stdexcept>
#include <vector>

#include "util.hpp"

#define PRLE_OCTA_GROUP_SIZE 8U

#define PRLE_COUNT_FIRST_BYTE_BITS 6U

#define PRLE_COUNT_FOLLOWING_BYTE_BITS 7U

#define MASK_SEVEN_LOW_BITS 0x7F
#define MASK_SIX_LOW_BITS   MASK_SEVEN_LOW_BITS >> 1

#define BIT_BUF_LEN      (sizeof(uint64_t) * BYTE_BIT_LENGTH)
#define BIT_BUF_LEN_HALF (BIT_BUF_LEN >> 1)

namespace qpl::test {
class source_provider {
public:
    source_provider(uint32_t number_of_elements, uint32_t bit_width, uint32_t seed,
                    qpl_parser parser = qpl_p_le_packed_array)
        : m_number_of_elements(number_of_elements), m_bit_width(bit_width), m_seed(seed), m_parser(parser) {
        m_byte_length = sizeof(uint8_t) * m_number_of_elements;
    }

    auto get_source() -> std::vector<uint8_t>;

    auto get_counter_source_expand_rle(uint16_t prologue = 0U) -> std::vector<uint8_t>;

    auto get_count_expand_rle_value() const -> uint32_t;

private:
    enum class prle_encoding_t { parquet, run_length_encoding };

    auto generate_expand_rle_prle_stream() -> std::vector<uint8_t>;

    static auto get_prle_header_bytes_size(uint32_t count) -> uint32_t;

    auto generate_prle_stream() -> std::vector<uint8_t>;

    static void store_prle_header(std::vector<uint8_t>::iterator& source_it, uint32_t count,
                                  prle_encoding_t prle_storage);

    void store_rle(std::vector<uint8_t>::iterator& source_it, uint32_t count, uint32_t element) const;

    void store_parquet_group(std::vector<uint8_t>::iterator& begin_it, std::vector<uint8_t>::iterator end_it,
                             uint32_t count, const std::vector<uint32_t>& elements) const;

    static auto bit_reverse32(uint32_t input) -> uint32_t;

    uint32_t   m_seed;
    uint32_t   m_number_of_elements;
    uint32_t   m_count_number_expand_rle = 0;
    uint32_t   m_bit_width;
    uint32_t   m_byte_length;
    qpl_parser m_parser;
};

auto generate_mask(uint32_t number_of_elements) -> std::vector<uint8_t>;
} // namespace qpl::test

#endif //QPL_TEST_SOURCE_PROVIDER_HPP
