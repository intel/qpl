/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file qpl_util.hpp
 *
 * @brief File that contains utility functions for checks between high-level API and Job API
 *
 * @note This file should be included only into internal files and should not be available
 *       for public usage
 */

#ifndef QPL_UTIL_HPP
#define QPL_UTIL_HPP

#include <stdexcept>

#include "qpl/qpl.h"
#include "qpl/cpp_api/util/exceptions.hpp"
#include "analytics/analytics_defs.hpp"

namespace qpl {
namespace util {

/**
 * @defgroup HL_UTIL Utilities
 * @ingroup HL_PRIVATE
 * @{
 * @brief Contains utility functions and entities
 */

/**
 * @brief Returns enum representation of output format
 */
static inline auto integer_to_qpl_output_format(uint32_t value) -> qpl_out_format {
    switch (value) {
        case 1: {
            return qpl_ow_nom;
        }
        case 8: {
            return qpl_ow_8;
        }
        case 16: {
            return qpl_ow_16;
        }
        case 32: {
            return qpl_ow_32;
        }
        default: {
            return qpl_ow_nom;
        }
    }
}

/**
 * @brief Returns enum representation of output bit width format
 */
static inline auto integer_to_output_format(uint32_t value) -> ml::analytics::output_bit_width_format_t {
    using namespace qpl::ml::analytics;
    switch (value) {
        case 1: {
            return output_bit_width_format_t::same_as_input;
        }
        case 8: {
            return output_bit_width_format_t::bits_8;
        }
        case 16: {
            return output_bit_width_format_t::bits_16;
        }
        case 32: {
            return output_bit_width_format_t::bits_32;
        }
        default: {
            return output_bit_width_format_t::same_as_input;
        }
    }
}

/**
 * @brief Maps high-level execution path to low-level one
 */
static inline auto execution_path_to_qpl_path(execution_path path) -> qpl_path_t {
    switch (path) {
        case execution_path::hardware: {
            return qpl_path_hardware;
        }
        case execution_path::software: {
            return qpl_path_software;
        }
        default: {
            return qpl_path_auto;
        }
    }
}

/**
 * @brief Maps high-level enum to low-level one
 */
static inline auto parser_to_qpl_parser(parsers parser) -> qpl_parser {
    switch (parser) {
        case parsers::little_endian_packed_array: {
            return qpl_p_le_packed_array;
        }
        case parsers::big_endian_packed_array: {
            return qpl_p_be_packed_array;
        }
        default: {
            return qpl_p_parquet_rle;
        }
    }
}

/**
 * @brief Maps high-level enum to middle-level one
 */
static inline auto parser_to_ml_parser(const parsers parser) noexcept -> ml::analytics::stream_format_t {
    switch (parser) {
        case parsers::little_endian_packed_array: {
            return ml::analytics::stream_format_t::le_format;
        }
        case parsers::big_endian_packed_array: {
            return ml::analytics::stream_format_t::be_format;
        }
        case parsers::parquet_rle: {
            return ml::analytics::stream_format_t::prle_format;
        }
        default: {
            return ml::analytics::stream_format_t::le_format;
        }
    }
}

/**
 * @brief Maps high-level comparator to low-level one
 */
static inline auto comparator_to_qpl_operation(comparators comparator, bool is_inclusive) -> qpl_operation {
    switch (comparator) {
        case comparators::less: {
            return is_inclusive ? qpl_op_scan_le : qpl_op_scan_lt;
        }
        case comparators::greater: {
            return is_inclusive ? qpl_op_scan_ge : qpl_op_scan_gt;
        }
        case comparators::equals: {
            return qpl_op_scan_eq;
        }
        default: {
            return qpl_op_scan_ne;
        }
    }
}

/**
 * @brief Function to get number of elements for bit vector output format
 */
static inline auto get_output_elements_as_bits(qpl_job *job,
                                               uint32_t output_vector_bit_width) noexcept -> uint32_t {
    if (job->out_bit_width == qpl_ow_nom) {
        const uint32_t valid_last_bits = job->last_bit_offset;

        return (job->total_out - (valid_last_bits ? 1 : 0)) * byte_bit_length + valid_last_bits;
    } else {
        return (job->total_out * byte_bit_length) / output_vector_bit_width;
    }
}

/**
 * @brief Function to get number of elements for bit vector output format
 */
static inline auto get_output_elements_as_bits(ml::analytics::analytic_operation_result_t *result,
                                               ml::analytics::output_bit_width_format_t out_bit_width_format,
                                               uint32_t output_vector_bit_width) noexcept -> uint32_t {
    if (out_bit_width_format == ml::analytics::output_bit_width_format_t::same_as_input) {
        const uint32_t valid_last_bits = result->last_bit_offset_;

        return (result->output_bytes_ - (valid_last_bits ? 1 : 0)) * byte_bit_length + valid_last_bits;
    } else {
        return (result->output_bytes_ * byte_bit_length) / output_vector_bit_width;
    }
}

/**
 * @brief Function to get number of elements for custom output format
 */
static inline auto get_output_elements_as_is(qpl_job *job,
                                             uint32_t input_vector_bit_width,
                                             uint32_t output_vector_bit_width) noexcept -> uint32_t {
    if (job->out_bit_width == qpl_ow_nom) {
        switch (input_vector_bit_width) {
            case byte_bit_length:
            case short_bit_length:
            case uint_bit_length: {
                return (job->total_out * byte_bit_length) / input_vector_bit_width;
            }
            default: {
                const uint32_t valid_last_bits   = job->last_bit_offset;
                const uint32_t output_bits_count = (job->total_out - (valid_last_bits ? 1 : 0))
                                                   * byte_bit_length + valid_last_bits;

                return output_bits_count / input_vector_bit_width;
            }
        }
    } else {
        return (job->total_out * byte_bit_length) / output_vector_bit_width;
    }
}

/**
 * @brief Function to get number of elements for custom output format
 */
static inline auto get_output_elements_as_is(ml::analytics::analytic_operation_result_t *result,
                                             ml::analytics::output_bit_width_format_t out_bit_width_format,
                                             uint32_t input_vector_bit_width,
                                             uint32_t output_vector_bit_width) noexcept -> uint32_t {
    if (out_bit_width_format == ml::analytics::output_bit_width_format_t::same_as_input) {
        switch (input_vector_bit_width) {
            case byte_bit_length:
            case short_bit_length:
            case uint_bit_length: {
                return (result->output_bytes_ * byte_bit_length) / input_vector_bit_width;
            }
            default: {
                const uint32_t valid_last_bits   = result->last_bit_offset_;
                const uint32_t output_bits_count = (result->output_bytes_ - (valid_last_bits ? 1 : 0))
                                                   * byte_bit_length + valid_last_bits;

                return output_bits_count / input_vector_bit_width;
            }
        }
    } else {
        return (result->output_bytes_ * byte_bit_length) / output_vector_bit_width;
    }
}

/**
 * @brief Configures analytic job with enabled decompression
 */
static inline void add_decompression_flags_to_job(bool gzip_mode, qpl_job *job) noexcept {
    job->flags |= QPL_FLAG_DECOMPRESS_ENABLE;

    if (gzip_mode) {
        job->flags |= QPL_FLAG_GZIP_MODE;
    } else {
        job->flags &= ~QPL_FLAG_GZIP_MODE;
    }
}

/** @} */

} // namespace util
} // namespace qpl

#endif // QPL_UTIL_HPP
