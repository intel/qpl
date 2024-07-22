/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "zlib_decorator.hpp"

#include "common/bit_reverse.hpp"
#include "compression/deflate/deflate.hpp"
#include "compression/deflate/streams/hw_deflate_state.hpp"
#include "compression/deflate/streams/sw_deflate_state.hpp"
#include "compression/inflate/inflate.hpp"
#include "compression/inflate/inflate_state.hpp"
#include "compression/verification/verify.hpp"
#include "simple_memory_ops.hpp"

namespace qpl::ml::compression {

namespace zlib_flags {
constexpr uint8_t dictionary        = 0x20U;
constexpr uint8_t compression_level = 0x40U | 0x80U;
} // namespace zlib_flags

namespace zlib_fields {
constexpr uint8_t  CM_ZLIB_DEFAULT_VALUE   = 8U;
constexpr uint32_t ZLIB_MIN_HEADER_SIZE    = 2U;
constexpr uint32_t ZLIB_INFO_OFFSET        = 4U;
constexpr uint32_t ZLIB_DICTIONARY_ID_SIZE = 4U;
} // namespace zlib_fields

struct wrapper_result_t {
    uint32_t status_code_;
    uint32_t bytes_done_;
};

auto zlib_decorator::read_header(const uint8_t* stream_ptr, uint32_t stream_size, zlib_header& header) noexcept
        -> qpl_ml_status {
    const uint8_t* stream_end_ptr = stream_ptr + stream_size;

    if (stream_size < zlib_fields::ZLIB_MIN_HEADER_SIZE) { return status_list::input_too_small; }

    header.byte_size = zlib_fields::ZLIB_MIN_HEADER_SIZE;

    const uint8_t compression_method_and_flag = *stream_ptr++;
    const uint8_t compression_method          = compression_method_and_flag & 0xf;
    const uint8_t compression_info            = compression_method_and_flag >> zlib_fields::ZLIB_INFO_OFFSET;

    if (zlib_fields::CM_ZLIB_DEFAULT_VALUE != compression_method) { return status_list::gzip_header_error; }

    const uint8_t flags = *stream_ptr++;

    if ((256 * compression_method_and_flag + flags) % 31 != 0) { return status_list::gzip_header_error; }

    header.compression_info = compression_info;

    header.flags = flags;

    header.dictionary_flag = (flags & zlib_flags::dictionary) != 0;

    if (header.dictionary_flag) {
        if (stream_ptr + zlib_fields::ZLIB_DICTIONARY_ID_SIZE <= stream_end_ptr) {
            return status_list::input_too_small;
        }

        header.dictionary_id = *reinterpret_cast<const uint32_t*>(stream_ptr);
        stream_ptr += zlib_fields::ZLIB_DICTIONARY_ID_SIZE;
        header.byte_size += zlib_fields::ZLIB_DICTIONARY_ID_SIZE;
    } else {
        header.dictionary_id = 0U;
    }

    header.compression_level = (flags & zlib_flags::compression_level);

    return status_list::ok;
}

/**
 * @brief Function to write zlib header.
 * Header structure (according to RFC1950):
 *   CMF (Compression Method and flags)
 *     bits 0-3  CM     Compression method
 *     bits 4-7  CINFO  Compression info
 *   FLAGS
 *     bits 0-4  FCHECK Checksum for CMF and FLAGS
 *     bit  5    FDICT  Preset dictionary
 *     bits 6-7  FLEVEL Compression level
 *
 * @todo Change to have window size as an input argument vs hardcoding.
*/
static inline auto own_write_header(uint8_t* const destination_ptr, const uint32_t size) noexcept -> wrapper_result_t {
    wrapper_result_t result {};

    if (size < zlib_sizes::zlib_header_size) {
        result.status_code_ = status_list::more_output_needed;
        return result;
    }

    core_sw::util::copy(default_zlib_header.data(), default_zlib_header.data() + zlib_sizes::zlib_header_size,
                        destination_ptr);

    result.status_code_ = status_list::ok;
    result.bytes_done_  = zlib_sizes::zlib_header_size;

    return result;
}

static inline auto own_swap_and_write_trailer(uint8_t* destination_ptr, const uint32_t size,
                                              const uint32_t adler32) noexcept -> wrapper_result_t {
    wrapper_result_t result {};

    if (size < zlib_sizes::zlib_trailer_size) {
        result.status_code_ = status_list::more_output_needed;
        return result;
    }

    uint32_t zlib_trailer = swap_bytes((adler32 & util::most_significant_16_bits) |
                                       ((adler32 & util::least_significant_16_bits) + 1) % util::adler32_mod);

    auto data_ptr = reinterpret_cast<uint8_t*>(&zlib_trailer);

    core_sw::util::copy(data_ptr, data_ptr + zlib_sizes::zlib_trailer_size, destination_ptr);

    result.status_code_ = status_list::ok;
    result.bytes_done_  = zlib_sizes::zlib_trailer_size;

    return result;
}

template <class F, class state_t, class... arguments>
auto zlib_decorator::unwrap(F function, state_t& state, arguments... args) noexcept
        -> decompression_operation_result_t {
    uint8_t* saved_output_ptr = state.get_output_data(); // state.get_output_buffer;
    uint32_t wrapper_bytes    = 0;

    decompression_operation_result_t result {};

    auto adler_value = state.get_crc(); // Adler saved inplace crc @todo fix

    if (state.is_first()) {
        zlib_header    header;
        uint8_t*       input_ptr  = state.get_input_data();
        const uint32_t input_size = state.get_input_size();

        auto status = read_header(input_ptr, input_size, header);

        if (status_list::ok != status) {
            result.status_code_ = status;

            return result;
        }

        if constexpr (std::is_same_v<state_t, inflate_state<execution_path_t::software>>) {
            if (header.dictionary_flag) {
                // If dictionary exists check if dictionary id match
                if (state.is_dictionary_available()) {
                    if (state.get_dictionary()->dictionary_id != header.dictionary_id) {
                        result.status_code_ = status_list::need_dictionary_error;
                        return result;
                    }
                } else {
                    // Dictionary is needed, but no actual dictionary provided, error
                    result.status_code_ = status_list::need_dictionary_error;
                    return result;
                }
            }
        }

        state.input(input_ptr + header.byte_size, input_ptr + input_size);
        wrapper_bytes = header.byte_size;
    }

    result = function(state, args...);

    if (result.status_code_) { return result; }

    adler_value = util::adler32(saved_output_ptr, result.output_bytes_, adler_value);

    state.crc_seed(adler_value);
    result.checksums_.crc32_ = adler_value;
    result.completed_bytes_ += wrapper_bytes;

    return result;
}

template <execution_path_t path>
using inflate_t = decltype(inflate<path, inflate_mode_t::inflate_default>)*;

template auto zlib_decorator::unwrap<inflate_t<execution_path_t::software>, inflate_state<execution_path_t::software>,
                                     end_processing_condition_t>(
        inflate_t<execution_path_t::software> function, inflate_state<execution_path_t::software>& state,
        end_processing_condition_t end_processing_condition) noexcept -> decompression_operation_result_t;

template auto zlib_decorator::unwrap<inflate_t<execution_path_t::hardware>, inflate_state<execution_path_t::hardware>,
                                     end_processing_condition_t>(
        inflate_t<execution_path_t::hardware> function, inflate_state<execution_path_t::hardware>& state,
        end_processing_condition_t end_processing_condition) noexcept -> decompression_operation_result_t;

/* ------ ZLIB WRAP ------ */

template <class F, class state_t>
auto zlib_decorator::wrap(F function, state_t& state, uint8_t* begin, const uint32_t current_in_size,
                          const uint32_t prev_adler32) noexcept -> compression_operation_result_t {
    compression_operation_result_t result {};

    auto data_ptr      = state.next_out();
    auto data_size     = state.avail_out();
    auto wrapper_bytes = 0U;

    if (state.is_first_chunk()) {
        auto wrapper_result = own_write_header(data_ptr, data_size);

        if (wrapper_result.status_code_) { return result; }

        wrapper_bytes = wrapper_result.bytes_done_;
        state.set_output_prologue(wrapper_bytes);
    }

    result = function(state, begin, current_in_size);

    // With qpl_path_auto, if execution on qpl_path_hardware writes header and then gets error in execution,
    // fallback to qpl_path_software will write header again before execution. In this case, compressed
    // stream will contain 2 copies of the headers. So, here we write the header,
    // but only make header bytes valid by adding them to result.output_bytes_ when result
    // status is OK. When execution on qpl_path_hardware returns error, result.output_bytes_ is not updated,
    // fallback to qpl_path_software will write the header at the same place as HW execution did and update
    // result.output_bytes_.
    // Note: since zlib headers are very small, writing header twice shouldn't have big performance impact
    if (!result.status_code_) { result.output_bytes_ += wrapper_bytes; }

    result.checksums_.adler32_ = qpl::ml::util::adler32(begin, current_in_size, prev_adler32);

    if (!result.status_code_ && state.is_last_chunk()) {
        auto wrapper_result = own_swap_and_write_trailer(data_ptr + result.output_bytes_,
                                                         data_size - result.output_bytes_, result.checksums_.adler32_);

        if (wrapper_result.status_code_) { return result; }

        result.output_bytes_ += wrapper_result.bytes_done_;
    }

    return result;
}

template <execution_path_t path>
using deflate_t = decltype(deflate<path, deflate_mode_t::deflate_default>)*;

template auto zlib_decorator::wrap<deflate_t<execution_path_t::software>, deflate_state<execution_path_t::software>>(
        deflate_t<execution_path_t::software> function, deflate_state<execution_path_t::software>& state,
        uint8_t* begin, const uint32_t current_in_size, const uint32_t prev_processed_size) noexcept
        -> compression_operation_result_t;

template auto zlib_decorator::wrap<deflate_t<execution_path_t::hardware>, deflate_state<execution_path_t::hardware>>(
        deflate_t<execution_path_t::hardware> function, deflate_state<execution_path_t::hardware>& state,
        uint8_t* begin, const uint32_t current_in_size, const uint32_t prev_processed_size) noexcept
        -> compression_operation_result_t;
} // namespace qpl::ml::compression
