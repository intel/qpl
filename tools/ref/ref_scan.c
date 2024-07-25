/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/26/2018
 * Contains an implementation of the @ref ref_compare function
 *
 */

#include "ref_scan.h"

#include "own_ref_defs.h"
#include "ref_checksums.h"
#include "ref_convert.h"
#include "ref_count.h"
#include "ref_prle.h"

/**
 * @defgroup REFERENCE_SCAN Scan
 * @ingroup REFERENCE_PRIVATE
 * @{
 * @brief Contains helper functions for the @ref ref_compare
 */

/**
 * @todo
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_prepare_job(qpl_job* const qpl_job_ptr);

/**
 * @todo
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_compare_le_be(qpl_job* const qpl_job_ptr);

/**
 * @todo
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_compare_prle(qpl_job* const qpl_job_ptr);

/**
 * @todo
 * @param source_ptr
 * @param number_of_elements
 * @param destination_ptr
 * @param low_value
 * @param high_value
 * @param operation
 * @return
 */
REF_INLINE qpl_status own_compare(const uint32_t* const source_ptr, uint32_t number_of_elements,
                                  uint32_t* const destination_ptr, uint32_t low_value, uint32_t high_value,
                                  qpl_operation operation);

/**
 * @param  source_bit_width  input vector bit width
 * @param  input_param       param_low or param_high parameter
 *
 * @brief If param_low or param_high bit width is more than input vector bitwidth, HW ignores most significant bits,
 *        this function emulates this behaviour
 *
 * @return corrected input_param
 */
REF_INLINE uint32_t own_correct_parameter(uint32_t source_bit_width, uint32_t input_param);

/**
 * @todo
 * @param source_ptr
 * @param number_of_elements
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_compare_output_to_format(const uint32_t* const source_ptr, uint32_t number_of_elements,
                                                   qpl_job* const qpl_job_ptr);

/** @} */

qpl_status ref_compare(qpl_job* const qpl_job_ptr) {
    REF_CHECK_FUNC_STS(own_prepare_job(qpl_job_ptr));

    switch (qpl_job_ptr->parser) {
        case qpl_p_be_packed_array: {
            return own_compare_le_be(qpl_job_ptr);
        }
        case qpl_p_le_packed_array: {
            return own_compare_le_be(qpl_job_ptr);
        }
        case qpl_p_parquet_rle: {
            return own_compare_prle(qpl_job_ptr);
        }
        default: {
            return QPL_STS_PARSER_ERR;
        }
    }
}

REF_INLINE qpl_status own_compare_le_be(qpl_job* const qpl_job_ptr) {
    uint8_t* source_ptr = qpl_job_ptr->next_in_ptr;

    uint32_t available_bytes    = qpl_job_ptr->available_in;
    uint32_t low_value          = qpl_job_ptr->param_low;  // value to compare with
    uint32_t high_value         = qpl_job_ptr->param_high; // value to compare with
    uint32_t source_bit_width   = qpl_job_ptr->src1_bit_width;
    uint32_t number_of_elements = qpl_job_ptr->num_input_elements;

    uint64_t bit_length = (uint64_t)number_of_elements * (uint64_t)source_bit_width;

    // check input length
    REF_BAD_ARG_RET((available_bytes < REF_BIT_2_BYTE(bit_length)), QPL_STS_SRC_IS_SHORT_ERR);

    uint32_t* extracted_ptr = (uint32_t*)malloc((uint64_t)number_of_elements * sizeof(uint32_t));
    uint32_t* results_ptr   = (uint32_t*)malloc((uint64_t)number_of_elements * sizeof(uint32_t));

    // convert source vector's elements to uint32_t format
    qpl_status status = ref_convert_to_32u_le_be(source_ptr, 0, source_bit_width, number_of_elements, extracted_ptr,
                                                 qpl_job_ptr->parser);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    // Correct low_value and high_value
    uint32_t corrected_low_value  = own_correct_parameter(source_bit_width, low_value);
    uint32_t corrected_high_value = own_correct_parameter(source_bit_width, high_value);

    // main action
    status = own_compare(extracted_ptr, number_of_elements, results_ptr, corrected_low_value, corrected_high_value,
                         qpl_job_ptr->op);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    // Update crc and xor checksum fields
    update_checksums(qpl_job_ptr);

    // store result
    status = own_compare_output_to_format(results_ptr, number_of_elements, qpl_job_ptr);

    REF_FREE_PTR2(extracted_ptr, results_ptr);

    return status;
}

