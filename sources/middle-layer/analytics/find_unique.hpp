/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef FIND_UNIQUE_OPERATION_HPP
#define FIND_UNIQUE_OPERATION_HPP

#include "input_stream.hpp"
#include "output_stream.hpp"

namespace qpl::ml::analytics {

template <execution_path_t path>
auto call_find_unique(input_stream_t &input_stream,
                      output_stream_t<bit_stream> &output_stream,
                      uint32_t low_bits_to_ignore,
                      uint32_t high_bits_to_ignore,
                      limited_buffer_t &unpack_buffer,
                      limited_buffer_t &set_buffer,
                      int32_t numa_id = -1) noexcept -> analytic_operation_result_t;

} // namespace qpl::ml::analytics

#endif // FIND_UNIQUE_OPERATION_HPP
