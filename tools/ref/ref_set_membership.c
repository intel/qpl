/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/14/2018
 * Contains an implementation of the @ref ref_set_membership
 */

#include "ref_bit_rev.h"
#include "ref_count.h"
#include "ref_convert.h"
#include "ref_scan.h"
#include "ref_checksums.h"
#include "stdbool.h"

/**
 * @defgroup REFERENCE_SET_MEMBERSHIP Set Membership
 * @ingroup REFERENCE_PRIVATE
 * @{
 * @brief Contains helper functions for the @ref ref_set_membership
 */

/**
 * @todo
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_prepare_job(qpl_job *const qpl_job_ptr);

/**
 * @todo
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_set_membership_le_be(qpl_job *const qpl_job_ptr);

/**
 * @todo
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_set_membership_prle(qpl_job *const qpl_job_ptr);

/**
 * @todo
 * @param source1_ptr
 * @param source2_ptr
 * @param source_bit_width
 * @param number_of_elements
 * @param low_value
 * @param high_value
 * @param destination_ptr
 * @return
 */
REF_INLINE qpl_status own_set_membership(const uint32_t *const source1_ptr,
                                         const uint8_t *const source2_ptr,
                                         uint32_t source_bit_width,
                                         uint32_t number_of_elements,
                                         uint32_t low_value,
                                         uint32_t high_value,
                                         uint32_t *const destination_ptr);

/**
 * @todo
 * @param source_ptr
 * @param number_of_elements
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_set_membership_output_to_format(const uint32_t *const source_ptr,
                                                          uint32_t number_of_elements,
                                                          qpl_job *const qpl_job_ptr);

/** @} */

qpl_status ref_set_membership(qpl_job *const qpl_job_ptr) {
    REF_CHECK_FUNC_STS(own_prepare_job(qpl_job_ptr));

    if (0 == qpl_job_ptr->available_out) {
        // Update required fields in Job structure
        qpl_job_ptr->total_in     = qpl_job_ptr->available_in;
        qpl_job_ptr->next_in_ptr += qpl_job_ptr->available_in;
        qpl_job_ptr->available_in = 0;
        return QPL_STS_OK;
    }

    if (qpl_job_ptr->flags & QPL_FLAG_SRC2_BE) {
        ref_reverse_bits_8u(qpl_job_ptr->next_src2_ptr, qpl_job_ptr->available_src2);
    }

    // Destination zeroing is required only in case of bit vector output, and only for the last byte
    if (qpl_ow_nom == qpl_job_ptr->out_bit_width) {
        uint32_t nSetBytes = QPL_MIN(REF_BIT_2_BYTE(qpl_job_ptr->num_input_elements), qpl_job_ptr->available_out);
        qpl_job_ptr->next_out_ptr[nSetBytes - 1u] = 0u;
    }

    switch (qpl_job_ptr->parser) {
        case qpl_p_be_packed_array: {
            return own_set_membership_le_be(qpl_job_ptr);
        }
        case qpl_p_le_packed_array: {
            return own_set_membership_le_be(qpl_job_ptr);
        }
        case qpl_p_parquet_rle: {
            return own_set_membership_prle(qpl_job_ptr);
        }
        default: {
            return QPL_STS_PARSER_ERR;
        }
    }
}

