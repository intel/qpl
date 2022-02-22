/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "tb_hl_common.hpp"

#include "qpl/cpp_api/operations/compression/deflate_operation.hpp"
#include <numeric>

namespace qpl::test {

constexpr uint32_t source_size = 1024u;

template<compression_levels level>
void process_dynamic_deflate_small_size() {
    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(250);

    std::iota(std::begin(source), std::end(source), 0);

    auto deflate_operation = qpl::deflate_operation::builder()
            .compression_level(level)
            .build();

    auto status = test::get_execute_status(deflate_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_MORE_OUTPUT_NEEDED) << "Fail on: destination size should be enough to hold compressed source";

    // Stored block case
    status = test::get_execute_status(deflate_operation,
                                      source.begin(),
                                      source.begin() + 256,
                                      destination.begin(),
                                      destination.end());

    EXPECT_EQ(status, QPL_STS_MORE_OUTPUT_NEEDED) << "Fail on: destination size should be enough to hold compressed source";

    destination.resize(1);
    
    status = test::get_execute_status(deflate_operation,
                                      source.begin(),
                                      source.end(),
                                      destination.begin(),
                                      destination.end());

    EXPECT_EQ(status, QPL_STS_MORE_OUTPUT_NEEDED) << "Fail on: destination size should be enough to hold compressed source";
}

template<compression_levels level>
void process_static_deflate_small_size() {
    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(250);

    std::iota(std::begin(source), std::end(source), 0);

    qpl::deflate_histogram deflate_histogram;

    qpl::update_deflate_statistics<qpl::software>(source.begin(), source.end(), deflate_histogram, compression_levels::default_level);

    auto deflate_huffman_table = qpl::make_deflate_table<qpl::software>(deflate_histogram);

    auto deflate_operation = qpl::deflate_operation::builder()
            .compression_mode<static_mode>(deflate_huffman_table)
            .compression_level(level)
            .build();

    auto status = test::get_execute_status(deflate_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_MORE_OUTPUT_NEEDED) << "Fail on: destination size should be enough to hold compressed source";

    // Stored block case
    status = test::get_execute_status(deflate_operation,
                                      source.begin(),
                                      source.begin() + 256,
                                      destination.begin(),
                                      destination.end());

    EXPECT_EQ(status, QPL_STS_MORE_OUTPUT_NEEDED) << "Fail on: destination size should be enough to hold compressed source";

    destination.resize(1);
    
    status = test::get_execute_status(deflate_operation,
                                      source.begin(),
                                      source.end(),
                                      destination.begin(),
                                      destination.end());

    EXPECT_EQ(status, QPL_STS_MORE_OUTPUT_NEEDED) << "Fail on: destination size should be enough to hold compressed source";
}

template<compression_levels level>
void process_static_deflate_buffer_overlap() {
    auto deflate_operation = qpl::deflate_operation::builder()
            .compression_level(level)
            .build();

    std::vector<uint8_t> buffer(source_size * 2);

    auto source_begin = buffer.data();
    auto destination_begin = source_begin + source_size - 1;

    auto status = test::get_execute_status(deflate_operation,
                                           source_begin,
                                           source_begin + source_size,
                                           destination_begin,
                                           destination_begin + source_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Destination overlaps Source";

    destination_begin = buffer.data();
    source_begin = destination_begin + source_size - 1;

    status = test::get_execute_status(deflate_operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + source_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Source overlaps destination";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(deflate, dynamic_small_default_destination_size) {
    process_dynamic_deflate_small_size<qpl::default_level>();
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(deflate, static_default_small_destination_size) {
    process_static_deflate_small_size<qpl::default_level>();
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(deflate, default_level_buffer_overlap) {
    process_static_deflate_buffer_overlap<qpl::default_level>();
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(deflate, dynamic_small_high_destination_size) {
    process_dynamic_deflate_small_size<qpl::high_level>();
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(deflate, static_high_small_destination_size) {
    process_static_deflate_small_size<qpl::high_level>();
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(deflate, high_level_buffer_overlap) {
    process_static_deflate_buffer_overlap<qpl::high_level>();
}

}
