/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TOOLS_UTILS_COMMON_DISPATCHER_CHECKS_HPP_
#define QPL_TOOLS_UTILS_COMMON_DISPATCHER_CHECKS_HPP_

#include "test_hw_dispatcher.hpp"
// tool_common
#include "system_info.hpp"

namespace qpl::test {

inline bool are_workqueues_available(int32_t numa_id = -1) {
    if (numa_id == -1) {
        numa_id = get_numa_id();
    }

#if defined( __linux__ )
    static auto &dispatcher = hw_dispatcher::get_instance();
    if (dispatcher.is_hw_support()) {
        for (size_t device_idx = 0; device_idx < dispatcher.device_count(); device_idx++) {
            if(dispatcher.device(device_idx).numa_id() != numa_id){
                continue;
            }
            if (dispatcher.device(device_idx).size() != 0) {
                return true;
            }
        }
    }
#endif

    return false;
}

}

#endif // QPL_TOOLS_UTILS_COMMON_DISPATCHER_CHECKS_HPP_
