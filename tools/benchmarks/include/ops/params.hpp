/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <memory>
#include <types.hpp>
#include <utility.hpp>

#include "qpl/qpl.h"

namespace bench::ops {
struct deflate_params_t {
    explicit deflate_params_t() = default;
    deflate_params_t(const data_t& data, std::int32_t level, huffman_type_e huffman, bool huffman_only = false,
                     bool no_headers = false, std::shared_ptr<qpl_huffman_table> huffman_table = nullptr)
        : p_source_data_(&data)
        , level_(level)
        , huffman_(huffman)
        , huffman_only_(huffman_only)
        , no_headers_(no_headers)
        , p_huffman_table_(std::move(huffman_table)) {}

    const data_t*                      p_source_data_ {nullptr};
    std::int32_t                       level_ {0};
    huffman_type_e                     huffman_ {huffman_type_e::fixed};
    bool                               huffman_only_ {false};
    bool                               no_headers_ {false};
    std::shared_ptr<qpl_huffman_table> p_huffman_table_ {nullptr};
};

struct inflate_params_t {
    explicit inflate_params_t() = default;
    inflate_params_t(const data_t& stream, size_t original_size, bool no_headers,
                     huffman_type_e                     mode          = huffman_type_e::fixed,
                     std::shared_ptr<qpl_huffman_table> huffman_table = nullptr)
        : p_stream_(&stream)
        , original_size_(original_size)
        , no_headers_(no_headers)
        , huffman_(mode)
        , p_huffman_table_(std::move(huffman_table)) {}

    const data_t*                      p_stream_ {nullptr};
    size_t                             original_size_ {0};
    bool                               no_headers_ {false};
    huffman_type_e                     huffman_ {huffman_type_e::fixed};
    std::shared_ptr<qpl_huffman_table> p_huffman_table_ {nullptr};
};

struct crc64_params_t {
    explicit crc64_params_t() = default;
    crc64_params_t(const data_t& stream, size_t original_size, crc_type_e type)
        : p_stream_(&stream), original_size_(original_size), crc_type(type) {}

    const data_t* p_stream_ {nullptr};
    size_t        original_size_ {0};
    crc_type_e    crc_type;
};

} // namespace bench::ops
