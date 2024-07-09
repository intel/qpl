/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 * HW device APIs for tests
 */

#if defined( __linux__ )

#include <algorithm>
#include <queue>

#include "test_hw_device.hpp"

constexpr uint8_t  accelerator_name[]      = "iax";                         /**< Accelerator name */
constexpr uint32_t accelerator_name_length = sizeof(accelerator_name) - 2U; /**< Last symbol index */


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

// Force Array Output Modification Support
auto hw_device::get_force_array_output_mod_support() const noexcept -> bool {
    return QPL_TEST_IC_FORCE_ARRAY_OUTPUT_MOD(iaa_cap_register_);
}

auto hw_device::numa_id() const noexcept -> uint64_t {
    return numa_node_id_;
}

auto hw_device::get_opcfg_enabled() const noexcept -> bool {
    return op_cfg_enabled_;
}

auto hw_device::get_operation_supported_on_wq(const uint32_t wq_idx, const uint32_t operation) const noexcept -> bool {
    return QPL_TEST_OC_GET_OP_SUPPORTED(op_configs_[wq_idx], operation);
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

    numa_node_id_     = qpl_test_accfg_device_get_numa_node(device_ptr);

    // Retrieve IAACAP if available
    uint64_t iaa_cap = 0U;
    const int32_t get_iaa_cap_status = qpl_test_accfg_device_get_iaa_cap(device_ptr, &iaa_cap);
    if (get_iaa_cap_status) {
        // @todo this is a workaround to optionally load qpl_test_accfg_device_get_iaa_cap
        if (version_major_ > 1U) {
            return QPL_TEST_HW_ACCELERATOR_LIBACCEL_NOT_FOUND;
        }
    }

    iaa_cap_register_ = iaa_cap;

    // Working queues initialization stage
    auto *wq_ptr = qpl_test_accfg_wq_get_first(device_ptr);
    auto wq_it   = working_queues_.begin();

    while (nullptr != wq_ptr) {
        if (QPL_TEST_HW_ACCELERATOR_STATUS_OK == wq_it->initialize_new_queue(wq_ptr)) {
            wq_it++;

            std::push_heap(working_queues_.begin(), wq_it,
                           [](const hw_queue &a, const hw_queue &b) -> bool {
                               return a.priority() < b.priority();
                           });
        }

        wq_ptr = qpl_test_accfg_wq_get_next(wq_ptr);
    }

    // Check number of working queues
    queue_count_ = std::distance(working_queues_.begin(), wq_it);

    if (queue_count_ > 1) {
        auto begin = working_queues_.begin();
        auto end   = begin + queue_count_;

        std::sort_heap(begin, end, [](const hw_queue &a, const hw_queue &b) -> bool {
            return a.priority() < b.priority();
        });
    }

    // Logic for op_cfg_enabled_ value
    op_cfg_enabled_ = working_queues_[0].get_op_configuration_support();

    for (uint32_t wq_idx = 0; wq_idx < queue_count_; wq_idx++) {
        for (uint32_t register_index = 0 ; register_index < QPL_TEST_TOTAL_OP_CFG_BIT_GROUPS; register_index++) {
            op_configs_[wq_idx] = working_queues_[wq_idx].get_op_config_register();
        }
    }

    return QPL_TEST_HW_ACCELERATOR_STATUS_OK;
}

auto hw_device::size() const noexcept -> size_t {
    return queue_count_;
}

auto hw_device::begin() const noexcept -> queues_container_t::const_iterator {
    return working_queues_.cbegin();
}

auto hw_device::end() const noexcept -> queues_container_t::const_iterator {
    return working_queues_.cbegin() + queue_count_;
}

}

#endif //__linux__
