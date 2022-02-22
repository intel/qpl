/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "rle_burst.hpp"
#include "descriptor_builder.hpp"
#include "util/descriptor_processing.hpp"

namespace qpl::ml::analytics {
template <analytic_pipeline pipeline_t,
        std::enable_if_t<pipeline_t == analytic_pipeline::inflate ||
                                 pipeline_t == analytic_pipeline::inflate_prle> * = nullptr>
static inline auto rle_burst(input_stream_t &counter_stream,
                             input_stream_t &symbol_stream,
                             output_stream_t<array_stream> &output_stream,
                             limited_buffer_t &unpack_buffer,
                             limited_buffer_t &symbol_buffer,
                             limited_buffer_t &output_buffer,
                             dispatcher::aggregates_function_ptr_t aggregates_callback,
                             aggregates_t &aggregates) noexcept -> uint32_t {
    auto        table          = dispatcher::kernels_dispatcher::get_instance().get_expand_rle_table();
    const auto  index          = dispatcher::get_expand_rle_index(counter_stream.bit_width(),
                                                                  symbol_stream.bit_width());
    const auto  rle_burst_impl = table[index];

    const auto counter_element_byte_size = util::bit_width_to_bytes(counter_stream.bit_width());
    const auto symbol_element_byte_size  = util::bit_width_to_bytes(symbol_stream.bit_width());

    uint32_t elements_to_process     = 0;
    uint32_t counter_idx             = 0;
    uint32_t rle_count               = 0;
    uint32_t counter_processed_bytes = 0;

    uint32_t current_uninflated_elements = 0;

    uint32_t num_input_elements = symbol_stream.elements_left();

    auto *inflate_buffer_current = unpack_buffer.data();
    auto *const inflate_buffer_begin   = unpack_buffer.data();

    // Main action
    if (counter_stream.bit_width() == 32) {
        num_input_elements--;
        symbol_stream.add_elements_processed(1);
    }

    auto drop_initial_bytes_status = counter_stream.skip_prologue(unpack_buffer);
    if (QPL_STS_OK != drop_initial_bytes_status) {
        return drop_initial_bytes_status;
    }

    auto counter_unpack = counter_stream.unpack<pipeline_t>(unpack_buffer);

    if (counter_unpack.unpacked_bytes == 0) {
        return status_list::source_is_short_error;
    } else {
        current_uninflated_elements += counter_unpack.unpacked_elements;
    }

    while (num_input_elements > 0) {
        if (elements_to_process == 0) {
            auto unpack_result = symbol_stream.unpack<analytic_pipeline::simple>(symbol_buffer);

            if (status_list::ok != unpack_result.status) {
                return unpack_result.status;
            }

            elements_to_process     = unpack_result.unpacked_elements;
            counter_processed_bytes = elements_to_process * counter_element_byte_size;

            // Check if we should inflate more elements
            if (current_uninflated_elements < elements_to_process) {
                // Shift inflate buffer
                util::copy(inflate_buffer_current,
                           inflate_buffer_current + current_uninflated_elements * (counter_stream.bit_width() / 8),
                           inflate_buffer_begin);

                unpack_buffer.set_byte_shift(current_uninflated_elements * (counter_stream.bit_width() / 8));
                counter_unpack = counter_stream.unpack<pipeline_t>(unpack_buffer);

                if (counter_unpack.unpacked_bytes == 0) {
                    return status_list::source_is_short_error;
                } else {
                    current_uninflated_elements += counter_unpack.unpacked_elements;
                }

                inflate_buffer_current = inflate_buffer_begin;
                counter_idx = 0;
            }
        }

        auto *output_position   = output_buffer.data();
        auto counter_idx_cached = counter_idx;

        auto sts = rle_burst_impl(inflate_buffer_begin,
                                  0,
                                  symbol_buffer.data(),
                                  elements_to_process,
                                  &output_position,
                                  output_buffer.end(),
                                  &rle_count,
                                  &counter_idx);

        const auto output_elements = (output_position - output_buffer.data()) / symbol_element_byte_size;

        if (output_elements != 0) {
            auto status = output_stream.perform_pack(output_buffer.data(), static_cast<uint32_t>(output_elements));

            if (status != status_list::ok) {
                return status;
            }

            aggregates_callback(output_buffer.data(),
                                static_cast<uint32_t>(output_elements),
                                &aggregates.min_value_,
                                &aggregates.max_value_,
                                &aggregates.sum_,
                                &aggregates.index_);
        }

        const auto counter_processed_elements = counter_idx - counter_idx_cached;

        current_uninflated_elements -= counter_processed_elements;
        inflate_buffer_current += (counter_processed_elements) * (counter_stream.bit_width() / 8);
        num_input_elements -= counter_processed_elements;

        elements_to_process = (elements_to_process == counter_idx) ? 0 : elements_to_process;
    }

    return status_list::ok;
}

template <analytic_pipeline pipeline_t,
        std::enable_if_t<pipeline_t == analytic_pipeline::simple> * = nullptr>
static inline auto rle_burst(input_stream_t &counter_stream,
                             input_stream_t &symbol_stream,
                             output_stream_t<array_stream> &output_stream,
                             limited_buffer_t &symbol_buffer,
                             limited_buffer_t &output_buffer,
                             dispatcher::aggregates_function_ptr_t aggregates_callback,
                             aggregates_t &aggregates) noexcept -> uint32_t {
    auto        table          = dispatcher::kernels_dispatcher::get_instance().get_expand_rle_table();
    const auto  index          = dispatcher::get_expand_rle_index(counter_stream.bit_width(),
                                                                  symbol_stream.bit_width());
    const auto  rle_burst_impl = table[index];

    const auto counter_element_byte_size = util::bit_width_to_bytes(counter_stream.bit_width());
    const auto symbol_element_byte_size  = util::bit_width_to_bytes(symbol_stream.bit_width());

    uint32_t elements_to_process     = 0;
    uint32_t counter_idx             = 0;
    uint32_t rle_count               = 0;
    uint32_t counter_processed_bytes = 0;

    // Main action
    if (counter_stream.bit_width() == 32) {
        symbol_stream.add_elements_processed(1);
        counter_stream.add_elements_processed(1);
    }

    while (!counter_stream.is_processed()) {
        if (elements_to_process == 0) {
            counter_stream.shift_current_ptr(counter_processed_bytes);

            auto unpack_result = symbol_stream.unpack<analytic_pipeline::simple>(symbol_buffer);

            if (status_list::ok != unpack_result.status) {
                return unpack_result.status;
            }

            elements_to_process     = unpack_result.unpacked_elements;
            counter_idx             = 0;
            counter_processed_bytes = elements_to_process * counter_element_byte_size;
        }

        auto *output_position   = output_buffer.data();
        auto counter_idx_cached = counter_idx;

        auto status = rle_burst_impl(counter_stream.current_ptr(),
                       counter_stream.stream_format() == stream_format_t::be_format,
                       symbol_buffer.data(),
                       elements_to_process,
                       &output_position,
                       output_buffer.end(),
                       &rle_count,
                       &counter_idx);

        if (status != status_list::ok && status != QPLC_STS_DST_IS_SHORT_ERR) {
            return status;
        }

        const auto output_elements = (output_position - output_buffer.data()) / symbol_element_byte_size;

        if (output_elements != 0) {
            auto pack_status = output_stream.perform_pack(output_buffer.data(), static_cast<uint32_t>(output_elements));
            if (pack_status != status_list::ok) {
                return pack_status;
            }
            aggregates_callback(output_buffer.data(),
                                static_cast<uint32_t>(output_elements),
                                &aggregates.min_value_,
                                &aggregates.max_value_,
                                &aggregates.sum_,
                                &aggregates.index_);
        }

        counter_stream.add_elements_processed(counter_idx - counter_idx_cached);
        elements_to_process = (elements_to_process == counter_idx) ? 0 : elements_to_process;
    }

    return status_list::ok;
}

template <analytic_pipeline pipeline_t,
        std::enable_if_t<pipeline_t == analytic_pipeline::prle> * = nullptr>
static inline auto rle_burst(input_stream_t &counter_stream,
                             input_stream_t &symbol_stream,
                             output_stream_t<array_stream> &output_stream,
                             limited_buffer_t &unpack_buffer,
                             limited_buffer_t &symbol_buffer,
                             limited_buffer_t &output_buffer,
                             dispatcher::aggregates_function_ptr_t aggregates_callback,
                             aggregates_t &aggregates) noexcept -> uint32_t {
    auto        table          = dispatcher::kernels_dispatcher::get_instance().get_expand_rle_table();
    const auto  index          = dispatcher::get_expand_rle_index(counter_stream.bit_width(),
                                                                  symbol_stream.bit_width());
    const auto  rle_burst_impl = table[index];

    const auto symbol_element_byte_size = util::bit_width_to_bytes(symbol_stream.bit_width());

    uint32_t elements_to_process = 0;
    uint32_t counter_idx         = 0;
    uint32_t rle_count           = 0;

    uint8_t  *counters_ptr        = unpack_buffer.data();
    uint32_t previous_count_value = 0u;
    uint32_t elements_to_unpack   = std::min(unpack_buffer.max_elements_count(), symbol_buffer.max_elements_count());

    bool delta_mode    = counter_stream.bit_width() == 32;
    bool first_attempt = true;

    if (delta_mode) {
        symbol_stream.add_elements_processed(1);
    }

    // Main action
    while (!symbol_stream.is_processed() || elements_to_process != 0) {
        if (elements_to_process == 0) {
            auto unpack_result = symbol_stream.unpack<analytic_pipeline::simple>(symbol_buffer, elements_to_unpack);

            if (status_list::ok != unpack_result.status) {
                return unpack_result.status;
            }

            elements_to_process = unpack_result.unpacked_elements;

            counter_idx = 0;

            if (delta_mode) {
                if (first_attempt) {
                    first_attempt = false;

                    unpack_result = counter_stream.unpack<analytic_pipeline::prle>(unpack_buffer,
                                                                                   elements_to_process + 1);

                    if (unpack_result.status != status_list::ok) {
                        return unpack_result.status;
                    }
                    previous_count_value =
                            *reinterpret_cast<uint32_t *>(unpack_buffer.data()
                                                          + (unpack_result.unpacked_elements - 1) * 4);
                } else {
                    unpack_buffer.set_byte_shift(4);
                    unpack_result = counter_stream.unpack<analytic_pipeline::prle>(unpack_buffer, elements_to_process);

                    if (unpack_result.status != status_list::ok) {
                        return unpack_result.status;
                    }

                    *reinterpret_cast<uint32_t *>(counters_ptr) = previous_count_value;
                    previous_count_value =
                            *reinterpret_cast<uint32_t *>(unpack_buffer.data()
                                                          + (unpack_result.unpacked_elements - 1) * 4);
                }
            } else {
                unpack_result = counter_stream.unpack<analytic_pipeline::prle>(unpack_buffer, elements_to_process);

                if (unpack_result.status != status_list::ok) {
                    return unpack_result.status;
                }
            }
        }

        auto *output_position = output_buffer.data();

        auto status = rle_burst_impl(counters_ptr,
                                     counter_stream.stream_format() == stream_format_t::be_format,
                                     symbol_buffer.data(),
                                     elements_to_process,
                                     &output_position,
                                     output_buffer.end(),
                                     &rle_count,
                                     &counter_idx);

        if (status != status_list::ok && status != status_list::destination_is_short_error) {
            return status;
        }

        const auto output_elements = (output_position - output_buffer.data()) / symbol_element_byte_size;

        if (output_elements != 0) {
            status = static_cast<qplc_status_t>(output_stream.perform_pack(output_buffer.data(),
                                                                           static_cast<uint32_t>(output_elements)));
            if (status != status_list::ok) {
                return status;
            }
            aggregates_callback(output_buffer.data(),
                                static_cast<uint32_t>(output_elements),
                                &aggregates.min_value_,
                                &aggregates.max_value_,
                                &aggregates.sum_,
                                &aggregates.index_);
        }

        elements_to_process = (elements_to_process == counter_idx) ? 0 : elements_to_process;
    }

    return status_list::ok;
}

template <>
auto call_rle_burst<execution_path_t::software>(input_stream_t &counter_stream,
                                                input_stream_t &symbol_stream,
                                                output_stream_t<array_stream> &output_stream,
                                                limited_buffer_t &unpack_buffer,
                                                limited_buffer_t &symbol_buffer,
                                                limited_buffer_t &output_buffer,
                                                int32_t numa_id) noexcept -> analytic_operation_result_t {
    // Get required aggregates kernel
    auto aggregates_table    = dispatcher::kernels_dispatcher::get_instance().get_aggregates_table();
    auto aggregates_index    = dispatcher::get_aggregates_index(1u);
    auto aggregates_callback = (counter_stream.are_aggregates_disabled()) ? &aggregates_empty_callback
                                                                          : aggregates_table[aggregates_index];

    aggregates_t aggregates{};

    uint32_t status_code = status_list::ok;

    switch (counter_stream.stream_format()) {
        case stream_format_t::prle_format: {
            if (counter_stream.is_compressed()) {
               status_code = rle_burst<analytic_pipeline::inflate_prle>(counter_stream,
                                                                        symbol_stream,
                                                                        output_stream,
                                                                        unpack_buffer,
                                                                        symbol_buffer,
                                                                        output_buffer,
                                                                        aggregates_callback,
                                                                        aggregates);
            } else {
                status_code = rle_burst<analytic_pipeline::prle>(counter_stream,
                                                                 symbol_stream,
                                                                 output_stream,
                                                                 unpack_buffer,
                                                                 symbol_buffer,
                                                                 output_buffer,
                                                                 aggregates_callback,
                                                                 aggregates);
            }
            break;
        }

        default: {
            if (counter_stream.is_compressed()) {
                status_code = rle_burst<analytic_pipeline::inflate>(counter_stream,
                                                                    symbol_stream,
                                                                    output_stream,
                                                                    unpack_buffer,
                                                                    symbol_buffer,
                                                                    output_buffer,
                                                                    aggregates_callback,
                                                                    aggregates);
            } else {
                status_code = rle_burst<analytic_pipeline::simple>(counter_stream,
                                                                    symbol_stream,
                                                                    output_stream,
                                                                    symbol_buffer,
                                                                    output_buffer,
                                                                    aggregates_callback,
                                                                    aggregates);
            }
            break;
        }
    }

    counter_stream.calculate_checksums();

    analytic_operation_result_t operation_result{};

    // Store operations result
    operation_result.status_code_      = status_code;
    operation_result.aggregates_       = aggregates;
    operation_result.checksums_.crc32_ = counter_stream.crc_checksum();
    operation_result.checksums_.xor_   = counter_stream.xor_checksum();
    operation_result.output_bytes_     = output_stream.bytes_written();

    operation_result.last_bit_offset_ = (1u == output_stream.bit_width())
                                        ? counter_stream.elements_left() & max_bit_index
                                        : 0u;

    return operation_result;
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage=4096"
#endif

template <>
auto call_rle_burst<execution_path_t::hardware>(input_stream_t &counter_stream,
                                                input_stream_t &symbol_stream,
                                                output_stream_t<array_stream> &output_stream,
                                                limited_buffer_t &unpack_buffer,
                                                limited_buffer_t &symbol_buffer,
                                                limited_buffer_t &output_buffer,
                                                int32_t numa_id) noexcept -> analytic_operation_result_t {
    hw_iaa_aecs_analytic HW_PATH_ALIGN_STRUCTURE aecs_analytic{};
    HW_PATH_VOLATILE hw_completion_record HW_PATH_ALIGN_STRUCTURE completion_record{};
    hw_descriptor HW_PATH_ALIGN_STRUCTURE                         descriptor{};

    descriptor_builder<qpl_op_rle_burst>(&completion_record, &aecs_analytic).operation(symbol_stream)
                                                                            .input(counter_stream)
                                                                            .output(output_stream)
                                                                            .build(&descriptor);

    return util::process_descriptor<analytic_operation_result_t, util::execution_mode_t::sync>(&descriptor,
                                                                                               &completion_record,
                                                                                               numa_id);
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

template <>
auto call_rle_burst<execution_path_t::auto_detect>(input_stream_t &counter_stream,
                                                   input_stream_t &symbol_stream,
                                                   output_stream_t<array_stream> &output_stream,
                                                   limited_buffer_t &unpack_buffer,
                                                   limited_buffer_t &symbol_buffer,
                                                   limited_buffer_t &output_buffer,
                                                   int32_t numa_id) noexcept -> analytic_operation_result_t {
    auto hw_result = call_rle_burst<execution_path_t::hardware>(counter_stream,
                                                                symbol_stream,
                                                                output_stream,
                                                                unpack_buffer,
                                                                symbol_buffer,
                                                                output_buffer,
                                                                numa_id);

    if (hw_result.status_code_ != status_list::ok) {
        return call_rle_burst<execution_path_t::software>(counter_stream,
                                                          symbol_stream,
                                                          output_stream,
                                                          unpack_buffer,
                                                          symbol_buffer,
                                                          output_buffer);
    }

    return hw_result;
}

}
