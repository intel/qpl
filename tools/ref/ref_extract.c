/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/19/2018
 * Contains an implementation of the @ref ref_extract function
 *
 */

#include "ref_checksums.h"
#include "ref_convert.h"
#include "ref_count.h"
#include "ref_prle.h"
#include "ref_store.h"
#include "stdbool.h"

#define QPL_MIN(a, b) (((a) < (b)) ? (a) : (b)) /**< Simple maximal value idiom */

/**
 * @defgroup REFERENCE_EXTRACT Extract
 * @ingroup REFERENCE_PRIVATE
 * @{
 * @brief Contains helper functions for the @ref ref_extract
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
REF_INLINE qpl_status own_extract_le_be(qpl_job* const qpl_job_ptr);

/**
 * @todo
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_extract_prle(qpl_job* const qpl_job_ptr);

/**
 * @todo
 * @param source_ptr
 * @param destination_ptr
 * @param first_index
 * @param last_index
 * @return
 */
REF_INLINE qpl_status own_extract(const uint32_t* const source_ptr, uint32_t* const destination_ptr,
                                  uint32_t first_index, uint32_t last_index);

/**
 * @todo
 * @param source_ptr
 * @param number_of_elements
 * @param source_bit_width
 * @param available_bytes
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_extract_output_to_format(const uint32_t* const source_ptr, uint32_t number_of_elements,
                                                   uint32_t source_bit_width, qpl_job* const qpl_job_ptr);

/** @} */

qpl_status ref_extract(qpl_job* const qpl_job_ptr) {
    REF_CHECK_FUNC_STS(own_prepare_job(qpl_job_ptr));

    switch (qpl_job_ptr->parser) {
        case qpl_p_be_packed_array: {
            return own_extract_le_be(qpl_job_ptr);
        }
        case qpl_p_le_packed_array: {
            return own_extract_le_be(qpl_job_ptr);
        }
        case qpl_p_parquet_rle: {
            return own_extract_prle(qpl_job_ptr);
        }
        default: {
            return QPL_STS_PARSER_ERR;
        }
    }
}

