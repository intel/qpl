/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

//* [QPL_LOW_LEVEL_EXPAND_EXAMPLE] */

#include <iostream>
#include <memory>
#include <numeric>
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
 * This example demonstrates the usage of the `Force Array Output Modification` feature. The feature allows the user to
 * force the output of filter operations to be an array of a size specified by the user. Without this feature, the output
 * of filter operations where the output size is 1 bit will be returned as a bit vector. The feature is enabled by setting
 * the `QPL_FLAG_FORCE_ARRAY_OUTPUT` flag in the job structure. The feature is supported only in `Hardware Path` on the
 * Intel® In-Memory Analytics Accelerator (Intel® IAA) 2.0 devices.
 *
 * @warning The use of `Force Array Output Modification` requires the use of the `Output Bit Width Modification` feature.
 * The `Output Bit Width Modification` feature allows the user to specify the bit width of the output of the filter operation.
 * The feature is enabled by setting the `out_bit_width` field in the job structure.
 *
 * @warning ---! Important !---
 * `Hardware Path` doesn't support all features declared for `Software Path`
 *
 */
constexpr const uint32_t source_size         = 5U;
constexpr const uint32_t mask_byte_length    = 1U;
constexpr const uint32_t input_vector_width  = 1U;
constexpr const uint32_t output_vector_width = 1U;
constexpr const uint8_t  mask                = 0b0000000'0U;
constexpr const uint32_t mask_size           = 1U;

auto main(int argc, char** argv) -> int {
    std::cout << "Intel(R) Query Processing Library version is " << qpl_get_library_version() << ".\n";

    // Default to Hardware Path
    qpl_path_t execution_path = qpl_path_hardware;

    // Get path from input argument
    const int parse_ret = parse_execution_path(argc, argv, &execution_path);
    if (parse_ret != 0) { return 1; }

    // Source and output containers
    std::vector<uint8_t> source      = {0b0000'0001U};
    std::vector<uint8_t> destination = {0U};
    std::vector<uint8_t> reference   = {0U};

    std::unique_ptr<uint8_t[]> job_buffer;
    uint32_t                   size = 0U;

    // Check if on software path
    if (execution_path == qpl_path_software) {
        std::cout << "Force Array Output Modification is not supported on qpl_path_software.\n";
        return 0;
    }

    // Job initialization
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

    // Performing an operation
    job->next_in_ptr        = source.data();
    job->available_in       = static_cast<uint32_t>(source.size());
    job->next_out_ptr       = destination.data();
    job->available_out      = static_cast<uint32_t>(destination.size());
    job->op                 = qpl_op_expand;
    job->src1_bit_width     = input_vector_width;
    job->src2_bit_width     = output_vector_width;
    job->available_src2     = mask_byte_length;
    job->num_input_elements = mask_size;
    job->out_bit_width      = qpl_ow_8;
    job->next_src2_ptr      = const_cast<uint8_t*>(&mask);

    // Enable Force Array Output Modification
    job->flags |= QPL_FLAG_FORCE_ARRAY_OUTPUT;

    status = qpl_execute_job(job);
    if (status == QPL_STS_NOT_SUPPORTED_MODE_ERR) {
        std::cout
                << "Force Array Output Modification is not supported. This feature is only available on Intel® In-Memory Analytics Accelerator (Intel® IAA) 2.0 with Hardware Path.\n";
        std::cout << "Note that Force Array Output Modification is supported only in Hardware Path.\n";
        return 0;
    } else if (status == QPL_STS_OUT_FORMAT_ERR) {
        std::cout
                << "Using Force Array Output Modification flag requires setting output bit width with `job->out_bit_width`\n";
        return 1;
    } else if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during performing expand.\n";
        return 1;
    }

    const auto expand_size = job->total_out;

    // Freeing resources
    status = qpl_fini_job(job);
    if (status != QPL_STS_OK) {
        std::cout << "An error " << status << " acquired during job finalization.\n";
        return 1;
    }

    if (expand_size != 1) {
        std::cout << "Error occurred, expected expand size 1, but got " << expand_size << "\n";
        return 1;
    } else {
        // Compare with reference
        if (destination[0] == reference[0]) {
            std::cout << "Expand with Force Array Output Modification was performed successfully.\n";
        } else {
            std::cout
                    << "Error occurred in Expand with Force Array Output Modification, expand result is not equal to reference."
                    << "\n";
            return 1;
        }
    }

    return 0;
}

//* [QPL_LOW_LEVEL_EXPAND_EXAMPLE] */
