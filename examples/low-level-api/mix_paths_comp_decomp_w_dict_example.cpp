/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_LOW_LEVEL_MIX_PATHS_COMP_DECOMP_W_DICT_EXAMPLE] */

#include <cstddef>
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

// Deallocate dictionary
uint8_t destroy_dictionary(qpl_dictionary** dictionary_ptr) {
    if (*dictionary_ptr != nullptr) {
        free(*dictionary_ptr); //NOLINT(cppcoreguidelines-no-malloc)
        *dictionary_ptr = nullptr;
    }
    return 0;
}

// Create dictionary with defined path
uint8_t create_dictionary(qpl_path_t execution_path, std::vector<uint8_t>& source, qpl_dictionary** dictionary_ptr) {
    std::size_t          dictionary_buffer_size = 0;
    sw_compression_level sw_compr_level         = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level         = hw_compression_level::HW_NONE;

    // Select dictionary levels
    if (execution_path == qpl_path_software) {
        sw_compr_level = sw_compression_level::LEVEL_1;
    } else {
        hw_compr_level = hw_compression_level::HW_LEVEL_1;
    }

    // To build the dictionary, users must provide a raw dictionary.
    // To better improve the compression ratio with dictionary, users should
    // set raw_dict_size to the maximum size of the raw dictionary,
    // refer to Intel® Query Processing Library (Intel® QPL) documentation.
    // The raw dictionary should contain pieces of data that are most likely to occur in the real
    // datasets to be compressed.
    // In this example, to make things simple, we just use the source data as the raw dictionary.
    const std::size_t raw_dict_size = source.size();
    const uint8_t*    raw_dict_ptr  = source.data();

    // Determine the size needed for the dictionary
    dictionary_buffer_size = qpl_get_dictionary_size(sw_compr_level, hw_compr_level, raw_dict_size);

    // Allocate memory for the dictionary
    *dictionary_ptr = (qpl_dictionary*)malloc(dictionary_buffer_size); //NOLINT(cppcoreguidelines-no-malloc)

    if (*dictionary_ptr == nullptr) {
        std::cout << "Failed to allocate memory for the dictionary.\n";
        return 1; // Memory allocation failed
    }

    // Build the dictionary
    const qpl_status status =
            qpl_build_dictionary(*dictionary_ptr, sw_compr_level, hw_compr_level, raw_dict_ptr, raw_dict_size);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " occurred during dictionary building.\n";
        destroy_dictionary(dictionary_ptr); // Clean up allocated memory
        return 1;
    }

    std::cout << "Dictionary was successfully built.\n";
    return 0;
}

// Dynamic Dictionary Compression with defined path
uint32_t compression(qpl_path_t execution_path, std::vector<uint8_t>& source, std::vector<uint8_t>& destination,
                     qpl_dictionary* dictionary_ptr) {
    std::unique_ptr<uint8_t[]> job_buffer;
    uint32_t                   job_size = 0U;

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

    std::cout << "Job was successfully initialized.\n";

    // Performing a compression operation with dictionary
    job->op            = qpl_op_compress;
    job->level         = qpl_default_level;
    job->next_in_ptr   = source.data();
    job->next_out_ptr  = destination.data();
    job->available_in  = source_size;
    job->available_out = static_cast<uint32_t>(destination.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN | QPL_FLAG_OMIT_VERIFY;
    job->dictionary    = dictionary_ptr;

    // Compression
    status = qpl_execute_job(job);

    // On qpl_path_hardware, if the Intel® In-Memory Analytics Accelerator (Intel® IAA) hardware available does not support dictionary compression, job will fail
    if (execution_path == qpl_path_hardware && status == QPL_STS_NOT_SUPPORTED_MODE_ERR) {
        std::cout
                << "Compression with dictionary is not supported on qpl_path_hardware. Note that only certain generations of Intel IAA support compression with dictionary.\n";
        return status;
    }

    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during compression.\n";
        return 1;
    }

    const uint32_t compressed_size = job->total_out;

    // Freeing resources
    status = qpl_fini_job(job);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during job finalization.\n";
        return 1;
    }

    // Update destination size
    destination.resize(compressed_size);

    std::cout << "Content was successfully compressed with dictionary on "
              << (execution_path == qpl_path_software ? "software"
                                                      : (execution_path == qpl_path_hardware ? "hardware" : "auto"))
              << " path.\n";

    return 0;
}

// Decompression with software_path
auto sw_decompression(std::vector<uint8_t>& destination, std::vector<uint8_t>& reference,
                      qpl_dictionary* dictionary_ptr) {
    std::unique_ptr<uint8_t[]> job_buffer;
    uint32_t                   job_size = 0U;

    // Get compression size from destination
    const uint32_t compressed_size = destination.size();

    // Job initialization
    qpl_status status = qpl_get_job_size(qpl_path_software, &job_size);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during job size getting.\n";
        return 1;
    }

    job_buffer   = std::make_unique<uint8_t[]>(job_size);
    qpl_job* job = reinterpret_cast<qpl_job*>(job_buffer.get());
    status       = qpl_init_job(qpl_path_software, job);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during compression job initializing.\n";
        return 1;
    }

    // Performing a decompression operation with the same dictionary used for compression
    job->op            = qpl_op_decompress;
    job->next_in_ptr   = destination.data();
    job->next_out_ptr  = reference.data();
    job->available_in  = compressed_size;
    job->available_out = static_cast<uint32_t>(reference.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;
    job->dictionary    = dictionary_ptr;

    // Decompression
    status = qpl_execute_job(job);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during decompression.\n";
        return 1;
    }

    // Freeing resources
    status = qpl_fini_job(job);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during job finalization.\n";
        return 1;
    }

    std::cout << "Content was successfully decompressed with dictionary.\n";

    return 0;
}

auto main(int argc, char** argv) -> int {
    std::cout << "Intel(R) Query Processing Library version is " << qpl_get_library_version() << ".\n";

    // Default to Hardware Path
    qpl_path_t execution_path = qpl_path_hardware;

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

    // Dictionary initialization
    qpl_dictionary* dictionary_ptr = nullptr;

    // Build dictionary and check if building failed
    if (create_dictionary(execution_path, source, &dictionary_ptr) != 0) { return 1; }

    // Compression and check if compression failed
    const uint8_t comp_status = compression(execution_path, source, destination, dictionary_ptr);
    if (comp_status == QPL_STS_NOT_SUPPORTED_MODE_ERR) {
        // Free dictionary
        destroy_dictionary(&dictionary_ptr);
        return 0;
    } else if (comp_status != 0) {
        // Free dictionary
        destroy_dictionary(&dictionary_ptr);
        return comp_status;
    }

    // Decompression with software_path and check if decompression failed
    const uint8_t decomp_status = sw_decompression(destination, reference, dictionary_ptr);
    if (decomp_status != 0) {
        // Free dictionary
        destroy_dictionary(&dictionary_ptr);
        return decomp_status;
    }

    // Free dictionary
    destroy_dictionary(&dictionary_ptr);

    // Compare source and reference
    for (size_t i = 0; i < source.size(); i++) {
        if (source[i] != reference[i]) {
            std::cout << "Content wasn't successfully compressed and decompressed with dictionary.\n";
            return 1;
        }
    }

    std::cout << "Content was successfully compressed and decompressed with dictionary.\n";
    std::cout << "Input size: " << source.size() << ", compressed size: " << destination.size()
              << ", compression ratio: " << (float)source.size() / (float)destination.size() << ".\n";

    return 0;
}

//* [QPL_LOW_LEVEL_MIX_PATHS_COMP_DECOMP_W_DICT_EXAMPLE] */
