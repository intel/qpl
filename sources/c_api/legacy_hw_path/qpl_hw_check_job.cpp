/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 03/23/2020
 * @brief Internal HW API functions for @ref hw_check_job API implementation
 */

#include "common/defs.hpp"
#include "compression/deflate/compression_units/stored_block_units.hpp"
#include "compression/dictionary/dictionary_utils.hpp"
#include "compression/stream_decorators/gzip_decorator.hpp"
#include "compression/stream_decorators/zlib_decorator.hpp"
#include "hardware_defs.h"
#include "hardware_state.h"
#include "hw_definitions.h"
#include "hw_iaa_flags.h"
#include "job.hpp"
#include "own_defs.h"
#include "own_ml_submit_operation_api.hpp"
#include "simple_memory_ops.hpp"
#include "util/checkers.hpp"
#include "util/checksum.hpp"
#include "util/completion_record.hpp"
#include "util/descriptor_processing.hpp"
#include "util/iaa_features_checks.hpp"

// middle-layer
#include "accelerator/hw_accelerator_api.h"

// core-iaa/include
#include "hw_completion_record_api.h"
#include "hw_devices.h"

namespace qpl::ml {

#define AECS_WRITTEN(p) ((((p)->op_code_op_flags >> 18U) & 3U) == AD_WRSRC2_ALWAYS)

qpl_status set_state_to_complete_and_wrap(qpl_job* const job_ptr, qpl_hw_state* state_ptr, const uint32_t checksum) {
    state_ptr->execution_history.execution_step = qpl_task_execution_step_completed;

    if (QPL_FLAG_GZIP_MODE & job_ptr->flags) {
        if (sizeof(ml::compression::gzip_decorator::gzip_trailer) > job_ptr->available_out) {
            return QPL_STS_DST_IS_SHORT_ERR;
        }

        // gzip trailer is the checksum and size of uncompressed stream
        ml::compression::gzip_decorator::gzip_trailer trailer {};
        trailer.crc32      = checksum;
        trailer.input_size = job_ptr->total_in;

        ml::compression::gzip_decorator::write_trailer_unsafe(job_ptr->next_out_ptr, job_ptr->available_out, trailer);

        job_ptr->next_out_ptr += sizeof(trailer);
        job_ptr->available_out -= sizeof(trailer);
        job_ptr->total_out += sizeof(trailer);
    }

    if (QPL_FLAG_ZLIB_MODE & job_ptr->flags) {
        if (sizeof(ml::compression::zlib_sizes::zlib_trailer_size) > job_ptr->available_out) {
            return QPL_STS_DST_IS_SHORT_ERR;
        }

        // zlib trailer is a single checksum value
        const uint32_t adler32 = job::get_adler32(job_ptr);

        const uint32_t trailer_value =
                swap_bytes((adler32 & util::most_significant_16_bits) |
                           ((adler32 & util::least_significant_16_bits) + 1) % util::adler32_mod);

        ml::compression::zlib_decorator::write_trailer_unsafe(job_ptr->next_out_ptr, trailer_value);

        job_ptr->next_out_ptr += sizeof(trailer_value);
        job_ptr->available_out -= sizeof(trailer_value);
        job_ptr->total_out += sizeof(trailer_value);
    }

    return QPL_STS_OK;
}

qpl_status hw_check_compress_job(qpl_job* qpl_job_ptr) {
    using namespace qpl;

    auto* const                               state_ptr = reinterpret_cast<qpl_hw_state*>(job::get_state(qpl_job_ptr));
    hw_decompress_analytics_descriptor* const desc_ptr  = &state_ptr->desc_ptr;
    hw_iaa_completion_record* const           comp_ptr  = &state_ptr->comp_ptr;
    const bool                                is_final_block = QPL_FLAG_LAST & qpl_job_ptr->flags;

    hw_iaa_aecs_compress* const cfg_in_ptr =
            hw_iaa_aecs_compress_get_aecs_ptr(state_ptr->ccfg, state_ptr->aecs_hw_read_offset, state_ptr->aecs_size);
    if (!cfg_in_ptr) { return QPL_STS_LIBRARY_INTERNAL_ERR; }
    hw_iaa_aecs_compress* const cfg_out_ptr = hw_iaa_aecs_compress_get_aecs_ptr(
            state_ptr->ccfg, state_ptr->aecs_hw_read_offset ^ 1U, state_ptr->aecs_size);
    if (!cfg_out_ptr) { return QPL_STS_LIBRARY_INTERNAL_ERR; }
    // Check verification step
    if (QPL_OPCODE_DECOMPRESS == ADOF_GET_OPCODE(desc_ptr->op_code_op_flags)) {
        OWN_QPL_CHECK_STATUS(ml::util::convert_status_iaa_to_qpl(reinterpret_cast<hw_completion_record*>(comp_ptr)))

        qpl_job_ptr->idx_num_written += comp_ptr->output_size / sizeof(uint64_t);

        // Invert AECS toggle for compression
        state_ptr->aecs_hw_read_offset ^= 1U;

        // Invert AECS toggle for verify
        state_ptr->verify_aecs_hw_read_offset ^= 1U;

        return (is_final_block && (comp_ptr->crc != state_ptr->execution_history.compress_crc))
                       ? QPL_STS_INTL_VERIFY_ERR
                       : QPL_STS_OK;
    }

    // Software fallback for overflowing
    if (comp_ptr->error_code == AD_ERROR_CODE_UNRECOVERABLE_OUTPUT_OVERFLOW) {
        HW_IMMEDIATELY_RET((qpl_job_ptr->flags & (QPL_FLAG_NO_HDRS)), QPL_STS_DST_IS_SHORT_ERR);

        if (qpl_job_ptr->mini_block_size) {
            HW_IMMEDIATELY_RET((64U * 1024U <= qpl_job_ptr->available_in), QPL_STS_INDEX_GENERATION_ERR)
            HW_IMMEDIATELY_RET(
                    (((qpl_job_ptr->flags & (QPL_FLAG_FIRST | QPL_FLAG_LAST)) != (QPL_FLAG_FIRST | QPL_FLAG_LAST)) &&
                     (0U ==
                      (qpl_job_ptr->flags & (QPL_FLAG_FIRST | QPL_FLAG_START_NEW_BLOCK | QPL_FLAG_DYNAMIC_HUFFMAN)))),
                    QPL_STS_INDEX_GENERATION_ERR)
        }

        // Get AECS buffers accumulated bit size
        uint32_t bits_to_flush =
                (qpl_task_execution_step_header_inserting == state_ptr->execution_history.execution_step)
                        ? state_ptr->saved_num_output_accum_bits
                        : hw_iaa_aecs_compress_accumulator_get_actual_bits(cfg_in_ptr);

        // Insert EOB
        if (qpl_task_execution_step_header_inserting != state_ptr->execution_history.execution_step) {
            hw_iaa_aecs_compress_accumulator_insert_eob(cfg_in_ptr, state_ptr->eob_code);
            bits_to_flush += state_ptr->eob_code.length; // @todo recheck logic
        }

        auto stored_blocks_required_size = compression::calculate_size_needed(qpl_job_ptr->available_in, bits_to_flush);

        HW_IMMEDIATELY_RET((stored_blocks_required_size > qpl_job_ptr->available_out), QPL_STS_MORE_OUTPUT_NEEDED)

        const uint8_t* const input_data_ptr  = qpl_job_ptr->next_in_ptr;
        const uint32_t       input_data_size = qpl_job_ptr->available_in;

        uint8_t*       output_ptr    = qpl_job_ptr->next_out_ptr;
        const uint32_t output_size   = qpl_job_ptr->available_out;
        uint32_t       bytes_written = 0U;

        // Flush AECS buffers
        HW_IMMEDIATELY_RET((256U + 64U <= bits_to_flush), QPL_STS_LIBRARY_INTERNAL_ERR);

        if (bits_to_flush) {
            hw_iaa_aecs_compress_accumulator_flush(cfg_in_ptr, &output_ptr, bits_to_flush);
            bytes_written += bits_to_flush / 8U;
        } else {
            cfg_in_ptr->num_output_accum_bits = 0U;
        }

        const int64_t stored_block_bytes =
                ml::compression::write_stored_blocks(const_cast<uint8_t*>(input_data_ptr), input_data_size, output_ptr,
                                                     output_size, bits_to_flush & 7U, is_final_block);

        if (stored_block_bytes < 0) {
            return QPL_STS_MORE_OUTPUT_NEEDED;
        } else {
            bytes_written += static_cast<uint32_t>(stored_block_bytes);
        }

        // Calculate checksums and update their values in job ptr
        uint32_t crc = 0U, xor_checksum = 0U;
        hw_iaa_aecs_compress_get_checksums(cfg_in_ptr, &crc, &xor_checksum);
        crc = !(qpl_job_ptr->flags & QPL_FLAG_CRC32C)
                      ? util::crc32_gzip(input_data_ptr, input_data_ptr + input_data_size, crc)
                      : util::crc32_iscsi_inv(input_data_ptr, input_data_ptr + input_data_size, crc);

        xor_checksum = util::xor_checksum(input_data_ptr, input_data_ptr + input_data_size, xor_checksum);

        hw_iaa_aecs_compress_set_checksums(cfg_out_ptr, crc, xor_checksum);

        job::update_checksums(qpl_job_ptr, crc, xor_checksum);

        if (QPL_FLAG_ZLIB_MODE & qpl_job_ptr->flags) {
            const uint32_t prev_adler32    = job::get_adler32(qpl_job_ptr);
            const uint32_t new_acc_adler32 = qpl::ml::util::adler32(input_data_ptr, input_data_size, prev_adler32);
            job::update_adler32(qpl_job_ptr, new_acc_adler32);
        }

        // Update output ptrs and offsets in job ptr
        job::update_output_stream(qpl_job_ptr, bytes_written, bytes_written);

        if (is_final_block) {
            // Flag state as completed and wrap stream into gzip or zlib if necessary
            set_state_to_complete_and_wrap(qpl_job_ptr, state_ptr, crc);
        } else {
            state_ptr->execution_history.execution_step = qpl_task_execution_step_header_inserting;
        }

        // Update input ptrs and offsets in job ptr
        job::update_input_stream(qpl_job_ptr, input_data_size);

        if (!(QPL_FLAG_OMIT_VERIFY & qpl_job_ptr->flags)) { return hw_submit_verify_job(qpl_job_ptr); }

        // Invert AECS toggle for compression
        state_ptr->aecs_hw_read_offset ^= 1;

        return QPL_STS_OK;
    }

    // Validate descriptor result

    const ml::qpl_ml_status hw_status =
            ml::util::convert_status_iaa_to_qpl(reinterpret_cast<hw_completion_record*>(comp_ptr));

    // If HW returns error, remove the header that is written for gzip/zlib case and reset fields in job
    if ((qpl_job_ptr->flags & QPL_FLAG_FIRST) && (hw_status != ml::status_list::ok)) {
        uint32_t header_size = 0U;
        if (QPL_FLAG_GZIP_MODE & qpl_job_ptr->flags) {
            header_size = ml::compression::gzip_sizes::gzip_header_size;
        } else if (QPL_FLAG_ZLIB_MODE & qpl_job_ptr->flags) {
            header_size = ml::compression::zlib_sizes::zlib_header_size;
        }
        qpl_job_ptr->next_out_ptr -= header_size;
        qpl_job_ptr->available_out += header_size;
        qpl_job_ptr->total_out -= header_size;
    }
    OWN_QPL_CHECK_STATUS(hw_status)

    // Fix for QPL_FLAG_HUFFMAN_BE in Intel® In-Memory Analytics Accelerator (Intel® IAA) generation 1.0.
    // The workaround: When writing to the AECS compress Huffman table, if using Intel® IAA generation 1.0 and the job is a LAST job,
    // and the job specifies Big-Endian-16 mode: set the Huffman code for LL[256] to be 8 bits of 00.
    // Also, set the compression flag for append EOB at end.
    // When such a job completes (i.e. one modified as above), then the output size should have
    // the low-order bit cleared (i.e. rounded down to a multiple of 2)
    // and the output_bits needs to be fixed for some cases.
    if (((qpl_job_ptr->flags & (QPL_FLAG_HUFFMAN_BE | QPL_FLAG_LAST)) == (QPL_FLAG_HUFFMAN_BE | QPL_FLAG_LAST)) &&
        !qpl::ml::util::are_iaa_gen_2_min_capabilities_present()) {
        if (comp_ptr->output_size % 2 == 1) {
            // odd output size
            if (comp_ptr->output_bits != 0) { comp_ptr->output_bits += 8; }
            comp_ptr->output_size &= ~1U;
        } else {
            // even output size
            if (comp_ptr->output_bits == 0) { comp_ptr->output_bits += 8; }
        }
    }

    // Check if 2-pass header generation is used. There are 3 different modes indicated by the flag value:
    // 5: 2-pass header gen without Deflate header
    // 6: 2-pass header gen with Deflate header
    // 7: 2-pass header gen with bFinal Deflate header
    const bool hw_2_pass_header_gen = ADCF_ENABLE_HDR_GEN(5U) == (ADCF_ENABLE_HDR_GEN(5U) & desc_ptr->decomp_flags) ||
                                      ADCF_ENABLE_HDR_GEN(6U) == (ADCF_ENABLE_HDR_GEN(6U) & desc_ptr->decomp_flags) ||
                                      ADCF_ENABLE_HDR_GEN(7U) == (ADCF_ENABLE_HDR_GEN(7U) & desc_ptr->decomp_flags);

    // Resubmit deflate task for dynamic deflate (statistics generated in first pass)
    // or 2-pass header generation (Huffman table generated in first pass).
    if (ADCF_STATS_MODE & desc_ptr->decomp_flags || hw_2_pass_header_gen) {
        qpl_job_ptr->crc          = comp_ptr->crc;
        qpl_job_ptr->xor_checksum = comp_ptr->xor_checksum;

        qpl_status status =
                hw_descriptor_compress_init_deflate_dynamic(desc_ptr, state_ptr, qpl_job_ptr, cfg_in_ptr, comp_ptr);
        OWN_QPL_CHECK_STATUS(status)

        // Setup dictionary in compression descriptor
        if (qpl_job_ptr->dictionary != nullptr) {
            qpl_dictionary*      dictionary          = qpl_job_ptr->dictionary;
            const uint32_t       dict_size_in_aecs   = qpl::ml::compression::get_dictionary_size_in_aecs(*dictionary);
            const uint8_t* const dictionary_data_ptr = qpl::ml::compression::get_dictionary_data(*dictionary);
            const uint8_t        load_dictionary_val = qpl::ml::compression::get_load_dictionary_flag(*dictionary);

            hw_iaa_descriptor_compress_setup_dictionary((hw_descriptor*)desc_ptr, dict_size_in_aecs,
                                                        dictionary_data_ptr, dictionary->aecs_raw_dictionary_offset,
                                                        state_ptr->ccfg, state_ptr->aecs_hw_read_offset,
                                                        state_ptr->aecs_size, load_dictionary_val);
        }

        status = ml::util::process_descriptor<qpl_status, ml::util::execution_mode_t::async>(
                (hw_descriptor*)desc_ptr, (hw_completion_record*)&state_ptr->comp_ptr, qpl_job_ptr->numa_id);

        HW_IMMEDIATELY_RET(0U != status, QPL_STS_QUEUES_ARE_BUSY_ERR);

        return QPL_STS_BEING_PROCESSED;
    }

    // Body encoding step: Update Job with descriptor results
    state_ptr->config_valid = AECS_WRITTEN(desc_ptr);

    // Calculate checksums and update their values in job ptr
    job::update_checksums(qpl_job_ptr, comp_ptr->crc, comp_ptr->xor_checksum);

    if (QPL_FLAG_ZLIB_MODE & qpl_job_ptr->flags) {
        const uint32_t prev_adler32 = job::get_adler32(qpl_job_ptr);
        const uint32_t new_acc_adler32 =
                qpl::ml::util::adler32(qpl_job_ptr->next_in_ptr, qpl_job_ptr->available_in, prev_adler32);
        job::update_adler32(qpl_job_ptr, new_acc_adler32);
    }

    // Update output ptrs and offsets in job ptr
    job::update_output_stream(qpl_job_ptr, comp_ptr->output_size, comp_ptr->output_bits);

    if (is_final_block) {
        // Flag state as completed and wrap stream into gzip or zlib if necessary
        set_state_to_complete_and_wrap(qpl_job_ptr, state_ptr, comp_ptr->crc);
    } else if (!(QPL_FLAG_DYNAMIC_HUFFMAN & qpl_job_ptr->flags)) {
        state_ptr->execution_history.execution_step = qpl_task_execution_step_data_processing;
    }

    // Update input ptrs and offsets in job ptr
    job::update_input_stream(qpl_job_ptr, qpl_job_ptr->available_in);

    if (!(QPL_FLAG_OMIT_VERIFY & qpl_job_ptr->flags)) { return hw_submit_verify_job(qpl_job_ptr); }

    //  Invert AECS toggle for compression
    state_ptr->aecs_hw_read_offset ^= 1U;

    return QPL_STS_OK;
}

} // namespace qpl::ml

