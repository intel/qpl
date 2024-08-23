/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "deflate.hpp"

#include "compression/deflate/compression_units/auxiliary_units.hpp"
#include "compression/deflate/implementations/deflate_implementation.hpp"
#include "compression/deflate/streams/hw_deflate_state.hpp"
#include "compression/verification/verification_state_builder.hpp"
#include "compression/verification/verify.hpp"
#include "dispatcher/hw_dispatcher.hpp"
#include "util/descriptor_processing.hpp"
#include "util/hw_status_converting.hpp"
#include "util/iaa_features_checks.hpp"

// core-iaa
#include "hw_aecs_api.h" // HW_AECS_COMPRESS_WITH_HT

namespace qpl::ml::compression {

auto deflate_pass(deflate_state<execution_path_t::software>& stream, uint8_t* begin, uint32_t size) noexcept
        -> compression_operation_result_t {
    compression_operation_result_t result;

    result.status_code_ = status_list::ok;

    stream.set_source(begin, size);

    compression_state_t state = compression_state_t::init_compression;

    auto implementation = build_implementation<block_type_t::deflate_block>(
            stream.compression_level(), stream.compression_mode(), stream.mini_blocks_support(),
            stream.dictionary_support());

    // Main pipeline
    do { //NOLINT(cppcoreguidelines-avoid-do-while)
        result.status_code_ = implementation.execute(stream, state);
    } while (!result.status_code_ && state != compression_state_t::finish_compression_process);

    if (!result.status_code_ && stream.mini_blocks_support() == mini_blocks_support_t::disabled) {
        update_checksum(stream);
    }

    return result;
}

template <>
auto deflate<execution_path_t::hardware, deflate_mode_t::deflate_no_headers>(
        deflate_state<execution_path_t::hardware>& state, uint8_t* begin, const uint32_t size) noexcept
        -> compression_operation_result_t {
    //    auto output_begin_ptr = state.next_out();

    compression_operation_result_t result;

    // Check if dictionary is supported in hardware
    bool is_hw_dict_compress_supported = false;
#if defined(__linux__)
    static auto& dispatcher = qpl::ml::dispatcher::hw_dispatcher::get_instance();
    if (!dispatcher.is_hw_support()) {
        const hw_accelerator_status hw_status = dispatcher.get_hw_init_status();
        result.status_code_                   = qpl::ml::util::convert_hw_accelerator_status_to_qpl_status(hw_status);
        return result;
    }

    const auto& device            = dispatcher.device(0);
    is_hw_dict_compress_supported = device.get_dict_compress_support();
#endif //__linux__

    // If dictionary is provided, check that the following are true:
    // 1. compression with dictionary is supported
    // 2. this is a single chunk job (multi-chunk dictionary is not supported on HW path).
    // 3. the hardware_dictionary_level is set
    qpl_dictionary* dictionary = state.dictionary_;
    if (dictionary) {
        const bool is_single_chunk = (state.is_first_chunk() && state.is_last_chunk());
        if (!is_hw_dict_compress_supported || !is_single_chunk ||
            (hardware_dictionary_level::HW_NONE == dictionary->hw_dict_level)) {
            result.status_code_ = status_list::not_supported_err;
            return result;
        }

        const uint32_t dict_size_in_aecs = get_dictionary_size_in_aecs(*dictionary);
        state.meta_data_->aecs_size      = HW_AECS_COMPRESS_WITH_HT + dict_size_in_aecs;

    } else {
        state.meta_data_->aecs_size = HW_AECS_COMPRESS_WITH_HT;
    }

    hw_iaa_descriptor_compress_set_termination_rule(state.compress_descriptor_, end_of_block);
    hw_iaa_descriptor_set_input_buffer(state.compress_descriptor_, begin, size);
    hw_iaa_descriptor_compress_set_aecs(state.compress_descriptor_, state.meta_data_->aecs_, hw_aecs_access_read,
                                        !qpl::ml::util::are_iaa_gen_2_min_capabilities_present());

    // Setup dictionary in compression descriptor
    if (dictionary) {
        const uint32_t       dict_size_in_aecs   = get_dictionary_size_in_aecs(*dictionary);
        const uint8_t* const dictionary_data_ptr = get_dictionary_data(*dictionary);
        const uint8_t        load_dictionary_val = get_load_dictionary_flag(*dictionary);

        hw_iaa_descriptor_compress_setup_dictionary(state.compress_descriptor_, dict_size_in_aecs, dictionary_data_ptr,
                                                    dictionary->aecs_raw_dictionary_offset, state.meta_data_->aecs_,
                                                    state.meta_data_->aecs_index, state.meta_data_->aecs_size,
                                                    load_dictionary_val);
    }

    result = util::process_descriptor<compression_operation_result_t, util::execution_mode_t::sync>(
            state.compress_descriptor_, state.completion_record_);

    if (result.status_code_ == status_list::ok) {
        result.completed_bytes_ = size;
    } else if (result.status_code_ == status_list::destination_is_short_error) {
        // Align with the behavior of non-canned mode deflate overflow (stored block also doesn't fit), which replaces
        // the returned error code "destination_is_short_error" from Intel® In-Memory Analytics Accelerator
        // with "more_output_needed"
        result.status_code_ = status_list::more_output_needed;
    }

    //    if (state.verify_descriptor_ && !result.status_code_) {
    //        result.indexes_written_ = state.prev_written_indexes;
    //
    //        hw_iaa_descriptor_compress_verification_write_initial_index(state.verify_descriptor_,
    //                                                                    state.aecs_verify_,
    //                                                                    0u,
    //                                                                    0u);
    //        result.indexes_written_ += 1;
    //
    //        auto header_bit_size = get_deflate_header_bits_size(state.huffman_table_);
    //
    //        hw_iaa_descriptor_set_input_buffer(state.verify_descriptor_,
    //                                           get_deflate_header_ptr(state.huffman_table_),
    //                                           util::bit_to_byte(header_bit_size));
    //
    //        hw_iaa_descriptor_init_inflate_header(state.verify_descriptor_,
    //                                              state.aecs_verify_,
    //                                              8u - header_bit_size & 7u,
    //                                              static_cast<const hw_aecs_access_policy>(hw_aecs_access_write
    //                                                                                       | hw_aecs_toggle_rw));
    //
    //        auto verify_result = util::process_descriptor<verification_pass_result_t,
    //                                                      util::execution_mode_t::sync>(state.verify_descriptor_,
    //                                                                                    state.completion_record_);
    //
    //        if (verify_result.status_code_) {
    //            result.status_code_ = qpl::ml::status_list::verify_error;
    //
    //            return result;
    //        }
    //
    //        hw_iaa_descriptor_set_input_buffer(state.verify_descriptor_, output_begin_ptr, result.output_bytes_);
    //
    //        hw_iaa_descriptor_inflate_set_aecs(state.verify_descriptor_,
    //                                           state.aecs_verify_,
    //                                           HW_AECS_FILTER_AND_DECOMPRESS,
    //                                           hw_aecs_access_read);
    //
    //        hw_iaa_descriptor_inflate_set_flush(state.verify_descriptor_);
    //
    //        verify_result = util::process_descriptor<verification_pass_result_t,
    //                                                 util::execution_mode_t::sync>(state.verify_descriptor_,
    //                                                                               state.completion_record_);
    //
    //        if (verify_result.status_code_ ||
    //            verify_result.checksums_.crc32_ != result.checksums_.crc32_) {
    //            result.status_code_ = qpl::ml::status_list::verify_error;
    //
    //            return result;
    //        }
    //
    //        result.indexes_written_ += verify_result.indexes_written_;
    //    }

    return result;
}

template <>
auto deflate<execution_path_t::hardware, deflate_mode_t::deflate_default>(
        deflate_state<execution_path_t::hardware>& state, uint8_t* source_begin_ptr,
        const uint32_t source_size) noexcept -> compression_operation_result_t {
    compression_operation_result_t result;
    auto                           actual_aecs      = state.meta_data_->aecs_index; // AECS used to read
    auto                           output_begin_ptr = state.next_out();

    // Check if features are supported in hardware
    bool is_hw_header_gen_supported    = false;
    bool is_hw_dict_compress_supported = false;

#if defined(__linux__)
    static auto& dispatcher = qpl::ml::dispatcher::hw_dispatcher::get_instance();
    if (!dispatcher.is_hw_support()) {
        const hw_accelerator_status hw_status = dispatcher.get_hw_init_status();
        result.status_code_                   = qpl::ml::util::convert_hw_accelerator_status_to_qpl_status(hw_status);
        return result;
    }

    const auto& device            = dispatcher.device(0);
    is_hw_header_gen_supported    = device.get_header_gen_support();
    is_hw_dict_compress_supported = device.get_dict_compress_support();
#endif //__linux__

    // If dictionary is provided, check that the following are true:
    // 1. compression with dictionary is supported
    // 2. this is a single chunk job (multi-chunk dictionary is not supported on HW path)
    // 3. the hardware_dictionary_level is set
    qpl_dictionary* dictionary = state.dictionary_;
    if (dictionary) {
        const bool is_single_chunk = (state.is_first_chunk() && state.is_last_chunk());
        if (!is_hw_dict_compress_supported || !is_single_chunk ||
            (hardware_dictionary_level::HW_NONE == dictionary->hw_dict_level)) {
            result.status_code_ = status_list::not_supported_err;
            return result;
        }

        const uint32_t dict_size_in_aecs = get_dictionary_size_in_aecs(*dictionary);
        state.meta_data_->aecs_size      = HW_AECS_COMPRESS_WITH_HT + dict_size_in_aecs;

    } else {
        state.meta_data_->aecs_size = HW_AECS_COMPRESS_WITH_HT;
    }
    hw_iaa_aecs_compress* actual_aecs_ptr =
            hw_iaa_aecs_compress_get_aecs_ptr(state.meta_data_->aecs_, actual_aecs, state.meta_data_->aecs_size);
    if (!actual_aecs_ptr) {
        result.status_code_ = status_list::internal_error;
        return result;
    }
    state.meta_data_->stored_bits = hw_iaa_aecs_compress_accumulator_get_actual_bits(actual_aecs_ptr);

    // There are two modes for header generation: 1-pass and 2-pass.
    // 1-pass can be used if dictionary is not used and source size is no larger than 4KB.
    // 1-pass header generation will calculate the statistics and Huffman Table together with compression
    // 2-pass header generation will calculate Huffman Table in the 1st pass, and compress data in the 2nd pass
    // (2-pass is similar to dynamic deflate without header generation support)
    bool is_hw_1_pass_header_gen = false;
    bool is_hw_2_pass_header_gen = false;
    if (state.collect_statistic_descriptor_ && is_hw_header_gen_supported) {
        if (source_size <= 4096U && !dictionary) {
            is_hw_1_pass_header_gen = true;
        } else {
            is_hw_2_pass_header_gen = true;
        }
    }

    // Collect statistic
    if (state.collect_statistic_descriptor_) { // Dynamic mode used
        if (state.multi_desc_status == qpl_none_completed) {
            if (is_hw_2_pass_header_gen) {
                hw_iaa_descriptor_init_statistic_collector_with_header_gen(
                        state.collect_statistic_descriptor_, source_begin_ptr, source_size, state.meta_data_->aecs_,
                        actual_aecs, state.is_last_chunk(), state.is_first_chunk());
                hw_iaa_descriptor_compress_set_mini_block_size(state.collect_statistic_descriptor_,
                                                               state.meta_data_->mini_block_size_);

                /* Append EOB so that EOB will have a Huffman code */
                hw_iaa_descriptor_compress_set_termination_rule(state.collect_statistic_descriptor_,
                                                                hw_iaa_terminator_t::end_of_block);

                // Setup dictionary in statistics collection descriptor
                if (dictionary) {
                    const uint32_t       dict_size_in_aecs   = get_dictionary_size_in_aecs(*dictionary);
                    const uint8_t* const dictionary_data_ptr = get_dictionary_data(*dictionary);
                    const uint8_t        load_dictionary_val = get_load_dictionary_flag(*dictionary);

                    hw_iaa_descriptor_compress_setup_dictionary(
                            state.collect_statistic_descriptor_, dict_size_in_aecs, dictionary_data_ptr,
                            dictionary->aecs_raw_dictionary_offset, state.meta_data_->aecs_,
                            state.meta_data_->aecs_index, state.meta_data_->aecs_size, load_dictionary_val);
                }

                // The 1st pass will generate Huffman table and deflate header
                result = util::process_descriptor<compression_operation_result_t, util::execution_mode_t::sync>(
                        state.collect_statistic_descriptor_, state.completion_record_);

                if (result.status_code_) { return result; }

                // Invert the AECS toggle for compress because src2 is written as AECS
                state.meta_data_->aecs_index ^= 1U;
                actual_aecs = state.meta_data_->aecs_index;

            } else if (!is_hw_header_gen_supported) {
                hw_iaa_aecs_compress* actual_aecs_ptr = hw_iaa_aecs_compress_get_aecs_ptr(
                        state.meta_data_->aecs_, actual_aecs, state.meta_data_->aecs_size);
                if (!actual_aecs_ptr) {
                    result.status_code_ = status_list::internal_error;
                    return result;
                }
                // If header generation is not supported, the software will compute the Huffman table
                // based on statistics generated in the 1st pass
                hw_iaa_descriptor_init_statistic_collector(state.collect_statistic_descriptor_, source_begin_ptr,
                                                           source_size, &actual_aecs_ptr->histogram);
                hw_iaa_descriptor_compress_set_mini_block_size(state.collect_statistic_descriptor_,
                                                               state.meta_data_->mini_block_size_);

                result = util::process_descriptor<compression_operation_result_t, util::execution_mode_t::sync>(
                        state.collect_statistic_descriptor_, state.completion_record_);

                if (result.status_code_) { return result; }

                hw_iaa_aecs_compress_write_deflate_dynamic_header_from_histogram(
                        actual_aecs_ptr, &actual_aecs_ptr->histogram, state.is_last_chunk());
            }

            /* For dynamic mode, append EOB after final token in the compression descriptor */
            hw_iaa_descriptor_compress_set_termination_rule(state.compress_descriptor_,
                                                            hw_iaa_terminator_t::end_of_block);
        }
    } else { // Static or fixed mode used
        if (state.is_first_chunk() || state.start_new_block) {
            hw_iaa_aecs_compress* actual_aecs_ptr = hw_iaa_aecs_compress_get_aecs_ptr(
                    state.meta_data_->aecs_, actual_aecs, state.meta_data_->aecs_size);
            if (!actual_aecs_ptr) {
                result.status_code_ = status_list::internal_error;
                return result;
            }

            // If we want to write a new deflate block and it's a continuable compression task, then insert EOB
            if (!state.is_first_chunk()) {
                hw_iaa_aecs_compress_accumulator_insert_eob(actual_aecs_ptr, state.meta_data_->eob_code);
            }

            if (state.huffman_table_) { // Static mode used
                result.status_code_ = hw_iaa_aecs_compress_write_deflate_dynamic_header(
                        actual_aecs_ptr, get_deflate_header_ptr(state.huffman_table_),
                        get_deflate_header_bits_size(state.huffman_table_), state.is_last_chunk());
            } else { // Fixed mode used
                result.status_code_ =
                        hw_iaa_aecs_compress_write_deflate_fixed_header(actual_aecs_ptr, state.is_last_chunk());
            }
        }

        if (result.status_code_) {
            result.status_code_ = QPL_STS_LIBRARY_INTERNAL_ERR;
            return result;
        }

        if (state.is_last_chunk()) {
            /* Append EOB after final token if single job is used or if we start a new block (starting with the new Huffman Table),
               or append EOB and bFinal Stored block in case of the "middle" of multiple jobs to produce valid stream */
            hw_iaa_descriptor_compress_set_termination_rule(state.compress_descriptor_,
                                                            !(state.start_new_block || state.is_first_chunk())
                                                                    ? hw_iaa_terminator_t::final_end_of_block
                                                                    : hw_iaa_terminator_t::end_of_block);
        }
    }

    // Prepare compression descriptor only if previous submission(s) have not done this
    if (state.multi_desc_status == qpl_none_completed) {
        auto access_policy = static_cast<hw_iaa_aecs_access_policy>(
                util::aecs_compress_access_lookup_table[state.processing_step] | actual_aecs);

        hw_iaa_descriptor_set_input_buffer(state.compress_descriptor_, source_begin_ptr, source_size);

        if (is_hw_1_pass_header_gen) {
            hw_iaa_descriptor_set_1_pass_header_gen(state.compress_descriptor_, state.meta_data_->aecs_, actual_aecs,
                                                    state.is_last_chunk(), state.is_first_chunk());
        } else {
            hw_iaa_descriptor_compress_set_aecs(state.compress_descriptor_, state.meta_data_->aecs_, access_policy,
                                                !qpl::ml::util::are_iaa_gen_2_min_capabilities_present());
        }

        // Setup dictionary in compression descriptor
        if (dictionary) {
            const uint32_t       dict_size_in_aecs   = get_dictionary_size_in_aecs(*dictionary);
            const uint8_t* const dictionary_data_ptr = get_dictionary_data(*dictionary);
            const uint8_t        load_dictionary_val = get_load_dictionary_flag(*dictionary);

            hw_iaa_descriptor_compress_setup_dictionary(state.compress_descriptor_, dict_size_in_aecs,
                                                        dictionary_data_ptr, dictionary->aecs_raw_dictionary_offset,
                                                        state.meta_data_->aecs_, state.meta_data_->aecs_index,
                                                        state.meta_data_->aecs_size, load_dictionary_val);
        }
    }

    // Submit compression descriptor if previous submission(s) fail to submit successfully
    if (state.multi_desc_status == qpl_none_completed || state.multi_desc_status == qpl_stats_collect_completed) {
        result = util::process_descriptor<compression_operation_result_t, util::execution_mode_t::sync>(
                state.compress_descriptor_, state.completion_record_);

        if (result.status_code_ == status_list::destination_is_short_error) {
            // There can't be multiple stored blocks while indexing
            if (source_size >= 64_kb && state.meta_data_->mini_block_size_) {
                result.status_code_ = status_list::index_generation_error;
            } else {
                result = write_stored_block(state);
            }
        }

        if (result.status_code_ == QPL_STS_QUEUES_ARE_BUSY_ERR) {
            result.multi_desc_status = qpl_stats_collect_completed;
            state.multi_desc_status  = qpl_stats_collect_completed;
        }
    }

    if (state.verify_descriptor_ && !result.status_code_) {
        auto verify_actual_aecs = state.meta_data_->verify_aecs_index; // AECS used to read for verify
        result.indexes_written_ = state.prev_written_indexes;

        auto verify_access_policy = static_cast<hw_iaa_aecs_access_policy>(
                util::aecs_verify_access_lookup_table[state.processing_step] | verify_actual_aecs);

        if (state.is_first_chunk()) {
            auto initial_bit_offset = static_cast<uint32_t>(state.meta_data_->prologue_size_ * byte_bits_size);

            if (state.meta_data_->mini_block_size_) {
                // Write initial index if indexing is enabled
                hw_iaa_descriptor_compress_verification_write_initial_index(state.verify_descriptor_,
                                                                            state.aecs_verify_, 0U, initial_bit_offset);
                result.indexes_written_ += 1;
            } else {
                // Just set initial decompression step otherwise
                hw_iaa_aecs_decompress_set_decompression_state(
                        &state.aecs_verify_->inflate_options,
                        hw_iaa_aecs_decompress_state::hw_aecs_at_start_block_header);
            }

            if (initial_bit_offset) {
                verify_access_policy =
                        static_cast<hw_iaa_aecs_access_policy>(verify_access_policy | hw_aecs_access_read);
            }
        }

        hw_iaa_descriptor_set_input_buffer(state.verify_descriptor_, output_begin_ptr, result.output_bytes_);

        hw_iaa_aecs_decompress_state_set_aecs_format(&state.aecs_verify_->inflate_options,
                                                     qpl::ml::util::are_iaa_gen_2_min_capabilities_present());

        if (verify_access_policy) { //NOLINT(bugprone-non-zero-enum-to-bool-conversion)
            hw_iaa_descriptor_inflate_set_aecs(state.verify_descriptor_, state.aecs_verify_,
                                               HW_AECS_FILTER_AND_DECOMPRESS, verify_access_policy);
        }

        if (state.is_last_chunk()) { hw_iaa_descriptor_inflate_set_flush(state.verify_descriptor_); }

        auto verify_result = util::process_descriptor<verification_pass_result_t, util::execution_mode_t::sync>(
                state.verify_descriptor_, state.completion_record_);

        if (verify_result.status_code_ ||
            (state.is_last_chunk() && verify_result.checksums_.crc32_ != result.checksums_.crc32_)) {
            result.status_code_ = qpl::ml::status_list::verify_error;

            if (verify_result.status_code_) {
                DIAG("Deflate verification error code is %d\n", verify_result.status_code_);
            }

            return result;
        }

        result.indexes_written_ += verify_result.indexes_written_;

        // Invert AECS toggle for verify
        state.meta_data_->verify_aecs_index ^= 1U;
    }

    if (result.status_code_ == status_list::ok) {
        // Invert AECS toggle for compression
        state.meta_data_->aecs_index ^= 1U;

        result.completed_bytes_ = source_size;
    }

    return result;
}

template <>
auto deflate<execution_path_t::software, deflate_mode_t::deflate_no_headers>(
        deflate_state<execution_path_t::software>& state, uint8_t* begin, const uint32_t size) noexcept
        -> compression_operation_result_t {
    state.compression_mode_ = canned_mode;
    auto output_begin_ptr   = state.next_out();

    auto result = deflate_pass(state, begin, size);

    if (state.is_verification_enabled_ && !result.status_code_) {
        auto builder =
                (state.is_first_chunk())
                        ? compression::verification_state_builder<execution_path_t::software>::create(state.allocator_)
                        : compression::verification_state_builder<execution_path_t::software>::restore(
                                  state.allocator_);

        auto verify_state = builder.build();

        verify_state.input(output_begin_ptr, state.next_out()).required_crc(state.checksum_.crc32);

        if (state.is_first_chunk() && state.compression_table_) {
            verify_state.decompress_table(get_deflate_header_ptr(state.compression_table_),
                                          get_deflate_header_bits_size(state.compression_table_));
        }

        auto verification_result =
                perform_verification<execution_path_t::software, verification_mode_t::verify_deflate_no_headers>(
                        verify_state);

        if (verification_result.status == parser_status_t::error) {
            result.status_code_ = qpl::ml::status_list::verify_error;

            return result;
        }
    }

    result.completed_bytes_  = state.isal_stream_ptr_->total_in;
    result.output_bytes_     = state.isal_stream_ptr_->total_out;
    result.indexes_written_  = state.index_table_.get_current_index();
    result.checksums_.crc32_ = state.checksum_.crc32;

    if (state.isal_stream_ptr_->internal_state.count) {
        result.status_code_ = qpl::ml::status_list::more_output_needed;
    }

    return result;
}

template <>
auto deflate<execution_path_t::software, deflate_mode_t::deflate_default>(
        deflate_state<execution_path_t::software>& state, uint8_t* begin, const uint32_t size) noexcept
        -> compression_operation_result_t {
    auto output_begin_ptr = state.next_out();

    compression_operation_result_t result = deflate_pass(state, begin, size);

    if (!(state.is_first_chunk() && state.is_last_chunk())) { state.save_bit_buffer(); }

    if (state.is_verification_enabled_ && !result.status_code_) {
        auto builder =
                (state.is_first_chunk())
                        ? compression::verification_state_builder<execution_path_t::software>::create(state.allocator_)
                        : compression::verification_state_builder<execution_path_t::software>::restore(
                                  state.allocator_);

        auto verify_state = builder.build();

        verify_state.input(output_begin_ptr, state.next_out()).required_crc(state.checksum_.crc32);

        auto verification_result =
                perform_verification<execution_path_t::software, verification_mode_t::verify_deflate_default>(
                        verify_state);

        if (verification_result.status == parser_status_t::error) {
            result.status_code_ = qpl::ml::status_list::verify_error;

            return result;
        }
    }

    result.completed_bytes_  = state.isal_stream_ptr_->total_in;
    result.output_bytes_     = state.isal_stream_ptr_->total_out;
    result.indexes_written_  = state.index_table_.get_current_index();
    result.checksums_.crc32_ = state.checksum_.crc32;

    if (state.isal_stream_ptr_->internal_state.count) {
        result.status_code_ = qpl::ml::status_list::more_output_needed;
    }

    return result;
}

} // namespace qpl::ml::compression
