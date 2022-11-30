/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include "../cmd_decl.hpp"

#ifdef __linux__
#include <time.h>
#endif
#include <types.hpp>
#include <x86intrin.h>
#include <vector>


//
// Defines
//
#define ERROR(M, ...) fprintf(stderr, "[ERROR] " M "\n", ##__VA_ARGS__)
#define ASSERT(C, M, ...) if (!(C)) { ERROR(M, ##__VA_ARGS__); exit(EXIT_FAILURE); }
#define ASSERT_NM(C) if (!(C)) { exit(EXIT_FAILURE); }

#define BENCHMARK_SET_DELAYED_INT(UNIQUE_REG, UNIQUE_SING) \
void UNIQUE_REG(); \
class UNIQUE_SING \
{ \
public: \
UNIQUE_SING() \
    { \
        auto &reg = bench::details::get_registry(); \
        reg.push_back(UNIQUE_REG); \
    } \
}; \
static UNIQUE_SING BENCHMARK_PRIVATE_NAME(_local_register_instance_); \
void UNIQUE_REG()

namespace bench::details
{
//
// Registration utils
//

using registry_call_t = void(*)();
using registry_t      = std::vector<registry_call_t>;
registry_t& get_registry();

//
// Registration utils
//

struct accel_info_t
{
    std::uint32_t              total_devices = 0;
    std::vector<std::uint32_t> socket;
};

struct extended_info_t
{
    std::string   host_name;
    std::string   kernel;
    std::uint32_t cpu_model                = 0;
    std::string   cpu_model_name;
    std::uint32_t cpu_stepping             = 0;
    std::uint32_t cpu_microcode            = 0;
    std::uint32_t cpu_logical_cores        = 0;
    std::uint32_t cpu_physical_cores       = 0;
    std::uint32_t cpu_sockets              = 0;
    std::uint32_t cpu_physical_per_socket  = 0;
    std::uint32_t cpu_physical_per_cluster = 0;
    accel_info_t  accelerators;
};

const extended_info_t& get_sys_info();

int get_current_numa_accels() noexcept;


constexpr std::uint64_t submitRetryWaitNs = 0;

inline void retry_sleep()
{
    if constexpr (submitRetryWaitNs == 0)
        return;
    else
    {
#ifdef __linux__
    timespec spec { 0, submitRetryWaitNs };
    nanosleep(&spec, nullptr);
#else
#endif
    }
}

template <typename RangeT>
inline void mem_control(RangeT begin, RangeT end, mem_loc_e op) noexcept
{
    for (auto line = begin; line < end; line += 64u)
    {
        __builtin_ia32_clflush(&(*line));

        volatile char *m = (char*)&(*line);
        if(op == mem_loc_e::cache)
            *m = *m;
        else if(op == mem_loc_e::llc)
        {
            *m = *m;
            // CLDEMOTE
            asm volatile(".byte 0x0f, 0x1c, 0x07\t\n" :: "D" (m));
        }
    }
    __builtin_ia32_mfence();
}
}
