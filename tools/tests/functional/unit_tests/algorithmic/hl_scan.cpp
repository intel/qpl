/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <algorithm>

#include "qpl_test_environment.hpp"
#include "qpl/cpp_api/util/status_handler.hpp"
#include "qpl/cpp_api/operations/operation.hpp"
#include "qpl/cpp_api/util/qpl_util.hpp"
#include "../t_common.hpp"

#include "qpl/cpp_api/operations/analytics/scan_operation.hpp"
#include "qpl/cpp_api/operations/analytics/scan_range_operation.hpp"

namespace qpl::test {

    QPL_UNIT_API_ALGORITHMIC_TEST(hl_scan, get_output_bit_width) {
        const auto comparator = qpl::greater;
        const auto parser = qpl::little_endian_packed_array;
        uint32_t boundary = 0u;
        uint32_t input_bit_width = 15u;
        uint32_t number_of_elements = 100u;

        for (uint32_t output_bit_width : {1, 8, 16, 32}) {
            auto scan_operation = typename scan_operation::builder(comparator, boundary)
                .parser<parser>(number_of_elements)
                .output_vector_width(output_bit_width)
                .input_vector_width(input_bit_width)
                .build();

            auto result = scan_operation.get_output_vector_width();
            ASSERT_EQ(output_bit_width, result);
        }
    }

    QPL_UNIT_API_ALGORITHMIC_TEST(hl_scan_range, get_output_bit_width) {
        const auto comparator = qpl::greater;
        const auto parser = qpl::little_endian_packed_array;
        uint32_t input_bit_width = 15u;
        uint32_t lower_boundary = 0u;
        uint32_t upper_boundary = (1u << input_bit_width) - 1u;
        uint32_t number_of_elements = 100u;

        for (uint32_t output_bit_width : {1, 8, 16, 32}) {
            auto scan_operation = typename scan_range_operation::builder(lower_boundary, upper_boundary)
                .parser<parser>(number_of_elements)
                .output_vector_width(output_bit_width)
                .input_vector_width(input_bit_width)
                .build();

            auto result = scan_operation.get_output_vector_width();
            ASSERT_EQ(output_bit_width, result);
        }
    }
}
