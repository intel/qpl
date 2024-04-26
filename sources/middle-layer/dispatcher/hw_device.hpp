/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_MIDDLE_LAYER_DISPATCHER_HW_DEVICE_HPP_
#define QPL_SOURCES_MIDDLE_LAYER_DISPATCHER_HW_DEVICE_HPP_

#include <array>
#include <atomic>
#include <common/defs.hpp>

#include "qpl/c_api/defs.h"
#include "hw_queue.hpp"
#include "hw_devices.h"
#include "hw_status.h"

namespace qpl::ml::dispatcher {

#if defined( __linux__ )

class hw_device final {

    static constexpr uint32_t max_working_queues = MAX_NUM_WQ;

    using queues_container_t    = std::array<hw_queue, max_working_queues>;
    using op_config_register_t  = std::array<uint32_t, TOTAL_OP_CFG_BIT_GROUPS>;
    using opcfg_container_t     = std::array<op_config_register_t , max_working_queues>;

public:
    using descriptor_t = void;

    hw_device() noexcept = default;

    void fill_hw_context(hw_accelerator_context *hw_context_ptr) const noexcept;

    [[nodiscard]] auto enqueue_descriptor(void *desc_ptr) const noexcept -> hw_accelerator_status;

    [[nodiscard]] auto initialize_new_device(descriptor_t *device_descriptor_ptr) noexcept -> hw_accelerator_status;

    [[nodiscard]] auto size() const noexcept -> size_t;

    [[nodiscard]] auto numa_id() const noexcept -> uint64_t;

    [[nodiscard]] auto begin() const noexcept -> queues_container_t::const_iterator;

    [[nodiscard]] auto end() const noexcept -> queues_container_t::const_iterator;

    [[nodiscard]] auto get_cache_write_available() const noexcept -> bool;

    [[nodiscard]] auto get_max_set_size() const noexcept -> uint32_t;

    [[nodiscard]] auto get_max_decompressed_set_size() const noexcept -> uint32_t;

    [[nodiscard]] auto get_indexing_support_enabled() const noexcept -> uint32_t;

    [[nodiscard]] auto get_decompression_support_enabled() const noexcept -> bool;

    [[nodiscard]] auto get_max_transfer_size() const noexcept -> uint32_t;

    [[nodiscard]] auto get_cache_flush_available() const noexcept -> bool;

    [[nodiscard]] auto get_overlapping_available() const noexcept -> bool;

    [[nodiscard]] auto get_block_on_fault_available()const noexcept -> bool;

    [[nodiscard]] auto get_gen_2_min_capabilities() const noexcept -> bool;

    [[nodiscard]] auto get_header_gen_support() const noexcept -> bool;

    [[nodiscard]] auto get_dict_compress_support() const noexcept -> bool;

    [[nodiscard]] auto get_force_array_output_support() const noexcept -> bool;

    [[nodiscard]] auto get_operation_supported_on_wq(const uint32_t wq_idx, const uint32_t operation) const noexcept -> bool;

    [[nodiscard]] auto get_load_partial_aecs_support() const noexcept -> bool;

private:
    queues_container_t working_queues_   = {};    /**< Set of available HW working queues */
    opcfg_container_t  op_configs_       = {};    /**< Array of OPCFG register content for each available HW working queue */
    uint32_t           queue_count_      = 0u;    /**< Number of working queues that are available */
    uint64_t           gen_cap_register_ = 0u;    /**< GENCAP register content */
    uint64_t           iaa_cap_register_ = 0u;    /**< IAACAP register content */
    uint64_t           numa_node_id_     = 0u;    /**< NUMA node id of the device */
    uint32_t           version_major_    = 0u;    /**< Major version of discovered device */
    uint32_t           version_minor_    = 0u;    /**< Minor version of discovered device */
    bool               op_cfg_enabled_   = false; /**< Need to check workqueue's OPCFG register */
};

#endif

}
#endif //QPL_SOURCES_MIDDLE_LAYER_DISPATCHER_HW_DEVICE_HPP_
