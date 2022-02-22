/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/cpp_api/util/compression_utils.hpp"
#include "compression/inflate/inflate_state.hpp"
#include "compression/deflate/streams/sw_deflate_state.hpp"

namespace qpl {

class inflate_operation;

class deflate_operation;

namespace util {

template <>
auto get_buffer_size<inflate_operation>() -> uint32_t {
    return ml::compression::inflate_state<static_cast<ml::execution_path_t>(execution_path::software)>::get_buffer_size();
}

template <>
auto get_buffer_size<deflate_operation>() -> uint32_t {
    return ml::compression::deflate_state<static_cast<ml::execution_path_t>(execution_path::software)>::required_buffer_size();
}

} // namespace util

} // namespace qpl
