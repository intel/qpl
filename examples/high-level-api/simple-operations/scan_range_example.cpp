/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_HIGH_LEVEL_SCAN_RANGE_EXAMPLE] */

#include <iostream>
#include <numeric>
#include <string>
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
constexpr const auto execution_path          = qpl::software;
constexpr const uint32_t source_size         = 1000;
constexpr const uint32_t input_vector_width  = 8;
constexpr const uint32_t output_vector_width = 32;
constexpr const uint32_t lower_boundary      = 48;
constexpr const uint32_t upper_boundary      = 58;

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source(source_size, 0);
    std::vector<uint8_t> destination(source_size * 4, 4);

    const auto *indices = reinterpret_cast<const uint32_t *>(destination.data());

    // Filling source containers
    std::iota(std::begin(source), std::end(source), 0);

    // Performing an operation
    auto scan_range_operation = qpl::scan_range_operation::builder(lower_boundary, upper_boundary)
            .input_vector_width(input_vector_width)
            .output_vector_width(output_vector_width)
            .parser<qpl::parsers::big_endian_packed_array>(source_size)
            .build();

    const auto scan_range_result = qpl::execute<execution_path>(scan_range_operation,
                                                               std::begin(source),
                                                               std::end(source),
                                                               std::begin(destination),
                                                               std::end(destination));

    // Handle scan range result
    scan_range_result.handle([&source, &indices](uint32_t scan_range_size) -> void {
                                 // Check if everything was alright
                                 for (uint32_t i = 0; i < scan_range_size; i++) {
                                     const auto element = source[indices[i]];

                                     if (element < lower_boundary || element > upper_boundary) {
                                         throw std::runtime_error("Incorrect value was chosen while operation "
                                                                  "performing.");
                                     }
                                 }
                             },
                             [](uint32_t status_code) -> void {
                                 throw std::runtime_error("Error: Status code - " + std::to_string(status_code));
                             });

    std::cout << "Scan range was performed successfully." << std::endl;

    return 0;
}

//* [QPL_HIGH_LEVEL_SCAN_RANGE_EXAMPLE] */
