/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <assert.h>

#include "hw_aecs_api.h"
#include "hw_definitions.h"
#include "hw_descriptors_api.h"
#include "own_hw_definitions.h"
#include "simple_memory_ops_c_bind.h"

HW_PATH_IAA_API(void, descriptor_init_statistic_collector,
                (hw_descriptor* const descriptor_ptr, const uint8_t* const source_ptr, const uint32_t source_size,
                 hw_iaa_histogram* const histogram_ptr)) {
    hw_compress_descriptor* const this_ptr = (hw_compress_descriptor*)descriptor_ptr;

    this_ptr->source_ptr           = (uint8_t*)source_ptr;
    this_ptr->source_size          = source_size;
    this_ptr->destination_ptr      = (uint8_t*)histogram_ptr;
    this_ptr->max_destination_size = sizeof(hw_iaa_histogram);
    this_ptr->aecs_ptr             = NULL;
    this_ptr->aecs_size            = 0U;

    call_c_set_zeros_uint8_t(this_ptr->reserved, sizeof(this_ptr->reserved));

    this_ptr->op_code_op_flags = ADOF_OPCODE(QPL_OPCODE_COMPRESS);

    this_ptr->compression_flags = ADCF_STATS_MODE;

    // Reset Compression 2 flags
    this_ptr->compression_2_flags = 0U;

    // reserved in case of SWQ
    // , so need to make sure that there is no garbage is stored
    this_ptr->trusted_fields = 0U;

    // reserved when bit #4 in op_code_op_flags is 0
    // (Request Completion Interrupt flag is 0, no interrupt is generated)
    // , so need to make sure that there is no garbage is stored
    if (!(this_ptr->op_code_op_flags & (1 << 3))) this_ptr->interrupt_handle = 0U;
}

HW_PATH_IAA_API(void, descriptor_init_statistic_collector_with_header_gen,
                (hw_descriptor* const descriptor_ptr, const uint8_t* const source_ptr, const uint32_t source_size,
                 hw_iaa_aecs* const aecs_ptr, const uint8_t aecs_index, const uint32_t b_final,
                 const uint32_t b_first)) {
    hw_compress_descriptor* const this_ptr = (hw_compress_descriptor*)descriptor_ptr;

    this_ptr->source_ptr  = (uint8_t*)source_ptr;
    this_ptr->source_size = source_size;

    // The 1st pass will not perform the actual compression
    this_ptr->destination_ptr      = NULL;
    this_ptr->max_destination_size = 0U;

    // Source 2 will be read/written as AECS
    this_ptr->aecs_ptr  = (uint8_t*)aecs_ptr;
    this_ptr->aecs_size = HW_AECS_COMPRESS_WITH_HT;

    call_c_set_zeros_uint8_t(this_ptr->reserved, sizeof(this_ptr->reserved));

    this_ptr->op_code_op_flags = ADOF_OPCODE(QPL_OPCODE_COMPRESS);

    // Source 2 will be read except in the first job
    this_ptr->op_code_op_flags |= b_first ? 0U : ADOF_READ_SRC2(AD_RDSRC2_AECS);

    // Source 2 will be written as AEcs
    this_ptr->op_code_op_flags |= ADOF_WRITE_SRC2(AD_WRSRC2_ALWAYS);

    // Set AECS toggle
    this_ptr->op_code_op_flags |= aecs_index ? ADOF_AECS_SEL : 0U;

    if (b_final) {
        // Enable 2-Pass header gen and generate a bFinal Deflate Header
        this_ptr->compression_flags = ADCF_ENABLE_HDR_GEN(7U);
        this_ptr->compression_flags |= ADCF_FLUSH_OUTPUT;
    } else {
        // Enable 2-Pass header gen and generate a Deflate Header
        this_ptr->compression_flags = ADCF_ENABLE_HDR_GEN(6U);
    }

    // Set the Write AECS Huffman Tables flag in descriptor Compression 2 flags to enable writing AECS
    this_ptr->compression_2_flags = ADCF_WRITE_AECS_HT;

    // reserved in case of SWQ
    // so need to make sure that there is no garbage is stored
    this_ptr->trusted_fields = 0U;

    // reserved when bit #4 in op_code_op_flags is 0
    // (Request Completion Interrupt flag is 0, no interrupt is generated)
    // , so need to make sure that there is no garbage is stored
    if (!(this_ptr->op_code_op_flags & (1 << 3))) this_ptr->interrupt_handle = 0U;
}

