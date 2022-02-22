/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <array>

#include "qpl/cpp_api/operations/analytics/scan_operation.hpp"
#include "analytics/scan.hpp"
#include "qpl/cpp_api/util/qpl_util.hpp"
#include "util/checkers.hpp"

namespace qpl {

namespace internal {

template <execution_path path>
auto validate_operation(scan_operation &operation) -> uint32_t {
    using namespace qpl::ml;

    uint32_t input_bit_width = operation.input_vector_bit_width_;

    if (bad_argument::buffers_overlap(operation.source_,
                                      operation.source_size_,
                                      operation.destination_,
                                      operation.destination_size_)) {
        return status_list::buffers_overlap;
    }

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

    if (operation.output_vector_bit_width_ == 1) {
        if (ml::util::bit_to_byte(number_of_input_elements) > operation.destination_size_) {
            return status_list::destination_is_short_error;
        }
    }

    if (operation.parser_ != parsers::parquet_rle && !operation.is_decompression_enabled_) {
        if (ml::util::bit_to_byte(number_of_input_elements * input_bit_width)
                > operation.source_size_) {
            return status_list::source_is_short_error;
        }
    }

    const auto out_bit_width_format = qpl::util::integer_to_output_format(operation.output_vector_bit_width_);

    if (analytics::output_bit_width_format_t::same_as_input != out_bit_width_format)
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
auto execute(scan_operation &operation, int32_t numa_id) -> execution_result<uint32_t, sync> {
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

    std::array<uint8_t, 4_kb * sizeof(uint32_t)> buffer{};

    constexpr auto actual_path = static_cast<execution_path_t>(path);

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

    limited_buffer_t temporary_buffer(buffer.data(), buffer.data() + buffer.size(), input_stream.bit_width());

    analytics::analytic_operation_result_t scan_result;

    switch (operation.comparator_) {
        case less: {
            if (operation.is_inclusive_) {
                scan_result = analytics::call_scan<analytics::comparator_t::less_equals, actual_path>(input_stream,
                                                                                                      output_stream,
                                                                                                      operation.boundary_,
                                                                                                      0,
                                                                                                      temporary_buffer,
                                                                                                      numa_id);
            } else {
                scan_result = analytics::call_scan<analytics::comparator_t::less_than, actual_path>(input_stream,
                                                                                                    output_stream,
                                                                                                    operation.boundary_,
                                                                                                    0,
                                                                                                    temporary_buffer,
                                                                                                    numa_id);
            }
            break;
        }
        case greater: {
            if (operation.is_inclusive_) {
                scan_result = analytics::call_scan<analytics::comparator_t::greater_equals, actual_path>(input_stream,
                                                                                                         output_stream,
                                                                                                         operation.boundary_,
                                                                                                         0,
                                                                                                         temporary_buffer,
                                                                                                         numa_id);
            } else {
                scan_result = analytics::call_scan<analytics::comparator_t::greater_than, actual_path>(input_stream,
                                                                                                       output_stream,
                                                                                                       operation.boundary_,
                                                                                                       0,
                                                                                                       temporary_buffer,
                                                                                                       numa_id);
            }
            break;
        }
        case equals: {
            scan_result = analytics::call_scan<analytics::comparator_t::equals, actual_path>(input_stream,
                                                                                             output_stream,
                                                                                             operation.boundary_,
                                                                                             0,
                                                                                             temporary_buffer,
                                                                                             numa_id);
            break;
        }
        case not_equals: {
            scan_result = analytics::call_scan<analytics::comparator_t::not_equals, actual_path>(input_stream,
                                                                                                 output_stream,
                                                                                                 operation.boundary_,
                                                                                                 0,
                                                                                                 temporary_buffer,
                                                                                                 numa_id);
            break;
        }
        default: {
            scan_result.output_bytes_ = 0u;
            scan_result.status_code_  = QPL_STS_OPERATION_ERR;
            break;
        }
    }

    auto output_elements_count = qpl::util::get_output_elements_as_bits(&scan_result,
                                                                        out_bit_width_format,
                                                                        operation.output_vector_bit_width_);

    return execution_result<uint32_t, sync>(scan_result.status_code_, output_elements_count);
}

template
auto execute<execution_path::software>(scan_operation &operation, int32_t numa_id) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::hardware>(scan_operation &operation, int32_t numa_id) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::auto_detect>(scan_operation &operation,
                                          int32_t numa_id) -> execution_result<uint32_t, sync>;

} // namespace qpl::internal

auto scan_operation::get_output_vector_width() const noexcept -> uint32_t {
    return output_vector_bit_width_;
}

void scan_operation::set_job_buffer(uint8_t *buffer) noexcept {
    // will be removed after ML introduction
}

auto scan_operation::scan_operation_builder::comparator(comparators value) -> scan_operation_builder & {
    parent_builder::operation_.comparator_ = value;

    return *this;
}

auto scan_operation::scan_operation_builder::boundary(uint32_t value) -> scan_operation_builder & {
    parent_builder::operation_.boundary_ = value;

    return *this;
}

auto scan_operation::scan_operation_builder::is_inclusive(bool value) -> scan_operation_builder & {
    parent_builder::operation_.is_inclusive_ = value;

    return *this;
}

} // namespace qpl
