/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_HIGH_LEVEL_ZERO_COMPRESSION_EXAMPLE] */

#include <iostream>
#include <string>
#include <numeric>
#include <vector>

#include "qpl/qpl.hpp"

/**
 * @brief @ref qpl::software (`Software Path`) means that computations will be done with CPU.
 * Accelerator can be used instead of CPU. In this case, @ref qpl::hardware (`Hardware Path`) must be specified.
 * If there is no difference where calculations should be done, @ref qpl::auto_detect (`Auto Path`) can be used to allow
 * the library to chose the path to execute.
 *
 * @warning ---! Important !---
 * `Hardware Path` doesn't support all features declared for `Software Path`
 *
 * @note More information about paths is in the documentation(doc/QPL_REFERENCE_MANUAL.md)
 */
constexpr const auto execution_path  = qpl::software;
constexpr const uint32_t source_size = 1000;

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source(source_size, 0);
    std::vector<uint8_t> destination(source_size / 2, 4);
    std::vector<uint8_t> reference(source_size, 7);

    // Filling source containers
    std::iota(std::begin(source) + 250, std::begin(source) + 500, 0);

    // Performing an operation
    auto zero_compress_operation   = qpl::zero_compress_operation(qpl::zero_input_format::word_32_bit);
    auto zero_decompress_operation = qpl::zero_decompress_operation(qpl::zero_input_format::word_32_bit);

    const auto compressed_result = qpl::execute<execution_path>(zero_compress_operation, source, destination);

    uint32_t compressed_size = 0;

    // Handle compression result
    compressed_result.handle([&compressed_size](uint32_t value) -> void {
                                 std::cout << "Compressed size: " << value << std::endl;
                                 compressed_size = value;
                             },
                             [](uint32_t status_code) -> void {
                                 throw std::runtime_error("Error: Status code - " + std::to_string(status_code));
                             });

    const auto decompressed_result = qpl::execute<execution_path>(zero_decompress_operation,
                                                                  destination.begin(),
                                                                  destination.begin() + compressed_size,
                                                                  reference.begin(),
                                                                  reference.end());

    // Handle decompression result
    decompressed_result.handle([&source, &reference](uint32_t decompressed_size) -> void {
                                   // Check if everything was alright
                                   if (decompressed_size != source.size()) {
                                       throw std::runtime_error("Decompressed buffer size is not the same "
                                                                "as source size.");
                                   }

                                   // Compare source and decompressed buffers
                                   for (size_t i = 0; i < source.size(); i++) {
                                       if (source[i] != reference[i]) {
                                           throw std::runtime_error("Content wasn't successfully compressed "
                                                                    "and decompressed.");
                                       }
                                   }
                               },
                               [](uint32_t status_code) -> void {
                                   throw std::runtime_error("Error: Status code - " + std::to_string(status_code));
                               });

    std::cout << "Content was successfully compressed and decompressed." << std::endl;
    std::cout << "Zero compression was performed successfully." << std::endl;

    return 0;
}

//* [QPL_HIGH_LEVEL_ZERO_COMPRESSION_EXAMPLE] */
