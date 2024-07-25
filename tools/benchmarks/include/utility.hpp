/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <algorithm> // vector search
#include <cstdint>
#include <string>
#include <vector>

#include "cmd_decl.hpp"
#include "details/utility.hpp"

//
// Defines
//
// This macro provides delayed registration for benchmark cases which depend on cmd arguments or file names.
// Cases still can be registered via native Google Benchmark interfaces
#define BENCHMARK_SET_DELAYED(NAME)                                           \
    BENCHMARK_SET_DELAYED_INT(BENCHMARK_PRIVATE_NAME(_local_register_##NAME), \
                              BENCHMARK_PRIVATE_NAME(_local_register_singleton_##NAME))

namespace bench {
//
// Common string operations
//
std::string format(const char* format, ...) noexcept;

static inline void trim_left(std::string& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char val) { return !std::isspace(val); }));
}

static inline void trim_right(std::string& str) {
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char val) { return !std::isspace(val); }).base(),
              str.end());
}

static inline void trim(std::string& str) {
    trim_left(str);
    trim_right(str);
}

template <typename ArgT>
static inline std::string to_string(ArgT arg) {
    return std::to_string(arg);
}
template <typename ArgT>
static inline std::string to_name(ArgT arg, std::string name) {
    return std::string("/") + name + ":" + to_string(arg);
}
static inline std::string to_string(api_e api) {
    switch (api) {
        case api_e::c: return "c";
        case api_e::ml: return "ml";
        default: return "error";
    }
}
static inline std::string to_name(api_e api) {
    return std::string("/api:") + to_string(api);
}

static inline std::string to_string(path_e path) {
    switch (path) {
        case path_e::cpu: return "cpu";
        case path_e::iaa: return "iaa";
        case path_e::auto_: return "auto";
        default: return "error";
    }
}
static inline std::string to_name(path_e path) {
    return std::string("/path:") + to_string(path);
}
static inline std::string to_string(execution_e exec) {
    switch (exec) {
        case execution_e::sync: return "sync";
        case execution_e::async: return "async";
        default: return "error";
    }
}
static inline std::string to_name(execution_e exec) {
    return std::string("/exec:") + to_string(exec);
}
static inline std::string to_string(operation_e op) {
    switch (op) {
        case operation_e::deflate: return "deflate";
        case operation_e::inflate: return "inflate";
        default: return "error";
    }
}
static inline std::string level_to_name(std::int32_t level) {
    return std::string("/lvl:") + std::to_string(level);
}
static inline std::string to_string(huffman_type_e huffman) {
    switch (huffman) {
        case huffman_type_e::fixed: return "fixed";
        case huffman_type_e::static_: return "static";
        case huffman_type_e::dynamic: return "dynamic";
        case huffman_type_e::canned: return "canned";
        default: return "error";
    }
}
static inline std::string to_name(huffman_type_e huffman) {
    return std::string("/huffman:") + to_string(huffman);
}

static inline std::string crc_to_string(crc_type_e type) {
    switch (type) {
        case crc_type_e::crc32_gzip: return "crc32_gzip";
        case crc_type_e::crc32_iscsi: return "crc32_iscsi";
        case crc_type_e::crc32_wimax: return "crc32_wimax";
        case crc_type_e::T10DIF: return "T10DIF";
        case crc_type_e::crc16_ccitt: return "crc16_ccitt";
        case crc_type_e::crc64: return "crc64";
        default: return "error";
    }
}
static inline std::string crc_to_name(crc_type_e type) {
    return std::string("/crc:") + crc_to_string(type);
}

static inline std::string to_string(mem_loc_e loc) {
    switch (loc) {
        case mem_loc_e::cache: return "cache";
        case mem_loc_e::llc: return "llc";
        case mem_loc_e::ram: return "ram";
        case mem_loc_e::pmem: return "pmem";
        case mem_loc_e::cc_ram: return "cc_ram";
        case mem_loc_e::cc_pmem: return "cc_pmem";
        default: return "error";
    }
}

//
// Common cases utils
//
static inline bool get_mem_cc(mem_loc_e mem) {
    return (mem == mem_loc_e::cc_ram || mem == mem_loc_e::cc_pmem);
}

