/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_HIGH_LEVEL_RLE_BURST_EXAMPLE] */

#include <iostream>
#include <vector>
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
constexpr const auto execution_path      = qpl::software;
constexpr const uint32_t source_size     = 3;
constexpr const uint32_t input_bit_width = 8;

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source{1, 2, 3};
    std::vector<uint8_t> source_counter{2, 5, 4};
    std::vector<uint8_t> destination(2 + 5 + 4, 0);
    std::vector<uint8_t> reference{1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3};

    auto operation = qpl::rle_burst_operation::builder(source_counter.data(), input_bit_width)
            .input_vector_width(input_bit_width)
            .parser<qpl::parsers::little_endian_packed_array>(source_size)
            .counter_bit_width(input_bit_width)
            .build();

    qpl::execute<execution_path>(operation, source, destination)
            .handle([&destination, &reference](uint32_t element_count) {
                        // Check if everything was alright
                        for (uint32_t i = 0; i < reference.size(); i++) {
                            if (reference[i] != destination[i]) {
                                throw std::runtime_error("Incorrect value was chosen while job execution.");
                            }
                        }
                    },
                    [](auto status) {
                        throw std::runtime_error("RLE-burst ended up with code: " + std::to_string(status));
                    });

    std::cout << "RLE burst was performed successfully." << std::endl;

    return 0;
}

//* [QPL_HIGH_LEVEL_RLE_BURST_EXAMPLE] */
