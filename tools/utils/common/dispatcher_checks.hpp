/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TOOLS_UTILS_COMMON_DISPATCHER_CHECKS_HPP_
#define QPL_TOOLS_UTILS_COMMON_DISPATCHER_CHECKS_HPP_

// tool_hw_dispatcher
#include "test_hw_dispatcher.hpp"

// tool_common
#include "topology.hpp"

namespace qpl::test {

inline bool are_workqueues_available(int32_t numa_id = -1) {
#if defined(__linux__)
    if (numa_id == -1) { numa_id = get_numa_id(); }

    static auto& dispatcher = hw_dispatcher::get_instance();
    if (dispatcher.is_hw_support()) {
        for (size_t device_idx = 0; device_idx < dispatcher.device_count(); device_idx++) {
            if (dispatcher.device(device_idx).numa_id() != numa_id) { continue; }
            if (dispatcher.device(device_idx).size() != 0) { return true; }
        }
    }
#endif

    return false;
}

/**
 * @brief Returns the maximum number of descriptor submissions that can be made to the dispatcher
 *        before the workqueues are full.
 * @param numa_id NUMA node id
 * @return Maximum number of descriptor submissions
 */
inline uint64_t max_descriptor_submissions(int32_t numa_id = -1) {
    uint64_t sum_descriptor_size = 0;

#if defined(__linux__)
    // found through testing different wq sizes & different # of enabled engines.
    constexpr uint32_t additional_jobs_per_engine[9] = {0, 33, 66, 99, 132, 165, 173, 181, 188};

    if (numa_id == -1) { numa_id = get_numa_id(); }

    static auto& dispatcher = hw_dispatcher::get_instance();
    if (dispatcher.is_hw_support()) {
        for (size_t device_idx = 0; device_idx < dispatcher.device_count(); device_idx++) {
            if (dispatcher.device(device_idx).numa_id() != numa_id) { continue; }
            for (auto wq_itr = dispatcher.device(device_idx).begin(); wq_itr != dispatcher.device(device_idx).end();
                 wq_itr++) {
                sum_descriptor_size += wq_itr->get_size();
            }
            uint32_t engines_for_device = dispatcher.device(device_idx).get_engine_count();
            if (engines_for_device > 8) { engines_for_device = 8; }
            sum_descriptor_size += additional_jobs_per_engine[engines_for_device];
        }
    }
#endif

    return sum_descriptor_size;
}

} // namespace qpl::test

#endif // QPL_TOOLS_UTILS_COMMON_DISPATCHER_CHECKS_HPP_
