/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_LOW_LEVEL_CRC64_CROSS_SOCKET_EXAMPLE] */

#include <iostream>
#include <memory>
#include <numeric>
#include <vector>

#include "qpl/qpl.h"

#include "examples_utils.hpp" // for argument parsing function

#if defined(__linux__)
#include <fstream>
#include <sched.h>
#include <string>
#endif

/**
 * @brief This example requires a command line argument to set the execution path. Valid values are `software_path`
 * and `hardware_path`.
 * In QPL, @ref qpl_path_software (`Software Path`) means that computations will be done with CPU.
 * Accelerator can be used instead of CPU. In this case, @ref qpl_path_hardware (`Hardware Path`) must be specified.
 * If there is no difference where calculations should be done, @ref qpl_path_auto (`Auto Path`) can be used to allow
 * the library to chose the path to execute. The Auto Path usage is not demonstrated by this example.
 *
 * @note This example requires configuring accelerators to support multiple numa nodes,
 *       run `accel-config load-config -efc 2n1d1e1w-s.conf` to enable.
 *       Incorrect configuration will result in `503 QPL_STS_INIT_WORK_QUEUES_NOT_AVAILABLE` error.
 *
 *       For multinode execution,
 *       use `numactl --cpunodebind 0-1 --membind 0-1 ./ll_cpp_crc64_numa_example hardware_path`
 *
 * @warning ---! Important !---
 * `Hardware Path` doesn't support all features declared for `Software Path`
 *
 * If only 1 socket available, NUMA node selection will be set to automatic and uses same node.
 *
 */

/**
 * @brief This function finds the total sockets number on the system and the current socket ID
 *        results are returned using pointers.
 *
 * @param cpu_id - current CPU ID as int
 * @param total_sockets - total sockets number as int pointer
 * @param socket_id - current socket ID as int pointer
 *
 * @return error code as int
*/
int get_socket_info(int cpu_id, int* total_sockets, int* socket_id) {
#if defined(__linux__)
    std::ifstream cpu_info("/proc/cpuinfo");
    std::string   line;
    *total_sockets           = -1;
    *socket_id               = -1;
    int is_current_processor = 0;
    if (!cpu_info.is_open()) {
        std::cout << "An error /proc/cpuinfo cannot be opened.\n";
        return 1;
    }
    while (std::getline(cpu_info, line)) {
        // For processor line, check if is current processor
        if (line.find("processor") != std::string::npos) {
            if (cpu_id == std::stoi(line.substr(line.find(":") + 1)))
                is_current_processor = 1;
            else
                is_current_processor = 0;
        }
        // For physical id line
        if (line.find("physical id") != std::string::npos) {
            if ((*total_sockets < (std::stoi(line.substr(line.find(":") + 1)) + 1)))
                *total_sockets = std::stoi(line.substr(line.find(":") + 1)) + 1;
            if (is_current_processor) *socket_id = std::stoi(line.substr(line.find(":") + 1));
        }
    }
    cpu_info.close();
    return 0;
#else
    std::cout << "Unsupported OS for qpl_path_hardware.\n\n";
    return -1;
#endif
}

/**
 * @brief This function finds the total NUMA node number on the system and the current node ID
 *        results are returned using pointers.
 *
 * @param cpu_id - current CPU ID as int
 * @param total_nodes - total NUMA nodes number as int pointer
 * @param numa_id - current NUMA node ID as int pointer
 *
 * @return error code as int
*/
int get_numa_info(int cpu_id, int* total_nodes, int* numa_id) {
#if defined(__linux__)
    // Get number of available NUMA nodes
    std::ifstream numa_nodes("/sys/devices/system/node/online");
    *total_nodes = -1;
    if (!numa_nodes.is_open()) {
        std::cout << "An error /sys/devices/system/node/online cannot be opened.\n";
        return 1;
    }
    std::string line;
    std::getline(numa_nodes, line);
    if (line.find("-") != std::string::npos) *total_nodes = std::stoi(line.substr(line.find("-") + 1)) + 1;
    numa_nodes.close();

    // Calculate current NUMA node
    *numa_id = -1;
    for (int i = 0; i < *total_nodes; ++i) {
        std::ifstream numa_node("/sys/devices/system/node/node" + std::to_string(i) + "/cpulist");
        if (!numa_node.is_open()) {
            std::cout << "An error /sys/devices/system/node/node" + std::to_string(i) + "/cpulist cannot be opened.\n";
            return 2;
        }
        std::getline(numa_node, line);
        if (cpu_id <= std::stoi(line.substr(line.find("-") + 1))) {
            *numa_id = i;
            break;
        }
    }
    return 0;
#else
    std::cout << "Unsupported OS for qpl_path_hardware.\n\n";
    return -1;
#endif
}

