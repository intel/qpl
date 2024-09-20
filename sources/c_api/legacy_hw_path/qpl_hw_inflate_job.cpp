/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 3/23/2020
 * @brief Internal HW API functions for @ref hw_submit_analytic_job API implementation
 *
 * @defgroup HW_SUBMIT_DECOMPRESS_API Submit Decompress Job API
 * @ingroup HW_SUBMIT_API
 * @{
 */

// c_api
#include "job.hpp"

// middle-layer
#include "accelerator/hw_accelerator_api.h"
#include "compression/dictionary/dictionary_defs.hpp"
#include "compression/dictionary/dictionary_utils.hpp"
#include "compression/huffman_table/inflate_huffman_table.hpp"
#include "compression/stream_decorators/gzip_decorator.hpp"
#include "compression_operations/huffman_table.hpp"
#include "dispatcher/hw_dispatcher.hpp"
#include "util/checkers.hpp"
#include "util/descriptor_processing.hpp"
#include "util/iaa_features_checks.hpp"

// core-sw
#include "simple_memory_ops.hpp"

// core-iaa
#include "hardware_defs.h"
#include "hw_aecs_api.h"
#include "hw_definitions.h"
#include "hw_descriptors_api.h"
#include "own_defs.h"

// legacy_hw_path
#include "hardware_state.h"
#include "own_ml_submit_operation_api.hpp"

#define DEF_STATE_HDR      1U /**< @todo // looking at block header */
#define DEF_STATE_LL_TOKEN 0U /**< @todo // looking at block header */

extern "C" qpl_status hw_submit_decompress_job(qpl_job* qpl_job_ptr, uint32_t last_job, uint8_t* next_in_ptr,
                                               uint32_t available_in) {
    using namespace qpl;
    using namespace qpl::ml::util;
    using namespace qpl::ml::compression;
    auto* const state_ptr = reinterpret_cast<qpl_hw_state*>(job::get_state(qpl_job_ptr));

    hw_decompress_analytics_descriptor* const desc_ptr = &state_ptr->desc_ptr;
    hw_iaa_aecs_analytic*                     aecs_ptr = GET_DCFG(state_ptr);

    if (IS_RND_ACCESS_HDR(qpl_job_ptr->flags)) {
        aecs_ptr = (hw_iaa_aecs_analytic*)((uint8_t*)(state_ptr->dcfg) + HW_AECS_FILTER_AND_DECOMPRESS_WA_HB);
    }

    HW_IMMEDIATELY_RET_NULLPTR(aecs_ptr);
    last_job = (available_in > MAX_BUF_SIZE) ? 0U : last_job;

    // Descriptor buffers set
    desc_ptr->src1_ptr     = next_in_ptr;
    desc_ptr->src1_size    = std::min(available_in, MAX_BUF_SIZE);
    desc_ptr->dst_ptr      = qpl_job_ptr->next_out_ptr;
    desc_ptr->max_dst_size = std::min(qpl_job_ptr->available_out, MAX_BUF_SIZE);

    uint32_t operation_flags =
            ADOF_OPCODE(QPL_OPCODE_DECOMPRESS) | ((qpl_job_ptr->flags & QPL_FLAG_CRC32C) ? ADOF_CRC32C : 0U);

    uint16_t decompression_flags = ADDF_ENABLE_DECOMP |
                                   ((qpl_job_ptr->flags & QPL_FLAG_HUFFMAN_BE) ? ADDF_DECOMP_BE : 0U) |
                                   ADDF_IGNORE_END_BITS(qpl_job_ptr->ignore_end_bits & OWN_MAX_BIT_IDX);

    // Check availability of the ignore end bits extension bit
    if (qpl::ml::util::are_iaa_gen_2_min_capabilities_present()) {
        decompression_flags |= (qpl_job_ptr->ignore_end_bits > 7U ? ADDF_IGNORE_END_BITS_EXT : 0U);
    }

    const bool is_dictionary_mode =
            (qpl_job_ptr->flags & QPL_FLAG_FIRST || !state_ptr->execution_history.first_job_has_been_submitted) &&
            qpl_job_ptr->dictionary != NULL;
    const bool is_aecs_format2_expected = qpl::ml::util::are_iaa_gen_2_min_capabilities_present();

    if (state_ptr->execution_history.first_job_has_been_submitted) {
        operation_flags |= ADOF_READ_SRC2(AD_RDSRC2_AECS);
    } else {
        state_ptr->execution_history.first_job_has_been_submitted = true;

        // Decompress with dictionary
        if (is_dictionary_mode) { operation_flags |= ADOF_READ_SRC2(AD_RDSRC2_AECS); }

        // Decompress huffman only
        if (qpl_job_ptr->flags & QPL_FLAG_NO_HDRS) {
            operation_flags |= ADOF_READ_SRC2(AD_RDSRC2_AECS);
            HW_IMMEDIATELY_RET((nullptr == qpl_job_ptr->huffman_table), QPL_STS_INVALID_PARAM_ERR);
            OWN_QPL_CHECK_STATUS(
                    check_huffman_table_is_correct<compression_algorithm_e::huffman_only>(qpl_job_ptr->huffman_table))

            auto table_impl = use_as_huffman_table<compression_algorithm_e::huffman_only>(qpl_job_ptr->huffman_table);

            // Initialize decompression table
            decompression_huffman_table decompression_table(
                    table_impl->get_sw_decompression_table_buffer(), table_impl->get_hw_decompression_table_buffer(),
                    table_impl->get_deflate_header_buffer(), table_impl->get_lookup_table_buffer_ptr());

            hw_iaa_aecs_decompress_set_huffman_only_huffman_table(&aecs_ptr->inflate_options,
                                                                  decompression_table.get_sw_decompression_table());
        }
    }

    // Setup dictionary
    if (is_dictionary_mode) {
        auto* dictionary = qpl_job_ptr->dictionary;

        // For SW dictionary, decompression dictionary size will be the same as the raw dictionary size;
        // for HW dictionary, raw dictionary will be padded with 0 at the beginning if it is smaller than
        // the allocated history size in HW. So set the decompression dictionary size to the history size.
        const size_t decompress_dictionary_size = (dictionary->hw_dict_level == hardware_dictionary_level::HW_NONE)
                                                          ? dictionary->raw_dictionary_size
                                                          : get_history_size_for_dictionary(dictionary->hw_dict_level);

        // For SW dictionary, the raw dictionary will be used directly so set offset to 0
        // for HW dictionary, set the offset to the raw dictionary offset in AECS
        const uint32_t decompress_raw_dict_offset = (dictionary->hw_dict_level == hardware_dictionary_level::HW_NONE)
                                                            ? 0U
                                                            : dictionary->aecs_raw_dictionary_offset;

        hw_iaa_aecs_decompress_set_dictionary(&aecs_ptr->inflate_options, get_dictionary_data(*dictionary),
                                              dictionary->raw_dictionary_size, decompress_dictionary_size,
                                              decompress_raw_dict_offset);

        hw_iaa_aecs_decompress_set_decompression_state(&aecs_ptr->inflate_options,
                                                       hw_iaa_aecs_decompress_state::hw_aecs_at_start_block_header);
    }

    if (0U != qpl_job_ptr->ignore_start_bits) {
        if (IS_RND_ACCESS_HDR(qpl_job_ptr->flags)) {
            operation_flags |= ADOF_READ_SRC2(AD_RDSRC2_AECS);
            core_sw::util::set_zeros(reinterpret_cast<uint8_t*>(aecs_ptr), HW_AECS_FILTER_AND_DECOMPRESS_WA_HB);
            aecs_ptr->inflate_options.idx_bit_offset   = 7U & qpl_job_ptr->ignore_start_bits;
            aecs_ptr->inflate_options.decompress_state = DEF_STATE_HDR;
        }
        auto status = hw_iaa_aecs_decompress_set_input_accumulator(
                &aecs_ptr->inflate_options, desc_ptr->src1_ptr, qpl_job_ptr->available_in,
                (uint8_t)qpl_job_ptr->ignore_start_bits, (uint8_t)qpl_job_ptr->ignore_end_bits);

        if (qpl_job_ptr->available_in == 1U) {
            // Input accumulator already accounted for ignore end bits if only 1 byte in input
            decompression_flags &= ~ADDF_IGNORE_END_BITS(qpl_job_ptr->ignore_end_bits & OWN_MAX_BIT_IDX);
        }

        HW_IMMEDIATELY_RET((status != QPL_STS_OK), QPL_STS_LIBRARY_INTERNAL_ERR);

        desc_ptr->src1_ptr  = ++qpl_job_ptr->next_in_ptr;
        desc_ptr->src1_size = --qpl_job_ptr->available_in;
    }
    hw_iaa_aecs_decompress_state_set_aecs_format(&aecs_ptr->inflate_options, is_aecs_format2_expected);

    // AECS Write policy
    if (IS_RND_ACCESS_HDR(qpl_job_ptr->flags)) { operation_flags |= ADOF_AECS_SEL; }
    if (IS_RND_ACCESS_BODY(qpl_job_ptr->flags)) {
        decompression_flags |= ADDF_FLUSH_OUTPUT;
        hw_iaa_aecs_decompress_set_crc_seed(aecs_ptr, qpl_job_ptr->crc);
    } else if (last_job) {
        operation_flags |= ADOF_WRITE_SRC2(AD_WRSRC2_MAYBE);
        decompression_flags |= ADDF_FLUSH_OUTPUT;
    } else {
        operation_flags |= ADOF_WRITE_SRC2(AD_WRSRC2_ALWAYS);
        if (qpl_job_ptr->flags & QPL_FLAG_DECOMP_FLUSH_ALWAYS) { decompression_flags |= ADDF_FLUSH_OUTPUT; }
    }

    if (state_ptr->aecs_hw_read_offset != 0U) { operation_flags |= ADOF_AECS_SEL; }

    desc_ptr->op_code_op_flags = operation_flags;
    desc_ptr->decomp_flags     = decompression_flags;

    if (!(qpl_job_ptr->flags & QPL_FLAG_NO_HDRS)) {
        hw_iaa_descriptor_set_inflate_stop_check_rule(
                (hw_descriptor*)desc_ptr,
                static_cast<hw_iaa_decompress_start_stop_rule_t>(qpl_job_ptr->decomp_end_processing),
                last_job || (qpl_job_ptr->decomp_end_processing & qpl_check_on_nonlast_block));
    }

    return process_descriptor<qpl_status, execution_mode_t::async>(
            (hw_descriptor*)desc_ptr, (hw_completion_record*)&state_ptr->comp_ptr, qpl_job_ptr->numa_id);
}

/**
 * @brief @b hw_submit_verify_job - verifies compression @ref qpl_job result. Performs inflate operation with
 *                            compressed stream and checks CRC.
 *
 * @param[in,out]  qpl_job_ptr  Pointer to the initialized @ref qpl_job structure
 *
 * @return @ref QPL_STS_OK in case of successful execution, or non-zero value, otherwise
 * Return values:
 * - @ref QPL_STS_OK
 * - @ref QPL_STS_BEING_PROCESSED - in case if Job is still being processed
 *
 * @todo add Description for other statuses
 */
extern "C" qpl_status hw_submit_verify_job(qpl_job* qpl_job_ptr) {
    using namespace qpl;
    using namespace qpl::ml::util;
    auto* const state_ptr = reinterpret_cast<qpl_hw_state*>(job::get_state(qpl_job_ptr));

    hw_decompress_analytics_descriptor* desc_ptr = &state_ptr->desc_ptr;
    hw_iaa_completion_record*           comp_ptr = &state_ptr->comp_ptr;

    hw_iaa_aecs_decompress* aecs_inflate_ptr = &state_ptr->dcfg[state_ptr->verify_aecs_hw_read_offset].inflate_options;
    hw_iaa_aecs_compress*   aecs_deflate_ptr =
            hw_iaa_aecs_compress_get_aecs_ptr(state_ptr->ccfg, state_ptr->aecs_hw_read_offset, state_ptr->aecs_size);
    if (!aecs_deflate_ptr) { return QPL_STS_LIBRARY_INTERNAL_ERR; }
    const bool is_first_job        = QPL_FLAG_FIRST & qpl_job_ptr->flags;
    const bool is_last_job         = QPL_FLAG_LAST & qpl_job_ptr->flags;
    const bool is_huffman_only     = QPL_FLAG_NO_HDRS & qpl_job_ptr->flags;
    const bool is_indexing_enabled = qpl_job_ptr->mini_block_size;

    desc_ptr->src1_ptr  = state_ptr->execution_history.saved_next_out_ptr;
    desc_ptr->src1_size = (uint32_t)(qpl_job_ptr->next_out_ptr - state_ptr->execution_history.saved_next_out_ptr);

    desc_ptr->src2_ptr  = NULL;
    desc_ptr->src2_size = 0U;

    uint32_t op_flags            = ADOF_OPCODE(QPL_OPCODE_DECOMPRESS);
    uint32_t decompression_flags = ADDF_ENABLE_DECOMP | ADDF_STOP_ON_EOB | ADDF_SEL_BFINAL_EOB | ADDF_SUPPRESS_OUTPUT |
                                   ADDF_ENABLE_IDXING(qpl_job_ptr->mini_block_size) |
                                   ((QPL_FLAG_HUFFMAN_BE & qpl_job_ptr->flags) ? ADDF_DECOMP_BE : 0U);

    const bool is_aecs_format2_expected = qpl::ml::util::are_iaa_gen_2_min_capabilities_present();

    if (is_first_job) {
        if (is_indexing_enabled) {
            if (QPL_FLAG_GZIP_MODE & qpl_job_ptr->flags) {
                *qpl_job_ptr->idx_array            = qpl::ml::compression::OWN_GZIP_HEADER_LENGTH * OWN_BYTE_BIT_LEN;
                aecs_inflate_ptr->decompress_state = DEF_STATE_HDR;
                aecs_inflate_ptr->idx_bit_offset   = (uint32_t)*qpl_job_ptr->idx_array;

                op_flags |= ADOF_READ_SRC2(AD_RDSRC2_AECS);
                desc_ptr->src2_ptr  = (uint8_t*)&state_ptr->dcfg[0];
                desc_ptr->src2_size = sizeof(hw_iaa_aecs_analytic);
            } else {
                *qpl_job_ptr->idx_array = 0U;
            }
            qpl_job_ptr->idx_num_written = 1U;
        }

        if (is_huffman_only) {
            if (hw_iaa_aecs_decompress_set_huffman_only_huffman_table_from_histogram(
                        aecs_inflate_ptr, &aecs_deflate_ptr->histogram, is_aecs_format2_expected)) {
                return QPL_STS_INVALID_HUFFMAN_TABLE_ERR;
            }

            op_flags |= ADOF_READ_SRC2(AD_RDSRC2_AECS);
            desc_ptr->src2_ptr  = (uint8_t*)&state_ptr->dcfg[0];
            desc_ptr->src2_size = sizeof(hw_iaa_aecs_analytic);
        }
    } else {
        op_flags |= ADOF_READ_SRC2(AD_RDSRC2_AECS);
        desc_ptr->src2_ptr  = (uint8_t*)&state_ptr->dcfg[0];
        desc_ptr->src2_size = sizeof(hw_iaa_aecs_analytic);
    }

    if (is_last_job) {
        decompression_flags |= (!is_huffman_only) ? ADDF_CHECK_FOR_EOB | ADDF_STOP_ON_EOB | ADDF_SEL_BFINAL_EOB : 0U;
        decompression_flags |= ADDF_FLUSH_OUTPUT;
    } else {
        op_flags |= ADOF_WRITE_SRC2(AD_WRSRC2_ALWAYS);
        desc_ptr->src2_ptr  = (uint8_t*)&state_ptr->dcfg[0];
        desc_ptr->src2_size = sizeof(hw_iaa_aecs_analytic);
    }

    if (is_indexing_enabled) {
        HW_IMMEDIATELY_RET((qpl_job_ptr->idx_max_size < qpl_job_ptr->idx_num_written), QPL_STS_INDEX_ARRAY_TOO_SMALL)
        desc_ptr->dst_ptr      = (uint8_t*)(qpl_job_ptr->idx_array + qpl_job_ptr->idx_num_written);
        desc_ptr->max_dst_size = (qpl_job_ptr->idx_max_size - qpl_job_ptr->idx_num_written) * sizeof(uint64_t);
    } else {
        desc_ptr->dst_ptr = qpl_job_ptr->next_out_ptr; // should not be used
        desc_ptr->max_dst_size =
                (0U == desc_ptr->src1_size) ? 1U : 0U; // Hardware does not support src1_size and dstSize both 0
    }

    // Not needed, but make sure crc is being written:
    comp_ptr->crc    = 0U;
    comp_ptr->status = 0U;

    // Set operation flags
    op_flags |= (QPL_FLAG_CRC32C & qpl_job_ptr->flags) ? ADOF_CRC32C : 0U;
    op_flags |= (state_ptr->verify_aecs_hw_read_offset) ? ADOF_AECS_SEL : 0U;

    desc_ptr->op_code_op_flags = op_flags;
    desc_ptr->decomp_flags     = decompression_flags;
    desc_ptr->filter_flags     = 0U;

    // Fix for QPL_FLAG_HUFFMAN_BE in Intel® In-Memory Analytics Accelerator (Intel® IAA) generation 1.0.
    // This affects the verify operation when NO_HDRS is specified. The issue is that there is no EOB token,
    // so in some cases the 0 padding bits added to the end of the stream (to reach a byte boundary) may
    // decode into a valid output byte, causing the verify operation to fail.
    // The fix is that when the library is creating the verify job and NO_HDRS is specified, and it is a LAST job,
    // then the descriptor flags field for IGNORE_END_BITs should be set to (8u - output_bits),
    // or (16u - output_bits) for Big Endian 16
    // where output_bits is the output_bits field from the compress jobs completion record.
    if (((qpl_job_ptr->flags & (QPL_FLAG_LAST | QPL_FLAG_NO_HDRS)) == (QPL_FLAG_LAST | QPL_FLAG_NO_HDRS))) {
        if (qpl_job_ptr->flags & QPL_FLAG_HUFFMAN_BE) {
            const uint8_t ignore_end_bits = 16U - comp_ptr->output_bits;
            desc_ptr->decomp_flags |= ADDF_IGNORE_END_BITS(ignore_end_bits & OWN_MAX_BIT_IDX);

            // Check availability of the ignore end bits extension bit
            if (qpl::ml::util::are_iaa_gen_2_min_capabilities_present()) {
                desc_ptr->decomp_flags |= (ignore_end_bits > 7U ? ADDF_IGNORE_END_BITS_EXT : 0U);
            }
        } else {
            desc_ptr->decomp_flags |= ADDF_IGNORE_END_BITS(8U - comp_ptr->output_bits);
        }
    }

    hw_iaa_aecs_decompress_state_set_aecs_format(aecs_inflate_ptr, is_aecs_format2_expected);

    auto status = process_descriptor<qpl_status, execution_mode_t::async>(
            (hw_descriptor*)desc_ptr, (hw_completion_record*)&state_ptr->comp_ptr, qpl_job_ptr->numa_id);

    HW_IMMEDIATELY_RET(status, QPL_STS_QUEUES_ARE_BUSY_ERR)

    return QPL_STS_BEING_PROCESSED;
}
