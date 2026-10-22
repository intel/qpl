/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_MIDDLE_LAYER_DISPATCHER_HW_DEVICE_HPP
#define QPL_SOURCES_MIDDLE_LAYER_DISPATCHER_HW_DEVICE_HPP

#include <array>
#include <atomic>
#include <common/defs.hpp>

#include "qpl/c_api/defs.h"

// core-iaa
#include "hw_devices.h"
#include "hw_queue.hpp"
#include "hw_status.h"

// ml
#include "queue_selector.hpp"
#include "util/hw_timing_util.hpp"

namespace qpl::ml::dispatcher {

#if defined(__linux__)

class hw_device final {

    static constexpr uint32_t max_working_queues = MAX_NUM_WQ;

    using queues_container_t = std::array<hw_queue, max_working_queues>;

public:
    using descriptor_t = void;

    hw_device() noexcept = default;

    void fill_hw_context(hw_accelerator_context* hw_context_ptr) const noexcept;

    [[nodiscard]] auto enqueue_descriptor(void* desc_ptr, qpl::ml::util::execution_record_ext_t* record) const noexcept
            -> hw_accelerator_status;

    [[nodiscard]] auto initialize_new_device(descriptor_t* device_descriptor_ptr) noexcept -> hw_accelerator_status;

    [[nodiscard]] auto size() const noexcept -> size_t;

    [[nodiscard]] auto numa_id() const noexcept -> uint64_t;

    [[nodiscard]] auto socket_id() const noexcept -> uint64_t;

    [[nodiscard]] auto begin() const noexcept -> queues_container_t::const_iterator;

    [[nodiscard]] auto end() const noexcept -> queues_container_t::const_iterator;

    [[nodiscard]] auto get_cache_write_available() const noexcept -> bool;

    [[nodiscard]] auto get_indexing_support_enabled() const noexcept -> uint32_t;

    [[nodiscard]] auto get_decompression_support_enabled() const noexcept -> bool;

    [[nodiscard]] auto get_max_transfer_size() const noexcept -> uint32_t;

    [[nodiscard]] auto get_cache_flush_available() const noexcept -> bool;

    [[nodiscard]] auto get_overlapping_available() const noexcept -> bool;

    [[nodiscard]] auto get_block_on_fault_available() const noexcept -> bool;

    [[nodiscard]] auto get_gen_2_min_capabilities() const noexcept -> bool;

    [[nodiscard]] auto get_gen_3_min_capabilities() const noexcept -> bool;

    [[nodiscard]] auto get_header_gen_support() const noexcept -> bool;

    [[nodiscard]] auto get_dict_compress_support() const noexcept -> bool;

    [[nodiscard]] auto get_force_array_output_support() const noexcept -> bool;

    [[nodiscard]] auto get_load_partial_aecs_support() const noexcept -> bool;

    [[nodiscard]] auto is_matching_user_numa_policy(int32_t user_specified_numa_id) const noexcept -> bool;

private:
    queues_container_t working_queues_   = {}; /**< Set of available HW working queues */
    uint32_t           queue_count_      = 0U; /**< Number of working queues that are available */
    uint64_t           gen_cap_register_ = 0U; /**< GENCAP register content */
    uint64_t           iaa_cap_register_ = 0U; /**< IAACAP register content */
    uint64_t           numa_node_id_     = 0U; /**< NUMA node id of the device */
    uint32_t           version_major_    = 0U; /**< Major version of discovered device */
    uint32_t           version_minor_    = 0U; /**< Minor version of discovered device */
    uint64_t           socket_id_        = 0U; /**< Socket id of the device */
    queue_selector     queue_selection_;       /**< Queue Selection object */
};

#endif

} // namespace qpl::ml::dispatcher
#endif //QPL_SOURCES_MIDDLE_LAYER_DISPATCHER_HW_DEVICE_HPP
