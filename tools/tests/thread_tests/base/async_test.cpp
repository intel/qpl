/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <vector>
#include <string>

#include "gtest/gtest.h"

#include "tt_common.hpp"
#include "qpl/cpp_api/operations/compression/deflate_operation.hpp"
#include "qpl/cpp_api/operations/compression/inflate_operation.hpp"
#include "qpl/cpp_api/chaining/operation_chain.hpp"

#include "source_provider.hpp"
#include "util.hpp"
#include "check_result.hpp"

namespace qpl::test {

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(asynchronous_execution, default_callable) {
    constexpr uint32_t source_size      = 111261u;
    constexpr uint32_t number_of_copies = 7u;

    auto &dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();

    std::vector<std::vector<uint8_t>> source(number_of_copies);
    std::vector<uint32_t>             result_size;

    std::vector<qpl::execution_result<uint32_t, qpl::execution_mode::sync>> result_event;

    for (int i = 0; i < number_of_copies; i++) {
        source[i] = dataset.get_data().begin()->second;
    }

    std::vector<std::vector<uint8_t>> destination(number_of_copies, std::vector<uint8_t>(source[0].size()));

    auto chain = qpl::deflate_operation() |
                 qpl::inflate_operation();

    auto path = util::TestEnvironment::GetInstance().GetExecutionPath();
    result_event.reserve(number_of_copies);
    if (qpl_path_hardware == path) {
        for (int i = 0; i < number_of_copies; i++) {
            result_event.push_back(qpl::submit<qpl::hardware>(chain, source[i], destination[i]));
        }
    } else {
        for (int i = 0; i < number_of_copies; i++) {
            result_event.push_back(qpl::submit<qpl::software>(chain, source[i], destination[i]));
        }
    }

    for (int i = 0; i < number_of_copies; i++) {
        uint32_t res_size = 0;
        result_event[i].handle([&res_size](uint32_t value) -> void {
                                   res_size = value;
                               },
                               [](uint32_t status) -> void {
                                   throw std::runtime_error("Error: Status code - " + std::to_string(status));
                               });

        result_size.push_back(res_size);
    }

    for (size_t i = 0; i < result_size.size(); i++) {
        EXPECT_EQ(source[i].size(), result_size[i]);
        EXPECT_TRUE(CompareVectors(source[i], destination[i]));
    }
}
} // namespace qpl::test
