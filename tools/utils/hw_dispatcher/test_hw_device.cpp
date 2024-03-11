/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 * HW device APIs for tests
 */

#if defined( __linux__ )

#include "test_hw_device.hpp"

static const uint8_t  accelerator_name[]      = "iax";                         /**< Accelerator name */
static const uint32_t accelerator_name_length = sizeof(accelerator_name) - 2U; /**< Last symbol index */


/**
 * @brief Function to check if the device is an Intel® In-Memory Analytics Accelerator (Intel® IAA) device.
 *
 * @param src_ptr Name of current device
 * @param name Intel IAA device name (in lower case)
 * @param name_size Last symbol index of Intel IAA device name
 * @return bool Indicates if there is a match with the specified name
*/
static inline bool own_search_device_name(const uint8_t *src_ptr,
                                          const uint32_t name,
                                          const uint32_t name_size) noexcept {
    const uint8_t null_terminator = '\0';

    for (size_t symbol_idx = 0U; null_terminator != src_ptr[symbol_idx + name_size]; symbol_idx++) {
        const auto *candidate_ptr = reinterpret_cast<const uint32_t *>(src_ptr + symbol_idx);

        // Convert the first 3 bytes to lower case and make the 4th 0xff
        if (name == (*candidate_ptr | QPL_TEST_CHAR_MSK)) {
            return true;
        }
    }

    return false;
}

namespace qpl::test {

auto hw_device::get_gen_2_min_capabilities() const noexcept -> bool {
    return QPL_TEST_IC_GEN_2_MIN_CAP(iaa_cap_register_);
}

auto hw_device::get_dict_compress_support() const noexcept -> bool {
    return QPL_TEST_IC_DICT_COMP(iaa_cap_register_);
}

auto hw_device::numa_id() const noexcept -> uint64_t {
    return numa_node_id_;
}

auto hw_device::initialize_new_device(descriptor_t *device_descriptor_ptr) noexcept -> qpl_test_hw_accelerator_status {
    // Device initialization stage
    auto       *device_ptr          = reinterpret_cast<accfg_device *>(device_descriptor_ptr);
    const auto *name_ptr            = reinterpret_cast<const uint8_t *>(qpl_test_accfg_device_get_devname(device_ptr));
    const bool  is_iaa_device       = own_search_device_name(name_ptr, QPL_TEST_IAA_DEVICE, accelerator_name_length);

    version_major_ = qpl_test_accfg_device_get_version(device_ptr)>>8U;
    version_minor_ = qpl_test_accfg_device_get_version(device_ptr)&0xFF;

    if (!is_iaa_device) {
        return QPL_TEST_HW_ACCELERATOR_WORK_QUEUES_NOT_AVAILABLE;
    }
    if (ACCFG_DEVICE_ENABLED != qpl_test_accfg_device_get_state(device_ptr)) {
        return QPL_TEST_HW_ACCELERATOR_WORK_QUEUES_NOT_AVAILABLE;
    }

    // Retrieve IAACAP if available
    uint64_t iaa_cap = 0U;
    int32_t get_iaa_cap_status = qpl_test_accfg_device_get_iaa_cap(device_ptr, &iaa_cap);
    if (get_iaa_cap_status) {
        // @todo this is a workaround to optionally load accfg_device_get_iaa_cap
        if (version_major_ > 1U) {
            return QPL_TEST_HW_ACCELERATOR_LIBACCEL_NOT_FOUND;
        }
    }

    iaa_cap_register_ = iaa_cap;

    numa_node_id_     = qpl_test_accfg_device_get_numa_node(device_ptr);

    return QPL_TEST_HW_ACCELERATOR_STATUS_OK;
}

}

#endif //__linux__
