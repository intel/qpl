/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "tb_hl_common.hpp"

#include "qpl/cpp_api/operations/analytics/extract_operation.hpp"

namespace qpl::test {

constexpr const uint32_t source_size      = 10;
constexpr const uint32_t destination_size = 40;

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(extract, incorrect_source_size) {
    constexpr const uint32_t lower_boundary = 1u;
    constexpr const uint32_t upper_boundary = 3u;

    std::vector<uint8_t> source(source_size, 0);
    std::vector<uint8_t> destination(destination_size, 1);

    { // parser = LE
        auto extract_operation = qpl::extract_operation::builder(lower_boundary, upper_boundary)
            .input_vector_width(8u)
            .parser<qpl::parsers::little_endian_packed_array>(source_size * 2u)
            .build();

        auto status = test::get_execute_status(extract_operation,
                                                source.begin(),
                                                source.end(),
                                                destination.begin(),
                                                destination.end());

        EXPECT_EQ(status, QPL_STS_SRC_IS_SHORT_ERR) << "Fail on (LE): Source can't contain given number of elements";
    }

    { // parser = BE
        auto extract_operation = qpl::extract_operation::builder(lower_boundary, upper_boundary)
            .input_vector_width(8u)
            .parser<qpl::parsers::big_endian_packed_array>(source_size * 2u)
            .build();

        auto status = test::get_execute_status(extract_operation,
            source.begin(),
            source.end(),
            destination.begin(),
            destination.end());

        EXPECT_EQ(status, QPL_STS_SRC_IS_SHORT_ERR) << "Fail on (BE): Source can't contain given number of elements";
    }
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(extract, zero_destination_size) {
constexpr const uint32_t zero_destination_size = 0;
constexpr const uint32_t lower_boundary        = 1u;
constexpr const uint32_t upper_boundary        = 5u;

std::vector<uint8_t> source(source_size);
std::vector<uint8_t> destination(zero_destination_size);

auto extract_operation = qpl::extract_operation(lower_boundary, upper_boundary);

auto status = test::get_execute_status(extract_operation,
                                        source.begin(),
                                        source.end(),
                                        destination.begin(),
                                        destination.end());

EXPECT_EQ(status, QPL_STS_DST_IS_SHORT_ERR) << "Fail on: destination size cannot be zero";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(extract, small_destination_size) {
constexpr const uint32_t lower_boundary         = 1u;
constexpr const uint32_t upper_boundary         = 5u;
constexpr const uint32_t small_destination_size = upper_boundary - lower_boundary;


std::vector<uint8_t> source(source_size);
std::vector<uint8_t> destination(small_destination_size);

auto extract_operation = qpl::extract_operation(lower_boundary, upper_boundary);

auto status = test::get_execute_status(extract_operation,
                                        source.begin(),
                                        source.end(),
                                        destination.begin(),
                                        destination.end());

EXPECT_EQ(status, QPL_STS_DST_IS_SHORT_ERR) << "Fail on: destination size cannot be less than extracted segment size";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(extract, incorrect_bit_width) {
    constexpr const uint32_t lower_boundary = 1u;
    constexpr const uint32_t upper_boundary = 5u;
    
    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(destination_size);

    ASSERT_THROW(auto extract_operation = qpl::extract_operation::builder(lower_boundary, upper_boundary)
            .input_vector_width(0)
            .build(), qpl::invalid_argument_exception);
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(extract, incorrect_prle_input_bit_width) {
    constexpr const uint32_t lower_boundary             = 1u;
    constexpr const uint32_t upper_boundary             = 5u;
    constexpr const uint32_t invalid_input_vector_width = 35;

    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(destination_size);

    source[0] = invalid_input_vector_width;

    auto extract_operation = qpl::extract_operation::builder(lower_boundary, upper_boundary)
            .parser<qpl::parsers::parquet_rle>(source_size)
            .build();

    auto status = test::get_execute_status(extract_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_BIT_WIDTH_ERR) << "Fail on: invalid value for prle input bit width";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(extract, num_elements_with_index_output) {
    constexpr const uint32_t lower_boundary = 1u;
    constexpr const uint32_t upper_boundary = 5u;

    std::vector<uint8_t> source(source_size, 0xFFu);
    std::vector<uint8_t> destination(destination_size, 0);

    auto max_avaliable_index_8u = 0xFFu;

    auto extract_operation_8u = qpl::extract_operation::builder(lower_boundary, upper_boundary)
        .parser<qpl::parsers::little_endian_packed_array>(source_size)
        .input_vector_width(1u)
        .initial_output_index(max_avaliable_index_8u)
        .output_vector_width(8u)
        .build();

    auto status = test::get_execute_status(extract_operation_8u,
        source.begin(),
        source.end(),
        destination.begin(),
        destination.end());

    EXPECT_EQ(status, QPL_STS_OUTPUT_OVERFLOW_ERR) << "Fail on: index of output element overflows size limit (8u)";

    auto max_avaliable_index_16u = 0xFFFFu;

    auto extract_operation_16u = qpl::extract_operation::builder(lower_boundary, upper_boundary)
        .parser<qpl::parsers::little_endian_packed_array>(source_size)
        .input_vector_width(1u)
        .initial_output_index(max_avaliable_index_16u)
        .output_vector_width(16u)
        .build();

    status = test::get_execute_status(extract_operation_16u,
        source.begin(),
        source.end(),
        destination.begin(),
        destination.end());

    EXPECT_EQ(status, QPL_STS_OUTPUT_OVERFLOW_ERR) << "Fail on: index of output element overflows size limit (16u)";

    auto max_avaliable_index_32u = 0xFFFFFFFFu;

    auto extract_operation_32u = qpl::extract_operation::builder(lower_boundary, upper_boundary)
        .parser<qpl::parsers::little_endian_packed_array>(source_size)
        .input_vector_width(1u)
        .initial_output_index(max_avaliable_index_32u)
        .output_vector_width(32u)
        .build();

    status = test::get_execute_status(extract_operation_32u,
        source.begin(),
        source.end(),
        destination.begin(),
        destination.end());

    EXPECT_EQ(status, QPL_STS_OUTPUT_OVERFLOW_ERR) << "Fail on: index of output element overflows size limit (32u)";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(extract, buffer_overlap) {
    constexpr const uint32_t input_bit_width = 8;
    constexpr const uint32_t lower_boundary = 1u;
    constexpr const uint32_t upper_boundary = 5u;

    std::vector<uint8_t> buffer(source_size + destination_size);

    auto source_begin = buffer.data();
    auto destination_begin = source_begin + source_size - 1;

    auto extract_operation = qpl::extract_operation::builder(lower_boundary, upper_boundary)
            .parser<qpl::parsers::parquet_rle>(source_size)
            .build();

    auto  status = test::get_execute_status(extract_operation,
                                            source_begin,
                                            source_begin + source_size,
                                            destination_begin,
                                            destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Destination overlaps Source";

    destination_begin = buffer.data();
    source_begin = destination_begin + destination_size - 1;

    status = test::get_execute_status(extract_operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Source overlaps destination";
}

}