/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <details/utility.hpp>
#include "cmd_decl.hpp"

//
// Defines
//
// This macro provides delayed registration for benchmark cases which depend on cmd arguments or file names.
// Cases still can be registered via native Google Benchmark interfaces
#define BENCHMARK_SET_DELAYED(NAME) \
BENCHMARK_SET_DELAYED_INT(BENCHMARK_PRIVATE_NAME(_local_register_##NAME), BENCHMARK_PRIVATE_NAME(_local_register_singleton_##NAME))

namespace bench
{
//
// Common string operations
//
std::string format(const char *format, ...) noexcept;

static inline void trim_left(std::string &str)
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char val) { return !std::isspace(val); }));
}

static inline void trim_right(std::string &str)
{
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char val) { return !std::isspace(val); }).base(), str.end());
}

static inline void trim(std::string &str)
{
    trim_left(str);
    trim_right(str);
}

template <typename ArgT>
static inline std::string to_string(ArgT arg)
{
    return std::to_string(arg);
}
template <typename ArgT>
static inline std::string to_name(ArgT arg, std::string name)
{
    return std::string("/") + name + ":" + to_string(arg);
}
static inline std::string to_string(api_e api)
{
    switch(api)
    {
    case api_e::c:   return "c";
    case api_e::ml:  return "ml";
    default:         return "error";
    }
}
static inline std::string to_name(api_e api)
{
    return std::string("/api:") + to_string(api);
}

static inline std::string to_string(path_e path)
{
    switch(path)
    {
    case path_e::cpu:    return "cpu";
    case path_e::iaa:    return "iaa";
    case path_e::dsa:    return "dsa";
    case path_e::auto_:  return "auto";
    default:             return "error";
    }
}
static inline std::string to_name(path_e path)
{
    return std::string("/path:") + to_string(path);
}
static inline std::string to_string(execution_e exec)
{
    switch(exec)
    {
    case execution_e::sync:  return "sync";
    case execution_e::async: return "async";
    default:         return "error";
    }
}
static inline std::string to_name(execution_e exec)
{
    return std::string("/exec:") + to_string(exec);
}
static inline std::string to_string(operation_e op)
{
    switch(op)
    {
    case operation_e::deflate: return "deflate";
    case operation_e::inflate: return "inflate";
    default:                   return "error";
    }
}
static inline std::string level_to_name(std::int32_t level)
{
    return std::string("/lvl:") + std::to_string(level);
}
static inline std::string to_string(huffman_type_e huffman)
{
    switch(huffman)
    {
    case huffman_type_e::fixed:   return "fixed";
    case huffman_type_e::static_: return "static";
    case huffman_type_e::dynamic: return "dynamic";
    case huffman_type_e::canned:  return "canned";
    default:                      return "error";
    }
}
static inline std::string to_name(huffman_type_e huffman)
{
    return std::string("/huffman:") + to_string(huffman);
}
static inline std::string to_string(mem_loc_e loc)
{
    switch(loc)
    {
    case mem_loc_e::cache:   return "cache";
    case mem_loc_e::llc:     return "llc";
    case mem_loc_e::ram:     return "ram";
    case mem_loc_e::pmem:    return "pmem";
    case mem_loc_e::cc_ram:  return "cc_ram";
    case mem_loc_e::cc_pmem: return "cc_pmem";
    default:                 return "error";
    }
}

//
// Common cases utils
//
static inline bool get_mem_cc(mem_loc_e mem)
{
    return (mem == mem_loc_e::cc_ram || mem == mem_loc_e::cc_pmem);
}

template<execution_e exec>
static inline void base_arguments(benchmark::internal::Benchmark *b)
{
    if constexpr (exec == execution_e::async)
    {
        const std::vector<int> threads_vector{1, 2, 4, 8, 16};

        if(!cmd::FLAGS_threads)
        {
            for(auto threads : threads_vector)
            {
                b->Threads(threads);
            }
        }
        else
            b->Threads(cmd::FLAGS_threads);
    }

    b->UseRealTime();
    b->Unit(benchmark::kNanosecond);
}

