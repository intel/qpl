/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_HIGH_LEVEL_DEFLATE_BLOCK_EXAMPLE] */

#include <iostream>
#include <vector>
#include <numeric>

#include "qpl/qpl.hpp"
#include <string>

constexpr uint32_t source_size      = 100000;

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source(source_size, 0);
    std::vector<uint8_t> destination(source_size, 4);

    // Filling source containers
    std::iota(std::begin(source), std::end(source), 0);

    auto deflate_operation = qpl::deflate_operation::builder()
            .compression_level(qpl::high_level)
            .build();

    auto result = qpl::build_deflate_block(deflate_operation,
                                           source.begin(),
                                           source.end(),
                                           qpl::mini_block_size_1k);

    if (result.compressed_size() >= source_size || !result.compressed_size()) {
        throw std::runtime_error("Invalid compressed size: " + std::to_string(result.compressed_size()));
    }

    for (size_t i = 0; i < result.size(); i++) {
        if (result[i] != source[i]) {
            throw std::runtime_error("Incorrect value was chosen while operation performing.");
        }
    }

    std::cout << "Content was successfully compressed by deflate_block." << std::endl;
    std::cout << "Compressed size: " << result.compressed_size() << std::endl;

    return 0;
}

//* [QPL_HIGH_LEVEL_DEFLATE_BLOCK_EXAMPLE] */
