/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <iostream>

#include "gtest/gtest.h"
#include "qpl/cpp_api/operations/compression/deflate_operation.hpp"
#include "qpl/cpp_api/operations/compression/inflate_operation.hpp"
#include "ta_hl_common.hpp"

namespace qpl::test {

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(simple_operations, execute) {
    std::vector<uint8_t> destination(500, 4);
    std::vector<uint8_t> source(1000, 5);
    std::vector<uint8_t> reference(1000, 7);

    auto deflate_operation = qpl::deflate_operation();
    auto deflate_size      = qpl::execute(deflate_operation,
                                          source.begin(),
                                          source.end(),
                                          destination.begin(),
                                          destination.end());

    auto inflate_operation = qpl::inflate_operation();
    auto inflate_size      = qpl::execute(inflate_operation, destination, reference);

    for (size_t i = 0; i < source.size(); i++) {
        ASSERT_EQ(source[i], reference[i]);
    }
}

} // namespace qpl::test
