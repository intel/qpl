/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "tb_hl_common.hpp"

#include "qpl/cpp_api/operations/analytics/set_membership_operation.hpp"

namespace qpl::test {

constexpr const uint32_t source_size      = 10;
constexpr const uint32_t destination_size = 2;
constexpr const uint32_t mask_size        = 2;

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(set_membership, zero_destination_size) {
    constexpr const uint32_t zero_destination_size = 0;

    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(zero_destination_size);

    auto set_membership_operation = qpl::set_membership_operation();

    auto status = test::get_execute_status(set_membership_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_DST_IS_SHORT_ERR) << "Fail on: destination size cannot be zero";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(set_membership, zero_mask_size) {
    constexpr const uint32_t zero_mask_size = 0;

    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(destination_size);
    std::vector<uint8_t> mask(zero_mask_size);

    auto set_membership_operation = qpl::set_membership_operation::builder(mask.data(), mask.size())
            .build();

    auto status = test::get_execute_status(set_membership_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_SRC2_IS_SHORT_ERR) << "Fail on: mask size cannot be zero";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(set_membership, incorrect_source_size) {
    constexpr const uint32_t invalid_source_size = 1;
    constexpr const uint32_t input_bit_width     = 15;

    std::vector<uint8_t> source(invalid_source_size);
    std::vector<uint8_t> destination(destination_size);
    std::vector<uint8_t> mask(destination_size);

    auto set_membership_operation = qpl::set_membership_operation::builder(mask.data(), mask.size())
            .parser<qpl::parsers::little_endian_packed_array>(source_size)
            .input_vector_width(input_bit_width)
            .build();

    auto status = test::get_execute_status(set_membership_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_SRC_IS_SHORT_ERR) << "Fail on: invalid value for source size";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(set_membership, incorrect_prle_input_bit_width) {
    constexpr const uint32_t invalid_input_vector_width = 25;

    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(destination_size);

    source[0] = invalid_input_vector_width;

    std::vector<uint8_t> mask(destination_size);

    auto set_membership_operation = qpl::set_membership_operation::builder(mask.data(), mask.size())
            .parser<qpl::parsers::parquet_rle>(source_size)
            .build();

    auto status = test::get_execute_status(set_membership_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_BIT_WIDTH_ERR) << "Fail on: invalid value for prle input bit width";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(set_membership, incorrect_drop_bits_count) {
    constexpr const uint32_t input_vector_width               = 1;
    constexpr const uint32_t number_low_order_bits_to_ignore  = 10;
    constexpr const uint32_t number_high_order_bits_to_ignore = 8;

    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(destination_size);
    std::vector<uint8_t> mask(destination_size);

    auto set_membership_operation = qpl::set_membership_operation::builder(mask.data(), mask.size())
            .input_vector_width(input_vector_width)
            .number_of_input_elements(source_size)
            .number_low_order_bits_to_ignore(number_low_order_bits_to_ignore)
            .number_high_order_bits_to_ignore(number_high_order_bits_to_ignore)
            .build();

    auto status = test::get_execute_status(set_membership_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_DROP_BITS_OVERFLOW_ERR) << "Fail on: invalid drop bits value";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(set_membership, incorrect_mask_size) {
    constexpr const uint32_t input_vector_width               = 8;
    constexpr const uint32_t number_low_order_bits_to_ignore  = 1;
    constexpr const uint32_t number_high_order_bits_to_ignore = 1;

    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(destination_size);
    std::vector<uint8_t> mask(destination_size);

    auto set_membership_operation = qpl::set_membership_operation::builder(mask.data(), mask.size())
            .input_vector_width(input_vector_width)
            .number_of_input_elements(source_size)
            .number_low_order_bits_to_ignore(number_low_order_bits_to_ignore)
            .number_high_order_bits_to_ignore(number_high_order_bits_to_ignore)
            .build();

    auto status = test::get_execute_status(set_membership_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_SRC2_IS_SHORT_ERR) << "Fail on: invalid drop bits value";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(set_membership, buffer_overlap) {
    constexpr const uint32_t input_vector_width               = 8;
    constexpr const uint32_t number_low_order_bits_to_ignore  = 1;
    constexpr const uint32_t number_high_order_bits_to_ignore = 1;
    constexpr const uint32_t mask_size_ = destination_size;

    std::vector<uint8_t> buffer(source_size + destination_size * 2 + mask_size_);

    auto mask_begin = buffer.data();
    auto source_begin = mask_begin + mask_size_ - 1;
    auto destination_begin = mask_begin + mask_size_ + source_size;

    auto set_membership_operation = qpl::set_membership_operation::builder(mask_begin, mask_size_)
            .input_vector_width(input_vector_width)
            .number_of_input_elements(source_size)
            .number_low_order_bits_to_ignore(number_low_order_bits_to_ignore)
            .number_high_order_bits_to_ignore(number_high_order_bits_to_ignore)
            .build();

    auto status = test::get_execute_status(set_membership_operation,
                                           source_begin,
                                           source_begin + source_size,
                                           destination_begin,
                                           destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Source overlaps Mask";

    destination_begin = mask_begin + mask_size_ - 1;
    source_begin = mask_begin + mask_size_ + destination_size;

    status = test::get_execute_status(set_membership_operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Destination overlaps Mask";

    source_begin = mask_begin + mask_size_;
    destination_begin = mask_begin + mask_size_ + source_size - 1;

    status = test::get_execute_status(set_membership_operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Destination overlaps Source";

    destination_begin = mask_begin + mask_size_;
    source_begin = mask_begin + mask_size_ + destination_size - 1;

    status = test::get_execute_status(set_membership_operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Source overlaps destination";

    destination_begin = buffer.data();
    mask_begin = destination_begin + destination_size - 1;
    source_begin = destination_begin + destination_size + mask_size_;

    set_membership_operation = qpl::set_membership_operation::builder(mask_begin, mask_size_)
            .input_vector_width(input_vector_width)
            .number_of_input_elements(source_size)
            .number_low_order_bits_to_ignore(number_low_order_bits_to_ignore)
            .number_high_order_bits_to_ignore(number_high_order_bits_to_ignore)
            .build();

    status = test::get_execute_status(set_membership_operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Mask overlaps destination";

    source_begin = buffer.data();
    mask_begin = destination_begin + destination_size - 1;
    destination_begin = source_begin + source_size + mask_size_;

    set_membership_operation = qpl::set_membership_operation::builder(mask_begin, mask_size_)
            .input_vector_width(input_vector_width)
            .number_of_input_elements(source_size)
            .number_low_order_bits_to_ignore(number_low_order_bits_to_ignore)
            .number_high_order_bits_to_ignore(number_high_order_bits_to_ignore)
            .build();

    status = test::get_execute_status(set_membership_operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + destination_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Mask overlaps source";
}

}
