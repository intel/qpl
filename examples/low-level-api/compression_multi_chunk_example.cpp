/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_LOW_LEVEL_COMPRESSION_MULTI_CHUNK_EXAMPLE] */

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>

#include "qpl/qpl.h"
#include "examples_utils.hpp" // for argument parsing function

#ifdef QPL_EXAMPLES_USE_LIBACCEL_CONFIG
#include <x86intrin.h>

extern "C" {

struct accfg_ctx;
struct accfg_device;
struct accfg_wq;

/* Instantiate a new library context */
int accfg_new(struct accfg_ctx **ctx);

/* Get first available device */
struct accfg_device *accfg_device_get_first(struct accfg_ctx *ctx);
/* Get next available device */
struct accfg_device *accfg_device_get_next(struct accfg_device *device);
/* Get numa id for device */
int accfg_device_get_numa_node(struct accfg_device *device);

/* macro to loop through all available devices */
#define accfg_device_foreach(ctx, device) \
	for (device = accfg_device_get_first(ctx); \
	     device != NULL; \
	     device = accfg_device_get_next(device))

/* Get first available workqueue on device */
struct accfg_wq *accfg_wq_get_first(struct accfg_device *device);
/* Get next available workqueue */
struct accfg_wq *accfg_wq_get_next(struct accfg_wq *wq);
/* Get max transfer size of workqueue */
uint64_t accfg_wq_get_max_transfer_size(struct accfg_wq *wq);

/* macro to loop through all available workqueues on device */
#define accfg_wq_foreach(device, wq) \
	for (wq = accfg_wq_get_first(device); \
	     wq != NULL; \
	     wq = accfg_wq_get_next(wq))

}

/**
 * @brief This function gets the current NUMA node id.
 */
int32_t get_numa_id() noexcept {
#if defined(__linux__)
    uint32_t tsc_aux = 0;

    __rdtscp(&tsc_aux);

    // Linux encodes NUMA node into [32:12] of TSC_AUX
    return static_cast<int32_t>(tsc_aux >> 12);
#else
    return -1;
#endif // if defined(__linux__)
}
#endif // #ifdef QPL_EXAMPLES_USE_LIBACCEL_CONFIG

/**
 * @brief This function gets the values for max transfer size from all available
 * workqueues on numa node (-1 for all) and sets max_transfer_size to the minimum
 * of the values returns a status code 0 is okay, -1 is an accel-config loading error
*/
int32_t get_min_max_transfer_size(uint64_t &max_transfer_size, int32_t numa_id = -1) {
#ifdef QPL_EXAMPLES_USE_LIBACCEL_CONFIG
    accfg_ctx    *ctx_ptr     = nullptr;
    accfg_device *device_ptr  = nullptr;
    accfg_wq     *wq_ptr      = nullptr;

    if (numa_id == -1) {
        numa_id = get_numa_id();
    }

    uint64_t current_min = UINT64_MAX;
    uint64_t current_value;

    int32_t context_creation_status = accfg_new(&ctx_ptr);
    if (0u != context_creation_status) {
        return -1;
    }
    accfg_device_foreach(ctx_ptr, device_ptr) {
        if(numa_id != accfg_device_get_numa_node(device_ptr)) {
            continue;
        }
        accfg_wq_foreach(device_ptr, wq_ptr) {
            current_value = accfg_wq_get_max_transfer_size(wq_ptr);
            if (current_value < current_min) {
                current_min = current_value;
            }
        }
    }
    max_transfer_size = current_min;
    return 0;
#else
    max_transfer_size = UINT64_MAX;
    return -1;
#endif // #ifdef QPL_EXAMPLES_USE_LIBACCEL_CONFIG
}


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
 * The example compresses data with multi-chunk and decompresses data using single job with Deflate fixed Huffman encoding.
 * If the accel-config library is available, this example will also check to ensure that the job size does not exceed the
 * accelerator configured maximum transfer size.
 */

