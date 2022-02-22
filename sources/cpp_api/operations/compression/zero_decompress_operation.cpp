/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "compression/zero_compression.hpp"
#include "qpl/cpp_api/operations/compression/zero_decompress_operation.hpp"
#include "util/checkers.hpp"

namespace qpl {

namespace internal {

template <execution_path path>
auto execute(zero_decompress_operation &operation, int32_t numa_id) -> execution_result<uint32_t, sync> {
    using namespace qpl::ml::compression;

    auto next_in_ptr   = const_cast<uint8_t *>(operation.source_);
    auto available_in  = static_cast<uint32_t>(operation.source_size_);
    auto next_out_ptr  = const_cast<uint8_t *>(operation.destination_);
    auto available_out = static_cast<uint32_t>(operation.destination_size_);

    if (qpl::ml::bad_argument::buffers_overlap(next_in_ptr,
                                               available_in,
                                               next_out_ptr,
                                               available_out)) {
        return execution_result<uint32_t, sync>(QPL_STS_BUFFER_OVERLAP_ERR, 0);
    }

    if (operation.input_format_ == zero_input_format::word_16_bit) {
        if (available_in < 8) {
            return execution_result<uint32_t, sync>(QPL_STS_INVALID_ZERO_DECOMP_HDR, 0);
        } else if (available_in & 1) {    // if available_in isn't even -> some elem is incomplete
            return execution_result<uint32_t, sync>(QPL_STS_SRC_IS_SHORT_ERR, 0);
        } else if (available_out == 0) {
            return execution_result<uint32_t, sync>(QPL_STS_DST_IS_SHORT_ERR, 0);
        }
    } else {
        if (available_in < 4) {
            return execution_result<uint32_t, sync>(QPL_STS_INVALID_ZERO_DECOMP_HDR, 0);
        } else if (available_in & 3) {    // if available_in isn't multiple of 4 -> some elem is incomplete
            return execution_result<uint32_t, sync>(QPL_STS_SRC_IS_SHORT_ERR, 0);
        } else if (available_out == 0) {
            return execution_result<uint32_t, sync>(QPL_STS_DST_IS_SHORT_ERR, 0);
        }
    }

    constexpr auto actual_path = static_cast<ml::execution_path_t>(path);

    auto input_format = static_cast<zero_input_format_t>(operation.input_format_);
    auto crc_type     = static_cast<crc_type_t>(operation.crc_type_);

    ml::buffer_t source_buffer(next_in_ptr, next_in_ptr + available_in);
    ml::buffer_t destination_buffer(next_out_ptr, next_out_ptr + available_out);

    auto zero_decompress_result = call_zero_operation<actual_path,
                                                      zero_operation_type::decompress>(input_format,
                                                                                       source_buffer,
                                                                                       destination_buffer,
                                                                                       crc_type,
                                                                                       numa_id);

    return execution_result<uint32_t, sync>(zero_decompress_result.status_code_, zero_decompress_result.output_bytes_);
}

template
auto execute<execution_path::software>(zero_decompress_operation &operation,
                                       int32_t numa_id) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::hardware>(zero_decompress_operation &operation,
                                       int32_t numa_id) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::auto_detect>(zero_decompress_operation &operation,
                                          int32_t numa_id) -> execution_result<uint32_t, sync>;

} // namespace internal

void zero_decompress_operation::set_job_buffer(uint8_t *buffer) noexcept {
    // will be removed after ML introduction
}

} // namespace qpl