REF_INLINE qpl_status own_set_membership_le_be(qpl_job *const qpl_job_ptr) {
    // Status of the process
    qpl_status status;

    // Start of the source1 vector
    uint8_t *source1_ptr = qpl_job_ptr->next_in_ptr;

    // Start of the source2 vector
    uint8_t *source2_ptr = qpl_job_ptr->next_src2_ptr;

    // Width of one element of the source vector
    uint32_t source_bit_width = qpl_job_ptr->src1_bit_width;

    // Number of elements in the source vector
    uint32_t number_of_elements = qpl_job_ptr->num_input_elements;

    // Const value to drop low bits
    uint32_t low_value = qpl_job_ptr->param_low;

    // Const value to drop high bits
    uint32_t high_value = qpl_job_ptr->param_high;

    // Extracted elements from source_ptr vector
    uint32_t *extracted_ptr = (uint32_t *) malloc((uint64_t) number_of_elements * sizeof(uint32_t));

    // Results of the operation
    uint32_t *results_ptr = (uint32_t *) malloc((uint64_t) number_of_elements * sizeof(uint32_t));

    // Convert source vector's elements to uint32_t format
    status = ref_convert_to_32u_le_be(source1_ptr,
                                      0,
                                      source_bit_width,
                                      number_of_elements,
                                      extracted_ptr,
                                      qpl_job_ptr->parser);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    // Main action
    status = own_set_membership(extracted_ptr,
                                source2_ptr,
                                source_bit_width,
                                number_of_elements,
                                low_value,
                                high_value,
                                results_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    // Update crc and xor checksum fields
    update_checksums(qpl_job_ptr);

    // Store result
    status = own_set_membership_output_to_format(results_ptr, number_of_elements, qpl_job_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    REF_FREE_PTR2(extracted_ptr, results_ptr);

    return status;
}

REF_INLINE qpl_status own_set_membership_prle(qpl_job *const qpl_job_ptr) {
    // Status of the process
    qpl_status status;

    // Start of the source1 vector
    uint8_t *source1_ptr = qpl_job_ptr->next_in_ptr;

    // End of the source1 vector
    uint8_t *source1_end_ptr = source1_ptr + qpl_job_ptr->available_in;

    // Start of the source2 vector
    uint8_t *source2_ptr = qpl_job_ptr->next_src2_ptr;

    // Extract source bit width
    uint32_t source_bit_width = *source1_ptr;

    // Const value to drop low bits
    uint32_t low_value = qpl_job_ptr->param_low;

    // Const value to drop high bits
    uint32_t high_value = qpl_job_ptr->param_high;

    // Bytes available in source_ptr vector
    uint32_t available_bytes = qpl_job_ptr->available_in;

    // Number of elements to process
    uint32_t number_of_elements;

    // Gettin number of elements
    status = ref_count_elements_prle(source1_ptr, source1_end_ptr, &number_of_elements, available_bytes);

    if (QPL_STS_OK != status) {
        return status;
    }

    // We should process qpl_job_ptr->num_input_elements, not less
    if (number_of_elements < qpl_job_ptr->num_input_elements) {
        return QPL_STS_SRC_IS_SHORT_ERR;
    }

    // Extracted elements from source_ptr vector
    uint32_t *extracted_ptr = (uint32_t *) malloc((uint64_t) number_of_elements * sizeof(uint32_t));

    // Results of the operation
    uint32_t *results_ptr = (uint32_t *) malloc((uint64_t) number_of_elements * sizeof(uint32_t));

    // Convert source vector's elements to uint32_t format
    status = ref_convert_to_32u_prle(source1_ptr, source1_end_ptr, extracted_ptr, &available_bytes);

    // we should process only qpl_job_ptr->num_input_elements, not more
    if (number_of_elements > qpl_job_ptr->num_input_elements) {
        number_of_elements = qpl_job_ptr->num_input_elements;
    }

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    // We have to process required number of elements or how much we have
    number_of_elements = QPL_MIN(qpl_job_ptr->num_input_elements, number_of_elements);

    // Main action
    status = own_set_membership(extracted_ptr,
                                source2_ptr,
                                source_bit_width,
                                number_of_elements,
                                low_value,
                                high_value,
                                results_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    // Update crc and xor checksum fields
    update_checksums(qpl_job_ptr);

    // Store result
    status = own_set_membership_output_to_format(results_ptr, number_of_elements, qpl_job_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    REF_FREE_PTR2(extracted_ptr, results_ptr);

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_set_membership(const uint32_t *const source1_ptr,
                                         const uint8_t *const source2_ptr,
                                         uint32_t source_bit_width,
                                         uint32_t number_of_elements,
                                         uint32_t low_value,
                                         uint32_t high_value,
                                         uint32_t *const destination_ptr) {
    // Temporary value
    uint32_t value;

    for (uint32_t i = 0; i < number_of_elements; ++i) {
        // Get current value
        value = source1_ptr[i];

        // Drop low bits
        value >>= low_value;

        // Drop high bits
        value &= ~(UINT64_MAX << (source_bit_width - low_value - high_value));

        // Check element by mask and store result
        destination_ptr[i] = (source2_ptr[value / 8u] & (1u << (value & 7u))) ? 1u : 0u;
    }

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_set_membership_output_to_format(const uint32_t *const source_ptr,
                                                          uint32_t number_of_elements,
                                                          qpl_job *const qpl_job_ptr) {
    qpl_status status = QPL_STS_OK;

    uint8_t *destination_ptr     = qpl_job_ptr->next_out_ptr;
    uint8_t *destination_end_ptr = destination_ptr + qpl_job_ptr->available_out;

    uint32_t output_bytes     = qpl_job_ptr->available_out;
    uint32_t source_bit_width = QPL_ONE_32U;
    uint32_t element_index    = qpl_job_ptr->initial_output_index;

    uint32_t output_format = (qpl_job_ptr->flags & QPL_FLAG_OUT_BE) + qpl_job_ptr->out_bit_width;

    bool output_be = (bool) (qpl_job_ptr->flags & QPL_FLAG_OUT_BE);

    if (qpl_ow_nom == qpl_job_ptr->out_bit_width) {
        // Check if destination has enough length
        REF_BAD_ARG_RET((output_bytes < REF_BIT_2_BYTE(number_of_elements)), QPL_STS_DST_IS_SHORT_ERR);
        qpl_job_ptr->last_bit_offset = number_of_elements & REF_MAX_BIT_IDX;
    }


    // Store values
    for (uint32_t i = 0; i < number_of_elements; ++i) {
        status = ref_store_result(source_ptr[i],
                                  i,
                                  &destination_ptr,
                                  destination_end_ptr,
                                  &element_index,
                                  output_format);

        if (QPL_STS_OK != status) {
            return status;
        }
    }

    if (QPL_STS_OK != status) {
        return status;
    }


    // Update required fields in Job structure
    if (qpl_ow_nom == output_format) {
        qpl_job_ptr->total_out = (uint32_t) REF_BIT_2_BYTE(number_of_elements);
    } else {
        qpl_job_ptr->total_out = (uint32_t) (ref_count_non_zero_elements_32u(source_ptr, number_of_elements)
                                             * REF_FMT_2_BYTES(output_format));
    }

    qpl_job_ptr->total_in     = qpl_job_ptr->available_in;
    qpl_job_ptr->next_in_ptr += qpl_job_ptr->total_in;
    qpl_job_ptr->next_out_ptr += qpl_job_ptr->total_out;
    qpl_job_ptr->available_in = 0;
    qpl_job_ptr->available_out -= qpl_job_ptr->total_out;

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_prepare_job(qpl_job *const qpl_job_ptr) {
    REF_BAD_PTR_RET(qpl_job_ptr);
    REF_BAD_PTR3_RET(qpl_job_ptr->next_in_ptr, qpl_job_ptr->next_out_ptr, qpl_job_ptr->next_src2_ptr);
    REF_BAD_SIZE_RET(qpl_job_ptr->available_in);
    REF_BAD_SIZE_RET(qpl_job_ptr->available_src2);
    REF_BAD_SIZE_RET(qpl_job_ptr->num_input_elements);

    REF_BAD_ARG_RET(REF_MAX_BIT_WIDTH < qpl_job_ptr->src1_bit_width || qpl_job_ptr->src1_bit_width < QPL_ONE_32U,
                    QPL_STS_BIT_WIDTH_ERR);
    REF_BAD_ARG_RET(qpl_job_ptr->src1_bit_width < qpl_job_ptr->param_high + qpl_job_ptr->param_low,
                    QPL_STS_DROP_BITS_OVERFLOW_ERR);

    uint32_t source2_required_size = 1u << (qpl_job_ptr->src1_bit_width - qpl_job_ptr->param_low - qpl_job_ptr->param_high);
    source2_required_size = REF_BIT_2_BYTE(source2_required_size);

    REF_BAD_ARG_RET(qpl_job_ptr->available_src2 < source2_required_size, QPL_STS_SRC2_IS_SHORT_ERR);
    REF_BAD_ARG_RET((QPL_ONE_32U != qpl_job_ptr->src2_bit_width), QPL_STS_BIT_WIDTH_ERR);
    REF_BAD_ARG_RET((qpl_op_set_membership != qpl_job_ptr->op), QPL_STS_OPERATION_ERR);
    REF_BAD_ARG_RET((qpl_p_parquet_rle < qpl_job_ptr->parser), QPL_STS_PARSER_ERR);

    // Update job's fields
    qpl_job_ptr->next_in_ptr += qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->available_in -= qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->total_in        = qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->last_bit_offset = 0;

    return QPL_STS_OK;
}
