/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/qpl.h"

#include "t_common.hpp"

// tool_hw_dispatcher
#include "test_hw_device.hpp"

// tool_common
#include "system_info.hpp"
#include "topology.hpp"
#include "util.hpp"

namespace qpl::test {

#ifdef __linux__

struct toy_device {
    uint64_t numa_node_id;
    uint64_t socket_id;
};

// note: using total NUMA nodes here so that it is aligned with extended_info_t format
constexpr uint64_t test_total_sockets    = 2U;
constexpr uint64_t test_total_numa_nodes = 8U;

auto create_devices(uint64_t sockets = test_total_sockets, uint64_t numa_nodes = test_total_numa_nodes)
        -> std::vector<toy_device> {
    std::vector<toy_device> devices;

    const uint64_t numa_nodes_per_socket = numa_nodes / sockets;

    for (uint64_t socket = 0; socket < sockets; ++socket) {
        for (uint64_t numa_node = 0; numa_node < numa_nodes_per_socket; ++numa_node) {
            devices.push_back({numa_node + socket * numa_nodes_per_socket, socket});
        }
    }

    return devices;
}

/**
 * Current policy is such that if device has no numa_id information,
 * it could be used with any device selection policy.
 */
QPL_UNIT_API_ALGORITHMIC_TEST(device_selection, numa_id_none) {
    QPL_SKIP_TEST_FOR(qpl_path_software);
    QPL_SKIP_TEST_FOR(qpl_path_auto);

    const toy_device device = {(uint64_t)(-1), (uint64_t)(-1)};

    ASSERT_TRUE(is_device_matching_user_numa_policy(device.numa_node_id, device.socket_id, QPL_DEVICE_NUMA_ID_ANY));
    ASSERT_TRUE(is_device_matching_user_numa_policy(device.numa_node_id, device.socket_id, QPL_DEVICE_NUMA_ID_CURRENT));
    ASSERT_TRUE(is_device_matching_user_numa_policy(device.numa_node_id, device.socket_id, QPL_DEVICE_NUMA_ID_SOCKET));
    ASSERT_TRUE(is_device_matching_user_numa_policy(device.numa_node_id, device.socket_id, 1));
}

/**
 * If numa_id is specified, device should have the same numa_id to be selected.
 */
QPL_UNIT_API_ALGORITHMIC_TEST(device_selection, numa_id_specific) {
    QPL_SKIP_TEST_FOR(qpl_path_software);
    QPL_SKIP_TEST_FOR(qpl_path_auto);

    const auto devices = create_devices();

    for (uint64_t numa_node = 0U; numa_node < test_total_numa_nodes; ++numa_node) {
        for (const auto& device : devices) {
            if (device.numa_node_id == numa_node) {
                ASSERT_TRUE(is_device_matching_user_numa_policy(device.numa_node_id, device.socket_id, numa_node));
            } else {
                ASSERT_FALSE(is_device_matching_user_numa_policy(device.numa_node_id, device.socket_id, numa_node));
            }
        }
    }
}

/**
 * If QPL_DEVICE_NUMA_ID_ANY is specified, device should be selected regardless of its numa_id or socket_id.
 */
QPL_UNIT_API_ALGORITHMIC_TEST(device_selection, numa_id_any) {
    QPL_SKIP_TEST_FOR(qpl_path_software);
    QPL_SKIP_TEST_FOR(qpl_path_auto);

    const auto devices = create_devices();

    for (const auto& device : devices) {
        ASSERT_TRUE(is_device_matching_user_numa_policy(device.numa_node_id, device.socket_id, QPL_DEVICE_NUMA_ID_ANY));
    }
}

/**
 * If QPL_DEVICE_NUMA_ID_CURRENT is specified, device should be selected only if its numa_id matches the current numa_id.
 */
QPL_UNIT_API_ALGORITHMIC_TEST(device_selection, numa_id_current) {
    QPL_SKIP_TEST_FOR(qpl_path_software);
    QPL_SKIP_TEST_FOR(qpl_path_auto);

    const auto& system_info = get_sys_info();
    const auto  devices     = create_devices(system_info.cpu_sockets, system_info.cpu_numa_nodes);

    for (const auto& device : devices) {
        if (device.numa_node_id == get_numa_id()) {
            ASSERT_TRUE(is_device_matching_user_numa_policy(device.numa_node_id, device.socket_id,
                                                            QPL_DEVICE_NUMA_ID_CURRENT));
        } else {
            ASSERT_FALSE(is_device_matching_user_numa_policy(device.numa_node_id, device.socket_id,
                                                             QPL_DEVICE_NUMA_ID_CURRENT));
        }
    }
}

/**
 * If QPL_DEVICE_NUMA_ID_SOCKET is specified, device should be selected only if its socket_id matches the current socket_id.
 */
QPL_UNIT_API_ALGORITHMIC_TEST(device_selection, numa_id_any_from_socket) {
    QPL_SKIP_TEST_FOR(qpl_path_software);
    QPL_SKIP_TEST_FOR(qpl_path_auto);

    const auto& system_info = get_sys_info();
    const auto  devices     = create_devices(system_info.cpu_sockets, system_info.cpu_numa_nodes);

    for (const auto& device : devices) {
        if (device.socket_id == get_socket_id()) {
            ASSERT_TRUE(is_device_matching_user_numa_policy(device.numa_node_id, device.socket_id,
                                                            QPL_DEVICE_NUMA_ID_SOCKET));
        } else {
            ASSERT_FALSE(is_device_matching_user_numa_policy(device.numa_node_id, device.socket_id,
                                                             QPL_DEVICE_NUMA_ID_SOCKET));
        }
    }
}

#endif // __linux__

} // namespace qpl::test
