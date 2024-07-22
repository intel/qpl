/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_LOW_LEVEL_COMPRESSION_STATIC_MULTI_CHUNK_EXAMPLE] */

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
 * The example compresses data with multi-chunk and decompresses data using single job with Deflate static Huffman encoding.
 *
 */

uint32_t sum(std::vector<uint32_t> vector) {
    uint32_t result = 0;
    for (size_t i = 0; i < vector.size(); i++) {
        result += vector[i];
    }
    return result;
}

constexpr const uint32_t source_size = 1000;

auto main(int argc, char** argv) -> int {
    std::cout << "Intel(R) Query Processing Library version is " << qpl_get_library_version() << ".\n";

    // Default to Software Path.
    qpl_path_t execution_path = qpl_path_software;

    // Get path from input argument.
    const int parse_ret = parse_execution_path(argc, argv, &execution_path);
    if (parse_ret != 0) { return 1; }

    // Source and output containers.
    std::vector<uint8_t> source(source_size, 5);
    std::vector<uint8_t> destination(source_size / 2, 4);
    std::vector<uint8_t> reference(source_size, 7);

    std::unique_ptr<uint8_t[]> job_buffer;
    uint32_t                   size = 0;

    // Allocate and initialize job structure.
    qpl_status status = qpl_get_job_size(execution_path, &size);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during job size getting.\n";
        return 1;
    }

    job_buffer   = std::make_unique<uint8_t[]>(size);
    qpl_job* job = reinterpret_cast<qpl_job*>(job_buffer.get());

    status = qpl_init_job(execution_path, job);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during job initializing.\n";
        return 1;
    }

    // Allocate Huffman table object (c_huffman_table).
    qpl_huffman_table_t c_huffman_table = nullptr;
    status = qpl_deflate_huffman_table_create(compression_table_type, execution_path, DEFAULT_ALLOCATOR_C,
                                              &c_huffman_table);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during Huffman table creation.\n";
        return 1;
    }

    // Initialize Huffman table using deflate tokens histogram.
    qpl_histogram histogram {};
    status = qpl_gather_deflate_statistics(source.data(), source_size, &histogram, qpl_default_level, execution_path);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during gathering statistics for Huffman table.\n";
        qpl_huffman_table_destroy(c_huffman_table);
        return 1;
    }

    status = qpl_huffman_table_init_with_histogram(c_huffman_table, &histogram);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during Huffman table initialization.\n";
        qpl_huffman_table_destroy(c_huffman_table);
        return 1;
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

    // In this example source data is split to 5 chunks with unequal chunk sizes.
    // Sum of all chunk sizes MUST be equal to source_size.
    std::vector<uint32_t> chunk_sizes {50, 250, 150, 350, 200};
    if (sum(chunk_sizes) != source_size) {
        std::cout << "Sum of all chunk sizes isn't equal to source_size.\n";
        qpl_huffman_table_destroy(c_huffman_table);
        return 1;
    }

    uint32_t source_bytes_processed_previously = 0U;
    for (size_t iteration_count = 0; iteration_count < chunk_sizes.size(); iteration_count++) {
        // Set the job to LAST on the last iteration.
        if (iteration_count == chunk_sizes.size() - 1) { job->flags |= QPL_FLAG_LAST; }

        // Advance `next_in_ptr` pointer for the next iteration by the amount
        // of bytes processed previously.
        // If writing into contiguous memory, this step is not necessary,
        // as the `next_in_ptr` will be updated at the end of previous execution by
        // number of bytes processed.
        job->next_in_ptr  = source.data() + source_bytes_processed_previously;
        job->available_in = chunk_sizes[iteration_count];

        // Execute compression operation.
        status = qpl_execute_job(job);
        if (status != QPL_STS_OK) {
            std::cout << "An error " << status << " acquired during compression.\n";
            qpl_huffman_table_destroy(c_huffman_table);
            return 1;
        }

        job->flags &= ~QPL_FLAG_FIRST;

        // Update offset for `next_in_ptr` by the total size of previous chunks.
        source_bytes_processed_previously += chunk_sizes[iteration_count];
    }
    const uint32_t compressed_size = job->total_out;

    // The code below checks if a compression operation works correctly.

    // Initialize qpl_job structure before performing a decompression operation.
    job->op            = qpl_op_decompress;
    job->next_in_ptr   = destination.data();
    job->next_out_ptr  = reference.data();
    job->available_in  = compressed_size;
    job->available_out = static_cast<uint32_t>(reference.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;

    // Execute decompression operation.
    status = qpl_execute_job(job);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during decompression.\n";
        qpl_huffman_table_destroy(c_huffman_table);
        return 1;
    }

    // Destroy c_huffman_table.
    status = qpl_huffman_table_destroy(c_huffman_table);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during destroying Huffman table.\n";
        return 1;
    }

    // Free resources.
    status = qpl_fini_job(job);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during job finalization.\n";
        return 1;
    }

    // Compare compressed then decompressed buffer with original source.
    for (size_t i = 0; i < source_size; i++) {
        if (source[i] != reference[i]) {
            std::cout << "Content wasn't successfully compressed and decompressed.\n";
            return 1;
        }
    }

    std::cout << "Content was successfully compressed and decompressed.\n";
    std::cout << "Input size: " << source_size << ", compressed size: " << compressed_size
              << ", compression ratio: " << (float)source_size / (float)compressed_size << ".\n";

    return 0;
}

//* [QPL_LOW_LEVEL_COMPRESSION_STATIC_MULTI_CHUNK_EXAMPLE] */