REF_INLINE qpl_status own_compare_prle(qpl_job* const qpl_job_ptr) {
    uint8_t* source_ptr     = qpl_job_ptr->next_in_ptr;
    uint8_t* source_end_ptr = source_ptr + qpl_job_ptr->available_in;

    uint32_t low_value          = qpl_job_ptr->param_low;    // value to compare with
    uint32_t high_value         = qpl_job_ptr->param_high;   // value to compare with
    uint32_t available_bytes    = qpl_job_ptr->available_in; // available bytes in source vector after conversion
    uint32_t number_of_elements = 0U;                        // count elements in source vector
    uint32_t source_bit_width   = (uint32_t)(*source_ptr);   // extract source bit width

    // Getting number of elements
    qpl_status status = ref_count_elements_prle(source_ptr, source_end_ptr, &number_of_elements, available_bytes);

    if (QPL_STS_OK != status) { return status; }

    // We should process qpl_job_ptr->num_input_elements, not less
    if (number_of_elements < qpl_job_ptr->num_input_elements) { return QPL_STS_SRC_IS_SHORT_ERR; }

    uint32_t* extracted_ptr = (uint32_t*)malloc((uint64_t)number_of_elements * sizeof(uint32_t));
    uint32_t* results_ptr   = (uint32_t*)malloc((uint64_t)number_of_elements * sizeof(uint32_t));

    // convert source vector's elements to uint32_t format
    status = ref_convert_to_32u_prle(source_ptr, source_end_ptr, extracted_ptr, &available_bytes);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    // We have to process required number of elements or how much we have
    number_of_elements = QPL_MIN(qpl_job_ptr->num_input_elements, number_of_elements);

    // Correct low_value and high_value
    uint32_t corrected_low_value  = own_correct_parameter(source_bit_width, low_value);
    uint32_t corrected_high_value = own_correct_parameter(source_bit_width, high_value);

    // main action
    status = own_compare(extracted_ptr, number_of_elements, results_ptr, corrected_low_value, corrected_high_value,
                         qpl_job_ptr->op);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    // Update crc and xor checksum fields
    update_checksums(qpl_job_ptr);

    // store result
    status = own_compare_output_to_format(results_ptr, number_of_elements, qpl_job_ptr);

    REF_FREE_PTR2(extracted_ptr, results_ptr);

    return status;
}

