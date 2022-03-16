/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_HIGH_LEVEL_EXPAND_EXAMPLE] */

#include <iostream>
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
constexpr const auto execution_path         = qpl::software;
constexpr const uint32_t destination_size   = 20;
constexpr const uint32_t input_elements     = 8;
constexpr const uint32_t mask_byte_length   = 1;
constexpr const uint32_t input_vector_width = 8;
constexpr const uint8_t  mask               = 0b10111001;

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source    = {1, 2, 3, 4, 5, 6 , 7, 8};
    std::vector<uint8_t> destination(destination_size, 0);
    std::vector<uint8_t> reference = {1, 0, 0, 2, 3, 4, 0, 5};

    // Performing an operation
    auto expand_operation = qpl::expand_operation::builder()
            .mask(&mask, mask_byte_length)
            .input_vector_width(input_vector_width)
            .parser<qpl::parsers::little_endian_packed_array>(input_elements)
            .build();

    auto result = qpl::execute<execution_path>(expand_operation, source, destination);

    // Handle expand result
    result.handle([&destination, &reference](uint32_t expand_size) -> void {
                      // Check if everything was alright
                      for (size_t i = 0; i < expand_size; i++) {
                          if (destination[i] != reference[i]) {
                              throw std::runtime_error("Incorrect value was chosen while operation performing.");
                          }
                      }
                  },
                  [](uint32_t status) -> void {
                      throw std::runtime_error("Error: Status code - " + std::to_string(status));
                  });

    std::cout << "Expand was performed successfully." << std::endl;

    return 0;
}

//* [QPL_HIGH_LEVEL_EXPAND_EXAMPLE] */
