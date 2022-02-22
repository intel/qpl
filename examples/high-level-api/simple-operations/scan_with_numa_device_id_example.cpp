/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "qpl/c_api/status.h"
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
constexpr const uint32_t boundary            = 48;
constexpr const uint32_t input_vector_width  = 8;
constexpr const uint32_t output_vector_width = 32;

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source(source_size, 0);
    std::vector<uint8_t> destination(source_size, 4);

    const auto *indices = reinterpret_cast<const uint32_t *>(destination.data());

    // Filling source containers
    std::iota(std::begin(source), std::end(source), 0);

    // Performing an operation
    auto scan_operation = qpl::scan_operation::builder(qpl::equals, boundary)
            .input_vector_width(input_vector_width)
            .output_vector_width(output_vector_width)
            .parser<qpl::parsers::big_endian_packed_array>(source_size)
            .is_inclusive(false)
            .build();

    constexpr const int32_t numa_id = 1;

    const auto scan_result = qpl::execute<execution_path>(scan_operation,
                                                         std::begin(source),
                                                         std::end(source),
                                                         std::begin(destination),
                                                         std::end(destination),
                                                         numa_id);

    // Handle scan result
    scan_result.handle([&source, &indices](uint32_t scan_size) -> void {
                           // Check if everything was alright
                           for (uint32_t i = 0; i < scan_size; i++) {
                               if (source[indices[i]] != boundary) {
                                   throw std::runtime_error("Incorrect index was chosen while operation performing.");
                               }
                           }
                       },
                       [](uint32_t status_code) -> void {
                           if (status_code >= QPL_INIT_ERROR_BASE) {
                               std::cout << "Hardware path initialization failed\n";
                               exit(0);
                           } else {
                               throw std::runtime_error("Error: Status code - " + std::to_string(status_code));
                           }
                       });

    std::cout << "Scan was performed successfully." << std::endl;

    return 0;
}
