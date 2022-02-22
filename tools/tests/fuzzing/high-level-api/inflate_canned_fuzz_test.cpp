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

    if (0 == Size) {
        return 0;
    }

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

    qpl::deflate_histogram histogram{};
    size_t  histogram_values_size = histogram.literals_lengths_histogram_size + histogram.offsets_histogram_size;

    uint32_t*   histogram_val_ptr = histogram.get_literals_lengths();
    uint32_t    indx_source = 0;
    for (uint32_t indx = 0; indx < histogram.literals_lengths_histogram_size; indx++) {
        histogram_val_ptr[indx] = source_data_ptr[indx_source];
        if (0 == histogram_val_ptr[indx]) {
            histogram_val_ptr[indx] = 1;
        }
        indx_source++;
        if (indx_source >= source_size) {
            indx_source = 0;
        }
    }

    histogram_val_ptr = histogram.get_offsets();
    for (uint32_t indx = 0; indx < histogram.offsets_histogram_size; indx++) {
        histogram_val_ptr[indx] = source_data_ptr[indx_source];
        if (0 == histogram_val_ptr[indx]) {
            histogram_val_ptr[indx] = 1;
        }
        indx_source++;
        if (indx_source >= source_size) {
            indx_source = 0;
        }
    }
    if (histogram_values_size < source_size) {
        source_data_ptr += histogram_values_size;
        source_size -= histogram_values_size;
    }

    std::vector<uint8_t> source(source_data_ptr, source_data_ptr + source_size);
    std::vector<uint8_t> destination(destination_size, 0xaa);

    auto huffman_table = qpl::make_deflate_table<qpl::software>(histogram);
    auto inflate_table = qpl::make_inflate_table<qpl::software>(huffman_table);

    try {
        auto inflate_operation = qpl::inflate_operation::builder()
            .compression_mode<qpl::compression_modes::canned_mode>(inflate_table)
            .build();

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
