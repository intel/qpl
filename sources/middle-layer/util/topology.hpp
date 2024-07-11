/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_MIDDLE_LAYER_UTIL_TOPOLOGY_HPP_
#define QPL_SOURCES_MIDDLE_LAYER_UTIL_TOPOLOGY_HPP_

#include <cstdint>

namespace qpl::ml::util {

int32_t get_numa_id() noexcept;

uint64_t get_socket_id() noexcept;

uint64_t get_socket_id(int numa_node) noexcept;

} // namespace qpl::ml::util

#endif //QPL_SOURCES_MIDDLE_LAYER_UTIL_TOPOLOGY_HPP_
