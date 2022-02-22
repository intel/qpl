/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "tb_hl_common.hpp"

#include "qpl/cpp_api/operations/compression/deflate_operation.hpp"
#include "qpl/cpp_api/operations/compression/inflate_operation.hpp"

#include <numeric>

namespace qpl::test {

constexpr uint32_t source_size       = 1024u;
constexpr uint32_t large_source_size = 65537u;

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(inflate, small_destination_size) {
    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> reference(source_size);
    std::vector<uint8_t> destination(source_size * 2);

    std::iota(std::begin(source), std::end(source), 0);

    auto deflate_operation = qpl::deflate_operation();

    auto compressed_result = qpl::execute(deflate_operation,
                                          source.begin(),
                                          source.end(),
                                          destination.begin(),
                                          destination.end());
    uint32_t compressed_source_size = 0;

    EXPECT_NO_THROW(compressed_result.handle([&compressed_source_size](uint32_t value) -> void {
                                compressed_source_size = value;
                            },
                            [](uint32_t status_code) -> void {
                                throw std::runtime_error("Error: Status code - " + std::to_string(status_code));
                            }));

    auto inflate_operation = qpl::inflate_operation();

    auto status = test::get_execute_status(inflate_operation,
                                           destination.begin(),
                                           destination.begin() + compressed_source_size,
                                           reference.begin(),
                                           reference.begin() + (source_size - 5));

    EXPECT_EQ(status, QPL_STS_MORE_OUTPUT_NEEDED) << compressed_source_size;
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(inflate, small_destination_size_large_source) {
    std::vector<uint8_t> source(large_source_size);
    std::vector<uint8_t> reference(large_source_size);
    std::vector<uint8_t> destination(large_source_size * 2);

    std::iota(std::begin(source), std::end(source), 0);

    auto deflate_operation = qpl::deflate_operation();

    auto compressed_result = qpl::execute(deflate_operation,
                                          source.begin(),
                                          source.end(),
                                          destination.begin(),
                                          destination.end());
    uint32_t compressed_source_size = 0;

    EXPECT_NO_THROW(compressed_result.handle([&compressed_source_size](uint32_t value) -> void {
                                compressed_source_size = value;
                            },
                            [](uint32_t status_code) -> void {
                                throw std::runtime_error("Error: Status code - " + std::to_string(status_code));
                            }));

    auto inflate_operation = qpl::inflate_operation();

    auto status = test::get_execute_status(inflate_operation,
                                           destination.begin(),
                                           destination.begin() + compressed_source_size,
                                           reference.begin(),
                                           reference.begin() + (large_source_size - 5));

    EXPECT_EQ(status, QPL_STS_MORE_OUTPUT_NEEDED) << compressed_source_size;
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(inflate, buffer_overlap) {
    std::vector<uint8_t> buffer(source_size * 3u);

    auto source_begin      = buffer.data();
    auto destination_begin = source_begin + source_size - 1;

    auto inflate_operation = qpl::inflate_operation();

    auto status = test::get_execute_status(inflate_operation,
                                           source_begin,
                                           source_begin + source_size,
                                           destination_begin,
                                           destination_begin + source_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Destination overlaps Source";

    destination_begin = buffer.data();
    source_begin      = destination_begin + source_size - 1;

    status = test::get_execute_status(inflate_operation,
                                      source_begin,
                                      source_begin + source_size,
                                      destination_begin,
                                      destination_begin + source_size);

    EXPECT_EQ(status, QPL_STS_BUFFER_OVERLAP_ERR) << "Source overlaps destination";
}

}
