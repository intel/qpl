/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/cpp_api/operations/operation.hpp"
#include "qpl/cpp_api/operations/compression/deflate_operation.hpp"

#include "compression/deflate/deflate.hpp"
#include "compression/deflate/streams/deflate_state_builder.hpp"
#include "compression/stream_decorators/default_decorator.hpp"
#include "compression/stream_decorators/gzip_decorator.hpp"
#include "util/checkers.hpp"

namespace qpl {
namespace internal {

template <execution_path path>
auto call_deflate(deflate_operation &operation,
                  uint8_t *buffer_ptr,
                  size_t buffer_size) -> std::pair<uint32_t, uint32_t> {
    using namespace qpl::ml;

    if (bad_argument::buffers_overlap(operation.source_,
                                      operation.source_size_,
                                      operation.destination_,
                                      operation.destination_size_)) {
        return {status_list::buffers_overlap, 0u};
    }

    constexpr const auto actual_path = static_cast<execution_path_t>(path);

    if constexpr(path == hardware) {
        if (operation.properties_.compression_level_ == high_level) {
            return {status_list::not_supported_err, 0};
        }
    } else {
        ml::util::set_zeros(buffer_ptr, static_cast<uint32_t>(buffer_size));
    }

    auto next_in_ptr   = const_cast<uint8_t *>(operation.source_);
    auto available_in  = static_cast<uint32_t>(operation.source_size_);
    auto next_out_ptr  = const_cast<uint8_t *>(operation.destination_);
    auto available_out = static_cast<uint32_t>(operation.destination_size_);

    allocation_buffer_t state_buffer(buffer_ptr, buffer_ptr + buffer_size);

    const qpl::ml::util::linear_allocator allocator(state_buffer);

    auto builder = compression::deflate_state_builder<actual_path>::create(allocator);

    builder.output(next_out_ptr, available_out)
           .compression_level(static_cast<compression::compression_level_t>(operation.properties_
                                                                                     .compression_level_))
           .terminate(true)
           .load_current_position(0);

    if (operation.properties_.mini_block_size_ != mini_block_sizes::mini_block_size_none) {
        static constexpr const uint32_t current_index_number = 0;
        builder.enable_indexing(static_cast<compression::mini_block_size_t>(operation.properties_.mini_block_size_),
                                reinterpret_cast<uint64_t *>(operation.properties_.index_array_),
                                current_index_number,
                                static_cast<uint32_t>(operation.properties_.index_array_size_));
    }

    if (operation.properties_.compression_mode_ == compression_modes::dynamic_mode) {
        builder.collect_statistics_step(true);
    } else {
        builder.start_new_block(true);

        if (operation.properties_.compression_mode_ == compression_modes::static_mode ||
            operation.properties_.compression_mode_ == compression_modes::canned_mode) {
            builder.compression_table(operation.properties_.huffman_table_.get_table_data());
        }
    }

    auto state = builder.build();

    compression::compression_operation_result_t result{};

    if (operation.properties_.compression_mode_ == compression_modes::canned_mode) {
        result = compression::deflate<actual_path,
                                      compression::deflate_mode_t::deflate_no_headers>(state,
                                                                                       next_in_ptr,
                                                                                       available_in);
    } else {
        if (operation.properties_.gzip_mode_) {
            result = compression::gzip_decorator::wrap(compression::deflate<actual_path,
                                                                            compression::deflate_mode_t::deflate_default>,
                                                       state,
                                                       next_in_ptr,
                                                       available_in);
        } else {
            result = compression::default_decorator::wrap(compression::deflate<actual_path,
                                                                               compression::deflate_mode_t::deflate_default>,
                                                          state,
                                                          next_in_ptr,
                                                          available_in);
        }
    }

    return {result.status_code_, result.output_bytes_};
}

template <execution_path path>
auto execute(deflate_operation &operation,
             uint8_t *buffer_ptr,
             size_t buffer_size) -> execution_result<uint32_t, sync> {
    auto result = call_deflate<path>(operation, buffer_ptr, buffer_size);

    return execution_result<uint32_t, sync>(result.first, result.second);
}

template
auto execute<execution_path::software>(deflate_operation &operation,
                                       uint8_t *buffer_ptr,
                                       const size_t buffer_size) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::hardware>(deflate_operation &operation,
                                       uint8_t *buffer_ptr,
                                       const size_t buffer_size) -> execution_result<uint32_t, sync>;
template <>
auto execute<execution_path::auto_detect>(deflate_operation &operation,
                                          uint8_t *buffer_ptr,
                                          const size_t buffer_size) -> execution_result<uint32_t, sync> {
    auto hw_result = call_deflate<execution_path::hardware>(operation, buffer_ptr, buffer_size);

    if (hw_result.first != ml::status_list::ok &&
        hw_result.first != ml::status_list::more_output_needed) {
        auto sw_result = call_deflate<execution_path::software>(operation, buffer_ptr, buffer_size);

        return execution_result<uint32_t, sync>(sw_result.first, sw_result.second);
    }

    return execution_result<uint32_t, sync>(hw_result.first, hw_result.second);
}

template <>
auto execute<execution_path::hardware>(deflate_operation &operation) -> execution_result<uint32_t, sync> {
    std::array<uint8_t, ml::compression::deflate_state<ml::execution_path_t::hardware>::required_buffer_size()> buffer{};

    auto result = call_deflate<qpl::hardware>(operation, buffer.data(), buffer.size());

    return execution_result<uint32_t, sync>(result.first, result.second);
}

} // namespace qpl::internal

void deflate_operation::set_job_buffer(uint8_t * /* buffer */) noexcept {
    // @todo will be removed after ML introduction
}

void deflate_operation::enable_random_access(internal::index *index_array,
                                             const uint32_t index_array_size,
                                             const mini_block_sizes mini_block_size) noexcept {
    properties_.index_array_      = index_array;
    properties_.index_array_size_ = index_array_size;
    properties_.mini_block_size_  = mini_block_size;
}

void deflate_operation::disable_random_access() noexcept {
    properties_.index_array_      = nullptr;
    properties_.index_array_size_ = 0;
    properties_.mini_block_size_  = mini_block_size_none;
}

auto deflate_operation::deflate_operation_builder::compression_level(compression_levels value)
-> deflate_operation_builder & {
    parent_builder::operation_.properties_.compression_level_ = value;

    return *this;
}

auto deflate_operation::get_gzip_mode() -> bool {
    return properties_.gzip_mode_;
}

auto deflate_operation::get_properties() -> deflate_properties {
    return properties_;
}

} // namespace qpl
