/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "algorithm"

#if defined( __linux__ )

#include <inttypes.h>

#include "hw_device.hpp"
#include "hw_descriptors_api.h"

#ifdef DYNAMIC_LOADING_LIBACCEL_CONFIG
#include "hw_configuration_driver.h"
#else //DYNAMIC_LOADING_LIBACCEL_CONFIG=OFF
#include "hw_devices.h"
#include "hw_definitions.h"
#include "libaccel_config.h"
#endif //DYNAMIC_LOADING_LIBACCEL_CONFIG

constexpr uint8_t  accelerator_name[]      = "iax";                         /**< Accelerator name */
constexpr uint32_t accelerator_name_length = sizeof(accelerator_name) - 2U; /**< Last symbol index */

static inline bool own_search_device_name(const uint8_t *src_ptr,
                                          const uint32_t name,
                                          const uint32_t name_size) noexcept {
    const uint8_t null_terminator = '\0';

    for (size_t symbol_idx = 0U; null_terminator != src_ptr[symbol_idx + name_size]; symbol_idx++) {
        const auto *candidate_ptr = reinterpret_cast<const uint32_t *>(src_ptr + symbol_idx);

        // Convert the first 3 bytes to lower case and make the 4th 0xff
        if (name == (*candidate_ptr | CHAR_MSK)) {
            return true;
        }
    }

    return false;
}

