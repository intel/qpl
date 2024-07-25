/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <benchmark/benchmark.h>
#include <details/measure_async.hpp>
#include <details/measure_sync.hpp>
#include <utility.hpp>

namespace bench {
template <execution_e exec, path_e path, typename OperationT, typename ParamsT>
static inline statistics_t measure(benchmark::State& state, const case_params_t& common_params, OperationT&& operations,
                                   ParamsT&& params) {
    if constexpr (exec == execution_e::async)
        return details::measure_async<path>(state, common_params, operations, params);
    else
        return details::measure_sync<path>(state, common_params, operations, params);
}
} // namespace bench
