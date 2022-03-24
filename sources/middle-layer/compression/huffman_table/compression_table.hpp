/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_MIDDLE_LAYER_COMPRESSION_COMPRESSION_TABLE_HPP
#define QPL_MIDDLE_LAYER_COMPRESSION_COMPRESSION_TABLE_HPP

#include <cstdint>

#include "igzip_lib.h"

namespace qpl::ml::compression {
constexpr uint32_t literals_table_size = 257u;
constexpr uint32_t matches_table_size = 29u;
constexpr uint32_t literals_matches_table_size = literals_table_size + matches_table_size;
constexpr uint32_t offsets_table_size = 30u;
constexpr uint32_t huffman_code_length_mask = 0xF;
constexpr uint32_t huffman_code_length_offset = 15u;
constexpr uint32_t length_mask = huffman_code_length_mask << huffman_code_length_offset;
constexpr uint32_t huffman_code_mask = (1u << huffman_code_length_offset) - 1;
constexpr uint32_t deflate_header_size = 218u;

/**
 * @brief Structure that holds Huffman codes for compression
 *
 * There are two different Huffman tables:
 *  One for literals and match lengths
 *  One for offsets
 *
 * Both of them have the same format:
 *  Bits [14:0] - code itself
 *  Bits [18:15] - code length
 *
 * Code is not bit-reversed, stored in LE
 */
struct sw_compression_huffman_table {
    uint32_t literals_matches[literals_matches_table_size];  /**< Huffman table for literals and match lengths */
    uint32_t offsets[offsets_table_size];                    /**< Huffman table for offsets */
};

inline auto get_ll_code(const sw_compression_huffman_table *table, uint32_t index) -> uint16_t {
    auto code_value = static_cast<uint16_t>(table->literals_matches[index] & huffman_code_mask);
    return code_value;
}

inline void write_ll_code(sw_compression_huffman_table *table, uint32_t index, uint16_t code) {
    uint32_t literal_length_value = static_cast<uint32_t>(code) & huffman_code_mask;

    table->literals_matches[index] |= literal_length_value;
}

inline void write_ll_code_length(sw_compression_huffman_table *table, uint32_t index, uint8_t code_length) {
    uint32_t code_length_value = static_cast<uint32_t>(code_length) & huffman_code_length_mask;
    table->literals_matches[index] |= (code_length_value << huffman_code_length_offset);
}

inline auto get_ll_code_length(const sw_compression_huffman_table *table, uint32_t index) -> uint8_t {
    auto ll_code = table->literals_matches[index];
    auto code_length = static_cast<uint8_t>((ll_code & length_mask) >> huffman_code_length_offset);
    return code_length;
}

inline auto get_offset_code(const sw_compression_huffman_table *table, uint32_t index) -> uint16_t {
    auto code_value = static_cast<uint16_t>(table->offsets[index] & huffman_code_mask);
    return code_value;
}

inline auto get_offset_code_length(const sw_compression_huffman_table *table, uint32_t index) -> uint8_t {
    auto offset_code = table->offsets[index];
    auto code_length = static_cast<uint8_t>((offset_code & length_mask) >> huffman_code_length_offset);
    return code_length;
}

constexpr uint32_t hw_compression_huffman_table_size = 1u;

/**
 * @brief Structure that represents hardware compression table
 * This is just a stab and is not used anywhere yet
 */
struct hw_compression_huffman_table {
    uint8_t data[hw_compression_huffman_table_size];
};

/**
 * @brief Structure that holds information about deflate header
 */
struct deflate_header {
    uint32_t header_bit_size;            /**< Deflate header bit size */
    uint8_t  data[deflate_header_size];  /**< Deflate header content */
};

struct qpl_triplet {
    uint8_t character;
    uint8_t code_length;
    uint16_t code;
};

class compression_huffman_table {
public:
    compression_huffman_table(uint8_t *sw_table_ptr,
                              uint8_t *isal_table_ptr,
                              uint8_t *hw_table_ptr,
                              uint8_t *deflate_header_ptr) noexcept;

    auto get_deflate_header_data() noexcept -> uint8_t *;
    auto get_sw_compression_table() noexcept -> sw_compression_huffman_table *;
    auto get_isal_compression_table() noexcept -> isal_hufftables *;
    auto get_hw_compression_table() noexcept -> hw_compression_huffman_table *;
    auto get_deflate_header_bit_size() noexcept -> uint32_t;

    void set_deflate_header_bit_size(uint32_t value) noexcept ;

    void enable_sw_compression_table() noexcept;
    void enable_hw_compression_table() noexcept;
    void enable_deflate_header() noexcept;
    void make_huffman_only() noexcept;

    auto is_sw_compression_table_used() const noexcept -> bool;
    auto is_hw_compression_table_used() const noexcept -> bool;
    auto is_deflate_header_used() const noexcept -> bool;
    auto is_huffman_only() const noexcept -> bool;

private:
    hw_compression_huffman_table *hw_compression_table_ptr_;
    sw_compression_huffman_table *sw_compression_table_ptr_;
    isal_hufftables              *isal_compression_table_ptr_;
    deflate_header               *deflate_header_ptr_;

    bool sw_compression_table_flag_;
    bool hw_compression_table_flag_;
    bool deflate_header_flag_;
    bool huffman_only_flag_;
};
}

#endif // QPL_MIDDLE_LAYER_COMPRESSION_COMPRESSION_TABLE_HPP
