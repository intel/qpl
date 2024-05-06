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
#include <cstdint>

#include "huffman_table.hpp"

using namespace bench;

template <execution_e exec, api_e api, path_e path, path_e com_path=path>
class inflate_t
{
public:
    static constexpr auto exec_v = exec;
    static constexpr auto api_v  = api;
    static constexpr auto path_v = path;

    void operator()(benchmark::State &state, const case_params_t &common_params, const data_t &data,
                    huffman_type_e huffman, const std::shared_ptr<qpl_huffman_table> &table, std::int64_t level) const
    {
        try
        {
            // Encode stream
            ops::deflate_params_t comp_params(data, level, huffman, false, false, table);
            ops::deflate_t<api, com_path> compression;
            compression.init(comp_params, common_params.node_);
            compression.async_submit();
            compression.async_wait();

            // Prepare decompression
            data_t stream;
            stream.buffer = compression.get_result().stream_;
            ops::inflate_params_t params(stream, data.buffer.size(), false, huffman, table);
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
static inline void register_inflate_case(const data_t &data,
                                         const huffman_type_e huffman,
                                         const std::shared_ptr<qpl_huffman_table> &table,
                                         const std::int32_t &level)
{
    if(path != path_e::cpu && cmd::FLAGS_no_hw)
        return;

    register_benchmarks_common("inflate", to_name(comp_path, "gen_path") + to_name(huffman) + level_to_name(level), inflate_t<execution_e::sync,  api_e::c,   path, comp_path>{}, case_params_t{}, data, huffman, table, level);
    register_benchmarks_common("inflate", to_name(comp_path, "gen_path") + to_name(huffman) + level_to_name(level), inflate_t<execution_e::async, api_e::c,   path, comp_path>{}, case_params_t{}, data, huffman, table, level);
}

template <path_e path, path_e comp_path = path>
static inline void prepare_cases(bench::dataset_t& dataset,
                                 const std::vector<std::int32_t>& levels,
                                 const std::vector<huffman_type_e>& compression_mode)
{
    for(const auto &data : dataset)
    {
        auto block_sizes = (cmd::get_block_size() >= 0)
                      ? std::vector<std::uint32_t>{static_cast<uint32_t>(cmd::get_block_size())}
                      : data::generate_block_sizes(data) ;

        for (const auto &level: levels)
        {
            std::shared_ptr<qpl_huffman_table> table(deflate_huffman_table_maker(combined_table_type,
                                                                                 data,
                                                                                 level_to_qpl(level),
                                                                                 path_to_qpl(path),
                                                                                 DEFAULT_ALLOCATOR_C),
                                                     any_huffman_table_deleter);
            for(const auto &size : block_sizes)
            {
                auto blocks = data::split_data(data, size);
                for(const auto &block : blocks)
                {
                    for(const auto &mode : compression_mode)
                    {
                        register_inflate_case<path, comp_path>(block, mode, table, level);
                    }
                }
            }
        }
    }
}

BENCHMARK_SET_DELAYED(inflate)
{
    /**
     * Input data for inflate benchmarks.
    */
    std::vector<huffman_type_e> compression_mode{huffman_type_e::fixed, huffman_type_e::dynamic, huffman_type_e::canned};
    std::vector<std::int32_t>   sw_levels{1, 3};
    std::vector<std::int32_t>   hw_levels{1};
    std::vector<std::int32_t>   sw_hw_levels{3};
    bench::dataset_t dataset = data::read_dataset(cmd::FLAGS_dataset);

    prepare_cases<path_e::cpu>(dataset, sw_levels, compression_mode);
    prepare_cases<path_e::iaa>(dataset, hw_levels, compression_mode);
    prepare_cases<path_e::iaa, path_e::cpu>(dataset, sw_hw_levels, compression_mode);
}
