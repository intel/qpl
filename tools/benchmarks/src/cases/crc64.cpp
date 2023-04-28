/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <benchmark/benchmark.h>

#include <ops/ops.hpp>
#include <data_providers.hpp>
#include <utility.hpp>
#include <measure.hpp>
#include <stdexcept>

using namespace bench;

template <execution_e exec, api_e api, path_e path>
class crc64_t
{
public:
    static constexpr auto exec_v = exec;
    static constexpr auto api_v  = api;
    static constexpr auto path_v = path;

    void operator()(benchmark::State &state, const case_params_t &common_params, const data_t &data, crc_type_e type) const
    {
        try
        {
            // Prepare crc64
            ops::crc64_params_t params(data, data.buffer.size(), type);
            std::vector<ops::crc64_t<api, path>> operations;

            // Measuring loop
            auto stat = measure<exec, path>(state, common_params, operations, params);

            // Set counters
            base_counters(state, stat, stat_type_e::compression);
        }
        catch(std::runtime_error &err) { state.SkipWithError(err.what()); }
        catch(...)                     { state.SkipWithError("Unknown exception"); }
    }
};

template <path_e path>
static inline void cases_set(data_t &data, crc_type_e type)
{
    if(path != path_e::cpu && cmd::FLAGS_no_hw)
        return;
    
    register_benchmarks_common("crc64", to_name(path, "gen_path") + crc_to_name(type), crc64_t<execution_e::sync,  api_e::c, path>{}, case_params_t{}, data, type);
    register_benchmarks_common("crc64", to_name(path, "gen_path") + crc_to_name(type), crc64_t<execution_e::async, api_e::c, path>{}, case_params_t{}, data, type);
}

BENCHMARK_SET_DELAYED(crc64)
{
    std::vector<std::int32_t>   block_sizes = (cmd::get_block_size() >= 0) ? std::vector<std::int32_t>{cmd::get_block_size()} : std::vector<std::int32_t>{4096, 8192, 16384, 65536, 0} ;
    std::vector<crc_type_e>     crc_types{crc_type_e::crc32_gzip, crc_type_e::crc32_wimax, crc_type_e::crc32_iscsi, crc_type_e::T10DIF, crc_type_e::crc16_ccitt, crc_type_e::crc64};

    auto dataset = data::read_dataset(cmd::FLAGS_dataset);
    for(auto &data : dataset)
    {
        for(auto &size : block_sizes)
        {
            auto blocks = data::split_data(data, size);
            for(auto &block : blocks)
            {
                for (auto &crc_type : crc_types)
                {
                    cases_set<path_e::iaa>(block, crc_type);
                    cases_set<path_e::cpu>(block, crc_type);
                }
            }
        }
    }
}