HW_PATH_IAA_API(void, descriptor_set_1_pass_header_gen,
                (hw_descriptor* const descriptor_ptr, hw_iaa_aecs* const aecs_ptr, const uint8_t aecs_index,
                 const uint32_t b_final, const uint32_t b_first)) {
    hw_compress_descriptor* const this_ptr = (hw_compress_descriptor*)descriptor_ptr;

    if (b_first && b_final) {
        // If there is only one job, source 2 will not be used as AECS
        this_ptr->aecs_ptr  = NULL;
        this_ptr->aecs_size = 0U;
    } else {
        // If there are multiple jobs, source 2 should be used as AECS
        this_ptr->aecs_ptr  = (uint8_t*)aecs_ptr;
        this_ptr->aecs_size = HW_AECS_COMPRESS_WITH_HT;
    }

    // Set AECS toggle
    this_ptr->op_code_op_flags |= aecs_index ? ADOF_AECS_SEL : 0U;

    if (b_final) {
        // Enable 1-Pass header gen and generate a bFinal Deflate Header
        this_ptr->compression_flags |= ADCF_ENABLE_HDR_GEN(3U);
        this_ptr->compression_flags |= ADCF_FLUSH_OUTPUT;
    } else {
        // Enable 1-Pass header gen and generate a Deflate Header
        this_ptr->compression_flags |= ADCF_ENABLE_HDR_GEN(2U);
    }

    // Source 2 will be read except in the first job and will be written except in the last job
    this_ptr->op_code_op_flags |= b_first ? 0U : ADOF_READ_SRC2(AD_RDSRC2_AECS);
    this_ptr->op_code_op_flags |= b_final ? 0U : ADOF_WRITE_SRC2(AD_WRSRC2_ALWAYS);

    // Reset compression_2_flags
    this_ptr->compression_2_flags = 0U;
}

HW_PATH_IAA_API(void, descriptor_compress_set_dictionary_mode,
                (hw_descriptor* const descriptor_ptr, uint32_t load_dictionary_value,
                 uint32_t dictionary_size_in_aecs)) {
    hw_compress_descriptor* const this_ptr = (hw_compress_descriptor*)descriptor_ptr;

    // Set Load dictionary flag, src2 read, src2 read size
    this_ptr->compression_flags |= ADCF_LOAD_DICT(load_dictionary_value);
    this_ptr->op_code_op_flags |= ADOF_READ_SRC2(AD_RDSRC2_AECS);
    this_ptr->aecs_size = HW_AECS_COMPRESS_WITH_HT + dictionary_size_in_aecs;
}

HW_PATH_IAA_API(void, descriptor_compress_setup_dictionary,
                (hw_descriptor* const descriptor_ptr, const uint32_t dict_size_in_aecs,
                 const uint8_t* const dictionary_data_ptr, const uint32_t aecs_raw_dictionary_offset,
                 hw_iaa_aecs_compress* ccfg_base, uint32_t aecs_index, uint32_t aecs_size,
                 uint32_t load_dictionary_val)) {

    hw_iaa_aecs_compress* actual_aecs_ptr = hw_iaa_aecs_compress_get_aecs_ptr(ccfg_base, aecs_index, aecs_size);
    if (!actual_aecs_ptr) { return; }
    hw_iaa_aecs_compress_set_dictionary(actual_aecs_ptr, dictionary_data_ptr, dict_size_in_aecs,
                                        aecs_raw_dictionary_offset);

    hw_iaa_descriptor_compress_set_dictionary_mode((hw_descriptor*)descriptor_ptr, load_dictionary_val,
                                                   dict_size_in_aecs);
}

