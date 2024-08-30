/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <benchmark/benchmark.h>

#include "ops/ops.hpp"
#include "topology.hpp"
#include "utility.hpp"

//#define PER_THREAD_STAT
#ifdef PER_THREAD_STAT
#include <chrono>
#include <mutex>
#endif

#include <stdexcept>

namespace bench::details {
template <path_e path, typename OperationT, typename ParamsT>
static statistics_t measure_async(benchmark::State& state, const case_params_t& common_params, OperationT&& operations,
                                  ParamsT&& params) {
    statistics_t res;
    auto         threads = state.threads();
    if constexpr (path == path_e::cpu) {
        res.queue_size = 1;
        res.operations = threads;
    } else {
        std::uint32_t devices = get_number_of_devices_matching_numa_policy(common_params.node_);
        res.queue_size        = common_params.queue_size_;
        res.operations        = res.queue_size * devices;
    }

    res.operations_per_thread = res.operations / threads;
    if (res.operations_per_thread < 1) throw std::runtime_error("Operation pool is too small for given threads");

    operations.resize(res.operations_per_thread);
    for (auto& operation : operations) {
        operation.init(params, common_params.node_, get_mem_cc(common_params.out_mem_), common_params.full_time_);
        operation.mem_control(common_params.in_mem_, mem_loc_mask_e::src);
    }

    // Strategies:
    // - File at once. Each operation works on same file independently.
    // - Chunk at once. Measure each chunk independently one by one, gather aggregate in the end. Is this reasonable?
    // - File by chunks. Measure for the whole file processing different chunks in parallel (map file before processing). Like normal processing

    size_t completion_limit = res.operations_per_thread; // Do at least qdepth tasks for each iteration
    bool   first_iteration  = true;
#ifdef PER_THREAD_STAT
    std::chrono::high_resolution_clock::time_point timer_start;
    std::chrono::high_resolution_clock::time_point timer;
    timer_start         = std::chrono::high_resolution_clock::now();
    std::uint64_t polls = 0;
#endif

    for (auto _ : state) {
        if (first_iteration) {
            for (auto& operation : operations) {
                operation.async_submit();
            }
            first_iteration = false;
        }

        size_t completed = 0;
        while (completed < completion_limit) {
            for (size_t idx = 0; idx < operations.size(); ++idx) {
                auto task_status_e = operations[idx].async_poll();

#ifdef PER_THREAD_STAT
                if (task_status_e != task_status_e::retired) polls++;
#endif

                if (task_status_e == task_status_e::completed) {
                    completed++;
                    res.completed_operations++;
                    res.data_read += operations[idx].get_bytes_read();
                    res.data_written += operations[idx].get_bytes_written();

                    operations[idx].light_reset();
                    operations[idx].async_submit();
                }
            }
        }
    }
#ifdef PER_THREAD_STAT
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(
                                   std::chrono::high_resolution_clock::now() - timer_start)
                                   .count() *
                           1000000000;
#endif

    for (auto& operation : operations) {
        operation.async_wait();
    }

#ifdef PER_THREAD_STAT
    static std::mutex guard;
    guard.lock();
    auto per_op = elapsed_seconds / res.completed_operations;
    printf("Thread: %3d; iters: %6lu; ops: %3u; completed/iter: %3u; polls/op: %6u; time/op: %5.0f ns\n",
           state.thread_index(), state.iterations(), res.operations_per_thread,
           (std::uint32_t)(res.completed_operations / state.iterations()),
           (std::uint32_t)(polls / res.completed_operations), per_op);
    guard.unlock();
#endif

    return res;
}
} // namespace bench::details
