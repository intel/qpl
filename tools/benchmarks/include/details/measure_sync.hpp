/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <benchmark/benchmark.h>
#include <ops/ops.hpp>
#include <stdexcept>
#include <utility.hpp>

namespace bench::details {
template <path_e path, typename OperationT, typename ParamsT>
static statistics_t measure_sync(benchmark::State& state, const case_params_t& common_params, OperationT&& operations,
                                 ParamsT&& params) {
    statistics_t res;
    if constexpr (path == path_e::cpu) {
        res.queue_size = 1;
        res.operations = 1;
    } else {
        res.queue_size = common_params.queue_size_;
        res.operations = res.queue_size;
    }

    res.operations_per_thread = res.operations;
    if (state.threads() > 1) throw std::runtime_error("Synchronous measurements do not support threading");

    operations.resize(res.queue_size);
    for (auto& operation : operations) {
        operation.init(params, common_params.node_, get_mem_cc(common_params.out_mem_), common_params.full_time_);
        operation.mem_control(common_params.in_mem_, mem_loc_mask_e::src);
    }

    // Strategies:
    // - File at once. Each operation works on same file independently.
    // - Chunk at once. Measure each chunk independently one by one, gather aggregate in the end. Is this reasonable?
    // - File by chunks. Measure for the whole file processing different chunks in parallel (map file before processing). Like normal processing

    for (auto& operation : operations) {
        operation.async_submit();
        operation.async_wait();
        operation.light_reset();
    }

    for (auto _ : state) {
        for (auto& operation : operations) {
            operation.async_submit();
        }

        for (auto& operation : operations) {
            operation.async_wait();
            operation.light_reset();

            res.completed_operations++;
            res.data_read += operation.get_bytes_read();
            res.data_written += operation.get_bytes_written();
        }
    }

    return res;
}
} // namespace bench::details
