/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <qpl/qpl.h>
#include <utility.hpp>
#include <vector>

namespace bench::data {
static const uint32_t min_tested_block_size = 4096U;
static const uint32_t max_tested_block_size = 65536U;

static inline dataset_t read_dataset(const std::string& path) {
    dataset_t dataset;
    if (!path.size()) return dataset;

    auto real_path = std::filesystem::canonical(std::filesystem::absolute(std::filesystem::path(path.c_str())));

    for (const auto& entry : std::filesystem::directory_iterator(real_path)) {

        if (entry.is_symlink()) {
            std::cerr << "Skip this file because it is a symlink. Path=" << real_path << "/"
                      << entry.path().filename().string() << '\n';
        } else {
            std::ifstream file(entry.path(), std::ios::in | std::ios::binary);
            auto data = std::vector<uint8_t>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

            dataset.push_back({entry.path().filename(), std::move(data)});
        }
    }
    std::sort(dataset.begin(), dataset.end(), [](auto& a, auto& b) { return a.name < b.name; });

    return dataset;
}

// This function will return a vector of block sizes, from min_tested_block_size => max_tested_block_size
// where only one size is larger/same size as the full data size (0 is considered larger/same size)
static inline std::vector<std::uint32_t> generate_block_sizes(const data_t& data) {
    uint32_t              max_block_size_for_data = std::min((size_t)max_tested_block_size, data.buffer.size());
    std::vector<uint32_t> block_sizes;
    uint32_t              size = min_tested_block_size;
    for (size = min_tested_block_size; size <= max_block_size_for_data; size *= 2) {
        block_sizes.push_back(size);
    }
    if (size / 2 < data.buffer.size()) { block_sizes.push_back(0); }
    return block_sizes;
}

static inline auto split_data(const data_t& data, std::size_t block_size) {
    blocks_t blocks;
    if (!block_size || data.buffer.size() <= block_size) {
        blocks.resize(1);
        blocks[0]      = data;
        blocks[0].name = data.name;
    } else {
        // Trim tail since it cannot be used in statistics
        std::uint32_t count = data.buffer.size() / block_size;
        blocks.resize(count);
        for (std::uint32_t i = 0U; i < count; ++i) {
            auto begin = &data.buffer[i * block_size];
            auto end   = &data.buffer[i * block_size + block_size];
            blocks[i].buffer.insert(blocks[i].buffer.begin(), begin, end);
            std::string size_name;
            if (block_size < 1024)
                size_name = std::to_string(block_size) + "b";
            else if (block_size < 1024 * 1024)
                size_name = std::to_string(block_size / 1024) + "kb";
            else
                size_name = std::to_string(block_size / 1024 / 1024) + "mb";
            blocks[i].name = data.name + format("/%s/block:%d-%d", size_name.c_str(), i + 1, count);
        }
    }

    return blocks;
}
} // namespace bench::data
