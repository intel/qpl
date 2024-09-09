/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TOOLS_UTILS_COMMON_OPCFG_CHECKS_HPP_
#define QPL_TOOLS_UTILS_COMMON_OPCFG_CHECKS_HPP_

// tool_hw_dispatcher
#include "test_hw_dispatcher.hpp"

// tool_common
#include "topology.hpp"

namespace qpl::test {

// Operation codes for Intel® In-Memory Analytics Accelerator (Intel® IAA)
// Used in testing to check OPCFG if operation is enabled/disabled
enum qpl_test_opcodes {
    opcode_decompress = 0x42U, /**< Intel® IAA decompress operation code */
    opcode_compress   = 0x43U, /**< Intel® IAA compress operation code */
    opcode_crc64      = 0x44U, /**< Intel® IAA crc64 operation code */
    opcode_scan       = 0x50U, /**< Intel® IAA scan operation code */
    opcode_extract    = 0x52U, /**< Intel® IAA extract operation code */
    opcode_select     = 0x53U, /**< Intel® IAA select operation code */
    opcode_expand     = 0x56U, /**< Intel® IAA expand operation code */
};

inline bool is_opcfg_capabilities_present() {
    bool are_opcfg_capabilities_present = false;

#if defined(__linux__)
    static auto& dispatcher = hw_dispatcher::get_instance();
    if (dispatcher.is_hw_support()) {
        const auto& device             = dispatcher.device(0);
        are_opcfg_capabilities_present = device.get_opcfg_enabled();
    }
#endif

    return are_opcfg_capabilities_present;
}

/**< OPCFG CHECK FUNCTIONS */
inline bool is_operation_disabled_on_all_wq_on_node(qpl_test_opcodes opcode) {
    if (!is_opcfg_capabilities_present()) { return false; }

    bool    is_op_disabled = true;
    int32_t numa_id        = get_numa_id();

#if defined(__linux__)
    static auto& dispatcher = hw_dispatcher::get_instance();
    if (dispatcher.is_hw_support()) {
        for (size_t device_idx = 0; device_idx < dispatcher.device_count(); device_idx++) {
            if (dispatcher.device(device_idx).numa_id() != numa_id) { continue; }
            for (size_t wq_idx = 0; wq_idx < dispatcher.device(device_idx).size(); wq_idx++) {
                if (dispatcher.device(device_idx).get_operation_supported_on_wq(wq_idx, opcode)) { return false; }
            }
        }
    }
#endif

    return is_op_disabled;
}

} // namespace qpl::test

#endif // QPL_TOOLS_UTILS_COMMON_OPCFG_CHECKS_HPP_
