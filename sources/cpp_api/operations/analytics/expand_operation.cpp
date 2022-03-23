/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/cpp_api/operations/analytics/expand_operation.hpp"
#include "qpl/cpp_api/util/qpl_util.hpp"
#include "analytics/expand.hpp"
#include "util/checkers.hpp"

namespace qpl {
namespace internal {

template <execution_path path>
auto validate_operation(expand_operation &operation) -> uint32_t {
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

    if (operation.mask_byte_length_ == 0) {
        return status_list::source_2_is_short_error;
    }

    size_t number_of_input_elements = static_cast<size_t>(operation.number_of_input_elements_);

    if (operation.parser_ != parsers::parquet_rle && !operation.is_decompression_enabled_) {
        if (ml::util::bit_to_byte(number_of_input_elements * input_bit_width)
                > operation.source_size_) {
            return status_list::source_is_short_error;
        }
    }

    return status_list::ok;
}

template <execution_path path>
auto execute(expand_operation &operation, int32_t numa_id) -> execution_result<uint32_t, sync> {
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

    constexpr auto actual_path = static_cast<execution_path_t>(path);

    const auto input_stream_format  = qpl::util::parser_to_ml_parser(operation.parser_);
    const auto out_bit_width_format = qpl::util::integer_to_output_format(operation.output_vector_bit_width_);
    const auto output_stream_format = analytics::stream_format_t::le_format;

    std::array<uint8_t, 4_kb * sizeof(uint32_t)> src_buffer{};
    std::array<uint8_t, 4_kb * sizeof(uint32_t)> msk_buffer{};
    std::array<uint8_t, 4_kb * sizeof(uint32_t)> dst_buffer{};

    auto *src_begin  = const_cast<uint8_t *>(operation.source_);
    auto *src_end    = const_cast<uint8_t *>(operation.source_ + operation.source_size_);
    auto *dst_begin  = const_cast<uint8_t *>(operation.destination_);
    auto *dst_end    = const_cast<uint8_t *>(operation.destination_ + operation.destination_size_);
    auto *mask_begin = const_cast<uint8_t *>(operation.mask_);
    auto *mask_end   = const_cast<uint8_t *>(operation.mask_ + operation.mask_byte_length_);

    auto input_stream = analytics::input_stream_t::builder(src_begin, src_end)
            .element_count(number_of_input_elements)
            .omit_checksums(true)
            .omit_aggregates(true)
            .compressed(operation.is_decompression_enabled_)
            .stream_format(input_stream_format,input_bit_width)
            .build<actual_path>();

    auto mask_stream = analytics::input_stream_t::builder(mask_begin, mask_end)
            .element_count(operation.mask_byte_length_ * byte_bits_size)
            .stream_format(analytics::stream_format_t::le_format, bit_bit_length)
            .build<actual_path>();

    auto output_stream = analytics::output_stream_t<analytics::array_stream>::builder(dst_begin, dst_end)
            .stream_format(output_stream_format)
            .bit_format(out_bit_width_format,input_bit_width)
            .nominal(input_bit_width == bit_bit_length)
            .build<actual_path>();

    limited_buffer_t source_buffer(src_buffer.data(),
                                   src_buffer.data() + src_buffer.size(),
                                   input_stream.bit_width());
    limited_buffer_t mask_buffer(msk_buffer.data(), msk_buffer.data() + msk_buffer.size(), byte_bits_size);
    limited_buffer_t output_buffer(dst_buffer.data(), dst_buffer.data() + dst_buffer.size(), bit_bit_length);

    auto extract_result = analytics::call_expand<actual_path>(input_stream,
                                                              mask_stream,
                                                              output_stream,
                                                              source_buffer,
                                                              mask_buffer,
                                                              output_buffer,
                                                              numa_id);

    auto output_elements_count = qpl::util::get_output_elements_as_is(&extract_result,
                                                                      out_bit_width_format,
                                                                     input_bit_width,
                                                                      operation.output_vector_bit_width_);

    return execution_result<uint32_t, sync>(extract_result.status_code_, output_elements_count);
}

template
auto execute<execution_path::software>(expand_operation &operation,
                                       int32_t numa_id) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::hardware>(expand_operation &operation,
                                       int32_t numa_id) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::auto_detect>(expand_operation &operation,
                                          int32_t numa_id) -> execution_result<uint32_t, sync>;

}

void expand_operation::reset_mask(const uint8_t *mask, size_t mask_byte_length) noexcept {
    mask_             = mask;
    mask_byte_length_ = mask_byte_length;
}

void expand_operation::set_job_buffer(uint8_t * /* buffer */) noexcept {
    // will be removed after ML introduction
}

} // namespace qpl
