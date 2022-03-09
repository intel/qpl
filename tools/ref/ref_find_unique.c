/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/14/2018
 * Contains an implementation of the @ref ref_find_unique function
 */

#include "ref_count.h"
#include "ref_convert.h"
#include "ref_bit_rev.h"
#include "ref_prle.h"
#include "ref_store.h"
#include "ref_checksums.h"

#define QPL_MIN(a, b) (((a) < (b)) ? (a) : (b))    /**< Simple maximal value idiom */

/**
 * @defgroup REFERENCE_SET_FIND_UNIQUE Find Unique
 * @ingroup REFERENCE_PRIVATE
 * @{
 * @brief Contains helper functions for the @ref ref_find_unique
 */

/**
 * @brief Writes the result of the ref_find_unique function into destination buffer in necessary format
 *
 * @param[out]  pp_destination        - bit buffer with ref_find_unique  result
 * @param[in]   bit_vector_ptr        - bit vector with ref_find_unique preliminary results
 * @param[in]   destination_end_ptr   - end of bit buffer with qpl_find_unique  result
 * @param[in]   bit_vector_size       - bit vector size in bits
 * @param[in]   output_format         - format of the output data
 * @param[in]   initial_output_index  - initial output index
 *
 * @return
 *   - QPL_STS_OK
 *   - QPL_STS_DST_IS_SHORT_ERR
 *
 */
qpl_status ref_write_in_output_format(uint8_t **const pp_destination,
                                      uint8_t *const bit_vector_ptr,
                                      const uint8_t *const destination_end_ptr,
                                      uint64_t bit_vector_size,
                                      uint32_t output_format,
                                      uint32_t initial_output_index);

/**
 * @todo
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_prepare_job(qpl_job *qpl_job_ptr);

/**
 * @todo
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_find_unique_le_be(qpl_job *const qpl_job_ptr);

/**
 * @todo
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_find_unique_prle(qpl_job *const qpl_job_ptr);

/**
 * @todo
 * @param source_ptr
 * @param number_of_elements
 * @param source_bit_width
 * @param low_value
 * @param high_value
 * @param destination_ptr
 * @return
 */
REF_INLINE qpl_status own_find_unique(const uint32_t *const source_ptr,
                                      uint32_t number_of_elements,
                                      uint32_t source_bit_width,
                                      uint32_t low_value,
                                      uint32_t high_value,
                                      uint32_t *const destination_ptr);

/**
 * @todo
 * @param source_ptr
 * @param number_of_elements
 * @param source_bit_width
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_find_uniqueOutputToFormat(const uint32_t *const source_ptr,
                                                    uint32_t number_of_elements,
                                                    uint32_t source_bit_width,
                                                    qpl_job *const qpl_job_ptr);

/** @} */

qpl_status ref_find_unique(qpl_job *const qpl_job_ptr) {
    REF_CHECK_FUNC_STS(own_prepare_job(qpl_job_ptr));

    switch (qpl_job_ptr->parser) {
        case qpl_p_be_packed_array: {
            return own_find_unique_le_be(qpl_job_ptr);
        }
        case qpl_p_le_packed_array: {
            return own_find_unique_le_be(qpl_job_ptr);
        }
        case qpl_p_parquet_rle: {
            return own_find_unique_prle(qpl_job_ptr);
        }
        default: {
            return QPL_STS_PARSER_ERR;
        }
    }
}

