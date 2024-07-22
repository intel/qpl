/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "bit_buffer.hpp"

#include <algorithm>

#include "bit_reverse.hpp"
#include "util/util.hpp"

namespace qpl::ml {

void bit_reader::load_buffer(uint8_t number_of_bits) noexcept {
    if (!is_big_endian_ && bits_in_buffer_ == 0 && static_cast<uint32_t>(source_end_ptr_ - current_source_ptr_) >= 8 &&
        number_of_bits == 64) {
        // Try to load 64 bits (fast load) for LE
        buffer_ = *(uint64_t*)current_source_ptr_;
        current_source_ptr_ += 8;
        bits_in_buffer_ = 64;
    } else {
        // Try to load 16 bits for BE or 8 bits for LE
        const uint8_t load_bit_size  = is_big_endian_ ? 16U : 8U;
        const uint8_t load_byte_size = is_big_endian_ ? 2U : 1U;

        while (bits_in_buffer_ <= (64U - load_bit_size) && number_of_bits > 0) {
            if (static_cast<uint32_t>(source_end_ptr_ - current_source_ptr_) <= 0) {
                is_overflowed_ = true;
                return;
            } else {
                // Cast the next load to uint64_t. The next load for LE is the next 16 bits,
                // while the next load for BE is the next 8 bits. Also reverse the bits for BE.
                uint64_t next_load = static_cast<uint64_t>(*current_source_ptr_);
                if (is_big_endian_) {
                    uint16_t next_word = *((uint16_t*)current_source_ptr_);
                    next_word          = reverse_bits(next_word);
                    next_load          = static_cast<uint64_t>(next_word);
                }

                if (static_cast<uint32_t>(source_end_ptr_ - current_source_ptr_) == load_byte_size) {
                    // If this is the last load, build mask only for valid bytes and load them
                    auto bit_count    = (last_bits_offset_ == 0) ? load_bit_size : last_bits_offset_;
                    auto bits_to_load = next_load & util::build_mask<uint64_t>(bit_count);
                    buffer_ |= (bits_to_load << bits_in_buffer_);
                    bits_in_buffer_ += bit_count;
                    current_source_ptr_ += load_byte_size;
                    number_of_bits -= bit_count;

                    if (number_of_bits > 0) { is_overflowed_ = true; }
                    return;
                } else {
                    // Load 16 bits (for BE) or 8 bits (for LE) at a time
                    buffer_ |= (next_load << bits_in_buffer_);
                    bits_in_buffer_ += load_bit_size;
                    current_source_ptr_ += load_byte_size;
                    number_of_bits -= load_bit_size;
                }
            }
        }
    }
}

/**
 * @brief Get the next specified number of bits without removing them from the buffer
*/
auto bit_reader::peek_bits(uint8_t number_of_bits) noexcept -> uint16_t {
    if (number_of_bits > bits_in_buffer_) {
        uint32_t bits_still_needed = std::max(8, number_of_bits - bits_in_buffer_);
        bits_still_needed          = util::bit_to_byte(bits_still_needed) * byte_bits_size;
        load_buffer(bits_still_needed);
    }

    uint16_t result = 0U;

    auto mask = util::build_mask<uint16_t>(number_of_bits);

    result = static_cast<uint16_t>(buffer_) & mask;

    return result;
}

/**
 * @brief Remove the next specified number of bits from the buffer
*/
void bit_reader::shift_bits(uint8_t number_of_bits) noexcept {
    if (bits_in_buffer_ > number_of_bits) {
        buffer_ >>= number_of_bits;
        bits_in_buffer_ -= number_of_bits;
    } else {
        buffer_         = 0;
        bits_in_buffer_ = 0;
    }
}

auto bit_reader::get_total_bytes_read() noexcept -> uint32_t {
    return static_cast<uint32_t>(current_source_ptr_ - source_begin_ptr_);
}

auto bit_reader::get_buffer_bit_count() const noexcept -> uint32_t {
    return bits_in_buffer_;
}

void bit_reader::set_last_bits_offset(uint8_t value) noexcept {
    last_bits_offset_ = value;
}

auto bit_reader::is_overflowed() const noexcept -> bool {
    return is_overflowed_;
}

auto bit_reader::is_source_end() noexcept -> bool {
    return ((current_source_ptr_ >= source_end_ptr_) && (0 == bits_in_buffer_));
}

void bit_reader::set_big_endian(bool is_big_endian) noexcept {
    is_big_endian_ = is_big_endian;
}
} // namespace qpl::ml
