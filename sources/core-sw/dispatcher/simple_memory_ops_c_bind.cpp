/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <cstdint>

#include "simple_memory_ops.hpp"

// C wrapper for util::copy with uint8_t type
extern "C" void call_c_copy_uint8_t(const uint8_t* source, uint8_t* destination, uint32_t length) {
    return qpl::core_sw::util::copy(source, source + length, destination);
}

// C wrapper for util::set_zeros with uint8_t type
extern "C" void call_c_set_zeros_uint8_t(uint8_t* destination, uint32_t length) {
    return qpl::core_sw::util::set_zeros(destination, length);
}
