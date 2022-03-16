/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_HIGH_LEVEL_MERGE_EXAMPLE] */

#include <iostream>
#include <numeric>
#include <string>

#include "qpl/qpl.hpp"

/**
 * @brief qpl::software (`Software Path`) means that computations will be done with CPU.
 * Accelerator can be used instead of CPU. In this case, qpl::hardware (`Hardware Path`) must be specified.
 * If there is no difference where calculations should be done, qpl::auto_detect (`Auto Path`) can be used to allow
 * the library to chose the path to execute.
 *
 * @warning ---! Important !---
 * `Hardware Path` doesn't support all features declared for `Software Path`
 *
 * @note More information about paths is in the documentation(doc/QPL_REFERENCE_MANUAL.md)
 */
constexpr const auto execution_path     = qpl::software;
constexpr const uint32_t source_size    = 1000;
constexpr const uint32_t lower_boundary = 48;
constexpr const uint32_t upper_boundary = 58;

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source(source_size, 0);
    std::vector<uint8_t> destination(source_size, 4);

    const auto *indices = reinterpret_cast<const uint32_t *>(destination.data());

    // Filling source container
    std::iota(std::begin(source), std::end(source), 0);

    // Performing an operation
    auto chain = qpl::deflate_operation() |
                 qpl::inflate_operation() |
                 qpl::merge(source_size) |
                 qpl::scan_range_operation(lower_boundary, upper_boundary);

    auto chain_result = qpl::execute<execution_path>(chain, source, destination);

    // Handle compression result
    chain_result.handle([&source, &indices](uint32_t result_size) -> void {
                            // Check if everything was alright
                            for (uint32_t i = 0; i < result_size; i++) {
                                const auto element = source[indices[i]];

                                if (element < lower_boundary || element > upper_boundary) {
                                    throw std::runtime_error("Incorrect value was chosen while operation performing.");
                                }
                            }
                        },
                        [](uint32_t status_code) -> void {
                            if (status_code == 52) {
                                // Chain merging feature is not fully supported @todo delete this branch once it is available
                                std::cout << "Chain merging feature is currently disabled.\n";
                                exit(0);
                            } else {
                                throw std::runtime_error("Error: Status code - " + std::to_string(status_code));
                            }
                        });

    std::cout << "Merged decompression with analytics was performed successfully." << std::endl;

    return 0;
}

//* [QPL_HIGH_LEVEL_MERGE_EXAMPLE] */
