/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/cpp_api/operations/analytics/rle_burst_operation.hpp"
#include "qpl/cpp_api/util/qpl_util.hpp"
#include "analytics/rle_burst.hpp"
#include "util/memory.hpp"
#include "util/checkers.hpp"

namespace qpl {
namespace internal {

constexpr uint32_t accumulate_counters_bit_width = 32u;
constexpr uint32_t minimal_number_of_elements_with_accumulate_counters = 2u;

static inline auto check_counters_bit_width(uint32_t bit_width) -> bool {
    switch(bit_width) {
        case 8:
            return true;
        case 16:
            return true;
        case 32:
            return true;
        default:
            return false;
    }
}

template <execution_path path>
auto validate_operation(rle_burst_operation &operation) -> uint32_t {
    using namespace qpl::ml;

    if (bad_argument::buffers_overlap(operation.source_,
                                      operation.source_size_,
                                      operation.destination_,
                                      operation.destination_size_)) {
        return status_list::buffers_overlap;
    }

    if (bad_argument::buffers_overlap(operation.source_,
                                      operation.source_size_,
                                      operation.mask_,
                                      operation.mask_byte_length_)) {
        return status_list::buffers_overlap;
    }

    if (bad_argument::buffers_overlap(operation.mask_,
                                      operation.mask_byte_length_,
                                      operation.destination_,
                                      operation.destination_size_)) {
        return status_list::buffers_overlap;
    }

    if (operation.input_vector_bit_width_ < 1 || operation.input_vector_bit_width_ > 32) {
        return status_list::bit_width_error;
    }

    if constexpr(path == software) {
        if (operation.is_decompression_enabled_) {
            return status_list::not_supported_err;
        }
    }

    if (operation.destination_size_ == 0) {
        return status_list::destination_is_short_error;
    }

    if (operation.mask_byte_length_ == 0) {
        return status_list::source_2_is_short_error;
    }

    if (!operation.is_decompression_enabled_) {
        uint32_t counters_bit_width = operation.counter_bit_width_;

        if (operation.parser_ == parsers::parquet_rle && !operation.is_decompression_enabled_) {
            counters_bit_width = *const_cast<uint8_t *>(operation.mask_);
        }

        if (!check_counters_bit_width(counters_bit_width)) {
            return status_list::bit_width_error;
        }

        auto symbols_number_of_elements = operation.number_of_input_elements_;

        if (accumulate_counters_bit_width == counters_bit_width) {
            // When counters are accumulative, there should be at least two elements
            if (symbols_number_of_elements < minimal_number_of_elements_with_accumulate_counters) {
                return status_list::size_error;
            }

            symbols_number_of_elements--;
        }

        auto expected_symbols_bits_length = symbols_number_of_elements *
                                            operation.input_vector_bit_width_;
        auto actual_symbols_bits_length   = byte_bit_length * operation.source_size_;

        if (expected_symbols_bits_length > actual_symbols_bits_length) {
            return status_list::source_is_short_error;
        }

        if (operation.parser_ != parsers::parquet_rle && !operation.is_decompression_enabled_) {
            // Check if we can read the 1st byte, and the src1 is accumulative counters
            if (accumulate_counters_bit_width == counters_bit_width) {

                uint32_t first_count_value = *(const_cast<uint32_t *>(
                                               reinterpret_cast<const uint32_t *>(operation.mask_)));

                if (0u != first_count_value) {
                    return status_list::invalid_rle_count;
                }
            }

            auto expected_counters_byte_length = ml::util::bit_to_byte(operation.number_of_input_elements_ *
                                                                       counters_bit_width);

            if (expected_counters_byte_length > operation.mask_byte_length_) {
                return status_list::source_2_is_short_error;
            }
        }
    }

    return status_list::ok;
}

template <execution_path path>
auto execute(rle_burst_operation &operation,
             uint8_t *source_buffer_ptr,
             size_t source_buffer_size,
             uint8_t *dest_buffer_ptr,
             size_t dest_buffer_size,
             uint8_t *symbol_buffer_ptr,
             size_t symbol_buffer_size,
             int32_t numa_id) -> execution_result<uint32_t, sync> {
    using namespace qpl::ml;

    if constexpr(path == software) {
        if (operation.is_decompression_enabled_) {
            return execution_result<uint32_t, sync>(ml::status_list::not_supported_err, 0);
        }
    }

    auto status = validate_operation<path>(operation);

    if (status != status_list::ok) {
        return execution_result<uint32_t, sync>(status, 0);
    }

    uint32_t counters_bit_width = operation.counter_bit_width_;

    if (!operation.is_decompression_enabled_) {

        if (operation.parser_ == parsers::parquet_rle && !operation.is_decompression_enabled_) {
            counters_bit_width = *const_cast<uint8_t *>(operation.mask_);
        }
    }

    constexpr auto actual_path = static_cast<ml::execution_path_t>(path);

    auto input_format         = qpl::util::parser_to_ml_parser(operation.parser_);
    auto out_bit_width_format = qpl::util::integer_to_output_format(operation.output_vector_bit_width_);

    auto output_stream_format  = analytics::stream_format_t::le_format;
    auto counter_stream_format = input_format == analytics::stream_format_t::prle_format
                                 ? analytics::stream_format_t::prle_format
                                 : analytics::stream_format_t::le_format;
    auto symbol_stream_format  = input_format == analytics::stream_format_t::prle_format
                                 ? analytics::stream_format_t::le_format
                                 : input_format;

    if constexpr(path != hardware) {
        ml::util::set_zeros(dest_buffer_ptr, dest_buffer_size);
        ml::util::set_zeros(symbol_buffer_ptr, symbol_buffer_size);
    }

    auto *symbols_begin  = const_cast<uint8_t *>(operation.source_);
    auto *symbols_end    = const_cast<uint8_t *>(operation.source_ + operation.source_size_);
    auto *dst_begin      = const_cast<uint8_t *>(operation.destination_);
    auto *dst_end        = const_cast<uint8_t *>(operation.destination_ + operation.destination_size_);
    auto *counters_begin = const_cast<uint8_t *>(operation.mask_);
    auto *counters_end   = const_cast<uint8_t *>(operation.mask_ + operation.mask_byte_length_);

    auto symbol_stream = analytics::input_stream_t::builder(symbols_begin, symbols_end)
            .element_count(operation.number_of_input_elements_)
            .omit_checksums(true)
            .omit_aggregates(true)
            .stream_format(symbol_stream_format, operation.input_vector_bit_width_)
            .build<actual_path>();

    auto counter_stream = analytics::input_stream_t::builder(counters_begin, counters_end)
            .element_count(operation.number_of_input_elements_)
            .omit_checksums(true)
            .omit_aggregates(true)
            .compressed(operation.is_decompression_enabled_)
            .stream_format(counter_stream_format, counters_bit_width)
            .build<actual_path>();

    auto output_stream = analytics::output_stream_t<analytics::array_stream>::builder(dst_begin, dst_end)
            .stream_format(output_stream_format)
            .bit_format(out_bit_width_format, operation.input_vector_bit_width_)
            .nominal(operation.input_vector_bit_width_ == bit_bit_length)
            .build<actual_path>();

    limited_buffer_t unpack_buffer(source_buffer_ptr,
                                   source_buffer_ptr + source_buffer_size,
                                   counters_bit_width);
    limited_buffer_t symbol_buffer(symbol_buffer_ptr,
                                   symbol_buffer_ptr + symbol_buffer_size,
                                   ml::util::bit_width_to_bits(operation.input_vector_bit_width_));
    limited_buffer_t output_buffer(dest_buffer_ptr,
                                   dest_buffer_ptr + dest_buffer_size,
                                   bit_bit_length);

    auto result = analytics::call_rle_burst<actual_path>(counter_stream,
                                                         symbol_stream,
                                                         output_stream,
                                                         unpack_buffer,
                                                         symbol_buffer,
                                                         output_buffer,
                                                         numa_id);

    auto output_elements_count = qpl::util::get_output_elements_as_is(&result,
                                                                      out_bit_width_format,
                                                                      operation.input_vector_bit_width_,
                                                                      operation.output_vector_bit_width_);

    return execution_result<uint32_t, sync>(result.status_code_, output_elements_count);
}

template
auto execute<execution_path::software>(rle_burst_operation &operation,
                                       uint8_t *source_buffer_ptr,
                                       size_t source_buffer_size,
                                       uint8_t *dest_buffer_ptr,
                                       size_t dest_buffer_size,
                                       uint8_t *mask_buffer_ptr,
                                       size_t mask_buffer_size,
                                       int32_t numa_id) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::hardware>(rle_burst_operation &operation,
                                       uint8_t *source_buffer_ptr,
                                       size_t source_buffer_size,
                                       uint8_t *dest_buffer_ptr,
                                       size_t dest_buffer_size,
                                       uint8_t *mask_buffer_ptr,
                                       size_t mask_buffer_size,
                                       int32_t numa_id) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::auto_detect>(rle_burst_operation &operation,
                                          uint8_t *source_buffer_ptr,
                                          size_t source_buffer_size,
                                          uint8_t *dest_buffer_ptr,
                                          size_t dest_buffer_size,
                                          uint8_t *mask_buffer_ptr,
                                          size_t mask_buffer_size,
                                          int32_t numa_id) -> execution_result<uint32_t, sync>;

}

void rle_burst_operation::reset_mask(const uint8_t *mask, size_t mask_byte_length) noexcept {
    mask_             = mask;
    mask_byte_length_ = mask_byte_length;
}

void rle_burst_operation::set_job_buffer(uint8_t * /* buffer */) noexcept {
    // will be removed after ML introduction
}

auto rle_burst_operation::rle_burst_operation_builder::counter_bit_width(uint32_t value) -> rle_burst_operation::rle_burst_operation_builder & {
    parent_builder::operation_.counter_bit_width_ = value;

    return *this;
}
}
