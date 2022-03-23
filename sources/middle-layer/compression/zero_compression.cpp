/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "zero_compression.hpp"
#include "dispatcher/dispatcher.hpp"
#include "dispatcher/hw_dispatcher.hpp"
#include "util/descriptor_processing.hpp"
#include "util/checksum.hpp"

#include "hw_descriptors_api.h"

namespace qpl::ml::compression {

template <zero_operation_type operation_type>
static inline auto get_zero_opcode(const zero_input_format_t input_format) -> uint32_t {
    if constexpr (operation_type == zero_operation_type::compress) {
        return input_format == zero_input_format_t::word_16_bit
               ? QPL_OPCODE_Z_COMP16
               : QPL_OPCODE_Z_COMP32;
    } else {
        return input_format == zero_input_format_t::word_16_bit
               ? QPL_OPCODE_Z_DECOMP16
               : QPL_OPCODE_Z_DECOMP32;
    }
}

template
auto call_zero_operation<execution_path_t::auto_detect,
                         zero_operation_type::compress>(const zero_input_format_t input_format,
                                                        const buffer_t &source_buffer,
                                                        const buffer_t &destination_buffer,
                                                        const crc_type_t crc_type,
                                                        int32_t numa_id) noexcept -> zero_operation_result_t;

template
auto call_zero_operation<execution_path_t::auto_detect,
                         zero_operation_type::decompress>(const zero_input_format_t input_format,
                                                          const buffer_t &source_buffer,
                                                          const buffer_t &destination_buffer,
                                                          const crc_type_t crc_type,
                                                          int32_t numa_id) noexcept -> zero_operation_result_t;

template
auto call_zero_operation<execution_path_t::software,
                         zero_operation_type::compress>(const zero_input_format_t input_format,
                                                        const buffer_t &source_buffer,
                                                        const buffer_t &destination_buffer,
                                                        const crc_type_t crc_type,
                                                        int32_t numa_id) noexcept -> zero_operation_result_t;

template
auto call_zero_operation<execution_path_t::software,
                         zero_operation_type::decompress>(const zero_input_format_t input_format,
                                                          const buffer_t &source_buffer,
                                                          const buffer_t &destination_buffer,
                                                          const crc_type_t crc_type,
                                                          int32_t numa_id) noexcept -> zero_operation_result_t;

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage=4096"
#endif

template
auto call_zero_operation<execution_path_t::hardware,
                         zero_operation_type::compress>(const zero_input_format_t input_format,
                                                        const buffer_t &source_buffer,
                                                        const buffer_t &destination_buffer,
                                                        const crc_type_t crc_type,
                                                        int32_t numa_id) noexcept -> zero_operation_result_t;

template
auto call_zero_operation<execution_path_t::hardware,
                         zero_operation_type::decompress>(const zero_input_format_t input_format,
                                                          const buffer_t &source_buffer,
                                                          const buffer_t &destination_buffer,
                                                          const crc_type_t crc_type,
                                                          int32_t numa_id) noexcept -> zero_operation_result_t;

template <zero_operation_type operation_type>
auto call_zero_operation_hw(const zero_input_format_t input_format,
                            const buffer_t &source_buffer,
                            const buffer_t &destination_buffer,
                            const crc_type_t UNREFERENCED_PARAMETER(crc_type),
                            int32_t numa_id) noexcept -> zero_operation_result_t {
    HW_PATH_VOLATILE hw_completion_record HW_PATH_ALIGN_STRUCTURE completion_record{};
    hw_descriptor HW_PATH_ALIGN_STRUCTURE                         descriptor{};

    hw_iaa_descriptor_init_zero_compress(&descriptor,
                                         get_zero_opcode<operation_type>(input_format),
                                         source_buffer.data(),
                                         destination_buffer.data(),
                                         source_buffer.size(),
                                         destination_buffer.size());

    return util::process_descriptor<zero_operation_result_t, util::execution_mode_t::sync>(&descriptor,
                                                                                           &completion_record,
                                                                                           numa_id);
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

static inline auto calculate_checksums(zero_operation_result_t *zero_operation_result,
                                       const uint8_t *data_ptr,
                                       const uint32_t length,
                                       const crc_type_t crc_type) -> void {
    constexpr static const uint32_t initial_checksum_value = 0;

    zero_operation_result->checksums_.crc32_ = (crc_type == crc_type_t::crc_32c ? 1 : 0) ?
                                               util::crc32_iscsi_inv(data_ptr, data_ptr + length, initial_checksum_value) :
                                               util::crc32_gzip(data_ptr, data_ptr + length, initial_checksum_value);

    zero_operation_result->checksums_.xor_ = util::xor_checksum(data_ptr, data_ptr + length, initial_checksum_value);
}

static inline auto perform_zero_compress(const zero_input_format_t input_format,
                                         const buffer_t &source_buffer,
                                         const buffer_t &destination_buffer,
                                         const crc_type_t crc_type) noexcept -> zero_operation_result_t {
    using zero_compress_f = qpl::ml::dispatcher::zero_compress_table_t::value_type;

    zero_operation_result_t zero_operation_result{};

    if (crc_type != crc_type_t::none) {
        calculate_checksums(&zero_operation_result, source_buffer.data(), source_buffer.size(), crc_type);
    }

    static const auto &table = qpl::ml::dispatcher::kernels_dispatcher::get_instance().get_zero_compress_table();

    auto function_index = (input_format == zero_input_format_t::word_16_bit) ? 0u : 2u;

    auto status = ((zero_compress_f) table[function_index])(source_buffer.data(),
                                                            source_buffer.size(),
                                                            destination_buffer.data(),
                                                            destination_buffer.size(),
                                                            &zero_operation_result.output_bytes_);

    if (status_list::ok == status) {
        zero_operation_result.aggregates_.max_value_ = 0;
        zero_operation_result.aggregates_.min_value_ = std::numeric_limits<uint32_t>::max();
        zero_operation_result.aggregates_.sum_       = 0;
        zero_operation_result.status_code_           = status_list::ok;
    } else {
        zero_operation_result.status_code_  = status;
        zero_operation_result.output_bytes_ = 0;
    }

    return zero_operation_result;
}

static inline auto perform_zero_decompress(const zero_input_format_t input_format,
                                           const buffer_t &source_buffer,
                                           const buffer_t &destination_buffer,
                                           const crc_type_t crc_type) noexcept -> zero_operation_result_t {
    using zero_compress_f = qpl::ml::dispatcher::zero_compress_table_t::value_type;

    zero_operation_result_t zero_operation_result{};

    const auto &table = qpl::ml::dispatcher::kernels_dispatcher::get_instance().get_zero_compress_table();

    auto function_index = (input_format == zero_input_format_t::word_16_bit) ? 1u : 3u;

    auto status = ((zero_compress_f) table[function_index])(source_buffer.data(),
                                                            source_buffer.size(),
                                                            destination_buffer.data(),
                                                            destination_buffer.size(),
                                                            &zero_operation_result.output_bytes_);
    if (status_list::ok == status) {
        zero_operation_result.aggregates_.max_value_ = 0;
        zero_operation_result.aggregates_.min_value_ = std::numeric_limits<uint32_t>::max();
        zero_operation_result.aggregates_.sum_       = 0;
        zero_operation_result.status_code_           = status_list::ok;

        if (crc_type != crc_type_t::none) {
            calculate_checksums(&zero_operation_result,
                                destination_buffer.data(),
                                zero_operation_result.output_bytes_,
                                crc_type);
        }
    } else {
        zero_operation_result.status_code_  = status;
        zero_operation_result.output_bytes_ = 0;
    }

    return zero_operation_result;
}

template <zero_operation_type operation_type>
auto call_zero_operation_sw(const zero_input_format_t input_format,
                            const buffer_t &source_buffer,
                            const buffer_t &destination_buffer,
                            const crc_type_t crc_type,
                            int32_t UNREFERENCED_PARAMETER(numa_id)) noexcept -> zero_operation_result_t {
    if constexpr (operation_type == zero_operation_type::compress) {
        return perform_zero_compress(input_format,
                                     source_buffer,
                                     destination_buffer,
                                     crc_type);
    } else {
        return perform_zero_decompress(input_format,
                                       source_buffer,
                                       destination_buffer,
                                       crc_type);
    }
}

template <execution_path_t path,
        zero_operation_type operation_type>
auto call_zero_operation(const zero_input_format_t input_format,
                         const buffer_t &source_buffer,
                         const buffer_t &destination_buffer,
                         const crc_type_t crc_type,
                         int32_t numa_id) noexcept -> zero_operation_result_t {
    if constexpr (path == execution_path_t::auto_detect) {
        auto hw_result = call_zero_operation_hw<operation_type>(input_format,
                                                                source_buffer,
                                                                destination_buffer,
                                                                crc_type,
                                                                numa_id);

        if (hw_result.status_code_ != status_list::ok) {
            return call_zero_operation_sw<operation_type>(input_format,
                                                          source_buffer,
                                                          destination_buffer,
                                                          crc_type,
                                                          numa_id);
        }

        return hw_result;
    } else if constexpr (path == execution_path_t::hardware) {
        return call_zero_operation_hw<operation_type>(input_format,
                                                      source_buffer,
                                                      destination_buffer,
                                                      crc_type,
                                                      numa_id);
    } else {
        return call_zero_operation_sw<operation_type>(input_format,
                                                      source_buffer,
                                                      destination_buffer,
                                                      crc_type,
                                                      numa_id);
    }
}

} // namespace qpl::ml::compression
