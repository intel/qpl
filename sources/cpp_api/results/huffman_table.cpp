/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/cpp_api/results/huffman_table.hpp"
#include "compression/huffman_table/canned_utils.hpp"

namespace qpl {

huffman_table<huffman_table_type::deflate>::huffman_table() = default;

huffman_table<huffman_table_type::deflate>::huffman_table(const huffman_table<huffman_table_type::deflate> &other) = default;

huffman_table<huffman_table_type::deflate>::huffman_table(huffman_table<huffman_table_type::deflate> &&other) noexcept = default;

auto huffman_table<huffman_table_type::deflate>::operator=(huffman_table<huffman_table_type::deflate> &&other) noexcept
-> huffman_table<huffman_table_type::deflate> & = default;

auto huffman_table<huffman_table_type::deflate>::operator=(const huffman_table<huffman_table_type::deflate> &other)
-> huffman_table<huffman_table_type::deflate> & = default;

huffman_table<huffman_table_type::deflate>::~huffman_table() = default;

auto huffman_table<huffman_table_type::deflate>::get_table_data() -> qpl_compression_huffman_table * {
    return table_.data();
}


huffman_table<huffman_table_type::inflate>::huffman_table() = default;

huffman_table<huffman_table_type::inflate>::huffman_table(const huffman_table<huffman_table_type::inflate> &other) = default;

huffman_table<huffman_table_type::inflate>::huffman_table(huffman_table<huffman_table_type::inflate> &&other) noexcept = default;

auto huffman_table<huffman_table_type::inflate>::operator=(huffman_table<huffman_table_type::inflate> &&other) noexcept
-> huffman_table<huffman_table_type::inflate> & = default;

auto huffman_table<huffman_table_type::inflate>::operator=(const huffman_table<huffman_table_type::inflate> &other)
-> huffman_table<huffman_table_type::inflate> & = default;

huffman_table<huffman_table_type::inflate>::~huffman_table() = default;

auto huffman_table<huffman_table_type::inflate>::get_table_data() -> qpl_decompression_huffman_table * {
    return table_.data();
}

}
