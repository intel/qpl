/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_LOW_LEVEL_COMPRESSION_STATIC_MULTI_CHUNK_EXAMPLE] */

#include <iostream>
#include <vector>
#include <memory>
#include <stdexcept> // for runtime_error

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
 * The example compresses data with multi-chunk and decompresses data using single job with Deflate static Huffman encoding.
 *
 */

uint32_t sum(std::vector<uint32_t> vector){
    uint32_t result = 0;
    for(size_t i = 0; i < vector.size(); i++){
        result += vector[i];
    }
    return result;
}

constexpr const uint32_t source_size = 1000;

auto main(int argc, char** argv) -> int {

    // Default to Software Path
    qpl_path_t execution_path = qpl_path_software;

    // Get path from input argument
    int parse_ret = parse_execution_path(argc, argv, &execution_path);
    if (parse_ret != 0) {
        return 1;
    }

    // Source and output containers
    std::vector<uint8_t> source(source_size, 5);
    std::vector<uint8_t> destination(source_size / 2, 4);
    std::vector<uint8_t> reference(source_size, 7);

    std::unique_ptr<uint8_t[]> job_buffer;
    qpl_status                 status;
    uint32_t                   size = 0;

    // Job initialization
    status = qpl_get_job_size(execution_path, &size);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during job size getting.");
    }

    job_buffer = std::make_unique<uint8_t[]>(size);
    qpl_job *job = reinterpret_cast<qpl_job *>(job_buffer.get());
    status = qpl_init_job(execution_path, job);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during compression job initializing.");
    }
    
    // The Huffman table object (huffman_table) allocation
    qpl_huffman_table_t c_huffman_table;
    status = qpl_deflate_huffman_table_create(compression_table_type,
                                              execution_path,
                                              DEFAULT_ALLOCATOR_C,
                                              &c_huffman_table);

    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during huffman table creation.");
    }

    // The Huffman table initialization using deflate tokens histogram.
    qpl_histogram histogram{};
    status = qpl_gather_deflate_statistics(source.data(),
                                           source_size,
                                           &histogram,
                                           qpl_default_level,
                                           execution_path);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during gathering statistics for huffman table.");
    }

    status = qpl_huffman_table_init_with_histogram(c_huffman_table, &histogram);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during huffman table initialization.");
    }

    // Initialize qpl_job structure before performing a compression operation.
    job->op            = qpl_op_compress;
    job->level         = qpl_default_level;
    job->next_in_ptr   = source.data();
    job->next_out_ptr  = destination.data();
    job->available_in  = source_size;
    job->available_out = static_cast<uint32_t>(destination.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_OMIT_VERIFY;
    job->huffman_table = c_huffman_table;
  
    // In this example source data has splitted up to 5 chunks with unequal chunk sizes. Sum of all chunk sizes is equal to source_size
    uint32_t iteration_count = 0;
    auto source_bytes_left = static_cast<uint32_t>(source_size);
    std::vector<uint32_t> chunk_sizes {50, 250, 150, 350, 200};  
    if (sum(chunk_sizes) != source_size) {
        throw std::runtime_error("Sum of all chunk sizes isn't equal to source_size.");
    }

    while (source_bytes_left > 0) {
        // QPL_FLAG_LAST is set in the last chunk
        if (chunk_sizes[iteration_count] >= source_bytes_left) {
            job->flags |= QPL_FLAG_LAST;
            chunk_sizes[iteration_count] = source_bytes_left;
        }
            
        job->next_in_ptr  = source.data() + chunk_sizes[iteration_count];
        job->available_in = chunk_sizes[iteration_count];

        // Execute compression operation
        status = qpl_execute_job(job);
        if (status != QPL_STS_OK) {
            throw std::runtime_error("Error while compression occurred.");
        }
            
        job->flags &= ~QPL_FLAG_FIRST;
        source_bytes_left -= chunk_sizes[iteration_count];
        iteration_count++;

    }
    const uint32_t compressed_size = job->total_out;

    // The code below checks if a compression operation works correctly

    // Initialize qpl_job structure before performing a decompression operation
    job->op            = qpl_op_decompress;
    job->next_in_ptr   = destination.data();
    job->next_out_ptr  = reference.data();
    job->available_in  = compressed_size;
    job->available_out = static_cast<uint32_t>(reference.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;

    // Execute decompression operation
    status = qpl_execute_job(job);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("Error while decompression occurred.");
    }

    // Freeing resources
    status = qpl_fini_job(job);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during job finalization.");
    }
    // Destroying huffman_table
    status = qpl_huffman_table_destroy(c_huffman_table);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during Huffman table destroyng.");
    }

    // Compare reference functions
    for (size_t i = 0; i < source_size; i++) {
        if (source[i] != reference[i]) {
            throw std::runtime_error("Content wasn't successfully compressed and decompressed.");
        }
    }

    std::cout << "Content was successfully compressed and decompressed." << std::endl;
    std::cout << "Compressed size: " << compressed_size << std::endl;

    return 0;
}

//* [QPL_LOW_LEVEL_COMPRESSION_STATIC_MULTI_CHUNK_EXAMPLE] */
