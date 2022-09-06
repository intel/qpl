/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_LOW_LEVEL_SET_MEMBERSHIP_EXAMPLE] */

#include <iostream>
#include <vector>
#include <numeric>

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
 */
constexpr const auto execution_path          = qpl_path_software;
constexpr const uint32_t source_size         = 1000;
constexpr const uint32_t input_vector_width  = 8;
constexpr const uint32_t output_vector_width = 1;
constexpr const uint32_t byte_bit_length     = 8;

auto main() -> int {
    // Source and output containers
    std::vector<uint8_t> source(source_size, 0);
    // Size of mask is 2^N where N is input bit width
    std::vector<uint8_t> mask_after_find_unique(((1u << input_vector_width) + 7u) / 8u, 4);
    std::vector<uint8_t> destination(source_size, 4);

    qpl_job    *job;
    qpl_status status;
    uint32_t   size = 0;

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

    // Performing a find unique operation
    job->next_in_ptr        = source.data();
    job->available_in       = source_size;
    job->next_out_ptr       = mask_after_find_unique.data();
    job->available_out      = static_cast<uint32_t>(mask_after_find_unique.size());
    job->op                 = qpl_op_find_unique;
    job->src1_bit_width     = input_vector_width;
    job->src2_bit_width     = output_vector_width;
    job->num_input_elements = source_size;
    job->out_bit_width      = qpl_ow_nom;

    status = qpl_execute_job(job);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during job execution.");
    }

    const auto find_unique_byte_size = job->total_out;
    const auto mask_length           = find_unique_byte_size;

    // Performing set membership operation
    job->next_in_ptr        = source.data();
    job->available_in       = source_size;
    job->next_out_ptr       = destination.data();
    job->available_out      = static_cast<uint32_t>(destination.size());
    job->op                 = qpl_op_set_membership;
    job->src1_bit_width     = input_vector_width;
    job->num_input_elements = source_size;
    job->out_bit_width      = qpl_ow_nom;
    job->next_src2_ptr      = mask_after_find_unique.data();
    job->available_src2     = mask_length;
    job->src2_bit_width     = output_vector_width;

    status = qpl_execute_job(job);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during job execution.");
    }

    const auto set_membership_byte_size = job->total_out;

    // Freeing resources
    status = qpl_fini_job(job);
    if (status != QPL_STS_OK) {
        throw std::runtime_error("An error acquired during job finalization.");
    }

    std::free(job);

    // Check if everything was alright
    if (set_membership_byte_size != source_size / byte_bit_length) {
        throw std::runtime_error("Incorrect value was chosen while operation performing.");
    }

    std::cout << "Set membership was performed successfully." << std::endl;

    return 0;
}

//* [QPL_LOW_LEVEL_SET_MEMBERSHIP_EXAMPLE] */
