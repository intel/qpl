/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_LOW_LEVEL_CANNED_COMPRESSION_WITH_DICTIONARY_EXAMPLE] */

#include <cstddef>
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
constexpr const uint32_t source_size = 2048U;

auto main(int argc, char** argv) -> int {
    std::cout << "Intel(R) Query Processing Library version is " << qpl_get_library_version() << ".\n";

    // Default to Software Path
    qpl_path_t execution_path = qpl_path_software;

    // Get path from input argument
    const int parse_ret = parse_execution_path(argc, argv, &execution_path);
    if (parse_ret != 0) { return 1; }

    // Get compression buffer size estimate
    const uint32_t compression_size = qpl_get_safe_deflate_compression_buffer_size(source_size);
    if (compression_size == 0) {
        std::cout << "Invalid source size. Source size exceeds the maximum supported size.\n";
        return 1;
    }

    // Source and output containers
    std::vector<uint8_t> source(source_size, 5);
    std::vector<uint8_t> destination(compression_size, 4);
    std::vector<uint8_t> reference(source_size, 7);

    std::unique_ptr<uint8_t[]> job_buffer;
    uint32_t                   job_size = 0U;
    qpl_histogram              deflate_histogram {};

    // Job initialization
    qpl_status status = qpl_get_job_size(execution_path, &job_size);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during job size getting.\n";
        return 1;
    }

    job_buffer   = std::make_unique<uint8_t[]>(job_size);
    qpl_job* job = reinterpret_cast<qpl_job*>(job_buffer.get());
    status       = qpl_init_job(execution_path, job);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during compression job initializing.\n";
        return 1;
    }

    // Dictionary initialization
    std::unique_ptr<uint8_t[]> dictionary_buffer;
    qpl_dictionary*            dictionary_ptr         = nullptr;
    std::size_t                dictionary_buffer_size = 0;
    sw_compression_level       sw_compr_level         = sw_compression_level::SW_NONE;
    hw_compression_level       hw_compr_level         = hw_compression_level::HW_NONE;
    std::size_t                raw_dict_size          = 0;

    // Select dictionary levels
    if (execution_path == qpl_path_software) {
        sw_compr_level = sw_compression_level::LEVEL_1;
    } else {
        hw_compr_level = hw_compression_level::HW_LEVEL_1;
    }

    // Huffman table initialization
    qpl_huffman_table_t huffman_table = nullptr;

    status = qpl_deflate_huffman_table_create(combined_table_type, execution_path, DEFAULT_ALLOCATOR_C, &huffman_table);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during Huffman table creation.\n";
        return 1;
    }

    // Filling deflate histogram first
    status = qpl_gather_deflate_statistics(source.data(), source_size, &deflate_histogram, qpl_default_level,
                                           execution_path);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during gathering statistics for Huffman table.\n";
        qpl_huffman_table_destroy(huffman_table);
        return 1;
    }

    // Building the Huffman table
    status = qpl_huffman_table_init_with_histogram(huffman_table, &deflate_histogram);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during Huffman table initialization.\n";
        qpl_huffman_table_destroy(huffman_table);
        return 1;
    }

    // To build the dictionary, users must provide a raw dictionary.
    // To better improve the compression ratio with dictionary, users should
    // set raw_dict_size to the maximum size of the raw dictionary,
    // refer to Intel® Query Processing Library (Intel® QPL) documentation.
    // The raw dictionary should contain pieces of data that are most likely to occur in the real
    // datasets to be compressed.
    // In this example, to make things simple, we just use the source data as the raw dictionary.
    raw_dict_size               = source.size();
    const uint8_t* raw_dict_ptr = source.data();
    dictionary_buffer_size      = qpl_get_dictionary_size(sw_compr_level, hw_compr_level, raw_dict_size);

    dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
    dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

    status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, raw_dict_ptr, raw_dict_size);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during dictionary building.\n";
        qpl_huffman_table_destroy(huffman_table);
        return 1;
    }

    // Performing canned compression with dictionary
    job->op            = qpl_op_compress;
    job->level         = qpl_default_level;
    job->next_in_ptr   = source.data();
    job->next_out_ptr  = destination.data();
    job->available_in  = source_size;
    job->available_out = static_cast<uint32_t>(destination.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE | QPL_FLAG_OMIT_VERIFY;
    job->dictionary    = dictionary_ptr;
    job->huffman_table = huffman_table;

    // Compression
    status = qpl_execute_job(job);

    // On qpl_path_hardware, if the Intel® In-Memory Analytics Accelerator (Intel® IAA) hardware available does not support dictionary compression, job will exit early and return the appropriate error code
    if (execution_path == qpl_path_hardware && status == QPL_STS_NOT_SUPPORTED_MODE_ERR) {
        std::cout
                << "Compression with dictionary is not supported on qpl_path_hardware. Note that only certain generations of Intel IAA support compression with dictionary.\n";
        qpl_huffman_table_destroy(huffman_table);
        return 0;
    }

    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during compression.\n";
        qpl_huffman_table_destroy(huffman_table);
        return 1;
    }

    const uint32_t compressed_size = job->total_out;

    // Performing a decompression operation with the same dictionary used for compression
    job->op            = qpl_op_decompress;
    job->next_in_ptr   = destination.data();
    job->next_out_ptr  = reference.data();
    job->available_in  = compressed_size;
    job->available_out = static_cast<uint32_t>(reference.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE;
    job->dictionary    = dictionary_ptr;
    job->huffman_table = huffman_table;

    // Decompression
    status = qpl_execute_job(job);

    // qpl_path_hardware does not support canned decompression with dictionary
    if (execution_path == qpl_path_hardware && status == QPL_STS_NOT_SUPPORTED_MODE_ERR) {
        std::cout << "Canned decompression with dictionary is not supported on qpl_path_hardware.\n";
        qpl_huffman_table_destroy(huffman_table);
        return 0;
    }

    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during decompression.\n";
        qpl_huffman_table_destroy(huffman_table);
        return 1;
    }

    // Freeing resources
    status = qpl_huffman_table_destroy(huffman_table);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during destroying Huffman table.\n";
        return 1;
    }

    status = qpl_fini_job(job);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during job finalization.\n";
        return 1;
    }

    // Compare reference functions
    for (size_t i = 0; i < source.size(); i++) {
        if (source[i] != reference[i]) {
            std::cout << "Content wasn't successfully compressed and decompressed.\n";
            return 1;
        }
    }

    std::cout << "Content was successfully compressed and decompressed.\n";
    std::cout << "Input size: " << source.size() << ", compressed size: " << compressed_size
              << ", compression ratio: " << (float)source.size() / (float)compressed_size << ".\n";

    return 0;
}

//* [QPL_LOW_LEVEL_CANNED_COMPRESSION_WITH_DICTIONARY_EXAMPLE] */
