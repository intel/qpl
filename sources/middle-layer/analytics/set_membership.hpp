/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_MIDDLE_LAYER_ANALYTICS_SET_MEMBERSHIP_HPP_
#define QPL_SOURCES_MIDDLE_LAYER_ANALYTICS_SET_MEMBERSHIP_HPP_

#include "input_stream.hpp"
#include "output_stream.hpp"

namespace qpl::ml::analytics {

template <execution_path_t path>
auto call_set_membership(input_stream_t &input_stream,
                         input_stream_t &set_stream,
                         output_stream_t<bit_stream> &output_stream,
                         uint32_t low_bits_to_ignore,
                         uint32_t high_bits_to_ignore,
                         limited_buffer_t &unpack_buffer,
                         limited_buffer_t &set_buffer,
                         int32_t numa_id = -1) noexcept -> analytic_operation_result_t;

} // namespace qpl::ml::analytics

#endif //QPL_SOURCES_MIDDLE_LAYER_ANALYTICS_SET_MEMBERSHIP_HPP_