REF_INLINE qpl_status own_extract_le_be(qpl_job* const qpl_job_ptr) {

    // Start of the source vector
    uint8_t* source_ptr = qpl_job_ptr->next_in_ptr;

    // Width of one element of the source vector
    uint32_t source_bit_width = qpl_job_ptr->src1_bit_width;

    // Number of elements in the source vector
    uint32_t number_of_elements = qpl_job_ptr->num_input_elements;

    // number of bytes available in source_ptr
    uint32_t available_bytes = qpl_job_ptr->available_in;

    // Start index of the operation
    uint32_t first_index = qpl_job_ptr->param_low;

    // End index of the operation
    uint32_t last_index = QPL_MIN(qpl_job_ptr->param_high, number_of_elements);

    // Number of elements in destination_ptr
    uint32_t destination_length = last_index - first_index + 1U;

    // Bit length of number of elements
    uint64_t bit_length = (uint64_t)number_of_elements * (uint64_t)source_bit_width;

    // Check if source vector has enough bits
    REF_BAD_ARG_RET((available_bytes < REF_BIT_2_BYTE(bit_length)), QPL_STS_SRC_IS_SHORT_ERR);

    // Extracted elements from source_ptr vector
    uint32_t* extracted_ptr = (uint32_t*)malloc((uint64_t)number_of_elements * sizeof(uint32_t));

    // Results of the operation
    uint32_t* results_ptr = (uint32_t*)malloc((uint64_t)number_of_elements * sizeof(uint32_t));

    // Convert source vector's elements to uint32_t format
    qpl_status status = ref_convert_to_32u_le_be(source_ptr, 0, source_bit_width, number_of_elements, extracted_ptr,
                                                 qpl_job_ptr->parser);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    // Update crc and xor checksum fields
    update_checksums(qpl_job_ptr);

    // Main action
    status = own_extract(extracted_ptr, results_ptr, first_index, last_index + 1);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    // Store result
    status = own_extract_output_to_format(results_ptr, destination_length, source_bit_width, qpl_job_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    REF_FREE_PTR2(extracted_ptr, results_ptr);

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_extract_prle(qpl_job* const qpl_job_ptr) {

    // Start of the source vector
    uint8_t* source_ptr = qpl_job_ptr->next_in_ptr;

    // End of the source vector
    uint8_t* source_end_ptr = source_ptr + qpl_job_ptr->available_in;

    // Extract source bit width
    uint32_t source_bit_width = *source_ptr;

    // Bytes available in source_ptr vector
    uint32_t available_bytes = qpl_job_ptr->available_in;

    // Start index of the operation
    uint32_t first_index = qpl_job_ptr->param_low;

    // End index of the operation
    uint32_t last_index = qpl_job_ptr->param_high;

    // Number of elements to process
    uint32_t number_of_elements = 0U;

    // Getting number of elements
    qpl_status status = ref_count_elements_prle(source_ptr, source_end_ptr, &number_of_elements, available_bytes);

    if (QPL_STS_OK != status) { return status; }

    // We should process qpl_job_ptr->num_input_elements, not less
    if (number_of_elements < qpl_job_ptr->num_input_elements) { return QPL_STS_SRC_IS_SHORT_ERR; }

    // Number of elements in destination_ptr
    uint32_t destination_length = 0U;

    // Due to the fact that the variable is unsigned,
    // we must add an additional check for uint underflow
    if (number_of_elements < first_index) {
        destination_length = 0U;
    } else {
        destination_length = QPL_MIN(last_index - first_index + 1, number_of_elements - first_index);
    }

    // Extracted elements from source_ptr vector
    uint32_t* extracted_ptr = (uint32_t*)malloc((uint64_t)number_of_elements * sizeof(uint32_t));

    // Results of the operation
    uint32_t* results_ptr = (uint32_t*)malloc((uint64_t)number_of_elements * sizeof(uint32_t));

    // Convert source vector's elements to uint32_t format
    status = ref_convert_to_32u_prle(source_ptr, source_end_ptr, extracted_ptr, &available_bytes);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    // We should process only qpl_job_ptr->num_input_elements, not more
    if (number_of_elements > qpl_job_ptr->num_input_elements) { number_of_elements = qpl_job_ptr->num_input_elements; }

    // Main action
    status = own_extract(extracted_ptr, results_ptr, first_index, QPL_MIN(last_index + 1, number_of_elements));

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    // Update crc and xor checksum fields
    update_checksums(qpl_job_ptr);

    // Store result
    status = own_extract_output_to_format(results_ptr, destination_length, source_bit_width, qpl_job_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, results_ptr);
        return status;
    }

    REF_FREE_PTR2(extracted_ptr, results_ptr);

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_extract(const uint32_t* const source_ptr, uint32_t* const destination_ptr,
                                  uint32_t first_index, uint32_t last_index) {
    uint32_t index = 0U;

    for (uint32_t i = first_index; i < last_index; ++i) {
        destination_ptr[index++] = source_ptr[i];
    }

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_extract_output_to_format(const uint32_t* const source_ptr, uint32_t number_of_elements,
                                                   uint32_t source_bit_width, qpl_job* const qpl_job_ptr) {

    // Destination vector
    uint8_t* destination_ptr = qpl_job_ptr->next_out_ptr;

    // End of the destination vector
    const uint8_t* destination_end_ptr = destination_ptr + qpl_job_ptr->available_out;

    // Index of the last element
    uint32_t element_index = qpl_job_ptr->initial_output_index;

    // Number of output bytes
    uint32_t output_bytes = 0U;

    // Output format
    qpl_out_format output_format = (qpl_out_format)qpl_job_ptr->out_bit_width;

    // Output LE or BE
    bool output_be = (bool)(qpl_job_ptr->flags & QPL_FLAG_OUT_BE);

    // Store result
    qpl_status status = ref_store_values(source_ptr, number_of_elements, source_bit_width, destination_ptr,
                                         destination_end_ptr, output_be, output_format, &element_index);

    if (QPL_STS_OK != status) { return status; }

    // Update required fields in job structure
    uint32_t output_elements = element_index - qpl_job_ptr->initial_output_index;
    status                   = ref_get_output_bytes(&qpl_job_ptr->last_bit_offset, output_elements, source_bit_width,
                                                    qpl_job_ptr->available_out, output_format, &output_bytes);

    if (QPL_STS_OK != status) { return status; }

    qpl_job_ptr->total_in  = qpl_job_ptr->available_in;
    qpl_job_ptr->total_out = output_bytes;
    qpl_job_ptr->next_in_ptr += qpl_job_ptr->available_in;
    qpl_job_ptr->next_out_ptr += output_bytes;
    qpl_job_ptr->available_in = 0;
    qpl_job_ptr->available_out -= output_bytes;

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_prepare_job(qpl_job* const qpl_job_ptr) {
    REF_BAD_PTR_RET(qpl_job_ptr);
    REF_BAD_PTR2_RET(qpl_job_ptr->next_in_ptr, qpl_job_ptr->next_out_ptr);
    REF_BAD_SIZE_RET(qpl_job_ptr->available_in);
    REF_BAD_SIZE_RET(qpl_job_ptr->available_out);
    REF_BAD_SIZE_RET(qpl_job_ptr->num_input_elements);
    REF_BAD_ARG_RET(
            (((QPL_ONE_32U > qpl_job_ptr->src1_bit_width) || (REF_MAX_BIT_WIDTH < qpl_job_ptr->src1_bit_width)) &&
             (qpl_p_parquet_rle != qpl_job_ptr->parser)),
            QPL_STS_BIT_WIDTH_ERR);
    REF_BAD_ARG_RET((qpl_job_ptr->available_in < qpl_job_ptr->drop_initial_bytes), QPL_STS_SIZE_ERR);
    REF_BAD_ARG_RET((qpl_op_extract != qpl_job_ptr->op), QPL_STS_OPERATION_ERR);
    REF_BAD_ARG_RET((qpl_p_parquet_rle < qpl_job_ptr->parser), QPL_STS_PARSER_ERR);
    REF_BAD_ARG_RET((qpl_job_ptr->param_high < qpl_job_ptr->param_low), QPL_STS_SIZE_ERR);

    // Update job's fields
    qpl_job_ptr->next_in_ptr += qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->available_in -= qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->total_in        = qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->last_bit_offset = 0;

    return QPL_STS_OK;
}
