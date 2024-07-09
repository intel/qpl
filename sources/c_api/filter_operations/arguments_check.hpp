/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_MIDDLE_LAYER_C_WRAPPER_ARGUMENTS_CHECK_HPP_
#define QPL_SOURCES_MIDDLE_LAYER_C_WRAPPER_ARGUMENTS_CHECK_HPP_

#include "util/checkers.hpp"
#include "own_defs.h"
#include "analytics/analytics_defs.hpp"
#include "util/util.hpp"
#include "job.hpp"
#include "analytics/input_stream.hpp"
#include "common/defs.hpp"


namespace qpl::job {
namespace details {
template <qpl_operation operation>
inline qpl_status validate_analytic_buffers(const qpl_job *const job_ptr) noexcept {
    if (nullptr == job_ptr || nullptr == job_ptr->next_in_ptr || nullptr == job_ptr->next_out_ptr) {
        return QPL_STS_NULL_PTR_ERR;
    }

    if (0U == job_ptr->available_in || 0U == job_ptr->available_out || 0U == job_ptr->num_input_elements) {
        return QPL_STS_SIZE_ERR;
    }

    if (ml::bad_argument::buffers_overlap(job_ptr->next_in_ptr, job_ptr->available_in,
                                      job_ptr->next_out_ptr, job_ptr->available_out)) {
        return QPL_STS_BUFFER_OVERLAP_ERR;
    }


    if constexpr(operation == qpl_op_expand ||
                 operation == qpl_op_select) {
        QPL_BAD_PTR_RET(job_ptr->next_src2_ptr)
        QPL_BAD_SIZE_RET(job_ptr->available_src2)

        if (ml::bad_argument::buffers_overlap(job_ptr->next_in_ptr, job_ptr->available_in,
                                          job_ptr->next_src2_ptr, job_ptr->available_src2)) {
            return QPL_STS_BUFFER_OVERLAP_ERR;
        }

        if (ml::bad_argument::buffers_overlap(job_ptr->next_src2_ptr, job_ptr->available_src2,
                                          job_ptr->next_out_ptr, job_ptr->available_out)) {
            return QPL_STS_BUFFER_OVERLAP_ERR;
        }

        if (job_ptr->drop_initial_bytes) {
            return QPL_STS_DROP_BYTES_ERR;
        }

    } else {
        if ((job_ptr->available_in < job_ptr->drop_initial_bytes && !(job_ptr->flags & QPL_FLAG_DECOMPRESS_ENABLE)) ||
            job_ptr->drop_initial_bytes > UINT16_MAX) {
            return QPL_STS_DROP_BYTES_ERR;
        }
    }

    return QPL_STS_OK;
}

namespace filter::common {
static inline qpl_status bad_arguments_check(const qpl_job *const job_ptr) {
    if ((job_ptr->out_bit_width < qpl_ow_nom) || (job_ptr->out_bit_width > qpl_ow_32)) {
        return QPL_STS_OUT_FORMAT_ERR;
    }

    // Check if the output bit width is nominal and the force array output flag is set
    if ((job_ptr->flags & QPL_FLAG_FORCE_ARRAY_OUTPUT) && job_ptr->out_bit_width == qpl_ow_nom) {
        // If the output bit width is nominal and the force array output flag is set, return an error
        return QPL_STS_OUT_FORMAT_ERR;
    }

    // Check if force array output mod is available when the force array output flag is set
    if ((job_ptr->flags & QPL_FLAG_FORCE_ARRAY_OUTPUT) && is_force_array_output_supported(job_ptr) == false) {
        // If the force array output mod flag is set, return unsupported error
        return QPL_STS_NOT_SUPPORTED_MODE_ERR;
    }

    // Check if running on hardware path when the force array output flag is set
    if ((job_ptr->flags & QPL_FLAG_FORCE_ARRAY_OUTPUT) && job_ptr->data_ptr.path != qpl_path_hardware) {
        // If the force array output mod flag is set and NOT running on hardware path, return unsupported error
        return QPL_STS_NOT_SUPPORTED_MODE_ERR;
    }

    uint32_t   source_bit_width            = job_ptr->src1_bit_width;
    const bool source_bit_width_is_unknown = (qpl_p_parquet_rle == job_ptr->parser &&
                                            (QPL_FLAG_DECOMPRESS_ENABLE & job_ptr->flags));

    if (qpl_p_parquet_rle == job_ptr->parser &&
        false == source_bit_width_is_unknown) {
        source_bit_width = static_cast<uint32_t>(job_ptr->next_in_ptr[0]);
    }

    if (false == source_bit_width_is_unknown &&
        (source_bit_width < 1U || source_bit_width > 32U)) {
        return QPL_STS_BIT_WIDTH_ERR;
    }

    if (job_ptr->parser > qpl_p_parquet_rle) {
        return QPL_STS_PARSER_ERR;
    }

    return QPL_STS_OK;
}
} // namespace filter::common

} // namespace details

using namespace qpl::ml;

template <>
inline qpl_status bad_arguments_check<qpl_operation::qpl_op_select>(const qpl_job *const job_ptr) noexcept {
    QPL_BADARG_RET((qpl_op_select != job_ptr->op), QPL_STS_OPERATION_ERR)
    QPL_BADARG_RET((1U != job_ptr->src2_bit_width), QPL_STS_BIT_WIDTH_ERR)

    QPL_BADARG_RET(job_ptr->initial_output_index, QPL_STS_INVALID_PARAM_ERR);

    if (job_ptr->parser != qpl_p_parquet_rle && !(job_ptr->flags & QPL_FLAG_DECOMPRESS_ENABLE)) {
        const uint64_t expected_source_byte_length = util::bit_to_byte((uint64_t)job_ptr->num_input_elements * (uint64_t)job_ptr->src1_bit_width);

        QPL_BADARG_RET((expected_source_byte_length > (uint64_t)job_ptr->available_in), QPL_STS_SRC_IS_SHORT_ERR)
    }

    const uint32_t expected_mask_byte_length = util::bit_to_byte(job_ptr->num_input_elements);
    QPL_BADARG_RET((expected_mask_byte_length > job_ptr->available_src2), QPL_STS_SRC_IS_SHORT_ERR)

    if ((qpl_ow_nom != job_ptr->out_bit_width) &&
        (1U == job_ptr->src1_bit_width)) {
        uint32_t max_possible_index = OWN_MAX_32U;
        if (qpl_ow_32 != job_ptr->out_bit_width) {
            max_possible_index = (qpl_ow_8 == job_ptr->out_bit_width) ? 0xFF : OWN_MAX_16U;
        }

        if (((uint64_t) job_ptr->initial_output_index + (uint64_t) job_ptr->num_input_elements - 1U)
            > (uint64_t) max_possible_index) {
            return QPL_STS_OUTPUT_OVERFLOW_ERR;
        }
    }

    return QPL_STS_OK;
}

template <>
inline qpl_status bad_arguments_check<qpl_operation::qpl_op_expand>(const qpl_job *const job_ptr) noexcept {
    QPL_BADARG_RET((qpl_op_expand != job_ptr->op), QPL_STS_OPERATION_ERR);
    QPL_BADARG_RET((1U != job_ptr->src2_bit_width), QPL_STS_BIT_WIDTH_ERR);

    QPL_BADARG_RET(job_ptr->initial_output_index, QPL_STS_INVALID_PARAM_ERR);

    // num_input_elements reflect elements in source-2 for expand operation
    const uint32_t expected_mask_byte_length = util::bit_to_byte(job_ptr->num_input_elements);
    QPL_BADARG_RET((expected_mask_byte_length > job_ptr->available_src2), QPL_STS_SRC_IS_SHORT_ERR);

    return QPL_STS_OK;
}

template <>
inline qpl_status bad_arguments_check<qpl_operation::qpl_op_extract>(const qpl_job *const job_ptr) noexcept {
    if ((qpl_p_parquet_rle != job_ptr->parser) &&
        !(QPL_FLAG_DECOMPRESS_ENABLE & job_ptr->flags)) {
        const uint64_t input_bits = (uint64_t)job_ptr->num_input_elements * (uint64_t)job_ptr->src1_bit_width;

        if (util::bit_to_byte(input_bits) > (uint64_t)job_ptr->available_in) {
            return QPL_STS_SRC_IS_SHORT_ERR;
        }
    }

    if ((qpl_ow_nom != job_ptr->out_bit_width) &&
        (1U == job_ptr->src1_bit_width)) {
        uint32_t max_possible_index = OWN_MAX_32U;
        if (qpl_ow_32 != job_ptr->out_bit_width) {
            max_possible_index = (qpl_ow_8 == job_ptr->out_bit_width) ? 0xFF : OWN_MAX_16U;
        }

        if (((uint64_t) job_ptr->initial_output_index + (uint64_t) job_ptr->num_input_elements - 1U)
            > (uint64_t) max_possible_index) {
            return QPL_STS_OUTPUT_OVERFLOW_ERR;
        }
    }

    return QPL_STS_OK;
}

template <>
inline qpl_status bad_arguments_check<qpl_operation::qpl_op_scan_eq>(const qpl_job *const job_ptr) noexcept {
    if (qpl_ow_nom == job_ptr->out_bit_width) {
        if (util::bit_to_byte(job_ptr->num_input_elements) > job_ptr->available_out) {
            return QPL_STS_DST_IS_SHORT_ERR;
        }
    }

    if ((qpl_p_parquet_rle != job_ptr->parser) &&
        !(QPL_FLAG_DECOMPRESS_ENABLE & job_ptr->flags)) {
        const uint64_t input_bits = (uint64_t)job_ptr->num_input_elements * (uint64_t)job_ptr->src1_bit_width;

        if (util::bit_to_byte(input_bits) > (uint64_t)job_ptr->available_in) {
            return QPL_STS_SRC_IS_SHORT_ERR;
        }
    }

    if (qpl_ow_nom != job_ptr->out_bit_width) {
        uint32_t max_possible_index = OWN_MAX_32U;
        if (qpl_ow_32 != job_ptr->out_bit_width) {
            max_possible_index = (qpl_ow_8 == job_ptr->out_bit_width) ? 0xFF : OWN_MAX_16U;
        }

        if (((uint64_t) job_ptr->initial_output_index + (uint64_t) job_ptr->num_input_elements - 1U)
            > (uint64_t) max_possible_index) {
            return QPL_STS_OUTPUT_OVERFLOW_ERR;
        }
    }

    return QPL_STS_OK;
}

template<>
inline qpl_status validate_operation<qpl_op_scan_eq>(const qpl_job *const job_ptr) noexcept {
    OWN_QPL_CHECK_STATUS(details::validate_analytic_buffers<qpl_op_scan_eq>(job_ptr));
    OWN_QPL_CHECK_STATUS(details::filter::common::bad_arguments_check(job_ptr));
    OWN_QPL_CHECK_STATUS(bad_arguments_check<qpl_op_scan_eq>(job_ptr));

    return QPL_STS_OK;
}

template<>
inline qpl_status validate_operation<qpl_op_extract>(const qpl_job *const job_ptr) noexcept {
    OWN_QPL_CHECK_STATUS(details::validate_analytic_buffers<qpl_op_extract>(job_ptr));
    OWN_QPL_CHECK_STATUS(details::filter::common::bad_arguments_check(job_ptr))
    OWN_QPL_CHECK_STATUS(bad_arguments_check<qpl_op_extract>(job_ptr));

    return QPL_STS_OK;
}

template<>
inline qpl_status validate_operation<qpl_op_select>(const qpl_job *const job_ptr) noexcept {
    OWN_QPL_CHECK_STATUS(details::validate_analytic_buffers<qpl_op_select>(job_ptr));
    OWN_QPL_CHECK_STATUS(details::filter::common::bad_arguments_check(job_ptr));
    OWN_QPL_CHECK_STATUS(bad_arguments_check<qpl_op_select>(job_ptr));

    return QPL_STS_OK;
}

template<>
inline qpl_status validate_operation<qpl_op_expand>(const qpl_job *const job_ptr) noexcept {
    OWN_QPL_CHECK_STATUS(details::validate_analytic_buffers<qpl_op_expand>(job_ptr));
    OWN_QPL_CHECK_STATUS(details::filter::common::bad_arguments_check(job_ptr));
    OWN_QPL_CHECK_STATUS(bad_arguments_check<qpl_op_expand>(job_ptr));

    return QPL_STS_OK;
}

} // namespace qpl::job