template <typename LambdaT, typename... ArgsT>
static ::benchmark::internal::Benchmark* register_benchmark_proxy(const std::string name,
                                                                  LambdaT&&   fn,
                                                                  ArgsT&&...  args)
{
    return ::benchmark::RegisterBenchmark(name.c_str(), std::forward<LambdaT>(fn), std::forward<ArgsT>(args)...);
}

struct case_params_t
{
    mem_loc_e     in_mem_{cmd::get_in_mem()};
    mem_loc_e     out_mem_{cmd::get_out_mem()};
    bool          full_time_{cmd::FLAGS_full_time};
    std::int32_t  queue_size_{cmd::FLAGS_queue_size};
    std::int32_t  batch_size_{cmd::FLAGS_batch_size};
};


template <typename CaseT, typename CaseParamsT, typename... ArgsT>
static ::benchmark::internal::Benchmark* register_benchmarks_common(const std::string   &case_name,
                                                                    const std::string   &case_name_ext,
                                                                    CaseT&&              case_functor,
                                                                    CaseParamsT        &&case_common_params,
                                                                    const data_t        &data,
                                                                    ArgsT&&...           case_args)
{
    std::string api_name   = to_name(case_functor.api_v);
    std::string exec_name  = to_name(case_functor.exec_v);
    std::string path_name  = to_name(case_functor.path_v);
    std::string in_mem     = to_name(case_common_params.in_mem_, "in_mem");
    std::string out_mem    = to_name(case_common_params.out_mem_, "out_mem");
    std::string timer_name;
    if(case_common_params.full_time_)
        timer_name = "/timer:full";
    else
        timer_name = "/timer:proc";
    std::string qsize_name;
    if(case_common_params.queue_size_ == 0)
    {
        if(case_functor.exec_v == execution_e::async)
            case_common_params.queue_size_ = 128;
        else
            case_common_params.queue_size_ = 1;
    }
    qsize_name = to_name(case_common_params.queue_size_, "qsize");
    std::string data_name  = (data.name.size()) ? std::string("/data:") + data.name : std::string{};
    std::string test_name  = case_name + api_name + path_name + exec_name + qsize_name + in_mem + out_mem + timer_name + data_name + case_name_ext;
    return register_benchmark_proxy(test_name, std::forward<CaseT>(case_functor), case_common_params, data, std::forward<ArgsT>(case_args)...)->Apply(base_arguments<case_functor.exec_v>);
}


static inline void base_counters(benchmark::State &state, statistics_t &stat, stat_type_e type = stat_type_e::filter)
{
    if(type == stat_type_e::compression)
    {
        state.counters["Ratio"]      = benchmark::Counter(static_cast<double>(stat.data_read) / static_cast<double>(stat.data_written),
                                                          benchmark::Counter::kAvgThreads);
        state.counters["Throughput"] = benchmark::Counter(static_cast<double>(stat.data_read),
                                                          benchmark::Counter::kIsIterationInvariantRate|benchmark::Counter::kAvgThreads,
                                                          benchmark::Counter::kIs1000);
    }
    else if(type == stat_type_e::decompression)
    {
        state.counters["Ratio"]      = benchmark::Counter(static_cast<double>(stat.data_written) / static_cast<double>(stat.data_read),
                                                          benchmark::Counter::kAvgThreads);
        state.counters["Throughput"] = benchmark::Counter(static_cast<double>(stat.data_written),
                                                          benchmark::Counter::kIsIterationInvariantRate|benchmark::Counter::kAvgThreads,
                                                          benchmark::Counter::kIs1000);
    }

    state.counters["Latency/Op"] = benchmark::Counter(stat.operations_per_thread,
                                                      benchmark::Counter::kIsIterationInvariantRate|benchmark::Counter::kAvgThreads|benchmark::Counter::kInvert,
                                                      benchmark::Counter::kIs1000);

    state.counters["Latency"]    = benchmark::Counter(1,
                                                      benchmark::Counter::kIsIterationInvariantRate |benchmark::Counter::kAvgThreads|benchmark::Counter::kInvert,
                                                      benchmark::Counter::kIs1000);
}
}
