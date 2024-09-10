/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TOOLS_UTILS_COMMON_SYSTEM_INFO_HPP_
#define QPL_TOOLS_UTILS_COMMON_SYSTEM_INFO_HPP_

#include <algorithm> // std::find_if
#include <cctype>    // std::isspace, std::isdigit
#include <fstream>   // std::ifstream
#include <mutex>     // std::mutex
#include <string>    // std::string, std::getline, substr, etc.
#include <vector>    // std::vector

#if defined(__linux__)
#include <sys/utsname.h>
#include <x86intrin.h>
#endif

namespace qpl::test {

/**
 * @brief Structure to keep system information not including accelerators.
 *
 * @note Copy of the one used in benchmarks with slights modifications.
*/
struct extended_info_t {
    std::string           host_name;
    std::string           kernel;
    std::vector<uint32_t> kernel_version_numerical;
    std::uint32_t         cpu_model = 0U;
    std::string           cpu_model_name;
    std::uint32_t         cpu_stepping            = 0U;
    std::uint32_t         cpu_microcode           = 0U;
    std::uint32_t         cpu_logical_cores       = 1U;
    std::uint32_t         cpu_physical_cores      = 1U;
    std::uint32_t         cpu_sockets             = 1U;
    std::uint32_t         cpu_physical_per_socket = 1U;
    std::uint32_t         cpu_numa_nodes          = 1U;
};

static void trim(std::string& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char val) { return !std::isspace(val); }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char val) { return !std::isspace(val); }).base(),
              str.end());
}

/**
 * @brief Check that the input string is a set of digits.
 *
 * @note Function returns "false" for an empty string or "3.14".
*/
static bool is_number(const std::string& s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) {
        ++it;
    }
    return (!s.empty() && it == s.end());
}

/**
 * @brief Giving the string in format "X.<non-digits>.Y.<other non-digits symbols>",
 * where X, Y are digits or sequences of digits, function returns a vector consisting of X, Y.
 * Arbitrary delimiter with arbitrary length could be used as well instead of ".".
*/
static std::vector<uint32_t> get_digits_between_delims(const std::string& s, const std::string& delim) {
    std::vector<uint32_t> vector;

    size_t delim_pos = 0U, offset = 0U;
    while (std::string::npos != delim_pos) {
        delim_pos = s.find(delim, offset);

        const size_t substr_start = offset;
        const size_t substr_end   = (delim_pos == std::string::npos) ? s.length() : delim_pos - substr_start;

        const std::string substr_to_check = s.substr(substr_start, substr_end);

        if (is_number(substr_to_check)) { vector.push_back(std::stoi(substr_to_check)); }

        offset = delim_pos + delim.length();
    }

    return vector;
}

/**
 * @brief Given the string, in format "<kernel>.<major>.<minor>-<patch>-<extra info>",
 * return a vector consisting of <kernel>, <major>, <minor>
 * and trim flavor info (i.e., patch and other additional symbols).
 *
 * @warning Function does not check for the string to be in correct format and assume "."
 * and "-" delimiters.
*/
static std::vector<uint32_t> get_kernel_major_minor(const std::string& s) {
    // Find "-" position to trim flavor info
    const size_t flavor_pos = s.find("-");

    return get_digits_between_delims(s.substr(0, flavor_pos), ".");
}

static std::ostream& operator<<(std::ostream& os, const extended_info_t& info) {
    /* Benchmarks output for system configuration details */
    os << "Host Name:            " << info.host_name.c_str() << "\n";
    os << "Kernel:               " << info.kernel.c_str() << "\n";
    os << "CPU:                  " << info.cpu_model_name.c_str() << ", " << info.cpu_model << "\n";
    os << "    Microcode:        " << info.cpu_microcode << "\n";
    os << "    Stepping:         " << info.cpu_stepping << "\n";
    os << "    Logical Cores:    " << info.cpu_logical_cores << "\n";
    os << "    Physical Cores:   " << info.cpu_physical_cores << "\n";
    os << "    Cores per Socket: " << info.cpu_physical_per_socket << "\n";
    os << "    Sockets:          " << info.cpu_sockets << "\n";
    os << "    NUMA Nodes:       " << info.cpu_numa_nodes << "\n";

    return os;
}