qpl_status ref_write_in_output_format(uint8_t **const pp_destination,
                                      uint8_t *const bit_vector_ptr,
                                      const uint8_t *const destination_end_ptr,
                                      uint64_t bit_vector_size,
                                      uint32_t output_format,
                                      uint32_t initial_output_index) {
    switch (output_format) {
        case qpl_ow_nom: {
            (*pp_destination) += REF_SIZE_IN_BYTES(bit_vector_size);
            break;
        }
        case qpl_ow_nom | QPL_FLAG_OUT_BE: {
            (*pp_destination) += REF_SIZE_IN_BYTES(bit_vector_size);
            break;
        }
        case qpl_ow_8:
        case qpl_ow_8 | QPL_FLAG_OUT_BE: {
            uint8_t       *current_ptr = (uint8_t *) (*pp_destination);
            for (uint64_t i            = 0; i < bit_vector_size; i++) {
                if (REF_CHECK_BIT_IN_VECTOR(bit_vector_ptr, i)) {
                    REF_CHECK_PTR_END((uint8_t *) current_ptr,
                                      destination_end_ptr,
                                      sizeof(uint8_t),
                                      QPL_STS_DST_IS_SHORT_ERR);
                    REF_OVERVALUE_8U_RET(i, initial_output_index);
                    *current_ptr = (uint8_t) (i + initial_output_index);
                    current_ptr++;
                }
            }
            (*pp_destination) = (uint8_t *) current_ptr;
            break;
        }
        case qpl_ow_16: {
            uint16_t * current_ptr = (uint16_t *) (*pp_destination);
            for (uint64_t i = 0; i < bit_vector_size; i++) {
                if (REF_CHECK_BIT_IN_VECTOR(bit_vector_ptr, i)) {
                    REF_CHECK_PTR_END((uint8_t *) current_ptr,
                                      destination_end_ptr,
                                      sizeof(uint16_t),
                                      QPL_STS_DST_IS_SHORT_ERR);
                    REF_OVERVALUE_16U_RET(i, initial_output_index);
                    *current_ptr = (uint16_t) (i + initial_output_index);
                    current_ptr++;
                }
            }
            (*pp_destination) = (uint8_t *) current_ptr;
            break;
        }
        case qpl_ow_16 | QPL_FLAG_OUT_BE: {
            uint16_t * current_ptr = (uint16_t *) (*pp_destination);
            for (uint64_t i = 0; i < bit_vector_size; i++) {
                if (REF_CHECK_BIT_IN_VECTOR(bit_vector_ptr, i)) {
                    REF_CHECK_PTR_END((uint8_t *) current_ptr,
                                      destination_end_ptr,
                                      sizeof(uint16_t),
                                      QPL_STS_DST_IS_SHORT_ERR);
                    REF_OVERVALUE_16U_RET(i, initial_output_index);
                    *current_ptr = swap_bytes16((uint16_t) (i + initial_output_index));
                    current_ptr++;
                }
            }
            (*pp_destination) = (uint8_t *) current_ptr;
            break;
        }

        case qpl_ow_32: {
            uint32_t * current_ptr = (uint32_t *) (*pp_destination);
            for (uint64_t i = 0; i < bit_vector_size; i++) {
                if (REF_CHECK_BIT_IN_VECTOR(bit_vector_ptr, i)) {
                    REF_CHECK_PTR_END((uint8_t *) current_ptr,
                                      destination_end_ptr,
                                      sizeof(uint32_t),
                                      QPL_STS_DST_IS_SHORT_ERR);
                    REF_OVERVALUE_32U_RET(i, initial_output_index);
                    *current_ptr = (uint32_t) (i + initial_output_index);
                    current_ptr++;
                }
            }
            (*pp_destination) = (uint8_t *) current_ptr;
            break;
        }
        case qpl_ow_32 | QPL_FLAG_OUT_BE: {
            uint32_t * current_ptr = (uint32_t *) (*pp_destination);
            for (uint64_t i = 0; i < bit_vector_size; i++) {
                if (REF_CHECK_BIT_IN_VECTOR(bit_vector_ptr, i)) {
                    REF_CHECK_PTR_END((uint8_t *) current_ptr,
                                      destination_end_ptr,
                                      sizeof(uint32_t),
                                      QPL_STS_DST_IS_SHORT_ERR);
                    REF_OVERVALUE_32U_RET(i, initial_output_index);
                    *current_ptr = swap_bytes32((uint32_t) (i + initial_output_index));
                    current_ptr++;
                }
            }
            (*pp_destination) = (uint8_t *) current_ptr;
            break;
        }
    }

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_find_unique_le_be(qpl_job *const qpl_job_ptr) {
    // Status of the process
    qpl_status status;

    // Start of the source1 vector
    uint8_t *source_ptr = qpl_job_ptr->next_in_ptr;

    // Bytes available in source_ptr
    uint32_t available_bytes = qpl_job_ptr->available_in;

    // Width of one element of the source vector
    uint32_t source_bit_width = qpl_job_ptr->src1_bit_width;

    // Number of elements in the source vector
    uint32_t number_of_elements = qpl_job_ptr->num_input_elements;

    // Const value to drop low bits
    uint32_t low_value = qpl_job_ptr->param_low;

    // Const value to drop high bits
    uint32_t high_value = qpl_job_ptr->param_high;

    // Element length in bits
    uint64_t bit_length = (uint64_t) number_of_elements * (uint64_t) source_bit_width;

    // Check input length
    REF_BAD_ARG_RET((available_bytes < REF_BIT_2_BYTE(bit_length)), QPL_STS_SRC_IS_SHORT_ERR);

    // Extracted elements from source_ptr vector
    uint32_t * extracted_ptr = (uint32_t *) malloc((uint64_t) number_of_elements * sizeof(uint32_t));

    // Results of the operation
    uint32_t * results_ptr = (uint32_t *) malloc((uint64_t) number_of_elements * sizeof(uint32_t));

    // Convert source vector's elements to uint32_t format
    status = ref_convert_to_32u_le_be(source_ptr,
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
    status = own_find_unique(extracted_ptr, number_of_elements, source_bit_width, low_value, high_value, results_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    // Update crc and xor checksum fields
    update_checksums(qpl_job_ptr);

    // Store result
    status = own_find_uniqueOutputToFormat(results_ptr, number_of_elements, source_bit_width, qpl_job_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    REF_FREE_PTR2(extracted_ptr, results_ptr);

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_find_unique_prle(qpl_job *const qpl_job_ptr) {
    // Status of the process
    qpl_status status;

    // Start of the source1 vector
    uint8_t *source_ptr = qpl_job_ptr->next_in_ptr;

    // End of the source1 vector
    uint8_t *source_end_ptr = source_ptr + qpl_job_ptr->available_in;

    // Extract source bit width
    uint32_t source_bit_width = *source_ptr;

    // Const value to drop low bits
    uint32_t low_value = qpl_job_ptr->param_low;

    // Const value to drop high bits
    uint32_t high_value = qpl_job_ptr->param_high;

    // Bytes available in source_ptr vector
    uint32_t available_bytes = qpl_job_ptr->available_in;

    // Number of elements to process
    uint32_t number_of_elements;

    // Getting number of elements
    status = ref_count_elements_prle(source_ptr, source_end_ptr, &number_of_elements, available_bytes);

    if (QPL_STS_OK != status) {
        return status;
    }

    // We should process qpl_job_ptr->num_input_elements, not less
    if (number_of_elements < qpl_job_ptr->num_input_elements) {
        return QPL_STS_SRC_IS_SHORT_ERR;
    }

    // Extracted elements from source_ptr vector
    uint32_t * extracted_ptr = (uint32_t *) malloc((uint64_t) number_of_elements * sizeof(uint32_t));

    // Results of the operation
    uint32_t * results_ptr = (uint32_t *) malloc((uint64_t) number_of_elements * sizeof(uint32_t));

    // Convert source vector's elements to uint32_t format
    status = ref_convert_to_32u_prle(source_ptr, source_end_ptr, extracted_ptr, &available_bytes);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    // We have to process required number of elements or how much we have
    number_of_elements = QPL_MIN(qpl_job_ptr->num_input_elements, number_of_elements);

    // Main action
    status = own_find_unique(extracted_ptr, number_of_elements, source_bit_width, low_value, high_value, results_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    // Update crc and xor checksum fields
    update_checksums(qpl_job_ptr);

    // Store result
    status = own_find_uniqueOutputToFormat(results_ptr, number_of_elements, source_bit_width, qpl_job_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    REF_FREE_PTR2(extracted_ptr, results_ptr);

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_find_unique(const uint32_t *const source_ptr,
                                      uint32_t number_of_elements,
                                      uint32_t source_bit_width,
                                      uint32_t low_value,
                                      uint32_t high_value,
                                      uint32_t *const destination_ptr) {
    // Current value
    uint32_t value;

    for (uint32_t i = 0; i < number_of_elements; ++i) {
        // Get value
        value = source_ptr[i];

        // Drop low bits
        value >>= low_value;

        // Drop high bits
        value &= ~(UINT64_MAX << (source_bit_width - low_value - high_value));

        destination_ptr[i] = value;
    }

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_find_uniqueOutputToFormat(const uint32_t *const source_ptr,
                                                    uint32_t number_of_elements,
                                                    uint32_t source_bit_width,
                                                    qpl_job *const qpl_job_ptr) {
    qpl_status status;

    uint8_t *destination_ptr     = qpl_job_ptr->next_out_ptr;
    uint8_t *destination_end_ptr = destination_ptr + qpl_job_ptr->available_out;
    uint8_t *current_destination_ptr;

    uint8_t buffer[REF_FIND_UNIQUE_BUF_SIZE];

    uint64_t bit_vector_bits = (uint64_t) (QPL_ONE_64U << ((uint64_t) (qpl_job_ptr->src1_bit_width -
                                                                       qpl_job_ptr->param_low -
                                                                       qpl_job_ptr->param_high)));

    uint32_t output_index = qpl_job_ptr->initial_output_index;

    uint32_t output_format = (qpl_job_ptr->flags & QPL_FLAG_OUT_BE)
                                                     + qpl_job_ptr->out_bit_width;

    if (qpl_ow_nom == output_format || (qpl_ow_nom | QPL_FLAG_OUT_BE) == output_format) {
        current_destination_ptr = destination_ptr;
        ref_fill_by_zero_8u(destination_ptr, qpl_job_ptr->available_out);
    } else {
        current_destination_ptr = buffer;
        ref_fill_by_zero_8u(current_destination_ptr, REF_FIND_UNIQUE_BUF_SIZE);
    }

    for (uint32_t i = 0; i < number_of_elements; ++i) {
        status = ref_set_bit_by_index(source_ptr[i], current_destination_ptr, output_format);

        if (QPL_STS_OK != status) {
            return status;
        }
    }

    status = ref_write_in_output_format(&destination_ptr,
                                        current_destination_ptr,
                                        destination_end_ptr,
                                        bit_vector_bits,
                                        output_format,
                                        output_index);

    if (status != QPL_STS_OK) {
        qpl_job_ptr->total_in  = 0;
        qpl_job_ptr->total_out = 0;
        return status;
    }

    // Update required fields in Job structure
    if (qpl_ow_nom == qpl_job_ptr->out_bit_width) {
        qpl_job_ptr->last_bit_offset = bit_vector_bits & REF_MAX_BIT_IDX;
    }
    qpl_job_ptr->total_in     = qpl_job_ptr->available_in;
    qpl_job_ptr->total_out    = (uint32_t) (destination_ptr - qpl_job_ptr->next_out_ptr);
    qpl_job_ptr->next_in_ptr += qpl_job_ptr->total_in;
    qpl_job_ptr->next_out_ptr += qpl_job_ptr->total_out;
    qpl_job_ptr->available_in = 0;
    qpl_job_ptr->available_out -= qpl_job_ptr->total_out;

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_prepare_job(qpl_job *qpl_job_ptr) {
    REF_BAD_PTR_RET(qpl_job_ptr);
    REF_BAD_PTR2_RET(qpl_job_ptr->next_in_ptr, qpl_job_ptr->next_out_ptr);
    REF_BAD_SIZE_RET(qpl_job_ptr->available_in);
    REF_BAD_SIZE_RET(qpl_job_ptr->available_out);
    REF_BAD_SIZE_RET(qpl_job_ptr->num_input_elements);
    REF_BAD_ARG_RET(REF_MAX_BIT_WIDTH < qpl_job_ptr->src1_bit_width ||
                    qpl_job_ptr->src1_bit_width < QPL_ONE_32U,
                    QPL_STS_BIT_WIDTH_ERR);
    REF_BAD_ARG_RET(qpl_job_ptr->src1_bit_width <= qpl_job_ptr->param_high + qpl_job_ptr->param_low,
                    QPL_STS_DROP_BITS_OVERFLOW_ERR);
    REF_BAD_ARG_RET((qpl_op_find_unique != qpl_job_ptr->op), QPL_STS_OPERATION_ERR);
    REF_BAD_ARG_RET((qpl_p_parquet_rle < qpl_job_ptr->parser), QPL_STS_PARSER_ERR);

    uint64_t bit_vector_bits = QPL_ONE_64U
            << (uint64_t) (qpl_job_ptr->src1_bit_width - qpl_job_ptr->param_low - qpl_job_ptr->param_high);

    uint32_t bit_vector_bytes    = (uint32_t) REF_SIZE_IN_BYTES(bit_vector_bits);
    uint32_t output_bytes        = qpl_job_ptr->available_out;

    if (qpl_ow_nom == qpl_job_ptr->out_bit_width) {
        // Check if destination has enough length
        REF_BAD_ARG_RET((output_bytes < bit_vector_bytes), QPL_STS_DST_IS_SHORT_ERR);
    }

    // Update job's fields
    qpl_job_ptr->next_in_ptr += qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->available_in -= qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->total_in        = qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->last_bit_offset = 0;

    return QPL_STS_OK;
}
