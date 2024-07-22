/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Job API (public C API)
 */

#ifndef QPL_SOURCES_C_API_COMPRESSION_OPERATIONS_ARGUMENTS_CHECK_HPP_
#define QPL_SOURCES_C_API_COMPRESSION_OPERATIONS_ARGUMENTS_CHECK_HPP_

#include "compression_state_t.h"
#include "job.hpp"
#include "own_checkers.h"
#include "util/checkers.hpp"
#include "util/iaa_features_checks.hpp"

namespace qpl::job {

namespace details {
namespace compression::common {

static inline qpl_status bad_arguments_check(const qpl_job* const job_ptr) {
    /* Check for Null Pointers and Buffer Overlap */

    if (ml::bad_argument::check_for_nullptr(job_ptr, job_ptr->next_in_ptr, job_ptr->next_out_ptr)) {
        return QPL_STS_NULL_PTR_ERR;
    }

    if (ml::bad_argument::buffers_overlap(job_ptr->next_in_ptr, job_ptr->available_in, job_ptr->next_out_ptr,
                                          job_ptr->available_out)) {
        return QPL_STS_BUFFER_OVERLAP_ERR;
    }

    return QPL_STS_OK;
}

} // namespace compression::common

} // namespace details

template <>
inline qpl_status bad_arguments_check<qpl_operation::qpl_op_compress>(const qpl_job* const job_ptr) noexcept {
    // Check for Size Errors
    if (job_ptr->available_in == 0 || job_ptr->available_out == 0) { return QPL_STS_SIZE_ERR; }
    // Check for indexing w/o index table
    if (job::is_indexing_enabled(job_ptr) && job_ptr->idx_array == nullptr) { return QPL_STS_MISSING_INDEX_TABLE_ERR; }

    /* Check for Flag conflicts */

    // Check for conflict between writing no headers (QPL_FLAG_NO_HDRS) and
    // writing huffman tokens in header in Big Endian format (QPL_FLAG_HUFFMAN_BE)
    if (job_ptr->flags & QPL_FLAG_HUFFMAN_BE && !(job_ptr->flags & QPL_FLAG_NO_HDRS)) {
        return QPL_STS_FLAG_CONFLICT_ERR;
    }

    auto const job = const_cast<qpl_job*>(job_ptr);
    // Sets job level to default if no headers and literals are generated
    if (job_ptr->flags & QPL_FLAG_NO_HDRS && job_ptr->flags & QPL_FLAG_GEN_LITERALS) {
        job->level = qpl_default_level;
    } else if (job_ptr->flags & QPL_FLAG_NO_HDRS && job_ptr->flags & QPL_FLAG_DYNAMIC_HUFFMAN &&
               !(is_single_job(job_ptr))) {
        return QPL_STS_FLAG_CONFLICT_ERR;
    }

    /* Check for Mode Not Supported */

    // Check for unsupported mode zlib/gzip w/ dictionary
    if (job_ptr->flags & (QPL_FLAG_ZLIB_MODE | QPL_FLAG_GZIP_MODE) && job::is_dictionary(job_ptr)) {
        return QPL_STS_NOT_SUPPORTED_MODE_ERR;
    }

    // Check for unsupported mode omit verify w/ dictionary
    if (!(job_ptr->flags & QPL_FLAG_OMIT_VERIFY) && job::is_dictionary(job_ptr)) {
        return QPL_STS_NOT_SUPPORTED_MODE_ERR;
    }

    // Check for huffman only compression w/o huffman table
    if (!(job_ptr->huffman_table) && job::is_huffman_only_compression(job_ptr)) {
        return QPL_STS_NOT_SUPPORTED_MODE_ERR;
    }

    // Check for Canned & Indexing mode
    if (job::is_canned_mode_compression(job_ptr) && job::is_indexing_enabled(job_ptr)) {
        return QPL_STS_NOT_SUPPORTED_MODE_ERR;
    }

    /* Check for Unsupported Compression Levels */

    // Check for unsupported compression levels
    if (job_ptr->level != qpl_high_level && job_ptr->level != qpl_default_level) {
        return QPL_STS_UNSUPPORTED_COMPRESSION_LEVEL;
    }
    // Check for high compression level on hw path
    if (job_ptr->level == qpl_high_level && (job::get_execution_path(job_ptr) == ml::execution_path_t::hardware)) {
        return QPL_STS_UNSUPPORTED_COMPRESSION_LEVEL;
    }

    constexpr auto USER_HUFFMAN_TABLE_USED = 1U;
    constexpr auto NO_USER_HUFFMAN_TABLE   = 0U;

    auto* compression_state = reinterpret_cast<own_compression_state_t*>(job_ptr->data_ptr.compress_state_ptr);

    auto compression_style = (job_ptr->flags & QPL_FLAG_DYNAMIC_HUFFMAN) |
                             ((job_ptr->huffman_table) ? USER_HUFFMAN_TABLE_USED : NO_USER_HUFFMAN_TABLE);
    // Check if the compression style is the same as the previous job
    if (!(job_ptr->flags & QPL_FLAG_FIRST) && compression_state->middle_layer_compression_style != compression_style) {
        return QPL_STS_INVALID_COMPRESS_STYLE_ERR;
    } else {
        // Set Compression Style
        compression_state->middle_layer_compression_style = compression_style;
    }

    return QPL_STS_OK;
}

template <>
inline qpl_status bad_arguments_check<qpl_operation::qpl_op_decompress>(const qpl_job* const job_ptr) noexcept {
    /* Check for Flag conflicts */

    // Check for conflict between writing no headers (QPL_FLAG_NO_HDRS) and
    // Indexing mode, which contains a header (QPL_FLAG_RND_ACCESS)
    if ((job_ptr->flags & QPL_FLAG_RND_ACCESS) && (job_ptr->flags & QPL_FLAG_NO_HDRS)) {
        return QPL_STS_FLAG_CONFLICT_ERR;
    }

    // Check for conflict between writing no headers (QPL_FLAG_NO_HDRS) and
    // gzip mode, which contains a header (QPL_FLAG_GZIP_MODE)
    if ((job_ptr->flags & QPL_FLAG_GZIP_MODE) && (job_ptr->flags & QPL_FLAG_NO_HDRS)) {
        return QPL_STS_FLAG_CONFLICT_ERR;
    }

    // Check for conflict between writing no headers (QPL_FLAG_NO_HDRS) and
    // zlib mode, which contains a header (QPL_FLAG_ZLIB_MODE)
    if ((job_ptr->flags & QPL_FLAG_ZLIB_MODE) && (job_ptr->flags & QPL_FLAG_NO_HDRS)) {
        return QPL_STS_FLAG_CONFLICT_ERR;
    }

    // Check for conflict between checking/skipping both gzip and zlib headers
    if ((job_ptr->flags & QPL_FLAG_ZLIB_MODE) && (job_ptr->flags & QPL_FLAG_GZIP_MODE)) {
        return QPL_STS_FLAG_CONFLICT_ERR;
    }

    // Checks for Huffman only decompression with BE16 format
    if (job::is_huffman_only_decompression(job_ptr) && (job_ptr->flags & QPL_FLAG_HUFFMAN_BE)) {
        // Check ignore bits fields
        // BE16 format processes a 16-bit word at a time, so
        // ignored bits may be up to 15
        if (job_ptr->ignore_start_bits > 15U || job_ptr->ignore_end_bits > 15U) { return QPL_STS_INVALID_PARAM_ERR; }

        // Intel® In-Memory Analytics Accelerator (Intel® IAA) generation 1.0 limitation,
        // Huffman only decompression with BE16 format cannot work if ignore_end_bits is greater than 7
        if (job::get_execution_path(job_ptr) == ml::execution_path_t::hardware) {
            // Check availability of the ignore end bits extension bit
            if (job_ptr->ignore_end_bits > 7U && !qpl::ml::util::are_iaa_gen_2_min_capabilities_present()) {
                return QPL_STS_HUFFMAN_BE_IGNORE_MORE_THAN_7_BITS_ERR;
            }
        }

        // Check input size
        if (job_ptr->available_in % 2 == 1) { return QPL_STS_HUFFMAN_BE_ODD_INPUT_SIZE_ERR; }

        // Check ignore bits fields for normal format
    } else if (job_ptr->ignore_start_bits > 7U || job_ptr->ignore_end_bits > 7U) {
        return QPL_STS_INVALID_PARAM_ERR;
    }

    // Check decomp_end_processing field
    if (job_ptr->decomp_end_processing > qpl_check_on_nonlast_block) { return QPL_STS_INVALID_DECOMP_END_PROC_ERR; }

    if (job_ptr->decomp_end_processing == OWN_RESERVED_INFLATE_MANIPULATOR) {
        return QPL_STS_INVALID_DECOMP_END_PROC_ERR;
    }

    // Check for huffman only decompression w/o huffman table
    if (!(job_ptr->huffman_table) && job::is_huffman_only_decompression(job_ptr)) {
        return QPL_STS_NOT_SUPPORTED_MODE_ERR;
    }

    // Check for canned w/ random access
    if (job::is_canned_mode_decompression(job_ptr) && job::is_random_decompression(job_ptr)) {
        return QPL_STS_NOT_SUPPORTED_MODE_ERR;
    }

    return QPL_STS_OK;
}

template <>
inline qpl_status validate_operation<qpl_op_compress>(const qpl_job* const job_ptr) noexcept {
    OWN_QPL_CHECK_STATUS(details::compression::common::bad_arguments_check(job_ptr));
    OWN_QPL_CHECK_STATUS(bad_arguments_check<qpl_operation::qpl_op_compress>(job_ptr));

    return QPL_STS_OK;
}

template <>
inline qpl_status validate_operation<qpl_op_decompress>(const qpl_job* const job_ptr) noexcept {
    OWN_QPL_CHECK_STATUS(details::compression::common::bad_arguments_check(job_ptr));
    OWN_QPL_CHECK_STATUS(bad_arguments_check<qpl_op_decompress>(job_ptr));

    return QPL_STS_OK;
}

} // namespace qpl::job

#endif //QPL_SOURCES_C_API_COMPRESSION_OPERATIONS_ARGUMENTS_CHECK_HPP_