HW_PATH_IAA_API(void, descriptor_init_compress_body, (hw_descriptor* const descriptor_ptr)) {
    hw_compress_descriptor* const this_ptr = (hw_compress_descriptor*)descriptor_ptr;

    this_ptr->trusted_fields    = 0U;
    this_ptr->op_code_op_flags  = ADOF_OPCODE(QPL_OPCODE_COMPRESS);
    this_ptr->compression_flags = 0U;

    call_c_set_zeros_uint8_t(this_ptr->reserved, sizeof(this_ptr->reserved));
}

HW_PATH_IAA_API(void, descriptor_init_deflate_body,
                (hw_descriptor* const descriptor_ptr, uint8_t* const source_ptr, const uint32_t source_size,
                 uint8_t* const destination_ptr, const uint32_t destination_size)) {
    hw_compress_descriptor* const this_ptr = (hw_compress_descriptor*)descriptor_ptr;

    this_ptr->trusted_fields    = 0U;
    this_ptr->op_code_op_flags  = ADOF_OPCODE(QPL_OPCODE_COMPRESS);
    this_ptr->compression_flags = 0U;

    this_ptr->source_ptr           = source_ptr;
    this_ptr->source_size          = source_size;
    this_ptr->destination_ptr      = destination_ptr;
    this_ptr->max_destination_size = destination_size;

    call_c_set_zeros_uint8_t(this_ptr->reserved, sizeof(this_ptr->reserved));
}

HW_PATH_IAA_API(void, descriptor_compress_set_aecs,
                (hw_descriptor* const descriptor_ptr, hw_iaa_aecs* const aecs_ptr,
                 const hw_iaa_aecs_access_policy access_policy, bool is_gen1)) {
    hw_compress_descriptor* const this_ptr = (hw_compress_descriptor*)descriptor_ptr;

    uint32_t read_flag  = (access_policy & hw_aecs_access_read) ? ADOF_READ_SRC2(AD_RDSRC2_AECS) : 0;
    uint32_t write_flag = (access_policy & hw_aecs_access_write) ? ADOF_WRITE_SRC2(AD_WRSRC2_ALWAYS) : 0U;

    uint32_t toggle_aecs_flag = (access_policy & hw_aecs_toggle_rw) ? ADOF_AECS_SEL : 0U;

    this_ptr->op_code_op_flags |= read_flag | write_flag | toggle_aecs_flag;

    bool is_final = !(access_policy & hw_aecs_access_write);

    this_ptr->aecs_ptr  = (uint8_t*)aecs_ptr;
    this_ptr->aecs_size = HW_AECS_COMPRESS_WITH_HT;

    this_ptr->compression_2_flags = 0U;

    if (is_final) {
        this_ptr->compression_flags |= ADCF_FLUSH_OUTPUT;

        // Fix for QPL_FLAG_HUFFMAN_BE in Intel® In-Memory Analytics Accelerator (Intel® IAA) generation 1.0.
        // The workaround: When writing to the AECS compress Huffman table, if using Intel® IAA 1.0 and the job is a LAST job,
        // and the job specifies Big-Endian-16 mode: set the Huffman code for LL[256] to be 8 bits of 00.
        // Also, set the compression flag for append EOB at end.
        if ((this_ptr->compression_flags & ADCF_COMP_BE) && is_gen1) {
            hw_iaa_aecs_compress* const compress_state = (hw_iaa_aecs_compress* const)aecs_ptr;
            // V1 work-around
            compress_state[toggle_aecs_flag].histogram.ll_sym[256] = 8U << 15U;
            // Append EOB at end + Need to apply
            this_ptr->compression_flags |= (this_ptr->compression_flags & ~0xCU) | 0x4U;
        }
    }
}
