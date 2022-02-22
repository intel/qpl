/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "set_membership.hpp"
#include "descriptor_builder.hpp"
#include "util/descriptor_processing.hpp"
#include "util/multi_descriptor_processing.hpp"

namespace qpl::ml::analytics {
template <analytic_pipeline pipeline_t>
static inline auto set_membership(input_stream_t &input_stream,
                                  limited_buffer_t &unpack_buffer,
                                  limited_buffer_t &set_buffer,
                                  output_stream_t<bit_stream> &output_stream,
                                  dispatcher::aggregates_function_ptr_t aggregates_callback,
                                  aggregates_t &aggregates,
                                  uint32_t bits_shift,
                                  uint32_t bits_set_size) noexcept -> uint32_t {
    const auto table               = dispatcher::kernels_dispatcher::get_instance().get_set_membership_i_table();
    const auto index               = dispatcher::get_set_membership_index(input_stream.bit_width());
    const auto set_membership_impl = table[index];

    const uint32_t bits_set_mask = bits_set_size - 1u;

    auto drop_initial_bytes_status = input_stream.skip_prologue(unpack_buffer);
    if (QPL_STS_OK != drop_initial_bytes_status) {
        return drop_initial_bytes_status;
    }

    // Main action
    while (!input_stream.is_processed()) {
        // Unpack the input
        auto unpack_result = input_stream.unpack<pipeline_t>(unpack_buffer);

        if (status_list::ok != unpack_result.status) {
            return unpack_result.status;
        }

        const uint32_t elements_to_process = unpack_result.unpacked_elements;

        set_membership_impl(unpack_buffer.data(),
                            set_buffer.data(),
                            elements_to_process,
                            bits_shift,
                            bits_set_mask);

        // Pack result
        auto status = output_stream.perform_pack(unpack_buffer.data(), elements_to_process);

        if (status_list::ok != status) {
            return status;
        }
    }

    // Calculate aggregates
    aggregates_callback(set_buffer.data(),
                        bits_set_size,
                        &aggregates.min_value_,
                        &aggregates.max_value_,
                        &aggregates.sum_,
                        &aggregates.index_);

    return status_list::ok;
}

template <>
auto call_set_membership<execution_path_t::software>(input_stream_t &input_stream,
                                                     input_stream_t &set_stream,
                                                     output_stream_t<bit_stream> &output_stream,
                                                     uint32_t low_bits_to_ignore,
                                                     uint32_t high_bits_to_ignore,
                                                     limited_buffer_t &unpack_buffer,
                                                     limited_buffer_t &set_buffer,
                                                     int32_t numa_id) noexcept -> analytic_operation_result_t {
    // Get required aggregates kernel
    auto aggregates_table    = dispatcher::kernels_dispatcher::get_instance().get_aggregates_table();
    auto aggregates_index    = dispatcher::get_aggregates_index(1u);
    auto aggregates_callback = (input_stream.are_aggregates_disabled()) ?
                                &aggregates_empty_callback :
                                aggregates_table[aggregates_index];

    uint32_t     bits_set_size = 1u << (input_stream.bit_width() - low_bits_to_ignore - high_bits_to_ignore);
    aggregates_t aggregates{};

    uint32_t status_code = status_list::ok;
    set_stream.unpack<analytic_pipeline::simple>(set_buffer);

    if (input_stream.stream_format() == stream_format_t::prle_format) {
        if (input_stream.is_compressed()) {
            status_code = set_membership<analytic_pipeline::inflate_prle>(input_stream,
                                                                          unpack_buffer,
                                                                          set_buffer,
                                                                          output_stream,
                                                                          aggregates_callback,
                                                                          aggregates,
                                                                          low_bits_to_ignore,
                                                                          bits_set_size);
        } else {
            status_code = set_membership<analytic_pipeline::prle>(input_stream,
                                                                  unpack_buffer,
                                                                  set_buffer,
                                                                  output_stream,
                                                                  aggregates_callback,
                                                                  aggregates,
                                                                  low_bits_to_ignore,
                                                                  bits_set_size);
        }
    } else {
        if (input_stream.is_compressed()) {
            status_code = set_membership<analytic_pipeline::inflate>(input_stream,
                                                                     unpack_buffer,
                                                                     set_buffer,
                                                                     output_stream,
                                                                     aggregates_callback,
                                                                     aggregates,
                                                                     low_bits_to_ignore,
                                                                     bits_set_size);
        } else {
            status_code = set_membership<analytic_pipeline::simple>(input_stream,
                                                                    unpack_buffer,
                                                                    set_buffer,
                                                                    output_stream,
                                                                    aggregates_callback,
                                                                    aggregates,
                                                                    low_bits_to_ignore,
                                                                    bits_set_size);
        }
    }

    input_stream.calculate_checksums();

    analytic_operation_result_t operation_result{};

    // Store operations result
    operation_result.status_code_      = status_code;
    operation_result.aggregates_       = aggregates;
    operation_result.checksums_.crc32_ = input_stream.crc_checksum();
    operation_result.checksums_.xor_   = input_stream.xor_checksum();
    operation_result.output_bytes_     = output_stream.bytes_written();

    operation_result.last_bit_offset_ = (1u == output_stream.bit_width())
                                        ? input_stream.elements_left() & max_bit_index
                                        : 0u;

    return operation_result;
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage=4096"
#endif

auto call_set_membership_multidescriptor(input_stream_t &input_stream,
                                         input_stream_t &set_stream,
                                         output_stream_t<bit_stream> &output_stream,
                                         uint32_t low_bits_to_ignore,
                                         uint32_t high_bits_to_ignore,
                                         limited_buffer_t &unpack_buffer,
                                         limited_buffer_t &set_buffer,
                                         int32_t numa_id) noexcept -> analytic_operation_result_t {
    hw_iaa_aecs_analytic HW_PATH_ALIGN_STRUCTURE reference_aecs{};
    HW_PATH_VOLATILE hw_completion_record HW_PATH_ALIGN_STRUCTURE reference_completion_record{};
    hw_descriptor HW_PATH_ALIGN_STRUCTURE                         reference_descriptor{};

    constexpr uint32_t number_of_descriptors = 8;
    alignas(HW_PATH_STRUCTURES_REQUIRED_ALIGN) std::array<hw_completion_record, number_of_descriptors> completion_records{};
    alignas(HW_PATH_STRUCTURES_REQUIRED_ALIGN) std::array<hw_descriptor, number_of_descriptors> descriptors{};

    descriptor_builder<qpl_op_set_membership>(&reference_completion_record, &reference_aecs).operation(set_stream,
                                                                                                       low_bits_to_ignore,
                                                                                                       high_bits_to_ignore)
                                                                                            .input(input_stream)
                                                                                            .output(output_stream)
                                                                                            .build(&reference_descriptor);

    split_descriptors<qpl_op_set_membership, number_of_descriptors>(reference_descriptor,descriptors);

    auto result = ml::util::process_descriptor<analytic_operation_result_t, number_of_descriptors>(descriptors,
                                                                                                   completion_records,
                                                                                                   numa_id);

    return result;
}

template <>
auto call_set_membership<execution_path_t::hardware>(input_stream_t &input_stream,
                                                     input_stream_t &set_stream,
                                                     output_stream_t<bit_stream> &output_stream,
                                                     uint32_t low_bits_to_ignore,
                                                     uint32_t high_bits_to_ignore,
                                                     limited_buffer_t &unpack_buffer,
                                                     limited_buffer_t &set_buffer,
                                                     int32_t numa_id) noexcept -> analytic_operation_result_t {
    if (is_operation_splittable(input_stream, output_stream)) {
        return call_set_membership_multidescriptor(input_stream,
                                                   set_stream,
                                                   output_stream,
                                                   low_bits_to_ignore,
                                                   high_bits_to_ignore,
                                                   unpack_buffer,
                                                   set_buffer,
                                                   numa_id);
    }

    hw_iaa_aecs_analytic HW_PATH_ALIGN_STRUCTURE aecs_analytic{};
    HW_PATH_VOLATILE hw_completion_record HW_PATH_ALIGN_STRUCTURE completion_record{};
    hw_descriptor HW_PATH_ALIGN_STRUCTURE                         descriptor{};

    descriptor_builder<qpl_op_set_membership>(&completion_record, &aecs_analytic).operation(set_stream,
                                                                                            low_bits_to_ignore,
                                                                                            high_bits_to_ignore)
                                                                                 .input(input_stream)
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
auto call_set_membership<execution_path_t::auto_detect>(input_stream_t &input_stream,
                                                        input_stream_t &set_stream,
                                                        output_stream_t<bit_stream> &output_stream,
                                                        uint32_t low_bits_to_ignore,
                                                        uint32_t high_bits_to_ignore,
                                                        limited_buffer_t &unpack_buffer,
                                                        limited_buffer_t &set_buffer,
                                                        int32_t numa_id) noexcept -> analytic_operation_result_t {
    auto hw_result = call_set_membership<execution_path_t::hardware>(input_stream,
                                                                     set_stream,
                                                                     output_stream,
                                                                     low_bits_to_ignore,
                                                                     high_bits_to_ignore,
                                                                     unpack_buffer,
                                                                     set_buffer,
                                                                     numa_id);

    if (hw_result.status_code_ != status_list::ok) {
        return call_set_membership<execution_path_t::software>(input_stream,
                                                               set_stream,
                                                               output_stream,
                                                               low_bits_to_ignore,
                                                               high_bits_to_ignore,
                                                               unpack_buffer,
                                                               set_buffer);
    }

    return hw_result;
}

}
