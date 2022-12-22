/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
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

template <execution_e exec, api_e api, path_e path, path_e com_path=path>
class inflate_t
{
public:
    static constexpr auto exec_v = exec;
    static constexpr auto api_v  = api;
    static constexpr auto path_v = path;

    void operator()(benchmark::State &state, const case_params_t &common_params, const data_t &data, huffman_type_e huffman, const canned_table_t &canned, std::int64_t level) const
    {
        try
        {
            // Encode stream
            ops::deflate_params_t comp_params(data, level, huffman, false, false, canned);
            ops::deflate_t<api, com_path> compression;
            compression.init(comp_params);
            compression.async_submit();
            compression.async_wait();

            // Prepare decompression
            data_t stream;
            stream.buffer = compression.get_result().stream_;
            ops::inflate_params_t params(stream, data.buffer.size(), false, canned);
            std::vector<ops::inflate_t<api, path>> operations;

            // Measuring loop
            auto stat = measure<exec, path>(state, common_params, operations, params);

            // Validation
            for (auto &operation : operations)
            {
                if(operation.get_result().data_ != data.buffer)
                    throw std::runtime_error("Verification failed");
            }

            // Set counters
            base_counters(state, stat, stat_type_e::decompression);
        }
        catch(std::runtime_error &err) { state.SkipWithError(err.what()); }
        catch(...)                     { state.SkipWithError("Unknown exception"); }
    }
};

template <path_e path, path_e comp_path = path>
static inline void cases_set(data_t &data, huffman_type_e huffman, const canned_table_t &canned, std::vector<std::int32_t> &levels)
{
    if(path != path_e::cpu && cmd::FLAGS_no_hw)
        return;

    for(auto &level : levels)
    {
        register_benchmarks_common("inflate", to_name(comp_path, "gen_path") + to_name(huffman) + level_to_name(level), inflate_t<execution_e::sync,  api_e::c,   path, comp_path>{}, case_params_t{}, data, huffman, canned, level);
        register_benchmarks_common("inflate", to_name(comp_path, "gen_path") + to_name(huffman) + level_to_name(level), inflate_t<execution_e::async, api_e::c,   path, comp_path>{}, case_params_t{}, data, huffman, canned, level);
    }
}

BENCHMARK_SET_DELAYED(inflate)
{
    std::vector<std::int32_t>   block_sizes = (cmd::get_block_size() >= 0) ? std::vector<std::int32_t>{cmd::get_block_size()} : std::vector<std::int32_t>{4096, 8192, 16384, 65536, 0} ;
    std::vector<huffman_type_e> huffman_modes{huffman_type_e::fixed, huffman_type_e::dynamic};
    std::vector<double>         canned_parts = (cmd::FLAGS_canned_part >= 0) ? std::vector<double>{cmd::FLAGS_canned_part} : std::vector<double>{0.1, 0.5, 0};
    std::vector<std::int32_t>   sw_levels{1, 3};
    std::vector<std::int32_t>   hw_levels{1};
    std::vector<std::int32_t>   sw_hw_levels{3};

    auto dataset = data::read_dataset(cmd::FLAGS_dataset);
    for(auto &data : dataset)
    {
        for(auto &size : block_sizes)
        {
            auto blocks = data::split_data(data, size);
            for(auto &block : blocks)
            {
                for(auto &huffman : huffman_modes)
                {
                    cases_set<path_e::iaa>(block, huffman, canned_table_t{}, hw_levels);
                    cases_set<path_e::cpu>(block, huffman, canned_table_t{}, sw_levels);
                    cases_set<path_e::iaa, path_e::cpu>(block, huffman, canned_table_t{}, sw_hw_levels);
                }
            }
        }
    }
}
