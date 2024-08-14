/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#if defined(__linux__)

#include <cpuid.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>
#include <x86intrin.h>

#endif

#include <cstdint>

namespace qpl::ml::util {

// Below are helper functions for get_socket_id

#if defined(__linux__)
/**
 * @brief Return value of [start, end] bits of input.
*/
int get_selected_bits(int input, size_t start, size_t end) {
    const int mask = (1 << (end + 1)) - 1;

    return (input & mask) >> start;
}

/**
 * @brief Return smt, core and package id of the current process
 * using cpuid instruction.
 *
 * @note __get_cpuid{_count} is safe, so no extra checks are needed.
 *
 * Implementation is based on Intel(R) 64 Architecture Topology
 * Enumeration, January 2018.
*/
int get_cpu_topology_from_apic(unsigned long* smt_id, unsigned long* core_id, unsigned long* pkg_id) {
    unsigned int eax = 0U, ebx = 0U, ecx = 0U, edx = 0U, subleaf = 0U;

    // check that the topology info is available
    if (!__get_cpuid(0x1F, &eax, &ebx, &ecx, &edx)) return 1; // topology info is not available

    int level_type = 0, level_shift = 0;
    int was_thread_reported = 0, was_core_reported = 0;

    unsigned long smt_mask_width = 0U, smt_mask = 0U;

    // go through all available sub-leaves
    do { //NOLINT(cppcoreguidelines-avoid-do-while)
        __get_cpuid_count(0x1F, subleaf, &eax, &ebx, &ecx, &edx);
        if (ebx == 0) break; // no more sub-leaves

        level_type  = get_selected_bits(ecx, 8, 15);
        level_shift = get_selected_bits(eax, 0, 4);

        if (level_type == 1) {
            smt_mask_width      = level_shift;
            smt_mask            = (1UL << level_shift) - 1;
            was_thread_reported = 1;
        } else if (level_type == 2) {
            was_core_reported = 1;
        } else {
            break;
        }
        subleaf++;
    } while (1);

    const unsigned long core_plus_smt_mask       = (1UL << level_shift) - 1;
    const unsigned long core_plus_smt_mask_width = level_shift;

    const unsigned long pkg_mask = (-1) ^ core_plus_smt_mask;

    unsigned long core_mask = 0U;
    if (was_thread_reported && was_core_reported)
        core_mask = core_plus_smt_mask ^ smt_mask;
    else
        return 1; // should never happen

    *smt_id  = edx & smt_mask;
    *core_id = (edx & core_mask) >> smt_mask_width;
    *pkg_id  = (edx & pkg_mask) >> core_plus_smt_mask_width;

    return 0;
}

/**
 * @brief Helper function to transform "X-Y" string to return X and Y.
*/
bool read_dashed_list(std::filesystem::path const& root, int* min_value, int* max_value) {
    if (!std::filesystem::exists(root)) { return false; }

    std::ifstream fs;
    fs.open(root);
    std::string strval;
    std::getline(fs, strval);

    auto dash = strval.find("-");
    if (std::string::npos == dash) { return false; }
    *min_value = std::stoi(strval.substr(0, dash));
    *max_value = std::stoi(strval.substr(dash + 1));

    return true;
}

/**
 * @brief Read and return info from physical_package_id for specific cpu<N>.
*/
int get_pkg_id_of_cpu(std::filesystem::path const& root, int cpu) {
    auto cpu_path = root;
    cpu_path.append("cpu");
    {
        std::stringstream ss;
        ss << "cpu" << cpu;
        cpu_path.append(ss.str());
    }

    if (!std::filesystem::exists(cpu_path)) { return -1; }

    auto package_path = std::move(cpu_path);
    package_path.append("topology/physical_package_id");

    if (!std::filesystem::exists(package_path)) { return -1; }

    std::ifstream fs;

    fs.open(package_path);
    std::string strval;
    std::getline(fs, strval);

    return std::stoll(strval, nullptr, 10);
}

/**
 * @brief Read cpulist for specific node<N> and return first cpu from the list.
*/
int get_first_cpu_from_node(std::filesystem::path const& root, int node) {
    auto node_path = root;
    node_path.append("node");
    {
        std::stringstream ss;
        ss << "node" << node;
        node_path.append(ss.str());
    }

    if (!std::filesystem::exists(node_path)) { return -1; }

    std::filesystem::path cpulist(std::move(node_path));
    cpulist.append("cpulist");

    int min_cpu = -1, max_cpu = 0;

    if (read_dashed_list(cpulist, &min_cpu, &max_cpu)) { return min_cpu; }

    return -1;
}
#endif // defined(__linux__)

// End of helper functions for get_socket_id

/**
 * @brief Return NUMA node id of the current process.
 * @warning Implementation for Windows is not available yet as it is not required.
 */
int32_t get_numa_id() noexcept {
#if defined(__linux__)
    uint32_t tsc_aux = 0;

    __rdtscp(&tsc_aux);

    // Linux encodes NUMA node into [32:12] of TSC_AUX
    return static_cast<int32_t>(tsc_aux >> 12);
#else
    // Implementation is not required for Windows
    return -1;
#endif
}

/**
 * @brief Return socket id of the current process.
 * @warning Implementation for Windows is not available yet as it is not required.
 */
uint64_t get_socket_id() noexcept {
#if defined(__linux__)
    unsigned long smt_id = 0U, core_id = 0U, pkg_id = 0U;

    if (0 == get_cpu_topology_from_apic(&smt_id, &core_id, &pkg_id)) { return pkg_id; }
#endif
    return (uint64_t)-1;
}

/**
 * @brief Return socket id for a give numa node.
 * @warning Implementation for Windows is not available yet as it is not required.
*/
uint64_t get_socket_id(int numa_node) noexcept {
#if defined(__linux__)
    const std::filesystem::path root("/sys/devices/system");

    const int min_cpu = get_first_cpu_from_node(root, numa_node);
    if (-1 != min_cpu) { return (uint64_t)get_pkg_id_of_cpu(root, min_cpu); }
#endif
    return (uint64_t)-1;
}

} // namespace qpl::ml::util
