/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "tb_hl_common.hpp"

#include "qpl/cpp_api/operations/compression/deflate_operation.hpp"

namespace qpl::test {

    constexpr const uint32_t source_size = 1024;
    constexpr const uint32_t destination_size = 1024;

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(deflate_block, high_dynamic) {

    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(destination_size);
    uint32_t status = 1;

    try {
        // Act
        auto deflate_operation = deflate_operation::builder()
            .compression_mode<qpl::compression_modes::dynamic_mode>()
            .compression_level(high_level)
            .build();

        auto deflate_block = qpl::build_deflate_block<qpl::hardware>(deflate_operation,
            source.begin(),
            source.end(),
            mini_block_size_512);
    }
    catch (qpl::operation_process_exception& e) {
        status = 0;
        e = e; // else MSVS warning C4101: 'e': unreferenced local variable
    }
    EXPECT_EQ(status, 0) << "Fail on: exception is absent for unsupported case";
}

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(deflate_block, high_static) {

    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(destination_size);
    uint32_t status = 1;

    try {
        // Build Huffman table
        qpl::deflate_histogram histogram{};
        qpl::update_deflate_statistics<qpl::hardware>(source.begin(), source.end(), histogram, qpl::high_level);

        // Build compression table
        auto huffman_table = qpl::make_deflate_table<qpl::hardware>(histogram);

        // Act
        auto deflate_operation = deflate_operation::builder()
            .compression_mode<qpl::static_mode>(huffman_table)
            .compression_level(qpl::high_level)
            .build();

        auto deflate_block = qpl::build_deflate_block<qpl::hardware>(deflate_operation,
            source.begin(),
            source.end(),
            mini_block_size_512);
    }
    catch (qpl::operation_process_exception& e) {
        status = 0;
        e = e; // else MSVS warning C4101: 'e': unreferenced local variable
    }
    EXPECT_EQ(status, 0) << "Fail on: exception is absent for unsupported case";
}

}
