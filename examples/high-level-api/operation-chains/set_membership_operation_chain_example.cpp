/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_HIGH_LEVEL_SET_MEMBERSHIP_CHAIN_EXAMPLE] */

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
constexpr const auto execution_path       = qpl::software;
constexpr const uint32_t source_size      = 1000;
constexpr const uint32_t destination_size = source_size * 4;
constexpr const uint32_t lower_boundary   = 48;
constexpr const uint32_t upper_boundary   = 58;

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source(source_size, 0);
    std::vector<uint8_t> source_for_set_membership(source_size, 0);
    std::vector<uint8_t> mask_after_scan_range(destination_size, 0);
    std::vector<uint8_t> destination(destination_size, 0);

    // Filling source containers
    std::iota(std::begin(source), std::end(source), 0);
    std::iota(std::begin(source_for_set_membership), std::end(source_for_set_membership), 0);

    // Performing a scanning
    auto scan_range_operation = qpl::scan_range_operation::builder(lower_boundary, upper_boundary)
            .parser<qpl::parsers::big_endian_packed_array>(source_size)
            .build();

    // Performing scanning -> selection -> find unique
    auto chain_with_scan_range = scan_range_operation |
                                 qpl::select_operation() |
                                 qpl::find_unique_operation();

    auto chain_with_scan_range_result = qpl::execute<execution_path>(chain_with_scan_range,
                                                                    source,
                                                                    mask_after_scan_range);

    // Handle chain with scan range result
    uint32_t mask_length = 0;
    chain_with_scan_range_result.handle([&mask_length](uint32_t value) -> void {
                                            mask_length = value;
                                        },
                                        [](uint32_t status) -> void {
                                            throw std::runtime_error("Error: Status code - " + std::to_string(status));
                                        });

    // Performing a set membership
    auto set_membership_operation = qpl::set_membership_operation::builder(mask_after_scan_range.data(), mask_length)
            .parser<qpl::parsers::big_endian_packed_array>(source_size)
            .input_vector_width(8)
            .output_vector_width(1)
            .build();

    // Performing set membership -> selection -> find unique
    auto chain_with_set_membership = set_membership_operation |
                                     qpl::select_operation() |
                                     qpl::find_unique_operation();

    auto chain_with_set_membership_result = qpl::execute(chain_with_set_membership,
                                                         source_for_set_membership,
                                                         destination);

    uint32_t mask_after_set_membership_length = 0;
    chain_with_set_membership_result.handle([&mask_after_set_membership_length](uint32_t value) -> void {
                                                mask_after_set_membership_length = value;
                                            },
                                            [](uint32_t status) -> void {
                                                throw std::runtime_error("Error: Status code - "
                                                                         + std::to_string(status));
                                            });

    if (mask_length == mask_after_set_membership_length) {
        for (uint32_t i = 0; i < mask_length; i++) {
            if (mask_after_scan_range[i] != destination[i]) {
                throw std::runtime_error("Error: Result after scan_range is not equal to result after set_membership");
            }
        }
        std::cout << "Result size: " << mask_after_set_membership_length << std::endl;
        std::cout << "Test passed successfully." << std::endl;
    } else {
        throw std::runtime_error("Error: Result after scan_range is not equal to result after set_membership");
    }

    return 0;
}

//* [QPL_HIGH_LEVEL_SET_MEMBERSHIP_CHAIN_EXAMPLE] */