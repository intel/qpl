/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_HIGH_LEVEL_COPY_EXAMPLE] */

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
    std::vector<uint8_t> destination(source_size, 4);

    // Filling source containers
    std::iota(std::begin(source), std::end(source), 0);

    // Performing an operation
    auto copy_operation = qpl::copy_operation();

    auto result = qpl::execute<execution_path>(copy_operation, source, destination);

    // Handle extract result
    result.handle([&destination, &source](uint32_t copy_size) -> void {
                      // Check if everything was alright
                      if ((copy_size != source_size) || (destination != source)) {
                          throw std::runtime_error("Incorrect value was chosen while operation performing.");
                      }
                  },
                  [](uint32_t status) -> void {
                      throw std::runtime_error("Error: Status code - " + std::to_string(status));
                  });

    std::cout << "Copy was performed successfully." << std::endl;

    return 0;
}

//* [QPL_HIGH_LEVEL_COPY_EXAMPLE] */
