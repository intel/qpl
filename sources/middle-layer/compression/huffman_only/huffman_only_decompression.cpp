/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "huffman_only.hpp"
#include "common/bit_reverse.hpp"
#include "util/util.hpp"
#include "util/descriptor_processing.hpp"
#include "simple_memory_ops.hpp"
#include "util/checksum.hpp"

namespace qpl::ml::compression {

static void restore_huffman_table(
        const qplc_huffman_table_flat_format &huffman_table,
        std::array<huffman_code, huffman_only_number_of_literals> &result_huffman_table) noexcept {
    // Main cycle
    for (uint32_t current_code_length = 1U; current_code_length < 16U; current_code_length++) {
        // Getting number of codes, first code, index of first literal and symbol with Huffman code length "i"
        const uint16_t number_of_codes   = huffman_table.number_of_codes[current_code_length - 1];
        const uint16_t first_code        = huffman_table.first_codes[current_code_length - 1];
        const uint16_t first_table_index = huffman_table.first_table_indexes[current_code_length - 1U];
        uint8_t        symbol            = huffman_table.index_to_char[first_table_index];

        if (0U == number_of_codes) {
            // We have no reason to continue this iteration
            continue;
        }

        // First iteration in outer scope
        result_huffman_table[symbol].code   = first_code;
        result_huffman_table[symbol].length = current_code_length;

        // Generate other codes and lengths
        for (uint32_t code_number = 1U; code_number < number_of_codes; code_number++) {
            symbol = huffman_table.index_to_char[first_table_index + code_number];

            result_huffman_table[symbol].code   = first_code + code_number;
            result_huffman_table[symbol].length = current_code_length;
        }
    }
}

static void build_lookup_table(
        const std::array<huffman_code, huffman_only_number_of_literals> &huffman_table,
        uint8_t *lookup_table_ptr) noexcept {
    // Main cycle
    for (uint64_t symbol = 0U; symbol < huffman_table.size(); symbol++) {
        const uint8_t  code_length             = huffman_table[symbol].length;
        const uint8_t  offset                  = 16U - code_length;
        const uint16_t code                    = reverse_bits(huffman_table[symbol].code, code_length);
        const uint16_t low_lookup_table_index  = 0U;
        const uint16_t high_lookup_table_index = util::build_mask<uint16_t>(offset);

        if (0U == code_length) {
            continue;
        }

        // Filling lookup table
        for (uint32_t i = low_lookup_table_index; i < high_lookup_table_index + 1U; i++) {
            const uint16_t symbol_position = (i << code_length) | code;

            lookup_table_ptr[symbol_position] = (uint8_t) symbol;
        }
    }
}

static auto perform_huffman_only_decompression(
        bit_reader &reader,
        uint8_t *destination_ptr,
        uint32_t destination_length,
        const uint8_t *lookup_table_ptr,
        const std::array<huffman_code, huffman_only_number_of_literals> &huffman_table,
        bool is_big_endian) noexcept -> decompression_operation_result_t {
    // Main cycle
    uint32_t current_symbol_index = 0U;

    decompression_operation_result_t result{};
    bool decode_next_symbol = true;

    result.status_code_ = status_list::ok;

    do {
        if (current_symbol_index >= destination_length) {
            if (!reader.is_source_end()) {
                result.status_code_ = status_list::more_output_needed;
            }
            break;
        }

        // Decode the next symbol
        // peek_bits will get the next bits without removing them from the bit buffer
        reader.set_big_endian(is_big_endian);
        const uint16_t next_bits = reader.peek_bits(huffman_code_bit_length);

        const uint8_t  symbol                     = lookup_table_ptr[next_bits];

        // Get the number of bits used to decode the symbol
        const uint8_t  current_symbol_code_length = huffman_table[symbol].length;

        // Remove the used bits from the bit buffer
        reader.shift_bits(current_symbol_code_length);

        // Writing symbol to output
        destination_ptr[current_symbol_index++] = symbol;

        decode_next_symbol = !reader.is_overflowed() ||
                             (reader.get_buffer_bit_count() > 0);
    } while (decode_next_symbol);

    result.output_bytes_    = current_symbol_index;

    return result;
}

template <>
auto decompress_huffman_only<execution_path_t::software>(
        huffman_only_decompression_state<execution_path_t::software> &decompression_state,
        decompression_huffman_table &decompression_table) noexcept -> decompression_operation_result_t {
    std::array<huffman_code, huffman_only_number_of_literals> restored_huffman_table;

    const auto *source_ptr = decompression_state.get_fields().current_source_ptr;
    const auto *source_end_ptr = source_ptr + decompression_state.get_fields().source_available;

    auto *destination_ptr = decompression_state.get_fields().current_destination_ptr;
    auto *destination_begin_ptr = decompression_state.get_fields().current_destination_ptr;
    auto available_out = decompression_state.get_fields().destination_available;

    const auto last_byte_valid_bits = decompression_state.get_fields().last_bits_offset;

    bit_reader reader(source_ptr, source_end_ptr);

    // Restore huffman table and build the lookup table
    restore_huffman_table(*decompression_table.get_sw_decompression_table(), restored_huffman_table);

    build_lookup_table(restored_huffman_table, decompression_state.get_lookup_table());

    decompression_operation_result_t result{};

    reader.set_last_bits_offset(last_byte_valid_bits);
    const bool is_big_endian = decompression_state.get_endianness() == endianness_t::big_endian;
    result = perform_huffman_only_decompression(reader,
                                                destination_ptr,
                                                available_out,
                                                decompression_state.get_lookup_table(),
                                                restored_huffman_table,
                                                is_big_endian);

    result.completed_bytes_ = reader.get_total_bytes_read();

    decompression_state.get_fields().crc_value = util::crc32_gzip(destination_begin_ptr,
                                                                  destination_begin_ptr + result.output_bytes_,
                                                                  decompression_state.get_fields().crc_value);

    decompression_state.get_fields().source_available   -= result.completed_bytes_;
    decompression_state.get_fields().current_source_ptr += result.completed_bytes_;
    decompression_state.get_fields().current_destination_ptr += result.output_bytes_;
    decompression_state.get_fields().destination_available   -= result.output_bytes_;


    return result;
}

/*
 * @brief Calculates CRC value while decompressing the compressed stream and compare with the reference CRC from compression
 * as compression verification
*/
template <>
auto verify_huffman_only<execution_path_t::software>(huffman_only_decompression_state<execution_path_t::software> &state,
                                                     decompression_huffman_table &decompression_table,
                                                     uint32_t required_crc) noexcept -> qpl_ml_status {
    uint32_t crc_value = state.get_fields().crc_value;

    // Restore huffman table and build the lookup table
    std::array<huffman_code, huffman_only_number_of_literals> restored_huffman_table;
    restore_huffman_table(*decompression_table.get_sw_decompression_table(), restored_huffman_table);
    build_lookup_table(restored_huffman_table, state.get_lookup_table());

    const uint8_t *lookup_table_ptr = state.get_lookup_table();

    const auto *source_ptr = state.get_fields().current_source_ptr;
    const auto *source_end_ptr = source_ptr + state.get_fields().source_available;
    bit_reader reader(source_ptr, source_end_ptr);
    const auto last_byte_valid_bits = state.get_fields().last_bits_offset;
    reader.set_last_bits_offset(last_byte_valid_bits);

    const bool is_big_endian = state.get_endianness() == endianness_t::big_endian;
    reader.set_big_endian(is_big_endian);

    bool decode_next_symbol = true;
    do {
        // Decode the next symbol
        const uint16_t next_bits = reader.peek_bits(huffman_code_bit_length);
        const uint8_t  symbol    = lookup_table_ptr[next_bits];

        // Get the number of bits used to decode the symbol
        const uint8_t  current_symbol_code_length = restored_huffman_table[symbol].length;

        // Remove the used bits from the bit buffer
        reader.shift_bits(current_symbol_code_length);

        // Instead of writing symbol to output, find the CRC of the symbol and update the overall CRC
        // @todo Add support for crc32 with iscsi polynomial, which should be used if QPL_FLAG_CRC32C is set
        crc_value = ~crc_value;
        crc_value = (crc_value >> 8) ^ qpl::ml::util::crc32_table_gzip_refl[(crc_value & 0x000000FF) ^ symbol];
        crc_value = ~crc_value;

        decode_next_symbol = !reader.is_overflowed() ||
                             (reader.get_buffer_bit_count() > 0);
    } while (decode_next_symbol);


    if (required_crc != crc_value) {
        return status_list::verify_error;
    } else {
        return status_list::ok;
    }
}

template <>
auto decompress_huffman_only<execution_path_t::hardware>(
        huffman_only_decompression_state<execution_path_t::hardware> &decompression_state,
        decompression_huffman_table &decompression_table) noexcept -> decompression_operation_result_t {
    auto completion_record = decompression_state.handler();
    auto descriptor        = decompression_state.decompress_table(decompression_table)
                                                .build_descriptor();

    auto result = util::process_descriptor<decompression_operation_result_t,
                                           util::execution_mode_t::sync>(descriptor, completion_record);

    if (result.status_code_ == status_list::ok) {
        result.completed_bytes_ = decompression_state.get_input_size();
    }

    return result;
}

}
