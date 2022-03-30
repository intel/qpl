/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/cpp_api/operations/operation.hpp"
#include "qpl/cpp_api/operations/compression/inflate_operation.hpp"

#include "compression/inflate/inflate.hpp"
#include "compression/stream_decorators/gzip_decorator.hpp"
#include "compression/stream_decorators/default_decorator.hpp"
#include "compression/huffman_table/canned_utils.hpp"
#include "util/checkers.hpp"

namespace qpl {
namespace internal {

template <execution_path path>
auto call_inflate(inflate_operation &operation,
                  uint8_t *buffer_ptr,
                  size_t buffer_size) -> std::pair<uint32_t, uint32_t> {
    using namespace qpl::ml;

    if (bad_argument::buffers_overlap(operation.source_,
                                               operation.source_size_,
                                               operation.destination_,
                                               operation.destination_size_)) {
        return {status_list::buffers_overlap, 0u};
    }

    if constexpr(path != hardware) {
        ml::util::set_zeros(buffer_ptr, static_cast<uint32_t>(buffer_size));
    }

    constexpr auto ml_execution_path = static_cast<execution_path_t>(path);

    auto next_in_ptr   = const_cast<uint8_t *>(operation.operation::source_);
    auto available_in  = static_cast<uint32_t>(operation.operation::source_size_);
    auto next_out_ptr  = const_cast<uint8_t *>(operation.operation::destination_);
    auto available_out = static_cast<uint32_t>(operation.operation::destination_size_);


    allocation_buffer_t state_buffer(buffer_ptr, buffer_ptr + buffer_size);

    const qpl::ml::util::linear_allocator allocator(state_buffer);

    auto state = compression::inflate_state<ml_execution_path>::create(allocator)
            .input(next_in_ptr, next_in_ptr + available_in)
            .output(next_out_ptr, next_out_ptr + available_out);

    compression::decompression_operation_result_t result{};

    if (operation.properties_.compression_mode_ == compression_modes::canned_mode) {
        constexpr auto    end_processing_property = compression::end_processing_condition_t::stop_and_check_any_eob;
        constexpr uint8_t ignore_start_bits       = 0;
        constexpr uint8_t ignore_end_bits         = 0;
        constexpr auto    is_random_access_mode   = true;

        auto *inflate_table = operation.properties_.huffman_table_.get_table_data();

        compression::decompression_huffman_table decompression_table(get_sw_decompression_table_buffer(inflate_table),
                                                                     get_hw_decompression_table_buffer(inflate_table),
                                                                     get_deflate_header_buffer(inflate_table),
                                                                     get_lookup_table_buffer_ptr(inflate_table));

        if (is_sw_representation_used(inflate_table)) {
            decompression_table.enable_sw_decompression_table();
        }

        if (is_hw_representation_used(inflate_table)) {
            decompression_table.enable_hw_decompression_table();
        }

        if (is_deflate_representation_used(inflate_table)) {
            decompression_table.enable_deflate_header();
        }

        state.input_access({is_random_access_mode,
                            ignore_start_bits,
                            ignore_end_bits})
             .decompress_table(decompression_table);

        if (decompression_table.is_deflate_header_used()) {
            result = compression::default_decorator::unwrap(compression::inflate<ml_execution_path,
                                                                    compression::inflate_mode_t::inflate_body>,
                                                            state,
                                                            end_processing_property);
        } else {
            result.status_code_ = qpl::ml::status_list::status_invalid_params;
        }
    } else {
        constexpr auto end_processing_property = compression::end_processing_condition_t::stop_and_check_for_bfinal_eob;

        state.terminate();

        if (operation.properties_.gzip_mode_) {
            result = compression::gzip_decorator::unwrap(compression::inflate<ml_execution_path,
                                                                 compression::inflate_mode_t::inflate_default>,
                                                         state,
                                                         end_processing_property);
        } else {
            result = compression::default_decorator::unwrap(compression::inflate<ml_execution_path,
                                                                    compression::inflate_mode_t::inflate_default>,
                                                            state,
                                                            end_processing_property);
        }
    }

    return {result.status_code_, result.output_bytes_};
}

template <execution_path path>
auto execute(inflate_operation &operation,
             uint8_t *buffer_ptr,
             size_t buffer_size) -> execution_result<uint32_t, sync> {
    auto result = call_inflate<path>(operation, buffer_ptr, buffer_size);

    return execution_result<uint32_t, sync>(result.first, result.second);
}

template
auto execute<execution_path::software>(inflate_operation &operation,
                                       uint8_t *buffer_ptr,
                                       const size_t buffer_size) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::hardware>(inflate_operation &operation,
                                       uint8_t *buffer_ptr,
                                       const size_t buffer_size) -> execution_result<uint32_t, sync>;

template <>
auto execute<execution_path::auto_detect>(inflate_operation &operation,
                                          uint8_t *buffer_ptr,
                                          const size_t buffer_size) -> execution_result<uint32_t, sync> {
    auto hw_result = call_inflate<execution_path::hardware>(operation,
                                                            buffer_ptr,
                                                            buffer_size);

    if (hw_result.first != ml::status_list::ok) {
        auto sw_result = call_inflate<execution_path::software>(operation,
                                                                buffer_ptr,
                                                                buffer_size);

        return execution_result<uint32_t, sync>(sw_result.first, sw_result.second);
    }

    return execution_result<uint32_t, sync>(hw_result.first, hw_result.second);
}

template <>
auto execute<execution_path::hardware>(inflate_operation &operation) -> execution_result<uint32_t, sync> {
    std::array<uint8_t, ml::compression::inflate_state<ml::execution_path_t::hardware>::get_buffer_size()> buffer{};

    auto result = call_inflate<qpl::hardware>(operation, buffer.data(), buffer.size());

    return execution_result<uint32_t, sync>(result.first, result.second);
}

} // namespace qpl::internal

void inflate_operation::set_job_buffer(uint8_t * /* buffer */) noexcept {
    // will be removed after ML introduction
}

auto inflate_operation::get_gzip_mode() -> bool {
    return properties_.gzip_mode_;
}

auto inflate_operation::get_properties() -> inflate_properties {
    return properties_;
}

} // namespace qpl
