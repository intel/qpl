/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_HIGH_LEVEL_SET_MEMBERSHIP_EXAMPLE] */

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
constexpr const auto execution_path          = qpl::software;
constexpr const uint32_t source_size         = 1000;
constexpr const uint32_t input_vector_width  = 8;
constexpr const uint32_t output_vector_width = 1;

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source(source_size, 0);
    // Size of mask is 2^N where N is input bit width
    std::vector<uint8_t> mask_after_find_unique(((1u << input_vector_width) + 7u) / 8u, 4);
    std::vector<uint8_t> destination(source_size, 4);

    // Filling source containers
    std::iota(std::begin(source), std::end(source), 0);

    // Performing an operation
    auto find_unique_operation = qpl::find_unique_operation();

    auto find_unique_result = qpl::execute(find_unique_operation, source, mask_after_find_unique);

    // Handle scan result
    uint32_t mask_length = 0;
    find_unique_result.handle([&mask_length](uint32_t value) -> void {
                                  // Converting total elements processed to the byte size of the mask.
                                  mask_length = (value + 7u) / 8u;
                              },
                              [](uint32_t status_code) -> void {
                                  throw std::runtime_error("Error: Status code - " + std::to_string(status_code));
                              });

    // Performing an operation
    auto set_membership_operation = qpl::set_membership_operation::builder(mask_after_find_unique.data(), mask_length)
            .parser<qpl::parsers::big_endian_packed_array>(source_size)
            .input_vector_width(input_vector_width)
            .output_vector_width(output_vector_width)
            .build();

    const auto set_membership_result = qpl::execute<execution_path>(set_membership_operation,
                                                                   std::begin(source),
                                                                   std::end(source),
                                                                   std::begin(destination),
                                                                   std::end(destination));

    // Handle set_membership result
    set_membership_result.handle([](uint32_t value) -> void {
                                     // Check if everything was alright
                                     if (value != source_size) {
                                         throw std::runtime_error("Incorrect value was chosen while operation "
                                                                  "performing.");
                                     }
                                 },
                                 [](uint32_t status_code) -> void {
                                     throw std::runtime_error("Error: Status code - " + std::to_string(status_code));
                                 });

    std::cout << "Set membership was performed successfully." << std::endl;

    return 0;
}

//* [QPL_HIGH_LEVEL_SET_MEMBERSHIP_EXAMPLE] */
