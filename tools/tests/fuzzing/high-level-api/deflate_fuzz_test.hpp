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
    size_t  property_gzip_mode;
};

static inline int deflate_fuzz(const uint8_t * Data, size_t Size,
    qpl::compression_modes compression_mode,
    qpl::compression_levels compression_level)
{
    const uint8_t*      source_data_ptr = Data;
    size_t              source_size = Size;
    size_t              destination_size = Size;
    bool                gzip_mode = false;

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
        if (properties->property_gzip_mode & 1) {
            gzip_mode = true;
        }
    }

    std::vector<uint8_t> source(source_data_ptr, source_data_ptr + source_size);
    std::vector<uint8_t> destination(destination_size, 0xaa);

    try {
        auto deflate_operation = (qpl::dynamic_mode == compression_mode) ?
            qpl::deflate_operation::builder()
            .compression_level(compression_level)
            .compression_mode<qpl::compression_modes::dynamic_mode>()
            .gzip_mode(gzip_mode)
            .build() :
            qpl::deflate_operation::builder()
            .compression_level(compression_level)
            .compression_mode<qpl::compression_modes::fixed_mode>()
            .gzip_mode(gzip_mode)
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
