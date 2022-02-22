/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "tb_hl_common.hpp"
#include "qpl/cpp_api/operations/analytics/find_unique_operation.hpp"

namespace qpl::test {

constexpr const uint32_t source_size      = 10;
constexpr const uint32_t input_bit_width  = 12;
constexpr const uint32_t destination_size = 1 << (input_bit_width - qpl::byte_bit_length);
constexpr const uint32_t mask_size        = 2;

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(find_unique, zero_destination_size) {
    constexpr const uint32_t zero_destination_size = 0;

    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(zero_destination_size);

    auto find_unique_operation = qpl::find_unique_operation();

    auto status = test::get_execute_status(find_unique_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_DST_IS_SHORT_ERR) << "Fail on: destination size cannot be zero";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(find_unique, small_destination_size) {
    constexpr const uint32_t zero_destination_size = 0;

    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(destination_size - 1);

    auto find_unique_operation = qpl::find_unique_operation::builder()
            .input_vector_width(input_bit_width)
            .build();

    auto status = test::get_execute_status(find_unique_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_DST_IS_SHORT_ERR)
            << "Fail on: destination size cannot be less than power of input_bit_width divided by 8";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(find_unique, incorrect_bit_width) {
    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(destination_size);

    ASSERT_THROW(auto find_unique_operation = qpl::find_unique_operation::builder()
            .input_vector_width(0)
            .build(), qpl::invalid_argument_exception);
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(find_unique, incorrect_source_size) {
    constexpr const uint32_t invalid_source_size = 1;

    std::vector<uint8_t> source(invalid_source_size);
    std::vector<uint8_t> destination(destination_size);

    auto find_unique_operation = qpl::find_unique_operation::builder()
            .parser<qpl::parsers::little_endian_packed_array>(source_size)
            .input_vector_width(input_bit_width)
            .build();

    auto status = test::get_execute_status(find_unique_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_SRC_IS_SHORT_ERR) << "Fail on: invalid value for source size";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(find_unique, buffer_overlap) {
    constexpr const uint32_t input_bit_width = 8;

    std::vector<uint8_t> buffer(source_size + destination_size);

    auto source_begin = buffer.data();
    auto destination_begin = source_begin + source_size - 1;

    auto find_unique_operation = qpl::find_unique_operation::builder()
            .parser<qpl::parsers::little_endian_packed_array>(source_size)
            .input_vector_width(input_bit_width)
            .build();

    auto status = test::get_execute_status(find_unique_operation,
                                           source_begin,
                                           source_begin + source_size,
                                           destination_begin,
                                           destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Destination overlaps Source";

    destination_begin = buffer.data();
    source_begin = destination_begin + destination_size - 1;

    status = test::get_execute_status(find_unique_operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Source overlaps destination";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(find_unique, incorrect_prle_input_bit_width) {
    constexpr const uint32_t invalid_input_vector_width = 16;

    std::vector<uint8_t> source(source_size * 2);
    std::vector<uint8_t> destination(1 << invalid_input_vector_width);

    source[0] = invalid_input_vector_width;

    auto find_unique_operation = qpl::find_unique_operation::builder()
            .parser<qpl::parsers::parquet_rle>(source_size)
            .build();

    auto status = test::get_execute_status(find_unique_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_BIT_WIDTH_ERR) << "Fail on: invalid value for prle input bit width";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(find_unique, incorrect_drop_bits_count) {
    constexpr const uint32_t input_vector_width               = 16;
    constexpr const uint32_t number_low_order_bits_to_ignore  = 10;
    constexpr const uint32_t number_high_order_bits_to_ignore = 8;

    std::vector<uint8_t> source(source_size * 2);
    std::vector<uint8_t> destination(destination_size);

    auto find_unique_operation = qpl::find_unique_operation::builder(number_low_order_bits_to_ignore,
                                                                     number_high_order_bits_to_ignore)
            .input_vector_width(input_vector_width)
            .number_of_input_elements(source_size)
            .build();

    auto status = test::get_execute_status(find_unique_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_DROP_BITS_OVERFLOW_ERR) << "Fail on: invalid drop bits value";
}

}
