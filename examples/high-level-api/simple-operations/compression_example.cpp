/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_HIGH_LEVEL_COMPRESSION_EXAMPLE] */

#include <iostream>
#include <string>

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
constexpr const auto execution_path = qpl::software;

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source(1000, 5);
    std::vector<uint8_t> destination(500, 4);
    std::vector<uint8_t> reference(1000, 7);

    // Performing an operations
    auto deflate_operation = qpl::deflate_operation::builder()
            .compression_level(qpl::compression_levels::default_level)
            .compression_mode<qpl::compression_modes::dynamic_mode>()
            .gzip_mode(false)
            .build();
    auto inflate_operation = qpl::inflate_operation();

    const auto compressed_result = qpl::execute<execution_path>(deflate_operation,
                                                               std::begin(source),
                                                               std::end(source),
                                                               std::begin(destination),
                                                               std::end(destination));

    // Handle compression result
    compressed_result.handle([](uint32_t value) -> void {
                                 std::cout << "Compressed size: " << value << std::endl;
                             },
                             [](uint32_t status_code) -> void {
                                 throw std::runtime_error("Error: Status code - " + std::to_string(status_code));
                             });

    const auto decompressed_result = qpl::execute<execution_path>(inflate_operation, destination, reference);

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

    return 0;
}

//* [QPL_HIGH_LEVEL_COMPRESSION_EXAMPLE] */
