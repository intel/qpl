/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/qpl.h"

#include "analytics/extract.hpp"
#include "qpl/cpp_api/operations/analytics/extract_operation.hpp"
#include "qpl/cpp_api/util/qpl_util.hpp"
#include "util/checkers.hpp"

namespace qpl {
namespace internal {

static inline auto get_output_bit_width(uint32_t input_bit_width, uint32_t output_bit_width) -> uint32_t {
    switch (output_bit_width) {
        case 1: {
            return input_bit_width;
        }
        case 8: {
            return output_bit_width;
        }
        case 16: {
            return output_bit_width;
        }
        case 32: {
            return output_bit_width;
        }
        default: {
            return input_bit_width;
        }
    }
}

template <execution_path path>
auto validate_operation(extract_operation &operation) -> uint32_t {
    using namespace qpl::ml;

    if (bad_argument::buffers_overlap(operation.source_,
                                      operation.source_size_,
                                      operation.destination_,
                                      operation.destination_size_)) {
        return status_list::buffers_overlap;
    }

    uint32_t input_bit_width = operation.input_vector_bit_width_;

    if (operation.parser_ == parsers::parquet_rle && !operation.is_decompression_enabled_) {
        input_bit_width = *const_cast<uint8_t *>(operation.source_);
    }

    if (input_bit_width < 1 || input_bit_width > 32) {
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

    const size_t output_elements        = static_cast<size_t>(operation.upper_index_ - operation.lower_index_ + 1);
    const uint32_t output_bit_width     = get_output_bit_width(input_bit_width,
                                                               operation.output_vector_bit_width_);

    const size_t output_elements_size = ml::util::bit_to_byte(output_elements * output_bit_width);

    if (output_elements_size > operation.destination_size_) {
        return status_list::destination_is_short_error;
    }

    size_t number_of_input_elements = static_cast<size_t>(operation.number_of_input_elements_);

    if (operation.parser_ != parsers::parquet_rle && !operation.is_decompression_enabled_) {
        if (ml::util::bit_to_byte(number_of_input_elements * input_bit_width)
                > operation.source_size_) {
            return status_list::source_is_short_error;
        }
    }

    const auto out_bit_width_format = qpl::util::integer_to_output_format(operation.output_vector_bit_width_);

    if ((analytics::output_bit_width_format_t::same_as_input != out_bit_width_format) &&
        (1u == input_bit_width))
    {
        uint32_t max_possible_index = std::numeric_limits<uint32_t>::max();
        if (analytics::output_bit_width_format_t::bits_32 != out_bit_width_format)

        {
            max_possible_index = (analytics::output_bit_width_format_t::bits_8 == out_bit_width_format) ? 0xFF : 0xFFFF;
        }
        if ((static_cast<size_t>(operation.initial_output_index_) + number_of_input_elements - 1u)
                > static_cast<size_t>(max_possible_index))
        {
            return status_list::output_overflow_error;
        }
    }

    return status_list::ok;
}

template <execution_path path>
auto execute(extract_operation &operation, int32_t numa_id) -> execution_result<uint32_t, sync> {
    using namespace qpl::ml;

    uint32_t input_bit_width = operation.input_vector_bit_width_;

    if (operation.parser_ == parsers::parquet_rle && !operation.is_decompression_enabled_) {
        input_bit_width = *const_cast<uint8_t *>(operation.source_);
    }

    auto status = validate_operation<path>(operation);
    if (status != status_list::ok) {
        return execution_result<uint32_t, sync>(status, 0);
    }

    const auto number_of_input_elements = operation.number_of_input_elements_ ?
            operation.number_of_input_elements_ :
            (operation.source_size_ * byte_bits_size) / input_bit_width;

    if constexpr(path == software) {
        if (operation.is_decompression_enabled_) {
            return execution_result<uint32_t, sync>(ml::status_list::not_supported_err, 0);
        }
    }

    if (operation.lower_index_ > operation.upper_index_) {
        return execution_result<uint32_t, sync>(status_list::ok, 0);
    }

    constexpr auto actual_path = static_cast<execution_path_t>(path);

    const auto input_stream_format = qpl::util::parser_to_ml_parser(operation.parser_);
    const auto out_bit_width_format = qpl::util::integer_to_output_format(operation.output_vector_bit_width_);
    const auto output_stream_format = analytics::stream_format_t::le_format;

    std::array<uint8_t, 4_kb * sizeof(uint32_t)> buffer{};

    auto *src_begin = const_cast<uint8_t *>(operation.source_);
    auto *src_end   = const_cast<uint8_t *>(operation.source_ + operation.source_size_);
    auto *dst_begin = const_cast<uint8_t *>(operation.destination_);
    auto *dst_end   = const_cast<uint8_t *>(operation.destination_ + operation.destination_size_);
    
    auto input_stream = analytics::input_stream_t::builder(src_begin, src_end)
            .element_count(number_of_input_elements)
            .omit_checksums(true)
            .omit_aggregates(true)
            .compressed(operation.is_decompression_enabled_)
            .stream_format(input_stream_format, input_bit_width)
            .build<actual_path>();

    if (input_stream_format == ml::analytics::stream_format_t::prle_format)
    {
        auto prle_bit_width = input_stream.bit_width();
        if ((prle_bit_width < 1u) || (prle_bit_width > 32u))
        {
            return execution_result<uint32_t, sync>(QPL_STS_BIT_WIDTH_ERR, 0);
        }
    }

    auto output_stream = analytics::output_stream_t<analytics::array_stream>::builder(dst_begin, dst_end)
            .stream_format(output_stream_format)
            .bit_format(out_bit_width_format, input_bit_width)
            .nominal(operation.input_vector_bit_width_ == bit_bit_length)
            .initial_output_index(operation.initial_output_index_)
            .build<actual_path>();

    limited_buffer_t temporary_buffer(buffer.data(), buffer.data() + buffer.size(), input_stream.bit_width());

    auto extract_result = analytics::call_extract<actual_path>(input_stream,
                                                               output_stream,
                                                               operation.lower_index_,
                                                               operation.upper_index_,
                                                               temporary_buffer,
                                                               numa_id);

    auto output_elements_count = qpl::util::get_output_elements_as_is(&extract_result,
                                                                      out_bit_width_format,
                                                                      operation.input_vector_bit_width_,
                                                                      operation.output_vector_bit_width_);

    return execution_result<uint32_t, sync>(extract_result.status_code_, output_elements_count);
}

template
auto execute<execution_path::software>(extract_operation &operation,
                                       int32_t numa_id) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::hardware>(extract_operation &operation,
                                       int32_t numa_id) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::auto_detect>(extract_operation &operation,
                                          int32_t numa_id) -> execution_result<uint32_t, sync>;

} // namespace qpl::internal

void extract_operation::set_job_buffer(uint8_t * /* buffer */) noexcept {
    // will be removed after ML introduction
}

auto extract_operation::extract_operation_builder::lower_index(uint32_t value) -> extract_operation_builder & {
    parent_builder::operation_.lower_index_ = value;

    return *this;
}

auto extract_operation::extract_operation_builder::upper_index(uint32_t value) -> extract_operation_builder & {
    parent_builder::operation_.upper_index_ = value;

    return *this;
}

} // namespace qpl
