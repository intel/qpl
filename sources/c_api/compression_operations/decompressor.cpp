/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Job API (public C API)
 */

// c_api
#include "arguments_check.hpp"
#include "compression_state_t.h"
#include "huffman_table.hpp"
#include "job.hpp"
#include "own_defs.h"

// ML
#include "common/defs.hpp"
#include "compression/huffman_only/huffman_only.hpp"
#include "compression/huffman_table/inflate_huffman_table.hpp"
#include "compression/inflate/inflate.hpp"
#include "compression/inflate/inflate_state.hpp"
#include "compression/stream_decorators/default_decorator.hpp"
#include "compression/stream_decorators/gzip_decorator.hpp"
#include "compression/stream_decorators/zlib_decorator.hpp"
#include "dispatcher/hw_dispatcher.hpp"
#include "util/iaa_features_checks.hpp"

namespace qpl {

using operation_result_t = ml::compression::decompression_operation_result_t;
using huffman_table_t    = ml::compression::decompression_huffman_table;

template <>
void inline job::update<operation_result_t>(qpl_job* job_ptr, operation_result_t& result) noexcept {
    job::update_input_stream(job_ptr, result.completed_bytes_);
    job::update_output_stream(job_ptr, result.output_bytes_, 0U);
    job::update_checksums(job_ptr, result.checksums_.crc32_, result.checksums_.xor_);
}

void inline set_representation_flags(qpl_decompression_huffman_table* qpl_decompression_table_ptr,
                                     huffman_table_t&                 ml_decompression_table) {
    if (is_sw_representation_used(qpl_decompression_table_ptr)) {
        ml_decompression_table.enable_sw_decompression_table();
    }

    if (is_hw_representation_used(qpl_decompression_table_ptr)) {
        ml_decompression_table.enable_hw_decompression_table();
    }

    if (is_deflate_representation_used(qpl_decompression_table_ptr)) { ml_decompression_table.enable_deflate_header(); }
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage=4096"
#endif

template <qpl::ml::execution_path_t path>
uint32_t perform_decompress(qpl_job* const job_ptr) noexcept {
    using namespace qpl::ml::compression;
    using namespace qpl::ml;

    if (job_ptr->flags & QPL_FLAG_FIRST) { job::reset<qpl_op_decompress>(job_ptr); }

    decompression_operation_result_t result {};

    OWN_QPL_CHECK_STATUS(qpl::job::validate_operation<qpl_op_decompress>(job_ptr));

    qpl::ml::allocation_buffer_t state_buffer(job_ptr->data_ptr.middle_layer_buffer_ptr,
                                              job_ptr->data_ptr.hw_state_ptr);

    const qpl::ml::util::linear_allocator allocator(state_buffer);

    if (job_ptr->flags & QPL_FLAG_NO_HDRS) {
        OWN_QPL_CHECK_STATUS(
                check_huffman_table_is_correct<compression_algorithm_e::huffman_only>(job_ptr->huffman_table))
        auto table_impl = use_as_huffman_table<compression_algorithm_e::huffman_only>(job_ptr->huffman_table);

        auto d_table_ptr =
                reinterpret_cast<qpl_decompression_huffman_table*>(table_impl->decompression_huffman_table<path>());

        // Initialize decompression table
        decompression_huffman_table decompression_table(
                table_impl->get_sw_decompression_table_buffer(), table_impl->get_hw_decompression_table_buffer(),
                table_impl->get_deflate_header_buffer(), table_impl->get_lookup_table_buffer_ptr());

        set_representation_flags(d_table_ptr, decompression_table);

        huffman_only_decompression_state<path> state(allocator);

        auto endianness = (job_ptr->flags & QPL_FLAG_HUFFMAN_BE) ? big_endian : little_endian;

        if constexpr (path == qpl::ml::execution_path_t::software) {
            state.input(job_ptr->next_in_ptr, job_ptr->next_in_ptr + job_ptr->available_in)
                    .output(job_ptr->next_out_ptr, job_ptr->next_out_ptr + job_ptr->available_out)
                    .crc_seed(job_ptr->crc)
                    .endianness(endianness);

            // For BE16, ignore_end_bits can range from 0-15, therefore last_bits_offset is calculated differently:
            // i.e. for BE16, last_bits_offset = 16 - ignore_end_bits
            // for normal format, last_bits_offset = 8 - ignore_end_bits
            if (endianness == big_endian) {
                state.last_bits_offset(static_cast<uint8_t>(2 * qpl::ml::byte_bits_size - job_ptr->ignore_end_bits));
            } else {
                state.last_bits_offset(static_cast<uint8_t>(qpl::ml::byte_bits_size - job_ptr->ignore_end_bits));
            }
        } else {
            state.input(job_ptr->next_in_ptr, job_ptr->next_in_ptr + job_ptr->available_in)
                    .output(job_ptr->next_out_ptr, job_ptr->next_out_ptr + job_ptr->available_out)
                    .crc_seed(job_ptr->crc)
                    .endianness(endianness);
            state.ignore_end_bits = job_ptr->ignore_end_bits;

            state.set_is_gen1_hw(!qpl::ml::util::are_iaa_gen_2_min_capabilities_present());
        }
        result = decompress_huffman_only<path>(state, decompression_table);
    } else {
        // Prepare decompression state
        auto state = (job_ptr->flags & QPL_FLAG_FIRST)
                             ? qpl::ml::compression::inflate_state<path>::template create<true>(allocator)
                             : qpl::ml::compression::inflate_state<path>::restore(allocator);

        if ((job_ptr->flags & QPL_FLAG_RND_ACCESS) && !(job_ptr->flags & QPL_FLAG_CANNED_MODE)) { // Random Access
            state.input(job_ptr->next_in_ptr, job_ptr->next_in_ptr + job_ptr->available_in)
                    .output(job_ptr->next_out_ptr, job_ptr->next_out_ptr + job_ptr->available_out)
                    .crc_seed(job_ptr->crc)
                    .input_access({static_cast<bool>(!(job_ptr->flags & QPL_FLAG_FIRST)), job_ptr->ignore_start_bits,
                                   job_ptr->ignore_end_bits});

        } else {
            state.input(job_ptr->next_in_ptr, job_ptr->next_in_ptr + job_ptr->available_in)
                    .output(job_ptr->next_out_ptr, job_ptr->next_out_ptr + job_ptr->available_out)
                    .crc_seed(job_ptr->crc)
                    .input_access({static_cast<bool>((job_ptr->flags & QPL_FLAG_RND_ACCESS) ||
                                                     (job_ptr->flags & QPL_FLAG_CANNED_MODE)),
                                   job_ptr->ignore_start_bits, job_ptr->ignore_end_bits});
        }

        if constexpr (qpl::ml::execution_path_t::hardware == path) {
            state.set_is_gen1_hw(!qpl::ml::util::are_iaa_gen_2_min_capabilities_present());
        }

        if (job::is_dictionary(job_ptr)) {
            if constexpr (qpl::ml::execution_path_t::software == path) {
                state.dictionary(*job_ptr->dictionary);
            } else {
                if (!job::is_canned_mode_decompression(job_ptr)) {
                    state.dictionary(*job_ptr->dictionary);
                } else {
                    return qpl::ml::status_list::not_supported_err;
                }
            }
        }

        if (job_ptr->flags & QPL_FLAG_CANNED_MODE) { // Canned decompression
            OWN_QPL_CHECK_STATUS(
                    check_huffman_table_is_correct<compression_algorithm_e::deflate>(job_ptr->huffman_table))
            auto table_impl = use_as_huffman_table<compression_algorithm_e::deflate>(job_ptr->huffman_table);

            // Initialize decompression table
            decompression_huffman_table decompression_table(
                    table_impl->get_sw_decompression_table_buffer(), table_impl->get_hw_decompression_table_buffer(),
                    table_impl->get_deflate_header_buffer(), table_impl->get_lookup_table_buffer_ptr());

            auto d_table_ptr =
                    reinterpret_cast<qpl_decompression_huffman_table*>(table_impl->decompression_huffman_table<path>());

            set_representation_flags(d_table_ptr, decompression_table);

            if (decompression_table.is_deflate_header_used()) {
                result = default_decorator::unwrap(inflate<path, inflate_mode_t::inflate_body>,
                                                   state.decompress_table(decompression_table), stop_and_check_any_eob);
            } else {
                result.status_code_ = qpl::ml::status_list::status_invalid_params;
            }
        } else if (!(job_ptr->flags & QPL_FLAG_RND_ACCESS)) { // Default inflating
            // Perform decompression in inflate standard
            auto end_processing_properties = static_cast<end_processing_condition_t>(job_ptr->decomp_end_processing);

            if (job_ptr->flags & QPL_FLAG_DECOMP_FLUSH_ALWAYS) { state.flush_out(); }

            if (job_ptr->flags & QPL_FLAG_LAST) { state.terminate(); }

            if (job_ptr->flags & QPL_FLAG_GZIP_MODE) {
                result = gzip_decorator::unwrap(inflate<path, inflate_mode_t::inflate_default>, state,
                                                end_processing_properties);
            } else if (job_ptr->flags & QPL_FLAG_ZLIB_MODE) {
                result = zlib_decorator::unwrap(inflate<path, inflate_mode_t::inflate_default>, state,
                                                end_processing_properties);
            } else {
                result = default_decorator::unwrap(inflate<path, inflate_mode_t::inflate_default>, state,
                                                   end_processing_properties);
            }
        } else { // Random Access
            if (job_ptr->flags & QPL_FLAG_FIRST) {
                result = inflate<path, inflate_mode_t::inflate_header>(state, stop_and_check_any_eob);
            } else {
                result = default_decorator::unwrap(inflate<path, inflate_mode_t::inflate_body>, state,
                                                   stop_and_check_any_eob);
            }
        }
    }

    /**
     * @warning In case the output buffer fills up before the input is completely processed upon decompression,
     * (i.e. getting QPL_STS_MORE_OUTPUT_NEEDED on sw path or internal status QPL_STS_INTL_OUTPUT_OVERFLOW on hw path)
     * need to update job structure to the valid state for continuing upon resubmission.
    */
    if (result.status_code_ == 0 ||
        (qpl::ml::execution_path_t::software == path && result.status_code_ == QPL_STS_MORE_OUTPUT_NEEDED) ||
        (qpl::ml::execution_path_t::hardware == path && result.status_code_ == QPL_STS_INTL_OUTPUT_OVERFLOW)) {
        job::update(job_ptr, result);
    }

    if (result.status_code_ == QPL_STS_INTL_OUTPUT_OVERFLOW) { return QPL_STS_MORE_OUTPUT_NEEDED; }

    return result.status_code_;
}

template uint32_t perform_decompress<qpl::ml::execution_path_t::hardware>(qpl_job* const job_ptr) noexcept;

template uint32_t perform_decompress<qpl::ml::execution_path_t::software>(qpl_job* const job_ptr) noexcept;

} // namespace qpl

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
