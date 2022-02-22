/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Job API (public C API)
 */

#include "own_defs.h"
#include "compression_state_t.h"
#include "common/defs.hpp"
#include "compression/decompression_table.hpp"
#include "compression/inflate/inflate.hpp"
#include "compression/huffman_only/huffman_only.hpp"
#include "compression/stream_decorators/gzip_decorator.hpp"
#include "compression/stream_decorators/zlib_decorator.hpp"
#include "compression/stream_decorators/default_decorator.hpp"
#include "canned_utility.h"
#include "job.hpp"
#include "arguments_check.hpp"

namespace qpl {

using operation_result_t = ml::compression::decompression_operation_result_t;
using huffman_table_t = ml::compression::decompression_huffman_table;

template <>
void inline job::update<operation_result_t>(qpl_job *job_ptr, operation_result_t &result) noexcept {
    job::update_input_stream(job_ptr, result.completed_bytes_);
    job::update_output_stream(job_ptr, result.output_bytes_, 0u);
    job::update_checksums(job_ptr, result.checksums_.crc32_, result.checksums_.xor_);
}

void inline set_representation_flags(qpl_decompression_huffman_table *qpl_decompression_table_ptr,
                                     huffman_table_t &ml_decompression_table) {
    if (is_sw_representation_used(qpl_decompression_table_ptr)) {
        ml_decompression_table.enable_sw_decompression_table();
    }

    if (is_hw_representation_used(qpl_decompression_table_ptr)) {
        ml_decompression_table.enable_hw_decompression_table();
    }

    if (is_deflate_representation_used(qpl_decompression_table_ptr)) {
        ml_decompression_table.enable_deflate_header();
    }
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage=4096"
#endif

template <qpl::ml::execution_path_t path>
uint32_t perform_decompress(qpl_job *const job_ptr) noexcept {
    using namespace qpl::ml::compression;
    using namespace qpl::ml;

    if (job_ptr->flags & QPL_FLAG_FIRST) {
        job::reset<qpl_op_decompress>(job_ptr);
    }

    decompression_operation_result_t result{};

    OWN_QPL_CHECK_STATUS(qpl::job::validate_operation<qpl_op_decompress>(job_ptr));

    auto *decompression_table_ptr = job_ptr->decompression_huffman_table;

    qpl::ml::allocation_buffer_t state_buffer(job_ptr->data_ptr.middle_layer_buffer_ptr,
                                              job_ptr->data_ptr.hw_state_ptr);

    const qpl::ml::util::linear_allocator allocator(state_buffer);

    if (job_ptr->flags & QPL_FLAG_NO_HDRS) {
        // Initialize decompression table
        decompression_huffman_table decompression_table(get_sw_decompression_table_buffer(decompression_table_ptr),
                                                        get_hw_decompression_table_buffer(decompression_table_ptr),
                                                        get_deflate_header_buffer(decompression_table_ptr),
                                                        get_lookup_table_buffer_ptr(decompression_table_ptr));

        set_representation_flags(decompression_table_ptr, decompression_table);

        huffman_only_decompression_state<path> state(allocator);

        auto endianness = (job_ptr->flags & QPL_FLAG_HUFFMAN_BE) ? big_endian : little_endian;

        state.input(job_ptr->next_in_ptr, job_ptr->next_in_ptr + job_ptr->available_in)
             .output(job_ptr->next_out_ptr, job_ptr->next_out_ptr + job_ptr->available_out)
             .crc_seed(job_ptr->crc)
             .endianness(endianness);

        // @todo Add ignore last bits option
        if constexpr (path == qpl::ml::execution_path_t::software) {
            state.last_bits_offset(static_cast<uint8_t>(qpl::ml::byte_bits_size - job_ptr->ignore_end_bits));
        }

        result = decompress_huffman_only<path>(state, decompression_table);
    } else {
        // Prepare decompression state
        auto state = (job_ptr->flags & QPL_FLAG_FIRST) ?
                     qpl::ml::compression::inflate_state<path>::template create<true>(allocator) :
                     qpl::ml::compression::inflate_state<path>::restore(allocator);

        state.input(job_ptr->next_in_ptr, job_ptr->next_in_ptr + job_ptr->available_in)
             .output(job_ptr->next_out_ptr, job_ptr->next_out_ptr + job_ptr->available_out)
             .crc_seed(job_ptr->crc)
             .input_access({static_cast<bool>((job_ptr->flags & QPL_FLAG_RND_ACCESS)),
                            job_ptr->ignore_start_bits,
                            job_ptr->ignore_end_bits});

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
            // Initialize decompression table
            decompression_huffman_table decompression_table(get_sw_decompression_table_buffer(decompression_table_ptr),
                                                            get_hw_decompression_table_buffer(decompression_table_ptr),
                                                            get_deflate_header_buffer(decompression_table_ptr),
                                                            get_lookup_table_buffer_ptr(decompression_table_ptr));

            set_representation_flags(decompression_table_ptr, decompression_table);

            if (decompression_table.is_deflate_header_used()) {
                result = default_decorator::unwrap(inflate<path, inflate_mode_t::inflate_body>,
                                                   state.decompress_table(decompression_table),
                                                   stop_and_check_any_eob);
            } else {
                result.status_code_ = qpl::ml::status_list::status_invalid_params;
            }
        } else if (!(job_ptr->flags & QPL_FLAG_NO_BUFFERING)) { // Default inflating
            // Perform decompression in inflate standard
            auto end_processing_properties = static_cast<end_processing_condition_t>(job_ptr->decomp_end_processing);

            if (job_ptr->flags & QPL_FLAG_DECOMP_FLUSH_ALWAYS) {
                state.flush_out();
            }

            if (job_ptr->flags & QPL_FLAG_LAST) {
                state.terminate();
            }

            if (job_ptr->flags & QPL_FLAG_GZIP_MODE) {
                result = gzip_decorator::unwrap(inflate<path, inflate_mode_t::inflate_default>,
                                                state,
                                                end_processing_properties);
            } else if (job_ptr->flags & QPL_FLAG_ZLIB_MODE) {
                result = zlib_decorator::unwrap(inflate<path, inflate_mode_t::inflate_default>,
                                                state,
                                                end_processing_properties);
            } else {
                result = default_decorator::unwrap(inflate<path, inflate_mode_t::inflate_default>,
                                                   state,
                                                   end_processing_properties);
            }
        } else { // Random Access
            if (job_ptr->flags & QPL_FLAG_FIRST) {
                result = inflate<path, inflate_mode_t::inflate_header>(state, stop_and_check_any_eob);
            } else {
                result = default_decorator::unwrap(inflate<path, inflate_mode_t::inflate_body>,
                                                   state,
                                                   stop_and_check_any_eob);
            }
        }
    }

    if (result.status_code_ == 0) {
        job::update(job_ptr, result);
    }

    return result.status_code_;
}

template
uint32_t perform_decompress<qpl::ml::execution_path_t::hardware>(qpl_job *const job_ptr) noexcept;

template
uint32_t perform_decompress<qpl::ml::execution_path_t::software>(qpl_job *const job_ptr) noexcept;

}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
