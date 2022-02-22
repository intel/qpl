/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "tb_hl_common.hpp"

#include "qpl/cpp_api/operations/analytics/rle_burst_operation.hpp"

namespace qpl::test {

constexpr const uint32_t destination_size = 2;
constexpr const uint32_t mask_size        = 2;
constexpr const uint32_t input_bit_width  = 8;

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(rle_burst, zero_destination_size) {
    // Source and output containers
    std::vector<uint8_t> source{1, 2, 3};
    std::vector<uint8_t> source_counter{2, 5, 4};
    std::vector<uint8_t> destination(0);

    auto operation = qpl::rle_burst_operation::builder(source_counter.data(), source_counter.size())
            .input_vector_width(input_bit_width)
            .parser<qpl::parsers::little_endian_packed_array>(static_cast<uint32_t>(source.size()))
            .counter_bit_width(input_bit_width)
            .build();

    auto status = test::get_execute_status(operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_DST_IS_SHORT_ERR) << "Fail on: destination size cannot be zero";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(rle_burst, buffer_overlap) {
    constexpr const uint32_t source_counter_size = 3u;
    constexpr const uint32_t source_size = 3u;

    std::vector<uint8_t> buffer(source_counter_size *2 + destination_size);

    auto source_counter_begin = buffer.data();
    auto source_begin = source_counter_begin + source_counter_size - 1;
    auto destination_begin = source_counter_begin + source_counter_size + source_size;

    auto operation = qpl::rle_burst_operation::builder(source_counter_begin, source_counter_size)
            .input_vector_width(input_bit_width)
            .parser<qpl::parsers::little_endian_packed_array>(source_size)
            .counter_bit_width(input_bit_width)
            .build();

    auto status = test::get_execute_status(operation,
                                           source_begin,
                                           source_begin + source_size,
                                           destination_begin,
                                           destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Source overlaps Mask";

    destination_begin = source_counter_begin + source_counter_size - 1;
    source_begin = source_counter_begin + source_counter_size + destination_size;

    status = test::get_execute_status(operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Destination overlaps Mask";

    source_begin = source_counter_begin + source_counter_size;
    destination_begin = source_counter_begin + source_counter_size + source_size - 1;

    status = test::get_execute_status(operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Destination overlaps Source";

    destination_begin = source_counter_begin + source_counter_size;
    source_begin = source_counter_begin + source_counter_size + destination_size - 1;

    status = test::get_execute_status(operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Source overlaps destination";

    destination_begin = buffer.data();
    source_counter_begin = destination_begin + destination_size - 1;
    source_begin = destination_begin + destination_size + source_counter_size;

    operation = qpl::rle_burst_operation::builder(source_counter_begin, source_counter_size)
            .input_vector_width(input_bit_width)
            .parser<qpl::parsers::little_endian_packed_array>(source_size)
            .counter_bit_width(input_bit_width)
            .build();

    status = test::get_execute_status(operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Mask overlaps destination";

    source_begin = buffer.data();
    source_counter_begin = destination_begin + destination_size - 1;
    destination_begin = source_begin + source_size + source_counter_size;

    operation = qpl::rle_burst_operation::builder(source_counter_begin, source_counter_size)
            .input_vector_width(input_bit_width)
            .parser<qpl::parsers::little_endian_packed_array>(source_size)
            .counter_bit_width(input_bit_width)
            .build();

    status = test::get_execute_status(operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Mask overlaps source";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(rle_burst, counters_bit_width_error) {
    // Source and output containers
    std::vector<uint8_t> source{1, 2, 3};
    std::vector<uint8_t> source_counter{2, 5, 4};
    std::vector<uint8_t> destination(2 + 5 + 4);

    auto operation = qpl::rle_burst_operation::builder(source_counter.data(), source_counter.size())
            .input_vector_width(input_bit_width)
            .counter_bit_width(7u)
            .parser<qpl::parsers::little_endian_packed_array>(static_cast<uint32_t>(source.size()))
            .build();

    auto status = test::get_execute_status(operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_BIT_WIDTH_ERR) << "Fail on: Counters bit width should be 8, 16, or 32";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(rle_burst, prle_counters_bit_width_error) {
    // Source and output containers
    std::vector<uint8_t> source{1, 2, 3};
    std::vector<uint8_t> source_counter{7, 5, 4};
    std::vector<uint8_t> destination(2 + 5 + 4);

    auto operation = qpl::rle_burst_operation::builder(source_counter.data(), source_counter.size())
            .input_vector_width(input_bit_width)
            .counter_bit_width(8u)
            .parser<qpl::parsers::parquet_rle>(static_cast<uint32_t>(source.size()))
            .build();

    auto status = test::get_execute_status(operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_BIT_WIDTH_ERR) << "Fail on: Symbols bit width should be between 1 and 32";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(rle_burst, small_number_of_elements_for_accumulate_counters) {
    // Source and output containers
    std::vector<uint8_t> source{1, 2, 3, 4};
    std::vector<uint8_t> source_counter{7, 5, 4, 2};
    std::vector<uint8_t> destination(2 + 5 + 4 + 2);

    auto operation = qpl::rle_burst_operation::builder(source_counter.data(), source_counter.size())
            .input_vector_width(input_bit_width)
            .counter_bit_width(32u)
            .parser<qpl::parsers::little_endian_packed_array>(1)
            .build();

    auto status = test::get_execute_status(operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_SIZE_ERR) << "Fail on: When counters are accumulative, there should be at least two elements";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(rle_burst, source_is_short) {
    // Source and output containers
    std::vector<uint8_t> source{1, 2};
    std::vector<uint8_t> source_counter{2, 5, 4};
    std::vector<uint8_t> destination(2 + 5 + 4);

    auto operation = qpl::rle_burst_operation::builder(source_counter.data(), source_counter.size())
            .input_vector_width(input_bit_width)
            .counter_bit_width(input_bit_width)
            .parser<qpl::parsers::little_endian_packed_array>(static_cast<uint32_t>(source.size() + 1))
            .build();

    auto status = test::get_execute_status(operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_SRC_IS_SHORT_ERR) << "Fail on: Source should be enough to hold number_of_input_elements";
}


QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(rle_burst, invalid_first_count_value_for_accumulate_counters) {
    // Source and output containers
    std::vector<uint8_t> source{1, 2, 3, 4};
    std::vector<uint8_t> source_counter{7, 5, 4, 2};
    std::vector<uint8_t> destination(2 + 5 + 4 + 2);

    auto operation = qpl::rle_burst_operation::builder(source_counter.data(), source_counter.size())
            .input_vector_width(input_bit_width)
            .counter_bit_width(32u)
            .parser<qpl::parsers::little_endian_packed_array>(static_cast<uint32_t>(source.size()))
            .build();

    auto status = test::get_execute_status(operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_INVALID_RLE_COUNT) << "Fail on: When counters are accumulative, first count value should be 0";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(rle_burst, counters_stream_is_short) {
    // Source and output containers
    std::vector<uint8_t> source{1, 2, 3};
    std::vector<uint8_t> source_counter{2, 5};
    std::vector<uint8_t> destination(2 + 5 + 4);

    auto operation = qpl::rle_burst_operation::builder(source_counter.data(), source_counter.size())
            .input_vector_width(input_bit_width)
            .counter_bit_width(input_bit_width)
            .parser<qpl::parsers::little_endian_packed_array>(static_cast<uint32_t>(source.size()))
            .build();

    auto status = test::get_execute_status(operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_SRC2_IS_SHORT_ERR) << "Fail on: Conuters stream size should be enough to hold number_of_input_elements";
}

}
