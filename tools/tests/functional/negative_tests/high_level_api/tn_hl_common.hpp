/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_BAD_ARGUMENT_TESTS_HIGH_LEVEL_API_TB_COMMON_HPP_
#define QPL_BAD_ARGUMENT_TESTS_HIGH_LEVEL_API_TB_COMMON_HPP_

#include "test_cases.hpp"
#include "test_name_format.hpp"
#include "qpl/cpp_api/operations/operation.hpp"

#define QPL_HIGH_LEVEL_API_NEGATIVE_TEST(operation, test) \
    TEST(QPL_HIGH_LEVEL_SUITE_NAME(tn, operation), test)

namespace qpl::test {

    template <class input_iterator_t,
        class output_iterator_t,
        class operation_t>
        auto get_execute_status(operation_t &op,
            const input_iterator_t source_begin,
            const input_iterator_t source_end,
            const output_iterator_t destination_begin,
            const output_iterator_t destination_end) -> uint32_t {
        auto operation_result = qpl::execute(op,
            source_begin,
            source_end,
            destination_begin,
            destination_end);

        uint32_t status = 0;
        operation_result.if_absent([&status](uint32_t status_code) -> void {
            status = status_code;
            });

        return status;
    }
}

#endif // QPL_BAD_ARGUMENT_TESTS_HIGH_LEVEL_API_TB_COMMON_HPP_
