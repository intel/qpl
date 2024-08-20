/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 3/23/2020
 * @brief Internal HW API functions for @ref hw_descriptor_compress_init_deflate_base API implementation
 */

#include "common/bit_reverse.hpp"
#include "compression/dictionary/dictionary_utils.hpp"
#include "compression/stream_decorators/gzip_decorator.hpp"
#include "compression/stream_decorators/zlib_decorator.hpp"
#include "compression_operations/huffman_table.hpp"
#include "dispatcher/hw_dispatcher.hpp"
#include "hardware_defs.h"
#include "hardware_state.h"
#include "hw_aecs_api.h"
#include "hw_definitions.h"
#include "hw_descriptors_api.h"
#include "own_checkers.h"
#include "own_ml_submit_operation_api.hpp"
#include "util/hw_status_converting.hpp"
#include "util/iaa_features_checks.hpp"

static inline qpl_comp_style own_get_compression_style(const qpl_job* const job_ptr) {
    if (job_ptr->flags & QPL_FLAG_DYNAMIC_HUFFMAN) {
        return qpl_cst_dynamic;
    } else if (job_ptr->huffman_table) {
        return qpl_cst_static;
    } else {
        return qpl_cst_fixed;
    }
}

extern "C" qpl_status hw_descriptor_compress_init_deflate_base(qpl_job*                                  qpl_job_ptr,
                                                               hw_decompress_analytics_descriptor* const descriptor_ptr,
                                                               hw_completion_record* const completion_record_ptr,
                                                               qpl_hw_state* const         state_ptr) {
    using namespace qpl::ml::compression;

    auto                  huffman_table_ptr = qpl_job_ptr->huffman_table;
    qpl_dictionary*       dictionary        = qpl_job_ptr->dictionary;
    hw_iaa_aecs_compress* configuration_ptr = nullptr;
    const uint32_t        flags             = qpl_job_ptr->flags;
    const qpl_comp_style  compression_style = own_get_compression_style(qpl_job_ptr);
    uint8_t*              next_out_ptr      = qpl_job_ptr->next_out_ptr;
    const uint32_t        available_out     = qpl_job_ptr->available_out;

    // Check if header generation and dictionary compression are supported in hardware
    bool is_hw_header_gen_supported    = false;
    bool is_hw_dict_compress_supported = false;

#if defined(__linux__)
    static auto& dispatcher = qpl::ml::dispatcher::hw_dispatcher::get_instance();
    if (!dispatcher.is_hw_support()) {
        const hw_accelerator_status hw_status = dispatcher.get_hw_init_status();
        return qpl::ml::util::convert_hw_accelerator_status_to_qpl_status(hw_status);
    }

    const auto& device            = dispatcher.device(0);
    is_hw_header_gen_supported    = device.get_header_gen_support();
    is_hw_dict_compress_supported = device.get_dict_compress_support();
#endif //__linux__

    // If dictionary is provided, check that the following are true:
    // 1. compression with dictionary is supported
    // 2. this is a single chunk job (multi-chunk dictionary is not supported on HW path)
    // 3. the hardware_dictionary_level is set
    if (dictionary) {
        const bool is_single_chunk =
                ((qpl_job_ptr->flags & (QPL_FLAG_FIRST | QPL_FLAG_LAST)) == (QPL_FLAG_FIRST | QPL_FLAG_LAST));
        if (!is_hw_dict_compress_supported || !is_single_chunk ||
            (hardware_dictionary_level::HW_NONE == dictionary->hw_dict_level)) {
            return QPL_STS_NOT_SUPPORTED_MODE_ERR;
        }

        const uint32_t dict_size_in_aecs = get_dictionary_size_in_aecs(*dictionary);
        state_ptr->aecs_size             = HW_AECS_COMPRESS_WITH_HT + dict_size_in_aecs;

    } else {
        state_ptr->aecs_size = HW_AECS_COMPRESS_WITH_HT;
    }

    if (flags & QPL_FLAG_FIRST) {
        uint32_t content_header_size                = 0U;
        state_ptr->execution_history.execution_step = (flags & QPL_FLAG_NO_HDRS)
                                                              ? qpl_task_execution_step_data_processing
                                                              : qpl_task_execution_step_header_inserting;
        state_ptr->execution_history.comp_style     = compression_style;
        state_ptr->aecs_hw_read_offset              = 0U;
        state_ptr->verify_aecs_hw_read_offset       = 0U;

        configuration_ptr                        = &state_ptr->ccfg[0];
        configuration_ptr->num_output_accum_bits = 0U;
        configuration_ptr->crc                   = 0U;
        configuration_ptr->xor_checksum          = 0U;

        if (flags & QPL_FLAG_GZIP_MODE) {
            if (available_out < qpl::ml::compression::OWN_GZIP_HEADER_LENGTH) return QPL_STS_DST_IS_SHORT_ERR;

            qpl::ml::compression::gzip_decorator::write_header_unsafe(next_out_ptr, available_out);

            content_header_size += qpl::ml::compression::OWN_GZIP_HEADER_LENGTH;
        }

        if (flags & QPL_FLAG_ZLIB_MODE) {
            if (available_out < qpl::ml::compression::zlib_sizes::zlib_header_size) return QPL_STS_DST_IS_SHORT_ERR;

            qpl::ml::compression::zlib_decorator::write_header_unsafe(next_out_ptr);

            content_header_size += qpl::ml::compression::zlib_sizes::zlib_header_size;
        }

        qpl_job_ptr->total_in = 0U;
        qpl_job_ptr->next_out_ptr += content_header_size;
        qpl_job_ptr->available_out -= content_header_size;
        qpl_job_ptr->total_out = content_header_size;

    } else {
        // Not first qpl_job_ptr
        HW_IMMEDIATELY_RET((state_ptr->execution_history.execution_step == qpl_task_execution_step_completed),
                           QPL_STS_JOB_NOT_CONTINUABLE_ERR);
        HW_IMMEDIATELY_RET((state_ptr->execution_history.comp_style != compression_style),
                           QPL_STS_INVALID_COMPRESS_STYLE_ERR);

        configuration_ptr = hw_iaa_aecs_compress_get_aecs_ptr(state_ptr->ccfg, state_ptr->aecs_hw_read_offset,
                                                              state_ptr->aecs_size);
        if (!configuration_ptr) { return QPL_STS_LIBRARY_INTERNAL_ERR; }
    }
    // End if first
    state_ptr->execution_history.saved_next_out_ptr = qpl_job_ptr->next_out_ptr;

    // If statistic collection required
    if (flags & QPL_FLAG_DYNAMIC_HUFFMAN) {
        HW_IMMEDIATELY_RET(!(state_ptr->execution_history.execution_step == qpl_task_execution_step_header_inserting) &&
                                   !(flags & QPL_FLAG_NO_HDRS),
                           QPL_STS_LIBRARY_INTERNAL_ERR);

        // TODO: enable Huffman only header gen
        is_hw_header_gen_supported = (flags & QPL_FLAG_NO_HDRS) ? 0U : is_hw_header_gen_supported;

        bool is_hw_1_pass_header_gen = false;
        bool is_hw_2_pass_header_gen = false;
        if (is_hw_header_gen_supported) {
            if (qpl_job_ptr->available_in <= 4096U && !dictionary) {
                is_hw_1_pass_header_gen = true;
            } else {
                is_hw_2_pass_header_gen = true;
            }
        }

        if (is_hw_1_pass_header_gen) {
            // For 1-pass header gen, Huffman Table generation and compression will be done in the same pass
            hw_iaa_descriptor_init_deflate_body((hw_descriptor*)descriptor_ptr, qpl_job_ptr->next_in_ptr,
                                                qpl_job_ptr->available_in, qpl_job_ptr->next_out_ptr,
                                                qpl_job_ptr->available_out);

            hw_iaa_descriptor_set_1_pass_header_gen((hw_descriptor*)descriptor_ptr, (hw_iaa_aecs*)state_ptr->ccfg,
                                                    state_ptr->aecs_hw_read_offset, flags & QPL_FLAG_LAST,
                                                    flags & QPL_FLAG_FIRST);

            hw_iaa_descriptor_compress_set_termination_rule((hw_descriptor*)descriptor_ptr,
                                                            hw_iaa_terminator_t::end_of_block);

        } else if (is_hw_2_pass_header_gen) {
            // 2-pass header generation, the first pass will calculate Huffman Table
            hw_iaa_descriptor_init_statistic_collector_with_header_gen(
                    (hw_descriptor*)descriptor_ptr, qpl_job_ptr->next_in_ptr, qpl_job_ptr->available_in,
                    (hw_iaa_aecs*)state_ptr->ccfg, state_ptr->aecs_hw_read_offset, flags & QPL_FLAG_LAST,
                    flags & QPL_FLAG_FIRST);

            // Append EOB so that EOB will have a Huffman code
            hw_iaa_descriptor_compress_set_termination_rule((hw_descriptor*)descriptor_ptr,
                                                            hw_iaa_terminator_t::end_of_block);

            // Setup dictionary in statistics collection descriptor
            if (dictionary) {
                const uint32_t       dict_size_in_aecs = qpl::ml::compression::get_dictionary_size_in_aecs(*dictionary);
                const uint8_t* const dictionary_data_ptr = qpl::ml::compression::get_dictionary_data(*dictionary);
                const uint8_t        load_dictionary_val = qpl::ml::compression::get_load_dictionary_flag(*dictionary);

                hw_iaa_descriptor_compress_setup_dictionary((hw_descriptor*)descriptor_ptr, dict_size_in_aecs,
                                                            dictionary_data_ptr, dictionary->aecs_raw_dictionary_offset,
                                                            state_ptr->ccfg, state_ptr->aecs_hw_read_offset,
                                                            state_ptr->aecs_size, load_dictionary_val);
            }

            // Invert AECS toggle for compress because src2 will be written as AECS
            state_ptr->aecs_hw_read_offset ^= 1U;

        } else {
            // Dynamic deflate, the first pass will calculate the statistics
            hw_iaa_descriptor_init_statistic_collector((hw_descriptor*)descriptor_ptr, qpl_job_ptr->next_in_ptr,
                                                       qpl_job_ptr->available_in, &configuration_ptr->histogram);
            if (flags & QPL_FLAG_GEN_LITERALS) {
                hw_iaa_descriptor_compress_set_huffman_only_mode((hw_descriptor*)descriptor_ptr);
            }

            // Setup dictionary in statistics collection descriptor
            if (dictionary) {
                const uint32_t       dict_size_in_aecs = qpl::ml::compression::get_dictionary_size_in_aecs(*dictionary);
                const uint8_t* const dictionary_data_ptr = qpl::ml::compression::get_dictionary_data(*dictionary);
                const uint8_t        load_dictionary_val = qpl::ml::compression::get_load_dictionary_flag(*dictionary);

                hw_iaa_descriptor_compress_setup_dictionary((hw_descriptor*)descriptor_ptr, dict_size_in_aecs,
                                                            dictionary_data_ptr, dictionary->aecs_raw_dictionary_offset,
                                                            state_ptr->ccfg, state_ptr->aecs_hw_read_offset,
                                                            state_ptr->aecs_size, load_dictionary_val);
            }
        }
        hw_iaa_descriptor_compress_set_mini_block_size((hw_descriptor*)descriptor_ptr,
                                                       (hw_iaa_mini_block_size_t)qpl_job_ptr->mini_block_size);
        hw_iaa_descriptor_set_completion_record((hw_descriptor*)descriptor_ptr, completion_record_ptr);
        completion_record_ptr->status = 0U;

        return QPL_STS_OK;
    } else {
        const bool is_final_block  = (flags & QPL_FLAG_LAST) ? 1U : 0U;
        const bool is_huffman_only = (flags & QPL_FLAG_NO_HDRS) != 0;

        if (flags & QPL_FLAG_START_NEW_BLOCK) {
            if (state_ptr->execution_history.execution_step != qpl_task_execution_step_header_inserting) {
                HW_IMMEDIATELY_RET(
                        (state_ptr->execution_history.execution_step != qpl_task_execution_step_data_processing),
                        QPL_STS_LIBRARY_INTERNAL_ERR)
                hw_iaa_aecs_compress_accumulator_insert_eob(configuration_ptr, state_ptr->eob_code);
                state_ptr->execution_history.execution_step = qpl_task_execution_step_header_inserting;
            }
        }

        // Prepare huffman table begin
        if (state_ptr->execution_history.execution_step == qpl_task_execution_step_header_inserting) {
            HW_IMMEDIATELY_RET(
                    ((qpl_job_ptr->mini_block_size) && (0U == (flags & (QPL_FLAG_FIRST | QPL_FLAG_START_NEW_BLOCK)))),
                    QPL_STS_INDEX_GENERATION_ERR);
            state_ptr->saved_num_output_accum_bits = configuration_ptr->num_output_accum_bits;

            auto table_impl = use_as_huffman_table<compression_algorithm_e::deflate>(qpl_job_ptr->huffman_table);
            // insert header
            if (huffman_table_ptr) {
                // Static mode used
                const uint32_t status = hw_iaa_aecs_compress_write_deflate_dynamic_header(
                        configuration_ptr, table_impl->get_deflate_header_ptr(),
                        table_impl->get_deflate_header_bits_size(), is_final_block);
                HW_IMMEDIATELY_RET((status != QPL_STS_OK), QPL_STS_LIBRARY_INTERNAL_ERR);

                const uint32_t code_length  = table_impl->get_literals_lengths_table_ptr()[256];
                const uint32_t eob_code_len = code_length >> 15U;
                state_ptr->eob_code.code    = reverse_bits(static_cast<uint16_t>(code_length)) >> (16U - eob_code_len);
                state_ptr->eob_code.length  = eob_code_len;
            } else {
                // Fixed mode used
                const uint32_t status =
                        hw_iaa_aecs_compress_write_deflate_fixed_header(configuration_ptr, is_final_block);

                HW_IMMEDIATELY_RET((status != QPL_STS_OK), QPL_STS_LIBRARY_INTERNAL_ERR);

                state_ptr->eob_code.code   = 0U;
                state_ptr->eob_code.length = 7U;
            }
        }

        if (!is_huffman_only) {
            if (huffman_table_ptr) {
                auto table_impl = use_as_huffman_table<compression_algorithm_e::deflate>(qpl_job_ptr->huffman_table);

                hw_iaa_aecs_compress_set_deflate_huffman_table(configuration_ptr,
                                                               table_impl->get_literals_lengths_table_ptr(),
                                                               table_impl->get_offsets_table_ptr());
            } else {
                hw_iaa_aecs_compress_set_deflate_huffman_table(configuration_ptr,
                                                               (hw_iaa_huffman_codes*)fixed_literals_table,
                                                               (hw_iaa_huffman_codes*)fixed_offsets_table);
            }
        }

        if (is_huffman_only) {
            if (huffman_table_ptr) {
                auto table_impl =
                        use_as_huffman_table<compression_algorithm_e::huffman_only>(qpl_job_ptr->huffman_table);

                hw_iaa_aecs_compress_set_huffman_only_huffman_table(configuration_ptr,
                                                                    table_impl->get_literals_lengths_table_ptr());
            } else {
                hw_iaa_aecs_compress_set_huffman_only_huffman_table(configuration_ptr,
                                                                    (hw_iaa_huffman_codes*)fixed_literals_table);
            }
        }
        // Prepare huffman table begin-end

        // Skip Gzip footer
        uint32_t max_output_size = qpl_job_ptr->available_out;

        if ((QPL_FLAG_GZIP_MODE | QPL_FLAG_LAST) == ((QPL_FLAG_GZIP_MODE | QPL_FLAG_LAST) & qpl_job_ptr->flags)) {
            max_output_size = (max_output_size > 8U) ? max_output_size - 8U : max_output_size;
        }

        // Prepare Compress task
        hw_iaa_descriptor_init_deflate_body((hw_descriptor*)descriptor_ptr, qpl_job_ptr->next_in_ptr,
                                            qpl_job_ptr->available_in, qpl_job_ptr->next_out_ptr, max_output_size);

        if (flags & QPL_FLAG_HUFFMAN_BE) {
            hw_iaa_descriptor_compress_set_be_output_mode((hw_descriptor*)descriptor_ptr);
        }

        if (flags & QPL_FLAG_GEN_LITERALS) {
            hw_iaa_descriptor_compress_set_huffman_only_mode((hw_descriptor*)descriptor_ptr);
        }

        if (flags & QPL_FLAG_CRC32C) { hw_iaa_descriptor_set_crc_rfc3720((hw_descriptor*)descriptor_ptr); }

        hw_iaa_descriptor_compress_set_mini_block_size((hw_descriptor*)descriptor_ptr,
                                                       (hw_iaa_mini_block_size_t)qpl_job_ptr->mini_block_size);

        auto access_policy = is_final_block
                                     ? hw_aecs_access_read | state_ptr->aecs_hw_read_offset
                                     : hw_aecs_access_read | hw_aecs_access_write | state_ptr->aecs_hw_read_offset;

        hw_iaa_descriptor_compress_set_aecs((hw_descriptor*)descriptor_ptr, state_ptr->ccfg,
                                            static_cast<hw_iaa_aecs_access_policy>(access_policy),
                                            !qpl::ml::util::are_iaa_gen_2_min_capabilities_present());

        if (is_final_block && !is_huffman_only) {
            descriptor_ptr->decomp_flags |=
                    (qpl_task_execution_step_header_inserting == state_ptr->execution_history.execution_step)
                            ? (ADCF_FLUSH_OUTPUT | ADCF_END_PROC(AD_APPEND_EOB))
                            : (ADCF_FLUSH_OUTPUT | ADCF_END_PROC(AD_APPEND_EOB_FINAL_SB));
        }

        // Setup dictionary in compression descriptor
        if (dictionary) {
            const uint32_t       dict_size_in_aecs   = qpl::ml::compression::get_dictionary_size_in_aecs(*dictionary);
            const uint8_t* const dictionary_data_ptr = qpl::ml::compression::get_dictionary_data(*dictionary);
            const uint8_t        load_dictionary_val = qpl::ml::compression::get_load_dictionary_flag(*dictionary);

            hw_iaa_descriptor_compress_setup_dictionary((hw_descriptor*)descriptor_ptr, dict_size_in_aecs,
                                                        dictionary_data_ptr, dictionary->aecs_raw_dictionary_offset,
                                                        state_ptr->ccfg, state_ptr->aecs_hw_read_offset,
                                                        state_ptr->aecs_size, load_dictionary_val);
        }
        hw_iaa_descriptor_set_completion_record((hw_descriptor*)descriptor_ptr, completion_record_ptr);
        completion_record_ptr->status = 0U;

        return QPL_STS_OK;
    }
}

