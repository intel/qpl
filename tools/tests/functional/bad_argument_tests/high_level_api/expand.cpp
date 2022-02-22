/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "tb_hl_common.hpp"

#include "qpl/cpp_api/operations/analytics/expand_operation.hpp"

namespace qpl::test {

constexpr const uint32_t source_size      = 10;
constexpr const uint32_t destination_size = 2;
constexpr const uint32_t mask_size        = 2;

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(expand, zero_destination_size) {
    constexpr const uint32_t zero_destination_size = 0;

    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(zero_destination_size);
    std::vector<uint8_t> mask(mask_size);

    auto expand_operation = qpl::expand_operation(mask.data(), mask.size());

    auto status = test::get_execute_status(expand_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_DST_IS_SHORT_ERR) << "Fail on: destination size cannot be zero";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(expand, zero_mask_size) {
    constexpr const uint32_t zero_mask_size = 0;

    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(destination_size);
    std::vector<uint8_t> mask(zero_mask_size);

    auto expand_operation = qpl::expand_operation::builder(mask.data(), mask.size())
            .build();

    auto status = test::get_execute_status(expand_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_SRC2_IS_SHORT_ERR) << "Fail on: mask size cannot be zero";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(expand, incorrect_source_size) {
    constexpr const uint32_t invalid_source_size = 1;
    constexpr const uint32_t input_bit_width     = 15;

    std::vector<uint8_t> source(invalid_source_size);
    std::vector<uint8_t> destination(destination_size);
    std::vector<uint8_t> mask(mask_size);

    auto expand_operation = qpl::expand_operation::builder(mask.data(), mask.size())
            .parser<qpl::parsers::little_endian_packed_array>(source_size)
            .input_vector_width(input_bit_width)
            .build();

    auto status = test::get_execute_status(expand_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_SRC_IS_SHORT_ERR) << "Fail on: invalid value for source size";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(expand, buffer_overlap) {
    constexpr const uint32_t input_bit_width = 8;
    constexpr const uint32_t mask_size_ = destination_size;

    std::vector<uint8_t> buffer(source_size + destination_size * 2 + mask_size_);

    auto mask_begin = buffer.data();
    auto source_begin = mask_begin + mask_size_ - 1;
    auto destination_begin = mask_begin + mask_size_ + source_size;

    auto expand_operation = qpl::expand_operation::builder(mask_begin, mask_size_)
            .parser<qpl::parsers::little_endian_packed_array>(source_size)
            .input_vector_width(input_bit_width)
            .build();

    auto status = test::get_execute_status(expand_operation,
                                           source_begin,
                                           source_begin + source_size,
                                           destination_begin,
                                           destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Source overlaps Mask";

    destination_begin = mask_begin + mask_size_ - 1;
    source_begin = mask_begin + mask_size_ + destination_size;

    status = test::get_execute_status(expand_operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Destination overlaps Mask";

    source_begin = mask_begin + mask_size_;
    destination_begin = mask_begin + mask_size_ + source_size - 1;

    status = test::get_execute_status(expand_operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Destination overlaps Source";

    destination_begin = mask_begin + mask_size_;
    source_begin = mask_begin + mask_size_ + destination_size - 1;

    status = test::get_execute_status(expand_operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Source overlaps destination";

    destination_begin = buffer.data();
    mask_begin = destination_begin + destination_size - 1;
    source_begin = destination_begin + destination_size + mask_size_;

    expand_operation = qpl::expand_operation::builder(mask_begin, mask_size_)
            .parser<qpl::parsers::little_endian_packed_array>(source_size)
            .input_vector_width(input_bit_width)
            .build();

    status = test::get_execute_status(expand_operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Mask overlaps destination";

    source_begin = buffer.data();
    mask_begin = destination_begin + destination_size - 1;
    destination_begin = source_begin + source_size + mask_size_;

    expand_operation = qpl::expand_operation::builder(mask_begin, mask_size_)
            .parser<qpl::parsers::little_endian_packed_array>(source_size)
            .input_vector_width(input_bit_width)
            .build();

    status = test::get_execute_status(expand_operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Mask overlaps source";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(expand, incorrect_bit_width) {
    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(destination_size);
    std::vector<uint8_t> mask(destination_size);

    ASSERT_THROW(auto expand_operation = qpl::expand_operation::builder(mask.data(), mask.size())
            .input_vector_width(0)
            .build(), qpl::invalid_argument_exception);
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(expand, incorrect_prle_input_bit_width) {
    constexpr const uint32_t invalid_input_vector_width = 35;

    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(destination_size);

    source[0] = invalid_input_vector_width;

    std::vector<uint8_t> mask(destination_size);

    auto expand_operation = qpl::expand_operation::builder(mask.data(), mask.size())
            .parser<qpl::parsers::parquet_rle>(source_size)
            .build();

    auto status = test::get_execute_status(expand_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_BIT_WIDTH_ERR) << "Fail on: invalid value for prle input bit width";
}

}
