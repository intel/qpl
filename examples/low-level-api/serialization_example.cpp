/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_LOW_LEVEL_SERIALIZATION_EXAMPLE] */

#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include "qpl/qpl.h"

#include "examples_utils.hpp" // for argument parsing function

/**
 * @brief This example requires a command line argument to set the execution path. Valid values are `software_path`
 * and `hardware_path`.
 * In QPL, @ref qpl_path_software (`Software Path`) means that computations will be done with CPU.
 * Accelerator can be used instead of CPU. In this case, @ref qpl_path_hardware (`Hardware Path`) must be specified.
 * If there is no difference where calculations should be done, @ref qpl_path_auto (`Auto Path`) can be used to allow
 * the library to chose the path to execute. The Auto Path usage is not demonstrated by this example.
 *
 * @warning ---! Important !---
 * `Hardware Path` doesn't support all features declared for `Software Path`
 *
 */
constexpr const uint32_t source_size = 1000;

auto main(int argc, char** argv) -> int {
    std::cout << "Intel(R) Query Processing Library version is " << qpl_get_library_version() << ".\n";

    // Default to Software Path
    qpl_path_t execution_path = qpl_path_software;

    // Get path from input argument
    const int parse_ret = parse_execution_path(argc, argv, &execution_path);
    if (parse_ret != 0) { return 1; }

    std::vector<uint8_t> source(source_size, 5);

    qpl_status status = QPL_STS_OK;

    // Memory allocation for Huffman table
    qpl_huffman_table_t huffman_table = nullptr;

    status = qpl_deflate_huffman_table_create(combined_table_type, execution_path, DEFAULT_ALLOCATOR_C, &huffman_table);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during Huffman table creation.\n";
        return 1;
    }

    // Creation of deflate histogram
    qpl_histogram deflate_histogram {};

    status = qpl_gather_deflate_statistics(source.data(), source_size, &deflate_histogram, qpl_default_level,
                                           execution_path);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during gathering statistics for Huffman table.\n";
        qpl_huffman_table_destroy(huffman_table);
        return 1;
    }

    // Initialization of Huffman table with deflate histogram
    status = qpl_huffman_table_init_with_histogram(huffman_table, &deflate_histogram);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during Huffman table initialization.\n";
        qpl_huffman_table_destroy(huffman_table);
        return 1;
    }

    size_t serialized_size = 0U;

    // Getting size of a buffer to store serialized table and allocating memory for it
    status = qpl_huffman_table_get_serialized_size(huffman_table, DEFAULT_SERIALIZATION_OPTIONS, &serialized_size);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during getting serialized size of Huffman table.\n";
        qpl_huffman_table_destroy(huffman_table);
        return 1;
    }

    const std::unique_ptr<uint8_t[]> unique_buffer = std::make_unique<uint8_t[]>(serialized_size);
    uint8_t*                         buffer        = reinterpret_cast<uint8_t*>(unique_buffer.get());

    // Serialization of a table
    status = qpl_huffman_table_serialize(huffman_table, buffer, serialized_size, DEFAULT_SERIALIZATION_OPTIONS);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during serializing Huffman table.\n";
        qpl_huffman_table_destroy(huffman_table);
        return 1;
    }

    // Deserialization of a table
    qpl_huffman_table_t other_huffman_table = nullptr;
    status = qpl_huffman_table_deserialize(buffer, serialized_size, DEFAULT_ALLOCATOR_C, &other_huffman_table);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during deserializing Huffman table.\n";
        qpl_huffman_table_destroy(huffman_table);
        return 1;
    }

    // Freeing resources
    status = qpl_huffman_table_destroy(huffman_table);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during destroying Huffman table.\n";
        return 1;
    }

    status = qpl_huffman_table_destroy(other_huffman_table);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during destroying Huffman table.\n";
        return 1;
    }

    std::cout << "Huffman table was successfully serialized and deserialized.\n";
    std::cout << "Serialized size: " << serialized_size << '\n';

    return 0;
}

//* [QPL_LOW_LEVEL_SERIALIZATION_EXAMPLE] */
