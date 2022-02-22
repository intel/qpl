/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Job API (public C API)
 */


#include "arguments_check.hpp"
#include "zero_compressor.hpp"
#include "compression/zero_compression.hpp"

typedef qpl::ml::compression::zero_operation_result_t
(*zero_function)(const qpl::ml::compression::zero_input_format_t input_format,
                 const qpl::ml::buffer_t &source_buffer,
                 const qpl::ml::buffer_t &destination_buffer,
                 const qpl::ml::compression::crc_type_t crc_type,
                 int32_t numa_id);

static zero_function call_zero_compress[3][2] = {
        // Auto
        qpl::ml::compression::call_zero_operation<qpl::ml::execution_path_t::auto_detect,
                                                  qpl::ml::compression::zero_operation_type::compress>,
        qpl::ml::compression::call_zero_operation<qpl::ml::execution_path_t::auto_detect,
                                                  qpl::ml::compression::zero_operation_type::decompress>,
        // Hardware
        qpl::ml::compression::call_zero_operation<qpl::ml::execution_path_t::hardware,
                                                  qpl::ml::compression::zero_operation_type::compress>,
        qpl::ml::compression::call_zero_operation<qpl::ml::execution_path_t::hardware,
                                                  qpl::ml::compression::zero_operation_type::decompress>,
        // Software
        qpl::ml::compression::call_zero_operation<qpl::ml::execution_path_t::software,
                                                  qpl::ml::compression::zero_operation_type::compress>,
        qpl::ml::compression::call_zero_operation<qpl::ml::execution_path_t::software,
                                                  qpl::ml::compression::zero_operation_type::decompress>,
};

namespace qpl {
uint32_t perform_zero_compress(qpl_job *job_ptr, uint8_t *buffer_ptr, const uint32_t buffer_size) noexcept {
    using namespace qpl::ml;

    OWN_QPL_CHECK_STATUS(qpl::job::validate_operation<qpl_op_z_compress16>(job_ptr))

    qpl::job::reset<qpl_op_z_compress16>(job_ptr);

    compression::zero_operation_result_t zero_result;

    compression::crc_type_t crc_type = (job_ptr->flags & QPL_FLAG_CRC32C) ?
                                       compression::crc_type_t::crc_32c :
                                       compression::crc_type_t::crc_32;

    auto operation_type = ((job_ptr->op == qpl_op_z_compress16) || (job_ptr->op == qpl_op_z_compress32)) ?
                          compression::zero_operation_type::compress :
                          compression::zero_operation_type::decompress;

    auto zero_input_format = ((job_ptr->op == qpl_op_z_compress16) || (job_ptr->op == qpl_op_z_decompress16)) ?
                             compression::zero_input_format_t::word_16_bit :
                             compression::zero_input_format_t::word_32_bit;

    qpl::ml::buffer_t source_buffer(job_ptr->next_in_ptr, job_ptr->next_in_ptr + job_ptr->available_in);
    qpl::ml::buffer_t destination_buffer(job_ptr->next_out_ptr, job_ptr->next_out_ptr + job_ptr->available_out);

    zero_result = call_zero_compress[job_ptr->data_ptr.path][static_cast<uint32_t>(operation_type)]
            (zero_input_format,
             source_buffer,
             destination_buffer,
             crc_type,
             job_ptr->numa_id);

    if (zero_result.status_code_ == 0) {
        qpl::job::update_input_stream(job_ptr, job_ptr->available_in);
        qpl::job::update_output_stream(job_ptr, zero_result.output_bytes_, 0u);
        qpl::job::update_checksums(job_ptr, zero_result.checksums_.crc32_, zero_result.checksums_.xor_);
    }

    return zero_result.status_code_;
}
}
