/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/
#include <iostream>
#include <string>
#include <vector>

#include "qpl/cpp_api/operations/compression/deflate_operation.hpp"
#include "qpl/cpp_api/operations/compression/inflate_operation.hpp"

struct deflate_properties {
    size_t  property_size;
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * Data, size_t Size) {
    const uint8_t* source_data_ptr = Data;
    size_t              source_size = Size;
    size_t              destination_size = Size;

    if (Size == 0) {
        return 0;
    }

    if (Size > sizeof(deflate_properties)) {
        deflate_properties* properties = (deflate_properties*)Data;
        source_data_ptr += sizeof(deflate_properties);
        source_size -= sizeof(deflate_properties);
        destination_size = properties->property_size;
        destination_size %= (source_size + source_size);
        if (0 == destination_size) {
            destination_size = source_size + source_size;
        }
    }

    std::vector<uint8_t> source(source_data_ptr, source_data_ptr + source_size);
    std::vector<uint8_t> destination(destination_size, 0xaa);

    try {
        qpl::deflate_histogram histogram{};
        qpl::update_deflate_statistics<qpl::software>(source.begin(), source.end(), histogram, qpl::default_level);

        // Build compression table
        auto huffman_table = qpl::make_deflate_table<qpl::software>(histogram);

        auto deflate_operation = qpl::deflate_operation::builder()
            .compression_mode<qpl::canned_mode>(huffman_table)
            .compression_level(qpl::default_level)
            .build();
        const auto compressed_result = qpl::execute<qpl::software>(deflate_operation,
            std::begin(source),
            std::end(source),
            std::begin(destination),
            std::end(destination));

        compressed_result.if_absent([](uint32_t status) -> void {
            // Valid behavior as the operation result is unpredictable
            });
    }
    catch (qpl::operation_process_exception& e) {
        // Handling Exceptions on misconfigured operation
    }
    return 0;
}
