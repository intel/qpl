/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_MIDDLE_LAYER_COMPRESSION_VERIFICATION_VERIFICATION_UNITS_HPP
#define QPL_MIDDLE_LAYER_COMPRESSION_VERIFICATION_VERIFICATION_UNITS_HPP

#include "compression/inflate/deflate_body_decompression.hpp"
#include "compression/inflate/deflate_header_decompression.hpp"
#include "compression/inflate/isal_kernels_wrappers.hpp"
#include "verification_defs.hpp"
#include "verification_state.hpp"

namespace qpl::ml::compression {
auto verify_deflate_header(verify_state<execution_path_t::software>& state) noexcept -> verification_result_t;

auto verify_deflate_stream_body(verify_state<execution_path_t::software>& state) noexcept -> verification_result_t;
} // namespace qpl::ml::compression
#endif //QPL_MIDDLE_LAYER_COMPRESSION_VERIFICATION_VERIFICATION_UNITS_HPP