template <execution_e exec>
static inline void base_arguments(benchmark::internal::Benchmark* b) {
    if constexpr (exec == execution_e::async) {
        const std::vector<int> threads_vector {1, 2, 4, 8, 16};

        if (!cmd::FLAGS_threads) {
            for (auto threads : threads_vector) {
                b->Threads(threads);
            }
        } else
            b->Threads(cmd::FLAGS_threads);
    }

    b->UseRealTime();
    b->Unit(benchmark::kNanosecond);
}

template <typename LambdaT, typename... ArgsT>
static ::benchmark::internal::Benchmark* register_benchmark_proxy(const std::string name, LambdaT&& fn,
                                                                  ArgsT&&... args) {
    return ::benchmark::RegisterBenchmark(name.c_str(), std::forward<LambdaT>(fn), std::forward<ArgsT>(args)...);
}

struct case_params_t {
    mem_loc_e    in_mem_ {cmd::get_in_mem()};
    mem_loc_e    out_mem_ {cmd::get_out_mem()};
    bool         full_time_ {cmd::FLAGS_full_time};
    std::int32_t queue_size_ {cmd::FLAGS_queue_size};
    std::int32_t node_ {cmd::FLAGS_node};
};

template <typename CaseT, typename CaseParamsT, typename... ArgsT>
static ::benchmark::internal::Benchmark*
register_benchmarks_common(const std::string& case_name, const std::string& case_name_ext, CaseT&& case_functor,
                           CaseParamsT&& case_common_params, const data_t& data, ArgsT&&... case_args) {
    std::string api_name  = to_name(case_functor.api_v);
    std::string exec_name = to_name(case_functor.exec_v);
    std::string path_name = to_name(case_functor.path_v);
    std::string in_mem    = to_name(case_common_params.in_mem_, "in_mem");
    std::string out_mem   = to_name(case_common_params.out_mem_, "out_mem");
    std::string timer_name;
    if (case_common_params.full_time_)
        timer_name = "/timer:full";
    else
        timer_name = "/timer:proc";
    std::string qsize_name;
    if (case_common_params.queue_size_ == 0) {
        if (case_functor.exec_v == execution_e::async)
            case_common_params.queue_size_ = 128;
        else
            case_common_params.queue_size_ = 1;
    }
    qsize_name            = to_name(case_common_params.queue_size_, "qsize");
    std::string data_name = (data.name.size()) ? std::string("/data:") + data.name : std::string {};
    std::string test_name = case_name + api_name + path_name + exec_name + qsize_name + in_mem + out_mem + timer_name +
                            data_name + case_name_ext;
    if (case_functor.exec_v == execution_e::async) {
        return register_benchmark_proxy(test_name, std::forward<CaseT>(case_functor), case_common_params, data,
                                        std::forward<ArgsT>(case_args)...)
                ->Apply(base_arguments<execution_e::async>);
    } else {
        return register_benchmark_proxy(test_name, std::forward<CaseT>(case_functor), case_common_params, data,
                                        std::forward<ArgsT>(case_args)...)
                ->Apply(base_arguments<execution_e::sync>);
    }
}

/**
 * This function calculates and sets the counters for benchmarking based on the given state, statistics
 * and operation type.
 * The counters include latency, latency per operation, throughput and ratio.
 *
 * @param [in,out] state The benchmark state object.
 * @param [in] stat The statistics object containing data read, data written, queue size and operations per thread.
 * @param [in] type The type of operation performed.
 */
