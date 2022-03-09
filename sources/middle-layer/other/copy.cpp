/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "copy.hpp"
#include "util/descriptor_processing.hpp"
#include "util/memory.hpp"

#include "hw_descriptors_api.h"

namespace qpl::ml::other {

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage=4096"
#endif

template <>
auto call_copy<execution_path_t::hardware>(const uint8_t *src_ptr,
                                           uint8_t *dst_ptr,
                                           uint32_t length,
                                           int32_t numa_id) noexcept -> copy_operation_result_t {
    HW_PATH_VOLATILE hw_completion_record HW_PATH_ALIGN_STRUCTURE completion_record{};
    hw_descriptor HW_PATH_ALIGN_STRUCTURE                         descriptor{};

    hw_iaa_descriptor_init_mem_copy(&descriptor,
                                    src_ptr,
                                    dst_ptr,
                                    length);

    return util::process_descriptor<copy_operation_result_t, util::execution_mode_t::sync>(&descriptor,
                                                                                           &completion_record,
                                                                                           numa_id);
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

template <>
auto call_copy<execution_path_t::software>(const uint8_t *src_ptr,
                                           uint8_t *dst_ptr,
                                           uint32_t length,
                                           int32_t numa_id) noexcept -> copy_operation_result_t {

    copy_operation_result_t operation_result{};
    uint32_t                status_code = status_list::ok;

    util::copy(src_ptr, src_ptr + length, dst_ptr);

    operation_result.status_code_  = status_code;
    operation_result.copied_bytes_ = length;

    return operation_result;
}

template <>
auto call_copy<execution_path_t::auto_detect>(const uint8_t *src_ptr,
                                              uint8_t *dst_ptr,
                                              uint32_t length,
                                              int32_t numa_id) noexcept -> copy_operation_result_t {
    auto hw_result = call_copy<execution_path_t::hardware>(src_ptr,
                                                           dst_ptr,
                                                           length,
                                                           numa_id);

    if (hw_result.status_code_ != status_list::ok) {
        return call_copy<execution_path_t::software>(src_ptr,
                                                     dst_ptr,
                                                     length);
    }

    return hw_result;
}

} // namespace qpl::ml::other
