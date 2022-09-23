/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_LOW_LEVEL_SERIALIZATION_EXAMPLE] */

#include <iostream>
#include <vector>
#include <memory>
#include <stdint.h>

#include "qpl/qpl.h"

constexpr const auto     execution_path = qpl_path_software;
constexpr const uint32_t source_size    = 1000;

auto main() -> int {
    std::vector<uint8_t> source(source_size, 5);

    qpl_status status = QPL_STS_OK;

    // Memory allocation for Huffman table
    qpl_huffman_table_t huffman_table;

    status = qpl_deflate_huffman_table_create(combined_table_type,
                                              execution_path,
                                              DEFAULT_ALLOCATOR_C,
                                              &huffman_table);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("Error while creation occurred.");
    }

    // Creation of deflate histogram
    qpl_histogram deflate_histogram{};

    status = qpl_gather_deflate_statistics(source.data(),
                                           source_size,
                                           &deflate_histogram,
                                           qpl_default_level,
                                           execution_path);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("Error while building histogram occurred.");
    }

    // Initialization of Huffman table with deflate histogram
    status = qpl_huffman_table_init_with_histogram(huffman_table, &deflate_histogram);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("Error while initializing occurred.");
    }

    size_t serialized_size;

    // Getting size of a buffer to store serialized table and allocating memory for it
    status = qpl_huffman_table_get_serialized_size(huffman_table,
                                                   DEFAULT_SERIALIZATION_OPTIONS,
                                                   &serialized_size);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("Error while getting serialize size occurred.");
    }

    uint8_t* buffer = (uint8_t*) std::malloc(serialized_size);

    // Serialization of a table
    status = qpl_huffman_table_serialize(huffman_table,
                                         buffer,
                                         serialized_size,
                                         DEFAULT_SERIALIZATION_OPTIONS);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("Error while serializing occurred.");
    }

    // Deserialization of a table
    qpl_huffman_table_t other_huffman_table;
    status = qpl_huffman_table_deserialize(buffer,
                                           serialized_size,
                                           DEFAULT_ALLOCATOR_C,
                                           &other_huffman_table);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("Error while deserializing occurred.");
    }

    // Freeing resources
    status = qpl_huffman_table_destroy(huffman_table);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("Error while destroying table occurred.");
    }

    status = qpl_huffman_table_destroy(other_huffman_table);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("Error while destroying table occurred.");
    }

    std::free(buffer);

    std::cout << "Huffman table was successfully serialized and deserialized." << std::endl;
    std::cout << "Serialized size: " << serialized_size << std::endl;

    return 0;
}

//* [QPL_LOW_LEVEL_SERIALIZATION_EXAMPLE] */