static inline void base_counters(benchmark::State& state, statistics_t& stat, stat_type_e type = stat_type_e::filter) {
    double throughput = 0.0;

    if (state.iterations() != 0) {
        if (type == stat_type_e::compression || type == stat_type_e::crc64) {
            throughput = static_cast<double>(stat.data_read / state.iterations());
        } else if (type == stat_type_e::decompression) {
            throughput = static_cast<double>(stat.data_written / state.iterations());
        }
    }

    // Note: The total data_read and data_written from all iterations are used
    // when calculating the Ratio to avoid potential rounding errors.
    if (type == stat_type_e::compression) {
        state.counters["Ratio"] =
                benchmark::Counter(static_cast<double>(stat.data_read) / static_cast<double>(stat.data_written),
                                   benchmark::Counter::kAvgThreads);
    } else if (type == stat_type_e::decompression) {
        state.counters["Ratio"] =
                benchmark::Counter(static_cast<double>(stat.data_written) / static_cast<double>(stat.data_read),
                                   benchmark::Counter::kAvgThreads);
    }

    state.counters["Latency"] =
            benchmark::Counter(1,
                               benchmark::Counter::kIsIterationInvariantRate | benchmark::Counter::kAvgThreads |
                                       benchmark::Counter::kInvert,
                               benchmark::Counter::kIs1000);
    state.counters["Latency/Op"] =
            benchmark::Counter(stat.operations_per_thread,
                               benchmark::Counter::kIsIterationInvariantRate | benchmark::Counter::kAvgThreads |
                                       benchmark::Counter::kInvert,
                               benchmark::Counter::kIs1000);
    state.counters["Throughput"] = benchmark::Counter(
            throughput, benchmark::Counter::kIsIterationInvariantRate | benchmark::Counter::kAvgThreads,
            benchmark::Counter::kIs1000);
}

extern std::vector<std::string> FILTER_op;
extern std::vector<std::string> FILTER_path;
extern std::vector<std::string> FILTER_execution_mode;
extern std::vector<std::string> FILTER_compression_mode;

/**
 * Check if the given operation, path or execution mode is present in the vector.
*/
static inline bool is_in_vector(const std::vector<std::string>& vec, const bench::operation_e& op) {
    return std::find(vec.begin(), vec.end(), to_string(op)) != vec.end();
}
static inline bool is_in_vector(std::vector<std::string> vec, bench::path_e path) {
    return std::find(vec.begin(), vec.end(), to_string(path)) != vec.end();
}
static inline bool is_in_vector(std::vector<std::string> vec, bench::execution_e exec) {
    return std::find(vec.begin(), vec.end(), to_string(exec)) != vec.end();
}

/**
 * Register the benchmark case if the operation, path or execution mode
 * matches the input parameter, or if the vector is empty.
 *
 * @note An empty vector indicates that we were not able to reliably parse the input filter
 * (e.g., the filter included "flat" instead of "deflate" or "inflate").
 * In this case, we don't want to filter anything out, so we will register everything.
*/
static inline bool continue_register(bench::operation_e op) {
    return (FILTER_op.empty() || is_in_vector(FILTER_op, op));
}
static inline bool continue_register(path_e path) {
    // Special case for --no_hw parameter
    if (path == path_e::iaa && cmd::FLAGS_no_hw) return false;

    return (FILTER_path.empty() || is_in_vector(FILTER_path, path));
}
static inline bool continue_register(execution_e exec) {
    return (FILTER_execution_mode.empty() || is_in_vector(FILTER_execution_mode, exec));
}

/**
 * @brief Converts a string representation of a Huffman type to its corresponding enum value.
 *
 * This function takes a string `val` and returns the corresponding `huffman_type_e` enum value.
 * The possible string values and their corresponding enum values are as follows:
 *   - "fixed"   : huffman_type_e::fixed
 *   - "static"  : huffman_type_e::static_
 *   - "dynamic" : huffman_type_e::dynamic
 *   - "canned"  : huffman_type_e::canned
 *
 * @param val The string representation of the Huffman type.
 * @return The corresponding `huffman_type_e` enum value.
 */
static inline huffman_type_e to_huffman_type(const std::string& val) {
    huffman_type_e type = huffman_type_e::fixed;
    if (val == "fixed")
        type = huffman_type_e::fixed;
    else if (val == "static")
        type = huffman_type_e::static_;
    else if (val == "dynamic")
        type = huffman_type_e::dynamic;
    else if (val == "canned")
        type = huffman_type_e::canned;

    return type;
}

/**
 * Converts a vector of strings to a vector of huffman_type_e values.
 *
 * @param vec The vector of strings to convert.
 * @return The resulting vector of huffman_type_e values.
 */
static inline std::vector<huffman_type_e> to_huffman_type(const std::vector<std::string>& vec) {
    std::vector<huffman_type_e> res;
    for (const auto& val : vec)
        res.push_back(to_huffman_type(val));

    return res;
}

} // namespace bench
