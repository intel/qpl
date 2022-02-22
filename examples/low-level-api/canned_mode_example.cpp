/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_LOW_LEVEL_CANNED_MODE_EXAMPLE] */

#include <iostream>
#include <vector>
#include <memory>

#include "qpl/qpl.h"

/**
 * @brief @ref qpl_path_software (`Software Path`) means that computations will be done with CPU.
 * Accelerator can be used instead of CPU. In this case, @ref qpl_path_hardware (`Hardware Path`) must be specified.
 * If there is no difference where calculations should be done, @ref qpl_path_auto (`Auto Path`) can be used to allow
 * the library to chose the path to execute.
 *
 * @warning ---! Important !---
 * `Hardware Path` doesn't support all features declared for `Software Path`
 *
 * @note More information about paths is in the documentation(doc/QPL_REFERENCE_MANUAL.md)
 */
constexpr const auto execution_path  = qpl_path_software;
constexpr const uint32_t source_size = 1000;

constexpr auto get_representation(qpl_path_t path) {
    if (path == qpl_path_software) {
        return QPL_SW_REPRESENTATION;
    }

    if (path == qpl_path_hardware) {
        return QPL_HW_REPRESENTATION;
    }

    return QPL_SW_REPRESENTATION | QPL_HW_REPRESENTATION;
}

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source(source_size, 5);
    std::vector<uint8_t> destination(source_size / 2, 4);
    std::vector<uint8_t> reference(source_size, 7);

    std::unique_ptr<uint8_t[]> job_buffer;
    qpl_status                 status;
    uint32_t                   size = 0;
    qpl_histogram              deflate_histogram{};

    // Job initialization
    status = qpl_get_job_size(execution_path, &size);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during job size getting.");
    }

    job_buffer = std::make_unique<uint8_t[]>(size);
    auto job = reinterpret_cast<qpl_job *>(job_buffer.get());
    status = qpl_init_job(execution_path, job);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during compression job initializing.");
    }

    // Huffman table initialization
    auto compression_table_buffer = std::make_unique<uint8_t[]>(static_cast<uint32_t>(QPL_COMPRESSION_TABLE_SIZE));
    auto *compression_table_ptr = reinterpret_cast<qpl_compression_huffman_table *>(compression_table_buffer.get());

    // Filling deflate histogram first
    status = qpl_gather_deflate_statistics(source.data(),
                                           source_size,
                                           &deflate_histogram,
                                           qpl_default_level,
                                           execution_path);

    if (status != QPL_STS_OK) {
        throw std::runtime_error("Error while compression occurred.");
    }

    // Building the Huffman table
    status = qpl_build_compression_table(&deflate_histogram,
                                         compression_table_ptr,
                                         QPL_DEFLATE_REPRESENTATION | get_representation(execution_path));

    if (status != QPL_STS_OK) {
        throw std::runtime_error("Error while compression occurred.");
    }

    // Now perform canned mode compression
    job->op            = qpl_op_compress;
    job->level         = qpl_default_level;
    job->next_in_ptr   = source.data();
    job->next_out_ptr  = destination.data();
    job->available_in  = source_size;
    job->available_out = static_cast<uint32_t>(destination.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_CANNED_MODE | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY;
    job->compression_huffman_table = compression_table_ptr;

    // Compression
    status = qpl_execute_job(job);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("Error while compression occurred.");
    }

    const uint32_t compressed_size = job->total_out;

    // Decompression job initialization
    status = qpl_init_job(execution_path, job);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during decompression job initializing.");
    }

    // Decompression table initialization
    auto decompression_table_buffer = std::make_unique<uint8_t[]>(static_cast<uint32_t>(QPL_DECOMPRESSION_TABLE_SIZE));
    auto *decompression_table_ptr = reinterpret_cast<qpl_decompression_huffman_table*>(decompression_table_buffer.get());

    // Build decompression table
    qpl_comp_to_decompression_table(compression_table_ptr,
                                    decompression_table_ptr,
                                    get_representation(execution_path) | QPL_DEFLATE_REPRESENTATION);

    // Performing a decompression operation
    job->op            = qpl_op_decompress;
    job->next_in_ptr   = destination.data();
    job->next_out_ptr  = reference.data();
    job->available_in  = compressed_size;
    job->available_out = static_cast<uint32_t>(reference.size());
    job->flags         = QPL_FLAG_NO_BUFFERING | QPL_FLAG_RND_ACCESS | QPL_FLAG_CANNED_MODE;
    job->decompression_huffman_table = decompression_table_ptr;

    // Decompression
    status = qpl_execute_job(job);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("Error while decompression occurred.");
    }

    // Freeing resources
    status = qpl_fini_job(job);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during job finalization.");
    }

    // Compare reference functions
    for (size_t i = 0; i < source.size(); i++) {
        if (source[i] != reference[i]) {
            throw std::runtime_error("Content wasn't successfully compressed and decompressed.");
        }
    }

    std::cout << "Content was successfully compressed and decompressed." << std::endl;
    std::cout << "Compressed size: " << compressed_size << std::endl;

    return 0;
}

//* [QPL_LOW_LEVEL_CANNED_MODE_EXAMPLE] */