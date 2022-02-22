/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/qpl.h"

#include "other/copy.hpp"
#include "qpl/cpp_api/operations/other/copy_operation.hpp"
#include "util/checkers.hpp"

namespace qpl {
namespace internal {

template <execution_path path>
auto execute(copy_operation &operation, int32_t numa_id) -> execution_result<uint32_t, sync> {
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

    if (available_in == 0 || available_out == 0) {
        return execution_result<uint32_t, sync>(QPL_STS_SIZE_ERR, 0);
    } else if (available_out < available_in) {
        return execution_result<uint32_t, sync>(QPL_STS_DST_IS_SHORT_ERR, 0);
    }

    constexpr auto actual_path = static_cast<ml::execution_path_t>(path);

    auto copy_result = qpl::ml::other::call_copy<actual_path>(next_in_ptr,
                                                              next_out_ptr,
                                                              available_in,
                                                              numa_id);

    return execution_result<uint32_t, sync>(copy_result.status_code_, copy_result.copied_bytes_);
}

template
auto execute<execution_path::software>(copy_operation &operation, int32_t numa_id) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::hardware>(copy_operation &operation, int32_t numa_id) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::auto_detect>(copy_operation &operation,
                                          int32_t numa_id) -> execution_result<uint32_t, sync>;

} // namespace qpl::internal

void copy_operation::set_job_buffer(uint8_t *buffer) noexcept {
    // will be removed after ML introduction
}

} // namespace qpl