/**
 * @brief This function finds an alternative NUMA node that is different than the current NUMA node
 *
 * @note This function is optional and is not a core part of this example. Alternative method of obtaining
 *       a NUMA ID to assign Intel® Query Processing Library (Intel® QPL) task to use.
 *
 * @warning If only 1 socket available, NUMA node selection will be set to automatic and uses same node.
 *
 * @param execution_path - execution path as qpl_path_t
 * @param inv_socket - pointer to store the ID of the different socket
 * @param inv_numa_id - pointer to store the ID of the different NUMA node
 *
 * @return error code as int
*/
int get_diff_socket_numa_node_id(qpl_path_t execution_path, int* inv_socket, int* inv_numa_id) {
#if defined(__linux__)
    // Check execution path
    if (execution_path == qpl_path_software) {
        std::cout << "Software path detected, no accelerators available for NUMA assignment.\n";
        return -1;
    }

    // Get currently used CPU
    const int cpu_id = sched_getcpu();

    // Get number of available sockets and current socket
    int total_sockets = -1;
    int socket_id     = -1;
    if (get_socket_info(cpu_id, &total_sockets, &socket_id)) { return -1; }
    if (total_sockets < 2) {
        std::cout << "Warning: Single socket architecture, running on same socket.\n";
        return -1;
    }

    // Get number of available numa nodes and current node
    int total_nodes = -1;
    int numa_id     = -1;
    if (get_numa_info(cpu_id, &total_nodes, &numa_id)) { return -1; }

    // Print stats
    std::cout << "Total:\n";
    std::cout << "\t"
              << "Socket(s):" << total_sockets << "\n";
    std::cout << "\t"
              << "NUMA(s):" << total_nodes << "\n";
    std::cout << "Current:\n";
    std::cout << "\t"
              << "Core ID:" << cpu_id << "\n";
    std::cout << "\t"
              << "Socket ID:" << socket_id << "\n";
    std::cout << "\t"
              << "NUMA ID:" << numa_id << "\n";

    // Calculate different NUMA node
    const int numa_per_socket = total_nodes / total_sockets;
    *inv_socket               = !socket_id; // Get the ID for a different socket
    *inv_numa_id              = numa_per_socket * *inv_socket;

    // Return success
    return 0;
#else
    std::cout << "Unsupported OS for qpl_path_hardware.\n\n";
    return -1;
#endif
}

constexpr const uint32_t source_size   = 1000;
constexpr const uint64_t poly          = 0x04C11DB700000000;
constexpr const uint64_t reference_crc = 6467333940108591104;

auto main(int argc, char** argv) -> int {
    std::cout << "Intel(R) Query Processing Library version is " << qpl_get_library_version() << ".\n\n";

    // Default to Software Path
    qpl_path_t execution_path = qpl_path_software;

    // Get path from input argument
    const int parse_ret = parse_execution_path(argc, argv, &execution_path);
    if (parse_ret != 0) { return 1; }

    // Source and output containers
    std::vector<uint8_t> source(source_size, 4);

    std::unique_ptr<uint8_t[]> job_buffer;
    uint32_t                   size = 0;

    // Filling source containers
    std::iota(std::begin(source), std::end(source), 0);

    // Job initialization
    qpl_status status = qpl_get_job_size(execution_path, &size);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during job size getting.\n";
        return 1;
    }

    job_buffer   = std::make_unique<uint8_t[]>(size);
    qpl_job* job = reinterpret_cast<qpl_job*>(job_buffer.get());

    status = qpl_init_job(execution_path, job);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during job initializing.\n";
        return 1;
    }

    // Performing an operation
    job->op           = qpl_op_crc64;
    job->next_in_ptr  = source.data();
    job->available_in = source_size;
    job->crc64_poly   = poly;

    // Setting NUMA node for device selection
    int inv_socket = -1;
    int numa_node  = -1;
    get_diff_socket_numa_node_id(execution_path, &inv_socket, &numa_node);
    std::cout << "Running on:\n";
    std::cout << "\t"
              << "Socket ID:" << inv_socket << "\n";
    std::cout << "\t"
              << "NUMA ID:" << numa_node << "\n\n";
    std::cout << "This example would be run using accelerator devices from NUMA node " << numa_node << "\n\n";
    job->numa_id = numa_node;

    status = qpl_execute_job(job);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during CRC calculation.\n";
        return 1;
    }

    const auto crc_value = job->crc64;

    // Freeing resources
    status = qpl_fini_job(job);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during job finalization.\n";
        return 1;
    }

    // Compare with reference
    if (crc_value != reference_crc) {
        std::cout << "CRC value was calculated incorrectly.\n";
        return 1;
    }

    std::cout << "CRC64 was performed successfully. Calculated CRC: " << crc_value << "\n";

    return 0;
}

//* [QPL_LOW_LEVEL_CRC64_CROSS_SOCKET_EXAMPLE] */
