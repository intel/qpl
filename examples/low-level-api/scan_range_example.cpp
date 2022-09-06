/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_LOW_LEVEL_SCAN_RANGE_EXAMPLE] */

#include <iostream>
#include <vector>
#include <numeric>

#include "qpl/qpl.h"

constexpr const uint32_t source_size         = 1000;
constexpr const uint32_t input_vector_width  = 8;
constexpr const uint32_t output_vector_width = 32;
constexpr const uint32_t lower_boundary      = 48;
constexpr const uint32_t upper_boundary      = 58;
constexpr const uint32_t byte_bit_length     = 8;

/**
 * @brief @ref qpl_path_software (`Software Path`) means that computations will be done with CPU.
 * Accelerator can be used instead of CPU. In this case, @ref qpl_path_hardware (`Hardware Path`) must be specified.
 * If there is no difference where calculations should be done, @ref qpl_path_auto (`Auto Path`) can be used to allow
 * the library to chose the path to execute.
 *
 * @warning ---! Important !---
 * `Hardware Path` doesn't support all features declared for `Software Path`
 *
 */
constexpr const auto execution_path = qpl_path_software;

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source(source_size, 0);
    std::vector<uint8_t> destination(source_size * 4, 4);

    qpl_job    *job;
    qpl_status status;
    uint32_t   size     = 0;
    const auto *indices = reinterpret_cast<const uint32_t *>(destination.data());

    // Filling source containers
    std::iota(std::begin(source), std::end(source), 0);

    // Job initialization
    status = qpl_get_job_size(execution_path, &size);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during job size getting.");
    }

    job    = (qpl_job *) std::malloc(size);
    status = qpl_init_job(execution_path, job);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during job initializing.");
    }

    // Performing an operation
    job->next_in_ptr        = source.data();
    job->available_in       = static_cast<uint32_t>(source.size());
    job->next_out_ptr       = destination.data();
    job->available_out      = static_cast<uint32_t>(destination.size());
    job->op                 = qpl_op_scan_range;
    job->src1_bit_width     = input_vector_width;
    job->num_input_elements = static_cast<uint32_t>(source.size());
    job->out_bit_width      = qpl_ow_32;
    job->param_low          = lower_boundary;
    job->param_high         = upper_boundary;

    status = qpl_execute_job(job);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during job execution.");
    }

    const auto scan_range_size            = job->total_out;
    const auto scan_range_elements_number = scan_range_size * byte_bit_length / output_vector_width;

    // Freeing resources
    status = qpl_fini_job(job);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during job finalization.");
    }

    std::free(job);

    // Check if everything was alright
    for (uint32_t i = 0; i < scan_range_elements_number; i++) {
        const auto element = source[indices[i]];

        if (element < lower_boundary || element > upper_boundary) {
            throw std::runtime_error("Incorrect value was chosen while operation performing.");
        }
    }

    std::cout << "Scan range was performed successfully." << std::endl;

    return 0;
}

//* [QPL_LOW_LEVEL_SCAN_RANGE_EXAMPLE] */
