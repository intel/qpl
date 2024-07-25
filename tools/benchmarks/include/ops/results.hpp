/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <types.hpp>
#include <utility.hpp>
#include <vector>

namespace bench::ops {
struct deflate_results_t {
    using data_type_t = std::vector<std::uint8_t>;

    explicit deflate_results_t() {}

    data_type_t stream_;
};

struct inflate_results_t {
    using data_type_t = std::vector<std::uint8_t>;

    explicit inflate_results_t() {}

    data_type_t data_;
};

struct crc64_results_t {
    using data_type_t = std::vector<std::uint8_t>;

    explicit crc64_results_t() {}

    data_type_t data_;
};
} // namespace bench::ops