namespace qpl::ml::analytics {
static inline auto validate_input_stream(const input_stream_t &stream,
                                         uint32_t low_border_width = 1,
                                         uint32_t upper_border_width = 32) noexcept -> uint32_t {
    if (stream.is_compressed()) {
        if (stream.decompression_status() != status_list::ok &&
            stream.decompression_status() != status_list::more_output_needed) {
            return stream.decompression_status();
        }

        if (stream.bit_width() < low_border_width || stream.bit_width() > upper_border_width) {
            return status_list::bit_width_error;
        }
    }

    return status_list::ok;
}

static inline auto get_stream_format(const qpl_parser parser) noexcept -> stream_format_t {
    switch (parser) {
        case qpl_p_le_packed_array: {
            return stream_format_t::le_format;
        }
        case qpl_p_be_packed_array: {
            return stream_format_t::be_format;
        }
        case qpl_p_parquet_rle: {
            return stream_format_t::prle_format;
        }
        default: {
            return stream_format_t::le_format;
        }
    }
}

static inline auto update_job(qpl_job *job_ptr, const analytic_operation_result_t operation_result) -> void {
    job_ptr->available_out -= job_ptr->total_out;
    job_ptr->total_in     = job_ptr->available_in;
    job_ptr->available_in = 0;

    job_ptr->first_index_min_value = operation_result.aggregates_.min_value_;
    job_ptr->last_index_max_value  = operation_result.aggregates_.max_value_;
    job_ptr->sum_value             = operation_result.aggregates_.sum_;
    job_ptr->last_bit_offset       = operation_result.last_bit_offset_;
    job_ptr->xor_checksum          = operation_result.checksums_.xor_;
    job_ptr->crc                   = operation_result.checksums_.crc32_;
}
} // namespace qpl::ml::analytics

#endif //QPL_SOURCES_MIDDLE_LAYER_C_WRAPPER_ARGUMENTS_CHECK_HPP_
