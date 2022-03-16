/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_HIGH_LEVEL_FIND_UNIQUE_EXAMPLE] */

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
 * @note More information about paths is in the documentation(doc/QPL_REFERENCE_MANUAL.md)
 */
constexpr const auto execution_path    = qpl::software;
constexpr const uint32_t source_size   = 1000;
constexpr const uint32_t alphabet_size = 256;

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source(source_size, 0);
    std::vector<uint8_t> destination(source_size, 4);

    // Filling source containers
    std::iota(std::begin(source), std::end(source), 0);

    // Performing an operation
    auto find_unique_operation = qpl::find_unique_operation();

    auto result = qpl::execute<execution_path>(find_unique_operation, source, destination);

    // Handle extract result
    result.handle([](uint32_t value) -> void {
                      // Check if everything was alright
                      if (value != alphabet_size) {
                          throw std::runtime_error("Incorrect result was chosen while operation performing.");
                      }
                  },
                  [](uint32_t status) -> void {
                      throw std::runtime_error("Error: Status code - " + std::to_string(status));
                  });

    std::cout << "Find unique was performed successfully." << std::endl;

    return 0;
}

//* [QPL_HIGH_LEVEL_FIND_UNIQUE_EXAMPLE] */
