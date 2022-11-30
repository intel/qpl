/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <utility.hpp>

#include <qpl/qpl.h>

#include <vector>
#include <filesystem>
#include <fstream>

namespace bench::data
{
static inline dataset_t read_dataset(const std::string &path)
{
    dataset_t dataset;
    if(!path.size())
        return dataset;

    auto real_path = std::filesystem::canonical(std::filesystem::absolute(std::filesystem::path(path.c_str())));

    for (const auto & entry : std::filesystem::directory_iterator(real_path))
    {
        std::ifstream file(entry.path(), std::ios::in | std::ios::binary);
        auto data = std::vector<uint8_t>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

        dataset.push_back({entry.path().filename(), std::move(data)});
    }
    std::sort(dataset.begin(), dataset.end(), [](auto &a, auto &b){ return a.name < b.name; });

    return dataset;
}

static inline auto split_data(const data_t &data, std::size_t block_size)
{
    blocks_t blocks;
    if(!block_size || data.buffer.size() <= block_size)
    {
        blocks.resize(1);
        blocks[0]      = data;
        blocks[0].name = data.name;
    }
    else
    {
        // Trim tail since it cannot be used in statistics
        std::uint32_t count = data.buffer.size()/block_size;
        blocks.resize(count);
        for(std::uint32_t i = 0; i < count; ++i)
        {
            auto begin = &data.buffer[i*block_size];
            auto end   = &data.buffer[i*block_size + block_size];
            blocks[i].buffer.insert(blocks[i].buffer.begin(), begin, end);
            std::string size_name;
            if(block_size < 1024)
                size_name = std::to_string(block_size) + "b";
            else if(block_size < 1024*1024)
                size_name = std::to_string(block_size/1024) + "kb";
            else
                size_name = std::to_string(block_size/1024/1024) + "mb";
            blocks[i].name = data.name + format("/%s/block:%d-%d", size_name.c_str(), i+1, count);
        }
    }

    return blocks;
}
}

