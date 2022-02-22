/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/cpp_api/results/deflate_histogram.hpp"

namespace qpl {

auto deflate_histogram::get_literals_lengths() noexcept -> uint32_t * {
    return literalsLengths_buffer_.data();
}

auto deflate_histogram::get_offsets() noexcept -> uint32_t * {
    return offsets_buffer_.data();
}

} // namespace qpl
