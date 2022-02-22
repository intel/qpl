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

struct inflate_properties {
    size_t  property_size;
};

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * Data, size_t Size)
{
    const uint8_t*  source_data_ptr = Data;
    size_t          source_size = Size;
    size_t          destination_size = Size;

    if (Size > sizeof(inflate_properties)) {
        inflate_properties* properties = (inflate_properties*)Data;
        source_data_ptr += sizeof(inflate_properties);
        source_size -= sizeof(inflate_properties);
        destination_size = properties->property_size;
        destination_size %= (source_size + source_size);
        if (0 == destination_size) {
            destination_size = source_size + source_size;
        }
    }
    std::vector<uint8_t> source(source_data_ptr, source_data_ptr + source_size);
    std::vector<uint8_t> destination(destination_size, 0xaa);

    try {
        auto inflate_operation = qpl::inflate_operation();

        const auto decompressed_result = qpl::execute<qpl::software>(inflate_operation, source, destination);

        decompressed_result.if_absent([](uint32_t status) -> void {
            // Valid behavior as the operation result is unpredictable
        });
    }
    catch (qpl::operation_process_exception& e) {
            // Handling Exceptions on misconfigured operation
    }
    // Compare source and decompressed buffers
    return 0;
}