REF_INLINE qpl_status own_compare(const uint32_t* const source_ptr, uint32_t number_of_elements,
                                  uint32_t* const destination_ptr, uint32_t low_value, uint32_t high_value,
                                  qpl_operation operation) {
    uint32_t comparison_result = 0U;

    for (uint32_t i = 0U; i < number_of_elements; ++i) {
        switch (operation) {
            case qpl_op_scan_lt: {
                comparison_result = (source_ptr[i] < low_value) ? QPL_ONE_32U : 0U;
                break;
            }
            case qpl_op_scan_le: {
                comparison_result = (source_ptr[i] <= low_value) ? QPL_ONE_32U : 0U;
                break;
            }
            case qpl_op_scan_gt: {
                comparison_result = (source_ptr[i] > low_value) ? QPL_ONE_32U : 0U;
                break;
            }
            case qpl_op_scan_ge: {
                comparison_result = (source_ptr[i] >= low_value) ? QPL_ONE_32U : 0U;
                break;
            }
            case qpl_op_scan_eq: {
                comparison_result = (source_ptr[i] == low_value) ? QPL_ONE_32U : 0U;
                break;
            }
            case qpl_op_scan_ne: {
                comparison_result = (source_ptr[i] != low_value) ? QPL_ONE_32U : 0U;
                break;
            }
            case qpl_op_scan_range: {
                comparison_result = ((source_ptr[i] >= low_value) && (source_ptr[i] <= high_value)) ? QPL_ONE_32U : 0U;
                break;
            }
            case qpl_op_scan_not_range: {
                comparison_result = ((source_ptr[i] < low_value) || (source_ptr[i] > high_value)) ? QPL_ONE_32U : 0U;
                break;
            }
            default: {
                return QPL_STS_OPERATION_ERR;
            }
        }
        destination_ptr[i] = comparison_result;
    }

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_compare_output_to_format(const uint32_t* const source_ptr, uint32_t number_of_elements,
                                                   qpl_job* const qpl_job_ptr) {
    qpl_status status              = QPL_STS_OK;
    uint8_t*   destination_ptr     = qpl_job_ptr->next_out_ptr;
    uint8_t*   destination_end_ptr = destination_ptr + qpl_job_ptr->available_out;

    const uint32_t* current_ptr = source_ptr;

    uint32_t output_format         = (qpl_job_ptr->flags & QPL_FLAG_OUT_BE) + qpl_job_ptr->out_bit_width;
    uint32_t number_of_input_bytes = qpl_job_ptr->available_in;
    uint32_t current_index         = qpl_job_ptr->initial_output_index;

    for (uint32_t i = 0U; i < number_of_elements; ++i) {
        status = ref_store_result(current_ptr[i], i, &destination_ptr, destination_end_ptr, &current_index,
                                  output_format);

        if (QPL_STS_OK != status) { return status; }
    }

    // Check if destination length has enough bytes
    if (qpl_ow_nom == qpl_job_ptr->out_bit_width) {
        REF_BAD_ARG_RET((REF_BIT_2_BYTE(number_of_elements) > qpl_job_ptr->available_out), QPL_STS_DST_IS_SHORT_ERR);
        qpl_job_ptr->last_bit_offset = qpl_job_ptr->num_input_elements & REF_MAX_BIT_IDX;
    }

    if (qpl_ow_nom == qpl_job_ptr->out_bit_width) {
        destination_ptr += REF_BIT_2_BYTE(qpl_job_ptr->num_input_elements);

        // if at least 1 bit is written and the last byte is not "full"
        if ((QPL_ONE_32U < qpl_job_ptr->last_bit_offset) && (destination_ptr > qpl_job_ptr->next_out_ptr)) {
            uint8_t bit_mask = 0U;

            if (qpl_job_ptr->flags & QPL_FLAG_OUT_BE) {
                bit_mask =
                        (uint8_t) ~((REF_HIGH_BIT_MASK >> (qpl_job_ptr->last_bit_offset - QPL_ONE_32U)) - QPL_ONE_32U);
            } else {
                bit_mask = (uint8_t)((REF_LOW_BIT_MASK << qpl_job_ptr->last_bit_offset) - QPL_ONE_32U);
            }

            destination_ptr[-1] &= bit_mask;
        }
    }

    // update required fields in Job structure
    qpl_job_ptr->total_in += number_of_input_bytes;
    qpl_job_ptr->total_out = (uint32_t)(destination_ptr - qpl_job_ptr->next_out_ptr);
    qpl_job_ptr->next_in_ptr += number_of_input_bytes;
    qpl_job_ptr->next_out_ptr += qpl_job_ptr->total_out;
    qpl_job_ptr->available_in -= number_of_input_bytes;
    qpl_job_ptr->available_out -= qpl_job_ptr->total_out;

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_prepare_job(qpl_job* const qpl_job_ptr) {
    REF_BAD_PTR_RET(qpl_job_ptr);
    REF_BAD_PTR2_RET(qpl_job_ptr->next_in_ptr, qpl_job_ptr->next_out_ptr);
    REF_BAD_SIZE_RET(qpl_job_ptr->available_in);
    REF_BAD_SIZE_RET(qpl_job_ptr->available_out);
    REF_BAD_SIZE_RET(qpl_job_ptr->num_input_elements);
    REF_BAD_ARG_RET((qpl_job_ptr->available_in < qpl_job_ptr->drop_initial_bytes), QPL_STS_SIZE_ERR);
    REF_BAD_ARG_RET(
            (((QPL_ONE_32U > qpl_job_ptr->src1_bit_width) || (REF_MAX_BIT_WIDTH < qpl_job_ptr->src1_bit_width)) &&
             (qpl_p_parquet_rle != qpl_job_ptr->parser)),
            QPL_STS_BIT_WIDTH_ERR);
    REF_BAD_ARG_RET((qpl_ow_32 < qpl_job_ptr->out_bit_width), QPL_STS_OUT_FORMAT_ERR);
    REF_BAD_ARG_RET((qpl_p_parquet_rle < qpl_job_ptr->parser), QPL_STS_PARSER_ERR);

    // Update job's fields
    qpl_job_ptr->next_in_ptr += qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->available_in -= qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->total_in        = qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->last_bit_offset = 0;

    return QPL_STS_OK;
}

REF_INLINE uint32_t own_correct_parameter(uint32_t source_bit_width, uint32_t input_param) {
    return (input_param & (UINT32_MAX >> (REF_MAX_BIT_WIDTH - source_bit_width)));
}
