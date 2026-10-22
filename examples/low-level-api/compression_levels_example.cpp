/*******************************************************************************
 * Copyright (C) 2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_LOW_LEVEL_COMPRESSION_LEVELS_EXAMPLE] */

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
 * the library to chose the path to execute.
 *
 * The example compresses and decompresses the same buffer at every compression level defined by
 * @ref qpl_compression_levels. A requested level that exceeds what the execution path supports is
 * automatically saturated to the highest supported level (software up to qpl_high_level,
 * accelerator generation 3.0 supports 0-7, and earlier generations use the default level only),
 * so the same code runs correctly on any hardware.
 *
 * The compression level is orthogonal to the Huffman mode: every level, including qpl_level_0,
 * works with dynamic Huffman coding.
 */
constexpr const uint32_t source_size = 64 * 1024;

// Compresses and decompresses source at the given level, returns the compressed size (0 on failure).
// The job is expected to be already initialized; it is reused across levels.
static uint32_t compress_and_verify(qpl_job* job, const std::vector<uint8_t>& source, qpl_compression_levels level) {
    std::vector<uint8_t> destination(qpl_get_safe_deflate_compression_buffer_size(source_size));
    std::vector<uint8_t> reference(source.size());

    job->op            = qpl_op_compress;
    job->level         = level;
    job->next_in_ptr   = const_cast<uint8_t*>(source.data());
    job->next_out_ptr  = destination.data();
    job->available_in  = static_cast<uint32_t>(source.size());
    job->available_out = static_cast<uint32_t>(destination.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN | QPL_FLAG_OMIT_VERIFY;

    qpl_status status = qpl_execute_job(job);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during compression at level " << static_cast<int>(level)
                  << ".\n";
        return 0;
    }

    const uint32_t compressed_size = job->total_out;

    job->op            = qpl_op_decompress;
    job->next_in_ptr   = destination.data();
    job->next_out_ptr  = reference.data();
    job->available_in  = compressed_size;
    job->available_out = static_cast<uint32_t>(reference.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;

    status = qpl_execute_job(job);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during decompression at level " << static_cast<int>(level)
                  << ".\n";
        return 0;
    }

    if (source != reference) {
        std::cout << "Content wasn't successfully compressed and decompressed at level " << static_cast<int>(level)
                  << ".\n";
        return 0;
    }

    return compressed_size;
}

auto main(int argc, char** argv) -> int {
    try {
        std::cout << "Intel(R) Query Processing Library version is " << qpl_get_library_version() << ".\n";

        // Default to Software Path
        qpl_path_t execution_path = qpl_path_software;

        // Get path from input argument
        const int parse_ret = parse_execution_path(argc, argv, &execution_path);
        if (parse_ret != 0) { return 1; }

        // Source buffer filled with pseudo-random text drawn from a small vocabulary.
        // This mimics realistic, moderately compressible data so different levels can
        // produce visibly different ratios, unlike a trivially repeating pattern.
        const char* const    words[] = {"the",   "quick", "brown", "fox",  "jumps", "over", "lazy",
                                        "dog",   "and",   "then",  "runs", "away",  "into", "forest",
                                        "where", "trees", "grow",  "tall", "near",  "river"};
        std::vector<uint8_t> source;
        source.reserve(source_size);
        uint32_t rng = 42;
        while (source.size() < source_size) {
            rng                    = rng * 1103515245U + 12345U;
            const char* const word = words[(rng >> 16) % 20];
            for (const char* c = word; *c != '\0'; c++) {
                source.push_back(static_cast<uint8_t>(*c));
            }
            source.push_back(static_cast<uint8_t>(' '));
        }
        source.resize(source_size);

        uint32_t   job_size = 0;
        qpl_status status   = qpl_get_job_size(execution_path, &job_size);
        if (status != QPL_STS_OK) {
            std::cout << "An error " << status << " acquired during job size getting.\n";
            return 1;
        }

        auto     job_buffer = std::make_unique<uint8_t[]>(job_size);
        qpl_job* job        = reinterpret_cast<qpl_job*>(job_buffer.get());

        status = qpl_init_job(execution_path, job);
        if (status != QPL_STS_OK) {
            std::cout << "An error " << status << " acquired during job initializing.\n";
            return 1;
        }

        const qpl_compression_levels all_levels[] = {qpl_level_0, qpl_level_1, qpl_level_2, qpl_level_3, qpl_level_4,
                                                     qpl_level_5, qpl_level_6, qpl_level_7, qpl_level_8, qpl_level_9};

        for (const qpl_compression_levels level : all_levels) {
            const uint32_t compressed_size = compress_and_verify(job, source, level);
            if (compressed_size == 0) { return 1; }

            std::cout << "Level " << static_cast<int>(level) << ": input size " << source.size() << ", compressed size "
                      << compressed_size << ", compression ratio "
                      << static_cast<float>(source.size()) / static_cast<float>(compressed_size) << ".\n";
        }

        status = qpl_fini_job(job);
        if (status != QPL_STS_OK) {
            std::cout << "An error " << status << " acquired during job finalization.\n";
            return 1;
        }

        std::cout << "Content was successfully compressed and decompressed at every level.\n";

        return 0;
    } catch (...) { return exception_handler(); }
}

//* [QPL_LOW_LEVEL_COMPRESSION_LEVELS_EXAMPLE] */