extern "C" qpl_status hw_descriptor_compress_init_deflate_dynamic(hw_decompress_analytics_descriptor* desc_ptr,
                                                                  qpl_hw_state* state_ptr, qpl_job* qpl_job_ptr,
                                                                  hw_iaa_aecs_compress*     cfg_in_ptr,
                                                                  hw_iaa_completion_record* comp_ptr) {
    using namespace qpl::ml::compression;
    const uint32_t flags           = qpl_job_ptr->flags;
    const bool     is_huffman_only = (flags & QPL_FLAG_NO_HDRS) != 0;
    const bool     is_final_block  = (flags & QPL_FLAG_LAST) ? 1U : 0U;

    // Check if header generation is supported in hardware
    bool is_hw_header_gen_supported = false;

#if defined(__linux__)
    static auto& dispatcher = qpl::ml::dispatcher::hw_dispatcher::get_instance();
    if (!dispatcher.is_hw_support()) {
        const hw_accelerator_status hw_status = dispatcher.get_hw_init_status();
        return qpl::ml::util::convert_hw_accelerator_status_to_qpl_status(hw_status);
    }

    const auto& device         = dispatcher.device(0);
    is_hw_header_gen_supported = device.get_header_gen_support();
#endif //__linux__

    // TODO: enable Huffman only header gen
    is_hw_header_gen_supported = (flags & QPL_FLAG_NO_HDRS) ? 0U : is_hw_header_gen_supported;

    state_ptr->saved_num_output_accum_bits = hw_iaa_aecs_compress_accumulator_get_actual_bits(cfg_in_ptr);

    if (is_huffman_only) {
        hw_iaa_aecs_compress_set_huffman_only_huffman_table_from_histogram(cfg_in_ptr, &cfg_in_ptr->histogram);

        auto table_impl = use_as_huffman_table<compression_algorithm_e::huffman_only>(qpl_job_ptr->huffman_table);

        const compression_huffman_table compression_table(table_impl->get_sw_compression_huffman_table_ptr(),
                                                          table_impl->get_isal_compression_huffman_table_ptr(),
                                                          table_impl->get_hw_compression_huffman_table_ptr(),
                                                          table_impl->get_deflate_header_ptr());

        hw_iaa_aecs_compress_store_huffman_only_huffman_table(cfg_in_ptr, compression_table.get_sw_compression_table());

        table_impl->set_deflate_header_bits_size(0U);
    } else if (!is_hw_header_gen_supported) {
        hw_iaa_aecs_compress_write_deflate_dynamic_header_from_histogram(cfg_in_ptr, &cfg_in_ptr->histogram,
                                                                         is_final_block);
    }

    uint32_t max_output_size = qpl_job_ptr->available_out;

    if ((QPL_FLAG_GZIP_MODE | QPL_FLAG_LAST) == ((QPL_FLAG_GZIP_MODE | QPL_FLAG_LAST) & qpl_job_ptr->flags)) {
        max_output_size = (max_output_size > 8U) ? max_output_size - 8U : max_output_size;
    }

    hw_iaa_descriptor_init_deflate_body((hw_descriptor*)desc_ptr, qpl_job_ptr->next_in_ptr, qpl_job_ptr->available_in,
                                        qpl_job_ptr->next_out_ptr, max_output_size);

    if (flags & QPL_FLAG_HUFFMAN_BE) { hw_iaa_descriptor_compress_set_be_output_mode((hw_descriptor*)desc_ptr); }

    if (flags & QPL_FLAG_GEN_LITERALS) { hw_iaa_descriptor_compress_set_huffman_only_mode((hw_descriptor*)desc_ptr); }

    if (flags & QPL_FLAG_CRC32C) { hw_iaa_descriptor_set_crc_rfc3720((hw_descriptor*)desc_ptr); }

    hw_iaa_descriptor_compress_set_mini_block_size((hw_descriptor*)desc_ptr,
                                                   (hw_iaa_mini_block_size_t)qpl_job_ptr->mini_block_size);

    auto access_policy = is_final_block ? hw_aecs_access_read | state_ptr->aecs_hw_read_offset
                                        : hw_aecs_access_read | hw_aecs_access_write | state_ptr->aecs_hw_read_offset;

    // For 2-pass header generation and dynamic deflate, this is the second pass for actual compression
    hw_iaa_descriptor_compress_set_aecs((hw_descriptor*)desc_ptr, state_ptr->ccfg,
                                        static_cast<hw_iaa_aecs_access_policy>(access_policy),
                                        !qpl::ml::util::are_iaa_gen_2_min_capabilities_present());

    hw_iaa_descriptor_set_completion_record((hw_descriptor*)desc_ptr, (hw_completion_record*)comp_ptr);
    comp_ptr->status = 0U;

    if (!is_huffman_only) { desc_ptr->decomp_flags |= ADCF_END_PROC(AD_APPEND_EOB); }

    return QPL_STS_OK;
}

