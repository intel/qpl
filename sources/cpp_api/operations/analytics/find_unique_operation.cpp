/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "analytics/find_unique.hpp"
#include "qpl/cpp_api/operations/analytics/find_unique_operation.hpp"
#include "qpl/cpp_api/util/qpl_util.hpp"
#include "util/checkers.hpp"

namespace qpl {

namespace internal {

template <execution_path path>
auto validate_operation(find_unique_operation &operation) -> uint32_t {
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

    size_t number_of_input_elements = static_cast<size_t>(operation.number_of_input_elements_);

    if (operation.parser_ != parsers::parquet_rle && !operation.is_decompression_enabled_) {
        if (ml::util::bit_to_byte(number_of_input_elements * input_bit_width)
                > operation.source_size_) {
            return status_list::source_is_short_error;
        }
    }

    const uint32_t drop_bits_count = operation.number_low_order_bits_ignored_
                                     + operation.number_high_order_bits_ignored_;
    if (input_bit_width <= drop_bits_count) {
        return status_list::drop_bits_overflow_error;
    }

    const uint32_t actual_bit_width = input_bit_width - drop_bits_count;

    if (actual_bit_width < 1 || actual_bit_width > 15) {
        return status_list::bit_width_error;
    }

    const uint32_t required_destination_bit_size  = 1u << actual_bit_width;
    const uint32_t required_destination_byte_size = ml::util::bit_to_byte(required_destination_bit_size);

    if (operation.destination_size_ < required_destination_byte_size) {
        return status_list::destination_is_short_error;
    }

    return status_list::ok;
}

template <execution_path path>
auto execute(find_unique_operation &operation, int32_t numa_id) -> execution_result<uint32_t, sync> {
    using namespace qpl::ml;

    uint32_t input_bit_width = operation.input_vector_bit_width_;

    if (operation.parser_ == parsers::parquet_rle && !operation.is_decompression_enabled_) {
        input_bit_width = *const_cast<uint8_t *>(operation.source_);
    }

    if constexpr(path == software) {
        if (operation.is_decompression_enabled_) {
            return execution_result<uint32_t, sync>(ml::status_list::not_supported_err, 0);
        }
    }

    auto status = validate_operation<path>(operation);
    if (status != status_list::ok) {
        return execution_result<uint32_t, sync>(status, 0);
    }

    const auto number_of_input_elements = operation.number_of_input_elements_ ?
            operation.number_of_input_elements_ :
            (operation.source_size_ * byte_bits_size) / input_bit_width;

    constexpr auto actual_path = static_cast<execution_path_t>(path);

    std::array<uint8_t, 4_kb * sizeof(uint32_t)> src_buffer{};
    std::array<uint8_t, 1u << 16u>               msk_buffer{};

    const auto input_stream_format  = qpl::util::parser_to_ml_parser(operation.parser_);
    const auto out_bit_width_format = qpl::util::integer_to_output_format(operation.output_vector_bit_width_);
    const auto output_stream_format = analytics::stream_format_t::le_format;

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

    auto output_stream = analytics::output_stream_t<analytics::bit_stream>::builder(dst_begin, dst_end)
            .stream_format(output_stream_format)
            .bit_format(out_bit_width_format, bit_bit_length)
            .nominal(true)
            .initial_output_index(operation.initial_output_index_)
            .build<actual_path>();

    limited_buffer_t unpack_buffer(src_buffer.data(), src_buffer.data() + src_buffer.size(), input_stream.bit_width());
    limited_buffer_t set_buffer(msk_buffer.data(), msk_buffer.data() + msk_buffer.size(), bit_bit_length);

    auto find_unique_result = analytics::call_find_unique<actual_path>(input_stream,
                                                                       output_stream,
                                                                       operation.number_low_order_bits_ignored_,
                                                                       operation.number_high_order_bits_ignored_,
                                                                       unpack_buffer,
                                                                       set_buffer,
                                                                       numa_id);

    auto output_elements_count = qpl::util::get_output_elements_as_bits(&find_unique_result,
                                                                        out_bit_width_format,
                                                                        operation.output_vector_bit_width_);

    return execution_result<uint32_t, sync>(find_unique_result.status_code_, output_elements_count);
}

template
auto execute<execution_path::software>(find_unique_operation &operation,
                                       int32_t numa_id) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::hardware>(find_unique_operation &operation,
                                       int32_t numa_id) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::auto_detect>(find_unique_operation &operation,
                                          int32_t numa_id) -> execution_result<uint32_t, sync>;

} // namespace qpl::internal

auto find_unique_operation::get_output_vector_width() const noexcept -> uint32_t {
    return output_vector_bit_width_;
}

void find_unique_operation::set_job_buffer(uint8_t *buffer) noexcept {
    // will be removed after ML introduction
}

auto find_unique_operation::find_unique_operation_builder::number_low_order_bits_to_ignore(uint32_t value)
-> find_unique_operation_builder & {
    parent_builder::operation_.number_low_order_bits_ignored_ = value;

    return *this;
}

auto find_unique_operation::find_unique_operation_builder::number_high_order_bits_to_ignore(uint32_t value)
-> find_unique_operation_builder & {
    parent_builder::operation_.number_high_order_bits_ignored_ = value;

    return *this;
}

} // namespace qpl