extern "C" qpl_status hw_check_job(qpl_job* qpl_job_ptr) {
    using namespace qpl;

    auto* const state_ptr = reinterpret_cast<qpl_hw_state*>(job::get_state(qpl_job_ptr));

    const auto* desc_ptr = &state_ptr->desc_ptr;
    const auto* comp_ptr = &state_ptr->comp_ptr;
    auto*       cfg_ptr  = GET_DCFG(state_ptr);

    if (!state_ptr->job_is_submitted) { return QPL_STS_JOB_NOT_SUBMITTED; }

    if (AD_STATUS_INPROG == comp_ptr->status) { return QPL_STS_BEING_PROCESSED; }

    if (TRIVIAL_COMPLETE == comp_ptr->status) {
        job::update_input_stream(qpl_job_ptr, comp_ptr->bytes_completed);

        return QPL_STS_OK;
    }

    // Simple Page Faults handling: if status AD_STATUS_READ_PAGE_FAULT or AD_STATUS_WRITE_PAGE_FAULT,
    // check that the Fault Address is available, touch the memory and resubmit descriptor again.
    if ((AD_STATUS_READ_PAGE_FAULT == comp_ptr->status || AD_STATUS_WRITE_PAGE_FAULT == comp_ptr->status) &&
        !state_ptr->is_page_fault_processed) {

        DIAG("Page Fault happened with completion record status equals %d, Fault Address is %p\n",
             (int)comp_ptr->status, (void*)comp_ptr->fault_address);

        // If Fault Address is available, try to resubmit the job.
        // TODO: Add logic for figuring out the size of the faulted memory to touch all the related pages.
        // TODO: On 2nd generation, we could additionally check if Fault Address is available via Fault Info.
        if (comp_ptr->fault_address != 0U) {
            char* fault_address = (char*)comp_ptr->fault_address;

            if (AD_STATUS_READ_PAGE_FAULT == comp_ptr->status) {
                volatile char* read_fault_address = fault_address;
                *read_fault_address;
            } else { // AD_STATUS_WRITE_PAGE_FAULT
                volatile char* write_fault_address = fault_address;
                *write_fault_address               = *write_fault_address;
            }

            auto status = ml::util::process_descriptor<qpl_status, ml::util::execution_mode_t::async>(
                    (hw_descriptor*)desc_ptr, (hw_completion_record*)&state_ptr->comp_ptr, qpl_job_ptr->numa_id);

            HW_IMMEDIATELY_RET(0U != status, QPL_STS_QUEUES_ARE_BUSY_ERR);

            // Set the flag to ensure we attempt only single resubmission.
            state_ptr->is_page_fault_processed = true;

            return QPL_STS_BEING_PROCESSED;
        }
    }

    if (qpl_op_compress == qpl_job_ptr->op) {
        if (job::is_canned_mode_compression(qpl_job_ptr)) {
            auto status = ml::util::convert_status_iaa_to_qpl(reinterpret_cast<const hw_completion_record*>(comp_ptr));
            // Align with the behavior of non-canned mode compression overflow (stored block also doesn't fit), which replaces
            // the returned error code "destination_is_short_error" from Intel® In-Memory Analytics Accelerator
            // with "more_output_needed"
            if (status == ml::status_list::destination_is_short_error) { status = ml::status_list::more_output_needed; }
            OWN_QPL_CHECK_STATUS(status)

            job::update_input_stream(qpl_job_ptr, qpl_job_ptr->available_in);
            job::update_output_stream(qpl_job_ptr, comp_ptr->output_size, comp_ptr->output_bits);
            job::update_checksums(qpl_job_ptr, comp_ptr->crc, comp_ptr->xor_checksum);

            return QPL_STS_OK;
        }

        return ml::hw_check_compress_job(qpl_job_ptr);
    }

    if (job::is_canned_mode_decompression(qpl_job_ptr)) {
        OWN_QPL_CHECK_STATUS(
                ml::util::convert_status_iaa_to_qpl(reinterpret_cast<const hw_completion_record*>(comp_ptr)))

        job::update_aggregates(qpl_job_ptr, comp_ptr->sum_agg, comp_ptr->min_first_agg, comp_ptr->max_last_agg);
        job::update_checksums(qpl_job_ptr, comp_ptr->crc, comp_ptr->xor_checksum);
        job::update_input_stream(qpl_job_ptr, desc_ptr->src1_size);
        job::update_output_stream(qpl_job_ptr, comp_ptr->output_size, comp_ptr->output_bits);

        return QPL_STS_OK;
    }

    if ((AD_STATUS_SUCCESS != comp_ptr->status) && (AD_STATUS_OUTPUT_OVERFLOW != comp_ptr->status)) {
        return static_cast<qpl_status>(
                ml::util::convert_status_iaa_to_qpl(reinterpret_cast<const hw_completion_record*>(comp_ptr)));
    }

    HW_IMMEDIATELY_RET((0U != comp_ptr->error_code), ml::status_list::hardware_error_base + comp_ptr->error_code);

    if ((IS_RND_ACCESS_BODY(qpl_job_ptr->flags)) && (0 != qpl_job_ptr->ignore_start_bits)) {
        hw_iaa_aecs_decompress_clean_input_accumulator(&cfg_ptr->inflate_options);
    }

    if (!(IS_RND_ACCESS_BODY(qpl_job_ptr->flags))) {
        const uint32_t wrSrc2   = (desc_ptr->op_code_op_flags >> 18U) & 3U;
        state_ptr->config_valid = ((AD_WRSRC2_ALWAYS == wrSrc2) ||
                                   ((AD_WRSRC2_MAYBE == wrSrc2) && (AD_STATUS_OUTPUT_OVERFLOW == comp_ptr->status)))
                                          ? 1U
                                          : 0U;
        FLIP_AECS_OFFSET(state_ptr);
    }

    // Update Aggregates
    if (!(qpl_op_crc64 == qpl_job_ptr->op)) {
        job::update_aggregates(qpl_job_ptr, comp_ptr->sum_agg, comp_ptr->min_first_agg, comp_ptr->max_last_agg);
        job::update_checksums(qpl_job_ptr, comp_ptr->crc, comp_ptr->xor_checksum);
    } else {
        // CRC64 Operations
        job::update_crc(qpl_job_ptr, ((uint64_t)comp_ptr->sum_agg << 32U) | (uint64_t)comp_ptr->max_last_agg);
    }

    // Update output
    const uint32_t available_out = qpl_job_ptr->available_out - desc_ptr->max_dst_size;
    const uint32_t bytes_written = comp_ptr->output_size;

    job::update_output_stream(qpl_job_ptr, bytes_written, comp_ptr->output_bits);

    // Update input stream
    uint32_t size = 0U;
    if (AD_STATUS_SUCCESS == comp_ptr->status) {
        size = desc_ptr->src1_size;
    } else if (AD_STATUS_OUTPUT_OVERFLOW == comp_ptr->status) {
        size = comp_ptr->bytes_completed;
    } else {
        size = 0U;
    }

    if (0U != state_ptr->accumulation_buffer.actual_bytes) {
        HW_IMMEDIATELY_RET((size > state_ptr->accumulation_buffer.actual_bytes), QPL_STS_LIBRARY_INTERNAL_ERR);

        state_ptr->accumulation_buffer.actual_bytes -= size;

        HW_IMMEDIATELY_RET(((0U != state_ptr->accumulation_buffer.actual_bytes) &&
                            (AD_STATUS_OUTPUT_OVERFLOW != comp_ptr->status)),
                           QPL_STS_LIBRARY_INTERNAL_ERR);

        if (0U != state_ptr->accumulation_buffer.actual_bytes) {
            core_sw::util::move(
                    state_ptr->accumulation_buffer.data + size,
                    state_ptr->accumulation_buffer.data + size + state_ptr->accumulation_buffer.actual_bytes,
                    state_ptr->accumulation_buffer.data);
        }
    } else {
        job::update_input_stream(qpl_job_ptr, size);
    }

    // Handle output overflow
    if (AD_STATUS_OUTPUT_OVERFLOW == comp_ptr->status) {
        if (0U == comp_ptr->output_size) {
            // No progress was made
            return QPL_STS_DST_IS_SHORT_ERR;
        }
        if (0U == available_out) { return QPL_STS_MORE_OUTPUT_NEEDED; }
        // The application gave us a large output buffer, but we could only use 2MB of it,
        // which filled up, so we need to use more of it in a new qpl_job_ptr.
    }

    if (0U != qpl_job_ptr->available_in) {
        // This should only happen if buffer > 2GB, or if buffering
        const qpl_status status = hw_submit_decompress_job(qpl_job_ptr, qpl_job_ptr->flags & QPL_FLAG_LAST,
                                                           qpl_job_ptr->next_in_ptr, qpl_job_ptr->available_in);

        return (QPL_STS_OK != status) ? status : QPL_STS_BEING_PROCESSED;
    }

    return QPL_STS_OK;
}
