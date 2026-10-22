/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TOOLS_UTILS_COMMON_DISPATCHER_CHECKS_HPP
#define QPL_TOOLS_UTILS_COMMON_DISPATCHER_CHECKS_HPP

#include <vector>

#include "qpl/qpl.h"

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

inline uint64_t get_max_transfer_size() {
    uint64_t      max_transfer_size = 0;
    const int32_t numa_id           = get_numa_id();

#if defined(__linux__)
    static auto& dispatcher = hw_dispatcher::get_instance();
    if (dispatcher.is_hw_support()) {
        for (size_t device_idx = 0; device_idx < dispatcher.device_count(); device_idx++) {
            if (dispatcher.device(device_idx).numa_id() != numa_id) { continue; }
            max_transfer_size = std::max(max_transfer_size, dispatcher.device(device_idx).get_max_transfer_size());
        }
    }
#endif

    return max_transfer_size;
}

/**
 * @brief Checks whether the accelerator is actually usable by this process.
 * Work queues can be listed yet unusable (e.g. no SVA/permissions), where jobs
 * silently run on software. Probes once by running a real compress on the
 * hardware path, which does not fall back: a QPL_STS_OK or queue-busy result
 * means the accelerator is truly usable. The result is cached across calls.
 */
inline bool is_hardware_execution_available() {
    static const bool available = [] {
        uint32_t job_size = 0U;
        if (qpl_get_job_size(qpl_path_hardware, &job_size) != QPL_STS_OK) { return false; }

        std::vector<uint8_t> job_buffer(job_size);
        auto*                job = reinterpret_cast<qpl_job*>(job_buffer.data());
        if (qpl_init_job(qpl_path_hardware, job) != QPL_STS_OK) { return false; }

        const std::vector<uint8_t> source(64U, 'a');
        std::vector<uint8_t>       destination(256U);

        job->op            = qpl_op_compress;
        job->level         = qpl_default_level;
        job->next_in_ptr   = const_cast<uint8_t*>(source.data());
        job->available_in  = static_cast<uint32_t>(source.size());
        job->next_out_ptr  = destination.data();
        job->available_out = static_cast<uint32_t>(destination.size());
        job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY | QPL_FLAG_DYNAMIC_HUFFMAN;

        const qpl_status status = qpl_execute_job(job);
        qpl_fini_job(job);

        // Queue contention is transient, the accelerator is still usable
        return status == QPL_STS_OK || status == QPL_STS_QUEUES_ARE_BUSY_ERR;
    }();
    return available;
}

/**
 * @brief Effective execution target for a job path: software for qpl_path_software,
 * otherwise the hardware path when the accelerator is actually usable (else auto
 * falls back to software).
 */
inline bool is_effective_hardware(qpl_path_t path) {
    if (path == qpl_path_software) { return false; }
    return is_hardware_execution_available();
}

} // namespace qpl::test

#endif // QPL_TOOLS_UTILS_COMMON_DISPATCHER_CHECKS_HPP