namespace qpl::ml::dispatcher {

/**
 * @brief Routine to restore device properties.
*/
void hw_device::fill_hw_context(hw_accelerator_context *const hw_context_ptr) const noexcept {
    // GENCAP-related
    hw_context_ptr->device_properties.max_set_size                  = hw_device::get_max_set_size();
    hw_context_ptr->device_properties.max_decompressed_set_size     = hw_device::get_max_decompressed_set_size();
    hw_context_ptr->device_properties.indexing_support_enabled      = hw_device::get_indexing_support_enabled();
    hw_context_ptr->device_properties.decompression_support_enabled = hw_device::get_decompression_support_enabled();
    hw_context_ptr->device_properties.max_transfer_size             = hw_device::get_max_transfer_size();
    hw_context_ptr->device_properties.cache_flush_available         = hw_device::get_cache_flush_available();
    hw_context_ptr->device_properties.cache_write_available         = hw_device::get_cache_write_available();
    hw_context_ptr->device_properties.overlapping_available         = hw_device::get_overlapping_available();
    hw_context_ptr->device_properties.block_on_fault_enabled        = hw_device::get_block_on_fault_available();

    // IAACAP-related
    hw_context_ptr->device_properties.gen_2_min_capabilities_available          = hw_device::get_gen_2_min_capabilities();
    hw_context_ptr->device_properties.header_gen_supported                      = hw_device::get_header_gen_support();
    hw_context_ptr->device_properties.dict_compression_supported                = hw_device::get_dict_compress_support();
    hw_context_ptr->device_properties.load_partial_aecs_supported               = hw_device::get_load_partial_aecs_support();
    hw_context_ptr->device_properties.force_array_output_mod_available          = hw_device::get_force_array_output_support();
}

auto hw_device::enqueue_descriptor(void *desc_ptr) const noexcept -> hw_accelerator_status {
    static thread_local std::uint32_t wq_idx = 0;
    bool is_op_supported_by_wq = false;
    uint32_t operation = hw_iaa_descriptor_get_operation((hw_descriptor *)desc_ptr);

    // For small low-latency cases WQ with small transfer size may be preferable
    // TODO: order WQs by priority and engines capacity, check transfer sizes and other possible features
    for (uint64_t try_count = 0U; try_count < queue_count_; ++try_count) {
        hw_iaa_descriptor_set_block_on_fault((hw_descriptor *) desc_ptr, working_queues_[wq_idx].get_block_on_fault());
        // If OPCFG functionality exists, check OPCFG register before submitting, otherwise try submission
        if ( !op_cfg_enabled_ || get_operation_supported_on_wq(wq_idx, operation)){
            // For submitting when OPCFG is supported, logic is :
            //   If all WQs don't support operation, return HW_ACCELERATOR_NOT_SUPPORTED_BY_WQ
            //   If any WQ supports operation, but submission fails, then return HW_ACCELERATOR_WQ_IS_BUSY
            qpl_status enqueue_status = working_queues_[wq_idx].enqueue_descriptor(desc_ptr);
            is_op_supported_by_wq = true;
            if (QPL_STS_OK == enqueue_status) {
                return HW_ACCELERATOR_STATUS_OK;
            }
        }
        wq_idx = (wq_idx+1) % queue_count_;
    }
    if (!is_op_supported_by_wq) {
        return HW_ACCELERATOR_NOT_SUPPORTED_BY_WQ;
    }
    else {
        return HW_ACCELERATOR_WQ_IS_BUSY;
    }
}

auto hw_device::get_max_set_size() const noexcept -> uint32_t {
    return GC_MAX_SET_SIZE(gen_cap_register_);
}

auto hw_device::get_max_decompressed_set_size() const noexcept -> uint32_t {
    return GC_MAX_DECOMP_SET_SIZE(gen_cap_register_);
}

auto hw_device::get_indexing_support_enabled() const noexcept -> uint32_t {
    return GC_IDX_SUPPORT(gen_cap_register_);
}

auto hw_device::get_decompression_support_enabled() const noexcept -> bool {
    return GC_DECOMP_SUPPORT(gen_cap_register_);
}

auto hw_device::get_max_transfer_size() const noexcept -> uint32_t {
    return GC_MAX_TRANSFER_SIZE(gen_cap_register_);
}

auto hw_device::get_cache_flush_available() const noexcept -> bool {
    return GC_CACHE_FLUSH(gen_cap_register_);
}

auto hw_device::get_cache_write_available() const noexcept -> bool {
    return GC_CACHE_WRITE(gen_cap_register_);
}

auto hw_device::get_overlapping_available() const noexcept -> bool {
    return GC_OVERLAPPING(gen_cap_register_);
}

auto hw_device::get_block_on_fault_available() const noexcept -> bool {
    return GC_BLOCK_ON_FAULT(gen_cap_register_);
}

auto hw_device::get_gen_2_min_capabilities() const noexcept -> bool {
    return IC_GEN_2_MIN_CAP(iaa_cap_register_);
}

auto hw_device::get_header_gen_support() const noexcept -> bool {
    return IC_HEADER_GEN(iaa_cap_register_);
}

auto hw_device::get_dict_compress_support() const noexcept -> bool {
    return IC_DICT_COMP(iaa_cap_register_);
}

auto hw_device::get_force_array_output_support() const noexcept -> bool {
    return IC_FORCE_ARRAY(iaa_cap_register_);
}

auto hw_device::get_operation_supported_on_wq(const uint32_t wq_idx, const uint32_t operation) const noexcept -> bool {
    return OC_GET_OP_SUPPORTED(op_configs_[wq_idx], operation);
}

auto hw_device::get_load_partial_aecs_support() const noexcept -> bool {
    return IC_LOAD_PARTIAL_AECS(iaa_cap_register_);
}

/**
 * @brief Function to query device and check its properties.
 * Returns HW_ACCELERATOR_STATUS_OK upon success and HW_ACCELERATOR_WORK_QUEUES_NOT_AVAILABLE for invalid device.
 *
 * @note Special cases are Intel® In-Memory Analytics Accelerator (Intel® IAA) generation 2.0,
 * where IAACAP is expected.
 * Error code HW_ACCELERATOR_LIBACCEL_NOT_FOUND is returned if libaccel doesn't have API for reading IAACAP
 * and error code HW_ACCELERATOR_SUPPORT_ERR is returned if IAACAP couldn't be read.
*/
auto hw_device::initialize_new_device(descriptor_t *device_descriptor_ptr) noexcept -> hw_accelerator_status {
    // Device initialization stage
    auto       *device_ptr          = reinterpret_cast<accfg_device *>(device_descriptor_ptr);
    const auto *name_ptr            = reinterpret_cast<const uint8_t *>(accfg_device_get_devname(device_ptr));
    const bool  is_iaa_device       = own_search_device_name(name_ptr, IAA_DEVICE, accelerator_name_length);

    version_major_ = accfg_device_get_version(device_ptr)>>8U;
    version_minor_ = accfg_device_get_version(device_ptr)&0xFF;

    DIAG("%5s: ", name_ptr);
    if (!is_iaa_device) {
        DIAGA("UNSUPPORTED %5s\n", name_ptr);
        return HW_ACCELERATOR_WORK_QUEUES_NOT_AVAILABLE;
    }
    if (ACCFG_DEVICE_ENABLED != accfg_device_get_state(device_ptr)) {
        DIAGA("DISABLED %5s\n", name_ptr);
        return HW_ACCELERATOR_WORK_QUEUES_NOT_AVAILABLE;
    }
    DIAGA("\n");

    gen_cap_register_ = accfg_device_get_gen_cap(device_ptr);
    numa_node_id_     = accfg_device_get_numa_node(device_ptr);

    DIAG("%5s: version: %d.%d\n", name_ptr, version_major_, version_minor_);
    DIAG("%5s: numa: %" PRIu64 "\n", name_ptr, numa_node_id_);
    DIAG("%5s: GENCAP: %" PRIu64 "\n", name_ptr, gen_cap_register_);
    DIAG("%5s: GENCAP: block on fault support:              %d\n",          name_ptr, get_block_on_fault_available());
    DIAG("%5s: GENCAP: overlapping copy support:            %d\n",          name_ptr, get_overlapping_available());
    DIAG("%5s: GENCAP: cache control support (memory):      %d\n",          name_ptr, get_cache_write_available());
    DIAG("%5s: GENCAP: cache control support (cache flush): %d\n",          name_ptr, get_cache_flush_available());
    DIAG("%5s: GENCAP: maximum supported transfer size:     %" PRIu32 "\n", name_ptr, get_max_transfer_size());
    DIAG("%5s: GENCAP: decompression support:               %d\n",          name_ptr, get_decompression_support_enabled());
    DIAG("%5s: GENCAP: indexing support:                    %d\n",          name_ptr, get_indexing_support_enabled());
    DIAG("%5s: GENCAP: maximum decompression set size:      %d\n",          name_ptr, get_max_decompressed_set_size());
    DIAG("%5s: GENCAP: maximum set size:                    %d\n",          name_ptr, get_max_set_size());

    // Retrieve IAACAP if available
    uint64_t iaa_cap = 0U;
    int32_t get_iaa_cap_status = accfg_device_get_iaa_cap(device_ptr, &iaa_cap);
    if (get_iaa_cap_status) {
        // @todo this is a workaround to optionally load accfg_device_get_iaa_cap
        DIAGA("%5s: IAACAP: Failed to read IAACAP, HW gen 2 features will not be used\n", name_ptr);

        if (version_major_ >= 2U) {
            // If function for reading IAACAP couldn't be loaded (accfg_device_get_iaa_cap returns positive 1 error code),
            // then exit with HW_ACCELERATOR_LIBACCEL_NOT_FOUND.
            // This case means that the libaccel used doesn't have required API.
            if (get_iaa_cap_status == 1) {
                return HW_ACCELERATOR_LIBACCEL_NOT_FOUND;
            }
            // If IAACAP couldn't be read and libaccel returns negative error code, then exit with HW_ACCELERATOR_SUPPORT_ERR.
            // This case might mean that the kernel version is too old (IAACAP register was not exposed yet).
            else {
                return HW_ACCELERATOR_SUPPORT_ERR;
            }
        }
    }

    iaa_cap_register_ = iaa_cap;
    DIAG("%5s: IAACAP: %" PRIu64 "\n", name_ptr, iaa_cap_register_);

    DIAG("%5s: IAACAP: generation 2 minimum capabilities:   %d\n",          name_ptr, get_gen_2_min_capabilities());
    DIAG("%5s: IAACAP: load partial AECS support:           %d\n",          name_ptr, get_load_partial_aecs_support());
    DIAG("%5s: IAACAP: header generation support:           %d\n",          name_ptr, get_header_gen_support());
    DIAG("%5s: IAACAP: dictionary compression support:      %d\n",          name_ptr, get_dict_compress_support());

    // Working queues initialization stage
    auto *wq_ptr = accfg_wq_get_first(device_ptr);
    auto wq_it   = working_queues_.begin();

    DIAG("%5s: getting device WQs\n", name_ptr);
    while (nullptr != wq_ptr) {
        if (HW_ACCELERATOR_STATUS_OK == wq_it->initialize_new_queue(wq_ptr)) {
            wq_it++;

            std::push_heap(working_queues_.begin(), wq_it,
                           [](const hw_queue &a, const hw_queue &b) -> bool {
                               return a.priority() < b.priority();
                           });
        }

        wq_ptr = accfg_wq_get_next(wq_ptr);
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

    if (queue_count_ == 0) {
        return HW_ACCELERATOR_WORK_QUEUES_NOT_AVAILABLE;
    }

    // Logic for op_cfg_enabled_ value
    op_cfg_enabled_ = working_queues_[0].get_op_configuration_support();

    for (uint32_t wq_idx = 0; wq_idx < queue_count_; wq_idx++) {
        for (uint32_t register_index = 0 ; register_index < TOTAL_OP_CFG_BIT_GROUPS; register_index++) {
            op_configs_[wq_idx] = working_queues_[wq_idx].get_op_config_register();
        }
    }


    return HW_ACCELERATOR_STATUS_OK;
}

auto hw_device::size() const noexcept -> size_t {
    return queue_count_;
}

auto hw_device::numa_id() const noexcept -> uint64_t {
    return numa_node_id_;
}

auto hw_device::begin() const noexcept -> queues_container_t::const_iterator {
    return working_queues_.cbegin();
}

auto hw_device::end() const noexcept -> queues_container_t::const_iterator {
    return working_queues_.cbegin() + queue_count_;
}

}

#endif //__linux__
