/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_HIGH_LEVEL_EXTRACT_EXAMPLE] */

#include <iostream>
#include <string>
#include <numeric>
#include <vector>

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
constexpr const auto execution_path         = qpl::software;
constexpr const uint32_t source_size        = 1000;
constexpr const uint32_t input_vector_width = 8;
constexpr const uint32_t lower_index        = 80;
constexpr const uint32_t upper_index        = 123;

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source(source_size, 0);
    std::vector<uint8_t> destination(source_size, 4);

    // Filling source containers
    std::iota(std::begin(source), std::end(source), 0);

    // Performing an operation
    auto extract_operation = qpl::extract_operation::builder(lower_index, upper_index)
            .input_vector_width(input_vector_width)
            .parser<qpl::parsers::big_endian_packed_array>(source_size)
            .build();

    auto result = qpl::execute<execution_path>(extract_operation, source, destination);

    // Handle extract result
    result.handle([&destination, &source](uint32_t extract_size) -> void {
                      // Check if everything was alright
                      for (size_t i = 0; i < extract_size; i++) {
                          if (destination[i] != source[i + lower_index]) {
                              throw std::runtime_error("Incorrect value was chosen while operation performing.");
                          }
                      }
                  },
                  [](uint32_t status) -> void {
                      throw std::runtime_error("Error: Status code - " + std::to_string(status));
                  });

    std::cout << "Extract was performed successfully." << std::endl;

    return 0;
}

//* [QPL_HIGH_LEVEL_EXTRACT_EXAMPLE] */
