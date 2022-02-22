/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "tn_hl_common.hpp"

#include "qpl/cpp_api/operations/analytics/extract_operation.hpp"

namespace qpl::test {

    constexpr const uint32_t source_size = 10;
    constexpr const uint32_t destination_size = 2;

    QPL_HIGH_LEVEL_API_NEGATIVE_TEST(extract, incorrect_prle_bit_width) {

        std::vector<uint8_t> source(source_size, 0);
        std::vector<uint8_t> destination(destination_size, 1);
        auto incorrect_bit_width = 55u;
        source[0] = incorrect_bit_width;

        auto extract_operation = qpl::extract_operation::builder(1u, 3u)
            .input_vector_width(8u) // this one should be ignored for prle
            .parser<qpl::parsers::parquet_rle>(source_size / 2u)
            .build();

        auto status = test::get_execute_status(extract_operation,
            source.begin(),
            source.end(),
            destination.begin(),
            destination.end());

        EXPECT_EQ(status, QPL_STS_BIT_WIDTH_ERR) << "Fail on: Incorrect bit_width for PRLE parser";
    }
}