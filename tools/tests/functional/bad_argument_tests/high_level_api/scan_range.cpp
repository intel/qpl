/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "tb_hl_common.hpp"

#include "qpl/cpp_api/operations/analytics/scan_range_operation.hpp"

namespace qpl::test {

constexpr const uint32_t source_size      = 10;
constexpr const uint32_t destination_size = 40;

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(scan_range, zero_destination_size) {
    constexpr const uint32_t zero_destination_size = 0;

    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(zero_destination_size);

    auto scan_range_operation = qpl::scan_range_operation();

    auto status = test::get_execute_status(scan_range_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_DST_IS_SHORT_ERR) << "Fail on: destination size cannot be zero";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(scan_range, incorrect_destination_size) {
    constexpr const uint32_t invalid_destination_size = 2;
    constexpr const uint32_t number_of_input_elements = 1000;
    constexpr const uint32_t output_vector_width      = 1;

    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(invalid_destination_size);

    auto scan_range_operation = qpl::scan_range_operation::builder(qpl::scan_range_operation())
            .number_of_input_elements(number_of_input_elements)
            .output_vector_width(output_vector_width)
            .build();

    auto status = test::get_execute_status(scan_range_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_DST_IS_SHORT_ERR) << "Fail on: invalid value for destination size";

    auto scan_range_operation_parser = qpl::scan_range_operation::builder(qpl::scan_range_operation())
            .parser<qpl::parsers::little_endian_packed_array>(number_of_input_elements)
            .output_vector_width(output_vector_width)
            .build();

    auto status_with_parser = test::get_execute_status(scan_range_operation_parser,
                                                       source.begin(),
                                                       source.end(),
                                                       destination.begin(),
                                                       destination.end());

    EXPECT_EQ(status_with_parser, QPL_STS_DST_IS_SHORT_ERR) << "Fail on: invalid value for destination size";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(scan_range, incorrect_source_size) {
    constexpr const uint32_t invalid_source_size = 1;
    constexpr const uint32_t input_bit_width     = 15;

    std::vector<uint8_t> source(invalid_source_size);
    std::vector<uint8_t> destination(destination_size);

    auto scan_range_operation = qpl::scan_range_operation::builder(qpl::scan_range_operation())
            .parser<qpl::parsers::little_endian_packed_array>(source_size)
            .input_vector_width(input_bit_width)
            .build();

    auto status = test::get_execute_status(scan_range_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_SRC_IS_SHORT_ERR) << "Fail on: invalid value for source size";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(scan_range, buffer_overlap) {
    constexpr const uint32_t input_bit_width = 8;

    std::vector<uint8_t> buffer(source_size + destination_size);

    auto source_begin = buffer.data();
    auto destination_begin = source_begin + source_size - 1;

    auto scan_operation = qpl::scan_range_operation::builder(qpl::scan_range_operation())
            .parser<qpl::parsers::little_endian_packed_array>(source_size)
            .input_vector_width(input_bit_width)
            .build();

    auto status = test::get_execute_status(scan_operation,
                                           source_begin,
                                           source_begin + source_size,
                                           destination_begin,
                                           destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Destination overlaps Source";

    destination_begin = buffer.data();
    source_begin = destination_begin + destination_size - 1;

    status = test::get_execute_status(scan_operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Source overlaps destination";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(scan_range, incorrect_prle_input_bit_width) {
    constexpr const uint32_t invalid_input_vector_width = 35;

    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(destination_size);

    source[0] = invalid_input_vector_width;

    auto scan_range_operation = qpl::scan_range_operation::builder(qpl::scan_range_operation())
            .parser<qpl::parsers::parquet_rle>(source_size)
            .build();

    auto status = test::get_execute_status(scan_range_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_BIT_WIDTH_ERR) << "Fail on: invalid value for prle input bit width";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(scan_range, num_elements_with_index_output) {
    constexpr const uint32_t number_of_input_elements = source_size;

    std::vector<uint8_t> source(source_size, 0xFFu);
    std::vector<uint8_t> destination(destination_size, 0);

    auto max_avaliable_index_8u = 0xFFu;

    auto scan_range_operation_8u = qpl::scan_range_operation::builder(qpl::scan_range_operation())
        .number_of_input_elements(number_of_input_elements)
        .input_vector_width(8u)
        .initial_output_index(max_avaliable_index_8u)
        .output_vector_width(8u)
        .build();

    auto status = test::get_execute_status(scan_range_operation_8u,
        source.begin(),
        source.end(),
        destination.begin(),
        destination.end());

    EXPECT_EQ(status, QPL_STS_OUTPUT_OVERFLOW_ERR) << "Fail on: index of output element overflows size limit (8u)";

    auto max_avaliable_index_16u = 0xFFFFu;

    auto scan_range_operation_16u = qpl::scan_range_operation::builder(qpl::scan_range_operation())
        .number_of_input_elements(number_of_input_elements)
        .input_vector_width(8u)
        .initial_output_index(max_avaliable_index_16u)
        .output_vector_width(16u)
        .build();

    status = test::get_execute_status(scan_range_operation_16u,
        source.begin(),
        source.end(),
        destination.begin(),
        destination.end());

    EXPECT_EQ(status, QPL_STS_OUTPUT_OVERFLOW_ERR) << "Fail on: index of output element overflows size limit (16u)";

    auto max_avaliable_index_32u = 0xFFFFFFFFu;

    auto scan_range_operation_32u = qpl::scan_range_operation::builder(qpl::scan_range_operation())
        .number_of_input_elements(number_of_input_elements)
        .input_vector_width(8u)
        .initial_output_index(max_avaliable_index_32u)
        .output_vector_width(32u)
        .build();

    status = test::get_execute_status(scan_range_operation_32u,
        source.begin(),
        source.end(),
        destination.begin(),
        destination.end());

    EXPECT_EQ(status, QPL_STS_OUTPUT_OVERFLOW_ERR) << "Fail on: index of output element overflows size limit (32u)";
}

}
