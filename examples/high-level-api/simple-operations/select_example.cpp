/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_HIGH_LEVEL_SELECT_EXAMPLE] */

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
 */
constexpr const auto execution_path               = qpl::software;
constexpr const uint32_t source_size              = 1000;
constexpr const uint32_t boundary                 = 48;
constexpr const uint32_t scan_input_vector_width  = 8;
constexpr const uint32_t scan_output_vector_width = 1;

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source(source_size, 0);
    std::vector<uint8_t> mask_after_scan(source_size / 8, 4);
    std::vector<uint8_t> destination(source_size, 4);

    // Filling source containers
    std::iota(std::begin(source), std::end(source), 0);

    // Performing an operation
    auto scan_operation = qpl::scan_operation::builder(qpl::equals, boundary)
            .input_vector_width(scan_input_vector_width)
            .output_vector_width(scan_output_vector_width)
            .parser<qpl::parsers::little_endian_packed_array>(source_size)
            .is_inclusive(false)
            .build();

    const auto scan_result = qpl::execute<execution_path>(scan_operation,
                                                         std::begin(source),
                                                         std::end(source),
                                                         std::begin(mask_after_scan),
                                                         std::end(mask_after_scan));

    // Handle scan result
    uint32_t mask_length = 0;
    scan_result.handle([&mask_length](uint32_t value) -> void {
                           // Converting total elements processed to the byte size of the mask.
                           mask_length = (value + 7u) / 8u;
                       },
                       [](uint32_t status_code) -> void {
                           throw std::runtime_error("Error: Status code - " + std::to_string(status_code));
                       });

    // Performing an operation
    auto select_operation = qpl::select_operation(mask_after_scan.data(), mask_length);

    const auto select_result = qpl::execute<execution_path>(select_operation,
                                                           std::begin(source),
                                                           std::end(source),
                                                           std::begin(destination),
                                                           std::end(destination));

    // Handle select result
    select_result.handle([&destination](uint32_t select_size) -> void {
                             // Check if everything was alright
                             for (uint32_t i = 0; i < select_size; i++) {
                                 if (destination[i] != boundary) {
                                     throw std::runtime_error("Incorrect value was chosen while operation "
                                                              "performing.");
                                 }
                             }
                         },
                         [](uint32_t status_code) -> void {
                             throw std::runtime_error("Error: Status code - " + std::to_string(status_code));
                         });

    std::cout << "Select was performed successfully." << std::endl;

    return 0;
}

//* [QPL_HIGH_LEVEL_SELECT_EXAMPLE] */