extern "C" qpl_status hw_descriptor_compress_init_deflate_canned(qpl_job* const job_ptr) {
    using namespace qpl::ml::compression;

    qpl_hw_state* const                       state_ptr      = (qpl_hw_state*)job_ptr->data_ptr.hw_state_ptr;
    hw_decompress_analytics_descriptor* const descriptor_ptr = &state_ptr->desc_ptr;
    const uint32_t                            flags          = job_ptr->flags;
    const bool                                is_final_block = (flags & QPL_FLAG_LAST) != 0;
    const bool                                is_first_block = (flags & QPL_FLAG_FIRST) != 0;
    qpl_dictionary*                           dictionary     = job_ptr->dictionary;

    // Check if dictionary compression is supported in hardware
    bool is_hw_dict_compress_supported = false;

#if defined(__linux__)
    static auto& dispatcher = qpl::ml::dispatcher::hw_dispatcher::get_instance();
    if (!dispatcher.is_hw_support()) {
        const hw_accelerator_status hw_status = dispatcher.get_hw_init_status();
        return qpl::ml::util::convert_hw_accelerator_status_to_qpl_status(hw_status);
    }

    const auto& device            = dispatcher.device(0);
    is_hw_dict_compress_supported = device.get_dict_compress_support();
#endif //__linux__

    // If dictionary is provided, check that the following are true:
    // 1. compression with dictionary is supported
    // 2. this is a single chunk job (multi-chunk dictionary is not supported on HW path).
    // 3. the hardware_dictionary_level is set
    if (dictionary) {
        if (!is_hw_dict_compress_supported || !(is_final_block && is_first_block) ||
            (hardware_dictionary_level::HW_NONE == dictionary->hw_dict_level)) {
            return QPL_STS_NOT_SUPPORTED_MODE_ERR;
        }

        const uint32_t dict_size_in_aecs = get_dictionary_size_in_aecs(*dictionary);
        state_ptr->aecs_size             = HW_AECS_COMPRESS_WITH_HT + dict_size_in_aecs;

    } else {
        state_ptr->aecs_size = HW_AECS_COMPRESS_WITH_HT;
    }

    hw_iaa_descriptor_init_deflate_body((hw_descriptor*)descriptor_ptr, job_ptr->next_in_ptr, job_ptr->available_in,
                                        job_ptr->next_out_ptr, job_ptr->available_out);

    if (flags & QPL_FLAG_HUFFMAN_BE) { hw_iaa_descriptor_compress_set_be_output_mode((hw_descriptor*)descriptor_ptr); }

    if (flags & QPL_FLAG_CRC32C) { hw_iaa_descriptor_set_crc_rfc3720((hw_descriptor*)descriptor_ptr); }

    if (is_first_block) {
        auto table_impl = use_as_huffman_table<compression_algorithm_e::deflate>(job_ptr->huffman_table);

        hw_iaa_aecs_compress_set_deflate_huffman_table(state_ptr->ccfg, table_impl->get_literals_lengths_table_ptr(),
                                                       table_impl->get_offsets_table_ptr());
    }

    auto access_policy = is_final_block ? hw_aecs_access_read : hw_aecs_access_read | hw_aecs_access_write;
    hw_iaa_descriptor_compress_set_aecs((hw_descriptor*)descriptor_ptr, state_ptr->ccfg,
                                        static_cast<hw_iaa_aecs_access_policy>(access_policy),
                                        !qpl::ml::util::are_iaa_gen_2_min_capabilities_present());

    // Setup dictionary in compression descriptor
    if (dictionary) {
        const uint32_t       dict_size_in_aecs   = qpl::ml::compression::get_dictionary_size_in_aecs(*dictionary);
        const uint8_t* const dictionary_data_ptr = qpl::ml::compression::get_dictionary_data(*dictionary);
        const uint8_t        load_dictionary_val = qpl::ml::compression::get_load_dictionary_flag(*dictionary);

        hw_iaa_descriptor_compress_setup_dictionary((hw_descriptor*)descriptor_ptr, dict_size_in_aecs,
                                                    dictionary_data_ptr, dictionary->aecs_raw_dictionary_offset,
                                                    state_ptr->ccfg, state_ptr->aecs_hw_read_offset,
                                                    state_ptr->aecs_size, load_dictionary_val);
    }

    descriptor_ptr->decomp_flags |= ADCF_END_PROC(AD_APPEND_EOB);

    hw_iaa_descriptor_set_completion_record((hw_descriptor*)descriptor_ptr,
                                            (hw_completion_record*)&state_ptr->comp_ptr);

    return QPL_STS_OK;
}
