/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "tb_ll_common.hpp"

uint8_t reserved_op_codes[RESERVED_OPCODES_COUNT] = {0x02, 0x03, 0x06, 0x07, 0x0A, 0x0B, 0x0E, 0x0F, 0x16, 0x17,
                                                     0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x28, 0x29};

void set_input_stream(qpl_job* job_ptr, uint8_t* source_ptr, uint32_t source_size, uint32_t input_bit_width,
                      uint32_t elements_to_process, qpl_parser parser) {
    job_ptr->next_in_ptr        = source_ptr;
    job_ptr->available_in       = source_size;
    job_ptr->src1_bit_width     = input_bit_width;
    job_ptr->num_input_elements = elements_to_process;
    job_ptr->parser             = parser;
    job_ptr->level              = qpl_default_level;
}

void set_mask_stream(qpl_job* job_ptr, uint8_t* mask_ptr, uint32_t mask_size, uint32_t mask_bit_width) {
    job_ptr->next_src2_ptr  = mask_ptr;
    job_ptr->available_src2 = mask_size;
    job_ptr->src2_bit_width = mask_bit_width;
}

void set_output_stream(qpl_job* job_ptr, uint8_t* destination_ptr, uint32_t destination_size,
                       qpl_out_format output_bit_width) {
    job_ptr->next_out_ptr  = destination_ptr;
    job_ptr->available_out = destination_size;
    job_ptr->out_bit_width = output_bit_width;
}

void set_operation_properties(qpl_job* job_ptr, uint32_t drop_initial_bytes, uint32_t flags, qpl_operation operation) {
    job_ptr->drop_initial_bytes = drop_initial_bytes;
    job_ptr->flags              = flags;
    job_ptr->op                 = operation;
}

void set_range(qpl_job* job_ptr, uint32_t low_param, uint32_t high_param) {
    job_ptr->param_low  = low_param;
    job_ptr->param_high = high_param;
}

void set_indexing_parameters(qpl_job* job_ptr, qpl_mini_block_size mini_block_size, uint64_t* index_array,
                             uint32_t index_array_size) {
    job_ptr->mini_block_size = mini_block_size;
    job_ptr->idx_array       = index_array;
    job_ptr->idx_max_size    = index_array_size;
}
