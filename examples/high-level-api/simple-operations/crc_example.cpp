/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_HIGH_LEVEL_CRC_EXAMPLE] */

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
constexpr const uint64_t poly          = 0x04C11DB700000000;
constexpr const uint64_t reference_crc = 6467333940108591104;

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source(source_size, 0);
    uint64_t             crc               = 0;
    auto                 destination_begin = reinterpret_cast<uint8_t *>(&crc);
    uint8_t              *destination_end  = destination_begin + 8u;

    // Filling source containers
    std::iota(std::begin(source), std::end(source), 0);

    // Performing an operation
    auto crc_operation = qpl::crc_operation::builder(poly)
            .bit_order<qpl::parsers::little_endian_packed_array>()
            .is_inverse(false)
            .build();

    auto result = qpl::execute<execution_path>(crc_operation, source.begin(), source.end(), destination_begin, destination_end);

    // Handle extract result
    result.handle([&crc](uint32_t crc_size) -> void {
                      // Check if everything was alright
                      if (crc != reference_crc) {
                          throw std::runtime_error("Incorrect value was chosen while operation performing.");
                      }
                  },
                  [](uint32_t status) -> void {
                      throw std::runtime_error("Error: Status code - " + std::to_string(status));
                  });

    std::cout << "CRC was performed successfully." << std::endl;

    return 0;
}

//* [QPL_HIGH_LEVEL_CRC_EXAMPLE] */
