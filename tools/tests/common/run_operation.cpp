/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "run_operation.hpp"

#include <cstdint>
#include <memory>
#include <vector>

#include "qpl/qpl.h"

// tool_generator
#include "igenerator.h"

// tests_common
#include "execution_wrapper.hpp"

namespace qpl::test {

qpl_status run_decompress_op(qpl_path_t execution_path) {
    constexpr uint32_t source_size = 64U;

    std::vector<uint8_t> source;
    std::vector<uint8_t> destination;

    std::vector<uint8_t> encoded_data_buffer(0);
    std::vector<uint8_t> decoded_data_buffer(0);

    GenStatus  generator_status = GEN_OK;
    TestFactor test_factor;
    test_factor.seed = 0;
    test_factor.type = NO_ERR_FIXED_BLOCK;

    const gz_generator::InflateGenerator data_generator;

    generator_status = gz_generator::InflateGenerator::generate(encoded_data_buffer, decoded_data_buffer, test_factor);

    std::unique_ptr<uint8_t[]> job_buffer;
    uint32_t                   size = 0U;

    source.resize(encoded_data_buffer.size());
    std::copy(encoded_data_buffer.begin(), encoded_data_buffer.end(), source.begin());

    destination.resize(decoded_data_buffer.size());

    qpl_status status = qpl_get_job_size(execution_path, &size);
    if (status != QPL_STS_OK) { return status; }

    job_buffer   = std::make_unique<uint8_t[]>(size);
    qpl_job* job = reinterpret_cast<qpl_job*>(job_buffer.get());

    status = qpl_init_job(execution_path, job);
    if (status != QPL_STS_OK) { return status; }

    job->op            = qpl_op_decompress;
    job->next_in_ptr   = source.data();
    job->next_out_ptr  = destination.data();
    job->available_in  = source.size();
    job->available_out = static_cast<uint32_t>(destination.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;

    status = run_job_api(job);

    qpl_fini_job(job);
    return status;
}

qpl_status run_compress_op(qpl_path_t execution_path) {
    constexpr uint32_t source_size = 64U;

    std::vector<uint8_t> source(source_size, 5);
    std::vector<uint8_t> destination(source_size / 2, 4);

    std::unique_ptr<uint8_t[]> job_buffer;
    uint32_t                   size = 0U;

    qpl_status status = qpl_get_job_size(execution_path, &size);
    if (status != QPL_STS_OK) { return status; }

    job_buffer   = std::make_unique<uint8_t[]>(size);
    qpl_job* job = reinterpret_cast<qpl_job*>(job_buffer.get());

    status = qpl_init_job(execution_path, job);
    if (status != QPL_STS_OK) { return status; }

    job->op            = qpl_op_compress;
    job->level         = qpl_default_level;
    job->next_in_ptr   = source.data();
    job->next_out_ptr  = destination.data();
    job->available_in  = source_size;
    job->available_out = static_cast<uint32_t>(destination.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN | QPL_FLAG_OMIT_VERIFY;

    status = run_job_api(job);

    qpl_fini_job(job);
    return status;
}

qpl_status run_crc64_op(qpl_path_t execution_path) {
    constexpr uint32_t source_size = 64U;
    constexpr uint64_t poly        = 0x04C11DB700000000U;

    std::vector<uint8_t> source(source_size, 4);

    std::unique_ptr<uint8_t[]> job_buffer;
    uint32_t                   size = 0U;

    qpl_status status = qpl_get_job_size(execution_path, &size);
    if (status != QPL_STS_OK) { return status; }

    job_buffer   = std::make_unique<uint8_t[]>(size);
    qpl_job* job = reinterpret_cast<qpl_job*>(job_buffer.get());

    status = qpl_init_job(execution_path, job);
    if (status != QPL_STS_OK) { return status; }

    job->op           = qpl_op_crc64;
    job->next_in_ptr  = source.data();
    job->available_in = source_size;
    job->crc64_poly   = poly;

    status = run_job_api(job);

    qpl_fini_job(job);
    return status;
}

qpl_status run_scan_op(qpl_path_t execution_path) {
    constexpr uint32_t source_size     = 64U;
    constexpr uint32_t input_bit_width = 8U;
    constexpr uint32_t value_to_find   = 48U;

    std::vector<uint8_t> source(source_size, 0);
    std::vector<uint8_t> destination(source_size, 4);

    std::unique_ptr<uint8_t[]> job_buffer;
    uint32_t                   size    = 0U;
    const auto*                indices = reinterpret_cast<const uint32_t*>(destination.data());

    qpl_status status = qpl_get_job_size(execution_path, &size);
    if (status != QPL_STS_OK) { return status; }

    job_buffer   = std::make_unique<uint8_t[]>(size);
    qpl_job* job = reinterpret_cast<qpl_job*>(job_buffer.get());

    status = qpl_init_job(execution_path, job);
    if (status != QPL_STS_OK) { return status; }

    job->next_in_ptr        = source.data();
    job->available_in       = source_size;
    job->next_out_ptr       = destination.data();
    job->available_out      = static_cast<uint32_t>(destination.size());
    job->op                 = qpl_op_scan_eq;
    job->src1_bit_width     = input_bit_width;
    job->num_input_elements = source_size;
    job->out_bit_width      = qpl_ow_32;
    job->param_low          = value_to_find;

    status = run_job_api(job);

    qpl_fini_job(job);
    return status;
}

qpl_status run_extract_op(qpl_path_t execution_path) {
    constexpr uint32_t source_size        = 64U;
    constexpr uint32_t input_vector_width = 8U;
    constexpr uint32_t lower_index        = 10U;
    constexpr uint32_t upper_index        = 53U;

    // Source and output containers
    std::vector<uint8_t> source(source_size, 0);
    std::vector<uint8_t> destination(source_size, 4);

    std::unique_ptr<uint8_t[]> job_buffer;
    uint32_t                   size = 0U;

    qpl_status status = qpl_get_job_size(execution_path, &size);
    if (status != QPL_STS_OK) { return status; }

    job_buffer   = std::make_unique<uint8_t[]>(size);
    qpl_job* job = reinterpret_cast<qpl_job*>(job_buffer.get());

    status = qpl_init_job(execution_path, job);
    if (status != QPL_STS_OK) { return status; }

    job->next_in_ptr        = source.data();
    job->available_in       = source_size;
    job->next_out_ptr       = destination.data();
    job->available_out      = static_cast<uint32_t>(destination.size());
    job->op                 = qpl_op_extract;
    job->src1_bit_width     = input_vector_width;
    job->param_low          = lower_index;
    job->param_high         = upper_index;
    job->num_input_elements = source_size;
    job->out_bit_width      = qpl_ow_nom;

    status = run_job_api(job);

    qpl_fini_job(job);
    return status;
}

qpl_status run_select_op(qpl_path_t execution_path) {
    constexpr uint32_t source_size = 64U;

    std::vector<uint8_t> source(source_size, 0);
    std::vector<uint8_t> mask(source_size / 8, 4);
    std::vector<uint8_t> destination(source_size, 4);

    std::unique_ptr<uint8_t[]> job_buffer;
    uint32_t                   size = 0U;

    qpl_status status = qpl_get_job_size(execution_path, &size);
    if (status != QPL_STS_OK) { return status; }

    job_buffer   = std::make_unique<uint8_t[]>(size);
    qpl_job* job = reinterpret_cast<qpl_job*>(job_buffer.get());

    status = qpl_init_job(execution_path, job);
    if (status != QPL_STS_OK) { return status; }

    job->next_in_ptr        = source.data();
    job->available_in       = source_size;
    job->next_out_ptr       = destination.data();
    job->available_out      = static_cast<uint32_t>(destination.size());
    job->op                 = qpl_op_select;
    job->src1_bit_width     = 8;
    job->num_input_elements = source_size;
    job->out_bit_width      = qpl_ow_nom;
    job->next_src2_ptr      = mask.data();
    job->available_src2     = source_size;
    job->src2_bit_width     = 1;

    status = run_job_api(job);

    qpl_fini_job(job);
    return status;
}

qpl_status run_expand_op(qpl_path_t execution_path) {
    constexpr uint32_t source_size         = 5U;
    constexpr uint32_t mask_byte_length    = 1U;
    constexpr uint32_t input_vector_width  = 8U;
    constexpr uint32_t output_vector_width = 1U;
    constexpr uint8_t  mask                = 0b10111001U;
    constexpr uint32_t mask_size           = 8U;

    std::vector<uint8_t>       source = {1, 2, 3, 4, 5};
    std::vector<uint8_t>       destination(source_size * 4, 0);
    const std::vector<uint8_t> reference = {1, 0, 0, 2, 3, 4, 0, 5};

    std::unique_ptr<uint8_t[]> job_buffer;
    uint32_t                   size = 0;

    qpl_status status = qpl_get_job_size(execution_path, &size);
    if (status != QPL_STS_OK) { return status; }

    job_buffer   = std::make_unique<uint8_t[]>(size);
    qpl_job* job = reinterpret_cast<qpl_job*>(job_buffer.get());

    status = qpl_init_job(execution_path, job);
    if (status != QPL_STS_OK) { return status; }

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

    status = run_job_api(job);

    qpl_fini_job(job);
    return status;
}

} // namespace qpl::test