/**
 * @brief Get the total number of NUMA nodes.
 */
static uint32_t get_total_numa_nodes() {
    uint32_t total_nodes = 1U;

#if defined(__linux__)
    std::ifstream numa_nodes("/sys/devices/system/node/online");
    if (!numa_nodes.is_open()) { throw std::runtime_error("Failed to open /sys/devices/system/node/online"); }

    std::string line;
    std::getline(numa_nodes, line);
    if (line.find("-") != std::string::npos) { total_nodes = std::stoi(line.substr(line.find("-") + 1U)) + 1U; }
    numa_nodes.close();
#endif

    return total_nodes;
}

/**
 * @note Implementation is borrowed from Benchmarks Framework.
 * A copy is stored since it is not desirable to introduce additional dependencies for benchmarks executable.
*/
static const extended_info_t& get_sys_info() {
    static extended_info_t info;
    static bool            is_setup {false};
    static std::mutex      guard;

    guard.lock();
    if (!is_setup) {
#if defined(__linux__)
        utsname uname_buf;
        uname(&uname_buf);
        info.host_name = uname_buf.nodename;
        info.kernel    = uname_buf.release;

        info.kernel_version_numerical = get_kernel_major_minor(info.kernel);

        std::ifstream info_file("/proc/cpuinfo");
        if (!info_file.is_open()) {
            guard.unlock();
            throw std::runtime_error("Failed to open /proc/cpuinfo");
        }

        info.cpu_logical_cores = 0U; // reset to 0, since we're going to actually count them

        std::string line;
        while (std::getline(info_file, line)) {
            if (line.empty()) continue;
            auto del_index = line.find(':');
            if (del_index == std::string::npos) continue;
            auto key = line.substr(0, del_index);
            auto val = line.substr(del_index + 1);
            trim(key);
            trim(val);

            if (key == "processor")
                info.cpu_logical_cores++;
            else if (key == "physical id")
                info.cpu_sockets = std::max(info.cpu_sockets, (std::uint32_t)atoi(val.c_str()) + 1);
            else if (key == "cpu cores")
                info.cpu_physical_per_socket = std::max(info.cpu_physical_per_socket, (std::uint32_t)atoi(val.c_str()));
            else if (!info.cpu_model_name.size() && key == "model name")
                info.cpu_model_name = val;
            else if (!info.cpu_model && key == "model")
                info.cpu_model = atoi(val.c_str());
            else if (!info.cpu_microcode && key == "microcode")
                info.cpu_microcode = strtol(val.c_str(), NULL, 16);
            else if (!info.cpu_stepping && key == "stepping")
                info.cpu_stepping = atoi(val.c_str());
        }
        info.cpu_physical_cores = info.cpu_physical_per_socket * info.cpu_sockets;
#endif
        is_setup = true;
    }
    guard.unlock();

    info.cpu_numa_nodes = get_total_numa_nodes();

    return info;
}

// Required for version checking in PF tests
// to ensure that MADV_PAGEOUT is available.
#define QPL_PF_TESTS_REQ_MAJOR 5U
#define QPL_PF_TESTS_REQ_MINOR 4U

static inline bool is_madv_pageout_available() {
    bool is_version_ge_5_4 = false;
#if defined(__linux__)
    is_version_ge_5_4 = (get_sys_info().kernel_version_numerical.size() >= 2U)
                                ? ((get_sys_info().kernel_version_numerical[0] > QPL_PF_TESTS_REQ_MAJOR) ||
                                   ((get_sys_info().kernel_version_numerical[0] == QPL_PF_TESTS_REQ_MAJOR) &&
                                    (get_sys_info().kernel_version_numerical[1] >= QPL_PF_TESTS_REQ_MINOR)))
                                : false;
#endif
    return is_version_ge_5_4;
}

} // namespace qpl::test

#endif // QPL_TOOLS_UTILS_COMMON_SYSTEM_INFO_HPP_
