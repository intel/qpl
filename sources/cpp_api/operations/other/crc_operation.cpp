/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/qpl.h"

#include "other/crc.hpp"
#include "qpl/cpp_api/operations/other/crc_operation.hpp"

namespace qpl {
namespace internal {

template <execution_path path>
auto execute(crc_operation &operation, int32_t numa_id) -> execution_result<uint32_t, sync> {
    auto next_in_ptr   = const_cast<uint8_t *>(operation.source_);
    auto available_in  = static_cast<uint32_t>(operation.source_size_);
    auto next_out_ptr  = const_cast<uint8_t *>(operation.destination_);
    auto available_out = static_cast<uint32_t>(operation.destination_size_);
    auto polynomial    = operation.polynomial_;

    if (available_out < 8) {
        return execution_result<uint32_t, sync>(QPL_STS_DST_IS_SHORT_ERR, 0);
    } else if (polynomial == 0) {
        return execution_result<uint32_t, sync>(QPL_STS_CRC64_BAD_POLYNOM, 0);;
    }

    constexpr auto actual_path = static_cast<ml::execution_path_t>(path);

    auto crc_result = qpl::ml::other::call_crc<actual_path>(next_in_ptr,
                                                            available_in,
                                                            polynomial,
                                                            operation.bit_order_ == big_endian_packed_array,
                                                            operation.is_inverse_,
                                                            numa_id);

    *reinterpret_cast<uint64_t *>(next_out_ptr) = crc_result.crc_;

    return execution_result<uint32_t, sync>{crc_result.status_code_, crc_result.processed_bytes_};
}

template
auto execute<execution_path::software>(crc_operation &operation, int32_t numa_id) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::hardware>(crc_operation &operation, int32_t numa_id) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::auto_detect>(crc_operation &operation,
                                          int32_t numa_id) -> execution_result<uint32_t, sync>;

} // namespace qpl::internal

void crc_operation::set_job_buffer(uint8_t * /* buffer */) noexcept {
    // will be removed after ML introduction
}

auto crc_operation::crc_operation_builder::is_inverse(bool value) -> crc_operation_builder & {
    operation_builder::operation_.is_inverse_ = value;

    return *this;
}
} // namespace qpl
