/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TOOLS_UTILS_HW_DISPATCHER_TEST_HW_DEVICE_HPP_
#define QPL_TOOLS_UTILS_HW_DISPATCHER_TEST_HW_DEVICE_HPP_

#if defined(__linux__)

#include <array>
#include <stddef.h>

#include "test_hw_configuration_driver.h"
#include "test_hw_queue.hpp"
#include "test_hw_status.h"

namespace qpl::test {
/**< Name used to search for devices */
#define QPL_TEST_IAA_DEVICE \
    ((uint32_t)(((uint32_t)0xFF << 24U) | ((uint32_t)('x') << 16U) | ((uint32_t)('a') << 8U) | (uint32_t)('i')))

/**< Max number of devices, used to initialize the devices array in hw_dispatcher */
#define QPL_TEST_MAX_NUM_DEV 100U

/**< Character mask to convert the first 3 bytes to lower case and make the 4th 0xff */
#define QPL_TEST_CHAR_MSK 0xFF202020

/**< IAACAP bit 0 - generation 2 Minimum Capabilities support */
#define QPL_TEST_IC_GEN_2_MIN_CAP(IAACAP) (((IAACAP)) & 0x01)

/**< IAACAP bit 3 - Force Array Output Modification Support */
#define QPL_TEST_IC_FORCE_ARRAY_OUTPUT_MOD(IAACAP) (((IAACAP) >> 3) & 0x01)

/**< IAACAP bit 7 - dictionary compression support */
#define QPL_TEST_IC_DICT_COMP(IAACAP) (((IAACAP) >> 7) & 0x01)

/**< OPCFG bit {OP}  - generic operation support */
#define QPL_TEST_OC_GET_OP_SUPPORTED(OPCFG, OP) ((((OPCFG)[7 - ((OP) / 32)]) >> (OP) % 32) & 0x01)

/**< Max number of workqueues, used to initialize the workqueue array in hw_dispatchwer */
#define QPL_TEST_MAX_NUM_WQ 100U

class hw_device final {

    static constexpr uint32_t max_working_queues = QPL_TEST_MAX_NUM_WQ;

    using queues_container_t   = std::array<hw_queue, max_working_queues>;
    using op_config_register_t = std::array<uint32_t, QPL_TEST_TOTAL_OP_CFG_BIT_GROUPS>;
    using opcfg_container_t    = std::array<op_config_register_t, max_working_queues>;

public:
    using descriptor_t = void;

    hw_device() noexcept = default;

    [[nodiscard]] auto initialize_new_device(descriptor_t* device_descriptor_ptr) noexcept
            -> qpl_test_hw_accelerator_status;

    [[nodiscard]] auto size() const noexcept -> size_t;

    [[nodiscard]] auto numa_id() const noexcept -> uint64_t;

    [[nodiscard]] auto socket_id() const noexcept -> uint64_t;

    [[nodiscard]] auto begin() const noexcept -> queues_container_t::const_iterator;

    [[nodiscard]] auto end() const noexcept -> queues_container_t::const_iterator;

    [[nodiscard]] auto get_gen_2_min_capabilities() const noexcept -> bool;

    [[nodiscard]] auto get_dict_compress_support() const noexcept -> bool;

    // Force Array Output Modification Support
    [[nodiscard]] auto get_force_array_output_mod_support() const noexcept -> bool;

    [[nodiscard]] auto get_opcfg_enabled() const noexcept -> bool;

    [[nodiscard]] auto get_operation_supported_on_wq(const uint32_t wq_idx, const uint32_t operation) const noexcept
            -> bool;

    [[nodiscard]] auto get_engine_count() const noexcept -> uint32_t;

private:
    queues_container_t working_queues_ = {}; /**< Set of available HW working queues */
    opcfg_container_t  op_configs_     = {}; /**< Array of OPCFG register content for each available HW working queue */
    uint32_t           queue_count_    = 0U; /**< Number of working queues that are available */
    uint64_t           iaa_cap_register_ = 0U;    /**< IAACAP register content */
    uint64_t           numa_node_id_     = 0U;    /**< NUMA node id of the device */
    uint32_t           version_major_    = 0U;    /**< Major version of discovered device */
    uint32_t           version_minor_    = 0U;    /**< Minor version of discovered device */
    bool               op_cfg_enabled_   = false; /**< Need to check workqueue's OPCFG register */
    uint32_t           engine_count_     = 0U;    /**< Number of engines */
    uint64_t           socket_id_        = 0U;    /**< Socket id of the device */
};

bool is_device_matching_user_numa_policy(uint64_t numa_node_id_, uint64_t socket_id_,
                                         int32_t user_specified_numa_id) noexcept;

} // namespace qpl::test

#endif //__linux__
#endif //QPL_TOOLS_UTILS_HW_DISPATCHER_TEST_HW_DEVICE_HPP_
