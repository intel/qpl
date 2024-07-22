/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 03/23/2020
 * @brief Internal HW API functions for @ref hw_enqueue_descriptor API implementation
 */

#include "dispatcher/hw_dispatcher.hpp"
#include "hw_definitions.h"
#include "hw_descriptors_api.h"

extern "C" hw_accelerator_status hw_enqueue_descriptor(void* desc_ptr, int32_t user_specified_numa_id) {
    hw_accelerator_status result = HW_ACCELERATOR_WORK_QUEUES_NOT_AVAILABLE;

#if defined(__linux__)
    static auto&                      dispatcher   = qpl::ml::dispatcher::hw_dispatcher::get_instance();
    static const auto                 device_count = dispatcher.device_count();
    static thread_local std::uint32_t device_idx   = 0;

    if (device_count == 0) { return HW_ACCELERATOR_WORK_QUEUES_NOT_AVAILABLE; }

    for (uint64_t try_count = 0U; try_count < device_count; ++try_count) {
        const auto& device = dispatcher.device(device_idx);
        device_idx         = (device_idx + 1) % device_count;

        if (!device.is_matching_user_numa_policy(user_specified_numa_id)) { continue; }

        hw_iaa_descriptor_hint_cpu_cache_as_destination((hw_descriptor*)desc_ptr, device.get_cache_write_available());

        const hw_accelerator_status enqueue_result = device.enqueue_descriptor(desc_ptr);
        if (enqueue_result == HW_ACCELERATOR_NOT_SUPPORTED_BY_WQ &&
            result == HW_ACCELERATOR_WORK_QUEUES_NOT_AVAILABLE) {
            result = HW_ACCELERATOR_NOT_SUPPORTED_BY_WQ;
        } else if (enqueue_result == HW_ACCELERATOR_WQ_IS_BUSY) {
            result = HW_ACCELERATOR_WQ_IS_BUSY;
        } else if (enqueue_result == HW_ACCELERATOR_STATUS_OK) {
            result = HW_ACCELERATOR_STATUS_OK;
            break;
        }
    }
#else
    // Not supported on Windows yet
    return HW_ACCELERATOR_SUPPORT_ERR;
#endif

    return result;
}

extern "C" hw_accelerator_status
hw_accelerator_submit_descriptor(hw_accelerator_context* const        UNREFERENCED_PARAMETER(accel_context_ptr),
                                 const hw_descriptor* const           descriptor_ptr,
                                 hw_accelerator_submit_options* const submit_options) {
    return hw_enqueue_descriptor((void*)descriptor_ptr, submit_options->numa_id);
}