constexpr uint32_t source_size = 21 * 1024 * 1024;

// In this example source data is split into `chunk_count` pieces.
// Compression is then performed via multiple job submissions.
constexpr uint32_t chunk_count = 7;

auto main(int argc, char** argv) -> int {
    std::cout << "Intel(R) Query Processing Library version is " << qpl_get_library_version() << ".\n";

    // Default to Software Path.
    qpl_path_t execution_path = qpl_path_software;

    // Get path from input argument.
    int parse_ret = parse_execution_path(argc, argv, &execution_path);
    if (parse_ret != 0) {
        return 1;
    }

    // Calculate chunk size for the compression.
    uint32_t chunk_size = source_size/chunk_count;

    if (execution_path == qpl_path_hardware) {
        uint64_t max_transfer_size = 0U;
        if(get_min_max_transfer_size(max_transfer_size) == 0){
            if (chunk_size > max_transfer_size) {
                std::cout << "Chunk size(" << chunk_size << ") exceeds configured max transfer size (" << max_transfer_size <<"), reducing chunk size.\n";
                chunk_size = max_transfer_size;
            }
        }
    }

    // Source and output containers.
    std::vector<uint8_t> source(source_size, 5);
    std::vector<uint8_t> destination(source_size / 2, 4);
    std::vector<uint8_t> reference(source_size, 7);

    std::unique_ptr<uint8_t[]> job_buffer;
    uint32_t                   size = 0;

    // Allocate and initialize job.
    qpl_status status = qpl_get_job_size(execution_path, &size);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during job size getting.\n";
        return 1;
    }

    job_buffer = std::make_unique<uint8_t[]>(size);
    qpl_job *job = reinterpret_cast<qpl_job *>(job_buffer.get());

    status = qpl_init_job(execution_path, job);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during job initializing.\n";
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
    job->huffman_table = NULL;

    uint32_t iteration_count   = 0U;
    uint32_t source_bytes_left = static_cast<uint32_t>(source.size());

    while (source_bytes_left > 0) {
        // Advance `next_in_ptr` pointer for the next iteration.
        // If writing into contiguous memory, this step is not necessary,
        // as the `next_in_ptr` will be updated at the end of previous execution by
        // number of bytes processed.
        job->next_in_ptr = source.data() + iteration_count * chunk_size;

        // In this example, all chunks are equal in size except for the last one.
        // So adjusting the size and setting the job to LAST.
        if (chunk_size >= source_bytes_left) {
            job->flags |= QPL_FLAG_LAST;
            chunk_size = source_bytes_left;
        }

        source_bytes_left -= chunk_size;
        job->available_in  = chunk_size;

        // Hardware requires that job->available_out does not exceed max_transfer_size
        job->available_out = std::min(chunk_size, job->available_out);

        // Execute compression operation.
        status = qpl_execute_job(job);
        if (status != QPL_STS_OK) {
            std::cout << "An error " << status << " acquired during compression.\n";
            return 1;
        }

        job->flags &= ~QPL_FLAG_FIRST;
        iteration_count++;
    }

    destination.resize(job->total_out);
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
        return 1;
    }

    // Free resources.
    status = qpl_fini_job(job);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during job finalization.\n";
        return 1;
    }

    // Compare compressed then decompressed buffer to original source.
    for (size_t i = 0; i < source.size(); i++) {
        if (source[i] != reference[i]) {
            std::cout << "Content wasn't successfully compressed and decompressed.\n";
            return 1;
        }
    }

    std::cout << "Content was successfully compressed and decompressed." << std::endl;
    std::cout << "Input size: " << source.size() << ", compressed size: " << compressed_size
    << ", compression ratio: " << (float)source.size()/(float)compressed_size << ".\n";

    return 0;
}

//* [QPL_LOW_LEVEL_COMPRESSION_MULTI_CHUNK_EXAMPLE] */
