/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_HIGH_LEVEL_COMPRESSION_STREAM_EXAMPLE] */

#include <iostream>
#include <vector>

#include "qpl/qpl.hpp"

constexpr const uint32_t source_size              = 1000;
constexpr const uint32_t destination_size         = 1000;
constexpr const uint32_t compression_chunk_size   = 250;
constexpr const uint32_t decompression_chunk_size = 125;

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source(source_size, 5);
    std::vector<uint8_t> destination(source_size / 2, 4);
    std::vector<uint8_t> reference(source_size, 7);

    // Perform compression
    auto deflate_operation = qpl::deflate_operation::builder()
            .compression_mode<qpl::compression_modes::dynamic_mode>()
            .build();
    auto deflate_stream    = qpl::deflate_stream(std::move(deflate_operation), destination_size);
    auto current_chunk     = source.begin();
    auto current_chunk_end = current_chunk + compression_chunk_size;

    while (std::distance(current_chunk_end, source.end()) > compression_chunk_size) {
        deflate_stream.push(current_chunk, current_chunk_end);

        current_chunk += compression_chunk_size;
        current_chunk_end += compression_chunk_size;
    }

    deflate_stream.flush(current_chunk, source.end());

    // Perform decompression
    auto inflate_stream = qpl::inflate_stream(qpl::inflate_operation(),
                                              deflate_stream.begin(),
                                              deflate_stream.end());
    current_chunk     = reference.begin();
    current_chunk_end = current_chunk + decompression_chunk_size;

    while (std::distance(current_chunk_end, reference.end()) > 0) {
        inflate_stream.extract(current_chunk, current_chunk_end);

        current_chunk += decompression_chunk_size;
        if (std::distance(current_chunk_end, reference.end()) < decompression_chunk_size) {
            current_chunk_end += std::distance(current_chunk_end, reference.end());
        } else {
            current_chunk_end += decompression_chunk_size;
        }
    }

    inflate_stream.extract(current_chunk, reference.end());

    // Compare source and decompressed buffers
    for (size_t i = 0; i < source.size(); i++) {
        if (source[i] != reference[i]) {
            throw std::runtime_error("Content wasn't successfully compressed and decompressed.");
        }
    }

    std::cout << "Content was successfully compressed and decompressed by compression_stream." << std::endl;

    return 0;
}

//* [QPL_HIGH_LEVEL_COMPRESSION_STREAM_EXAMPLE] */
