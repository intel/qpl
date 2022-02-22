/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_LOW_LEVEL_SCAN_EXAMPLE] */

#include <iostream>
#include <vector>
#include <numeric>

#include "qpl/qpl.h"

constexpr const uint32_t source_size     = 3;
constexpr const uint32_t input_bit_width = 8;

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
constexpr const auto execution_path = qpl_path_software;

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source{1, 2, 3};
    std::vector<uint8_t> source_counter{2, 5, 4};
    std::vector<uint8_t> destination(2 + 5 + 4, 0);
    std::vector<uint8_t> reference{1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3};

    qpl_job    *job;
    qpl_status status;
    uint32_t   size = 0;

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
    job->op                 = qpl_op_rle_burst;
    job->num_input_elements = source_size;
    job->next_in_ptr        = source_counter.data();
    job->available_in       = source_size;
    job->src1_bit_width     = input_bit_width;

    job->next_src2_ptr  = source.data();
    job->available_src2 = source_size;
    job->src2_bit_width = input_bit_width;

    job->next_out_ptr  = destination.data();
    job->available_out = static_cast<uint32_t>(destination.size());
    job->out_bit_width = qpl_ow_nom;

    status = qpl_execute_job(job);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during job execution.");
    }

    // Freeing resources
    status = qpl_fini_job(job);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during job finalization.");
    }

    std::free(job);

    // Check if everything was alright
    for (uint32_t i = 0; i < reference.size(); i++) {
        if (reference[i] != destination[i]) {
            throw std::runtime_error("Incorrect value was chosen while job execution.");
        }
    }

    std::cout << "RLE burst was performed successfully." << std::endl;

    return 0;
}

//* [QPL_LOW_LEVEL_SCAN_EXAMPLE] */
