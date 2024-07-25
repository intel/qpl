/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/19/2018
 * Contains an implementation of the @ref ref_select
 */

#include "ref_checksums.h"
#include "ref_convert.h"
#include "ref_copy.h"
#include "ref_count.h"
#include "ref_mask.h"
#include "ref_prle.h"
#include "ref_store.h"
#include "stdbool.h"

/**
 * @defgroup REFERENCE_SELECT Select
 * @ingroup REFERENCE_PRIVATE
 * @{
 * @brief Contains helper functions for the @ref ref_select
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
REF_INLINE qpl_status own_select_le_be(qpl_job* const qpl_job_ptr);

/**
 * @todo
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_select_prle(qpl_job* const qpl_job_ptr);

/**
 * @todo
 * @param source_ptr
 * @param number_of_elements
 * @param mask_ptr
 * @param destination_ptr
 * @param destination_length_ptr
 * @return
 */
REF_INLINE qpl_status own_select(const uint32_t* const source_ptr, uint32_t number_of_elements,
                                 const uint32_t* const mask_ptr, uint32_t* const destination_ptr,
                                 uint32_t* const destination_length_ptr);

/**
 * @todo
 * @param source_ptr
 * @param number_of_elements
 * @param source_bit_width
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_select_output_to_format(const uint32_t* const source_ptr, uint32_t number_of_elements,
                                                  uint32_t source_bit_width, qpl_job* const qpl_job_ptr);

/** @} */

qpl_status ref_select(qpl_job* const qpl_job_ptr) {
    REF_CHECK_FUNC_STS(own_prepare_job(qpl_job_ptr));

    switch (qpl_job_ptr->parser) {
        case qpl_p_be_packed_array: {
            return own_select_le_be(qpl_job_ptr);
        }
        case qpl_p_le_packed_array: {
            return own_select_le_be(qpl_job_ptr);
        }
        case qpl_p_parquet_rle: {
            return own_select_prle(qpl_job_ptr);
        }
        default: {
            return QPL_STS_PARSER_ERR;
        }
    }
}

REF_INLINE qpl_status own_select_le_be(qpl_job* const qpl_job_ptr) {
    // Start of the source vector
    uint8_t* source_ptr = qpl_job_ptr->next_in_ptr;

    // Start of the mask vector
    uint8_t* source_mask_ptr = qpl_job_ptr->next_src2_ptr;

    // Width of one element of the source vector
    uint32_t source_bit_width = qpl_job_ptr->src1_bit_width;

    // Number of elements in the source vector
    uint32_t number_of_elements = qpl_job_ptr->num_input_elements;

    // Number of bytes available in source_ptr
    uint32_t available_bytes = qpl_job_ptr->available_in;

    // Number of bytes available in mask
    uint32_t maskBytes = qpl_job_ptr->available_src2;

    // Src2 input format - LE (0) or BE (>0)
    uint32_t mask_be = qpl_job_ptr->flags & QPL_FLAG_SRC2_BE;

    // Number of elements in destination_ptr
    uint32_t destination_length = 0U;

    // Bit length of number of elements
    uint64_t bit_length = (uint64_t)number_of_elements * (uint64_t)source_bit_width;

    // Check input length
    REF_BAD_ARG_RET((available_bytes < REF_BIT_2_BYTE(bit_length)), QPL_STS_SRC_IS_SHORT_ERR);
    REF_BAD_ARG_RET((maskBytes < REF_BIT_2_BYTE(number_of_elements)), QPL_STS_SRC_IS_SHORT_ERR);
    REF_BAD_SIZE_RET(qpl_job_ptr->available_out);

    // Extracted elements from source_ptr vector
    uint32_t* extracted_ptr = (uint32_t*)malloc((uint64_t)number_of_elements * sizeof(uint32_t));

    // Extracted mask elements from source_mask_ptr vector
    uint32_t* extracted_mask_ptr = (uint32_t*)malloc((uint64_t)number_of_elements * sizeof(uint32_t));

    // Results of the operation
    uint32_t* results_ptr = (uint32_t*)malloc((uint64_t)number_of_elements * sizeof(uint32_t));

    // Convert source vector's elements to uint32_t format
    qpl_status status = ref_convert_to_32u_le_be(source_ptr, 0, source_bit_width, number_of_elements, extracted_ptr,
                                                 qpl_job_ptr->parser);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
        return status;
    }

    // Extract mask bits
    status = ref_extract_mask_bits(source_mask_ptr, number_of_elements, mask_be, extracted_mask_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
        return status;
    }

    // Main action
    status = own_select(extracted_ptr, number_of_elements, extracted_mask_ptr, results_ptr, &destination_length);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
        return status;
    }

    // Update crc and xor checksum fields
    update_checksums(qpl_job_ptr);

    // Store result
    status = own_select_output_to_format(results_ptr, destination_length, source_bit_width, qpl_job_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
        return status;
    }

    REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
    return QPL_STS_OK;
}

REF_INLINE qpl_status own_select_prle(qpl_job* const qpl_job_ptr) {

    // Start of the source vector
    uint8_t* source_ptr = qpl_job_ptr->next_in_ptr;

    // End of the source vector
    uint8_t* source_end_ptr = source_ptr + qpl_job_ptr->available_in;

    // Mask of the source vector
    uint8_t* source_mask_ptr = qpl_job_ptr->next_src2_ptr;

    // Extract source bit width
    uint32_t source_bit_width = *source_ptr;

    // Number of bytes in source_ptr vector
    uint32_t source_length = qpl_job_ptr->num_input_elements;

    // Mask input format - LE (0) or BE (>0)
    uint32_t mask_be = qpl_job_ptr->flags & QPL_FLAG_SRC2_BE;

    // Bytes available in source_ptr vector
    uint32_t available_bytes = qpl_job_ptr->available_in;

    // Number of elements to process
    uint32_t number_of_elements = 0U;

    // Number of elements in destination_ptr
    uint32_t destination_length = 0U;

    REF_BAD_SIZE_RET(qpl_job_ptr->available_out);

    // Getting number of elements
    qpl_status status = ref_count_elements_prle(source_ptr, source_end_ptr, &number_of_elements, available_bytes);

    if (QPL_STS_OK != status) { return status; }

    // We should process qpl_job_ptr->num_input_elements, not less
    if (number_of_elements < qpl_job_ptr->num_input_elements) { return QPL_STS_SRC_IS_SHORT_ERR; }

    // Extracted elements from source_ptr vector
    uint32_t* extracted_ptr = (uint32_t*)malloc((uint64_t)number_of_elements * sizeof(uint32_t));

    // Extracted mask elements from source_mask_ptr vector
    uint32_t* extracted_mask_ptr = (uint32_t*)malloc((uint64_t)source_length * sizeof(uint32_t));

    // Results of the operation
    uint32_t* results_ptr = (uint32_t*)malloc((uint64_t)source_length * sizeof(uint32_t));

    // Convert source vector's elements to uint32_t format
    status = ref_convert_to_32u_prle(source_ptr, source_end_ptr, extracted_ptr, &available_bytes);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
        return status;
    }

    // We should process only qpl_job_ptr->num_input_elements, not more
    if (number_of_elements > qpl_job_ptr->num_input_elements) { number_of_elements = qpl_job_ptr->num_input_elements; }

    // Extract mask bits
    status = ref_extract_mask_bits(source_mask_ptr, source_length, mask_be, extracted_mask_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
        return status;
    }

    // Main action
    status = own_select(extracted_ptr, QPL_MIN(number_of_elements, source_length), extracted_mask_ptr, results_ptr,
                        &destination_length);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
        return status;
    }

    // Update crc and xor checksum fields
    update_checksums(qpl_job_ptr);

    // Store result
    status = own_select_output_to_format(results_ptr, destination_length, source_bit_width, qpl_job_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
        return status;
    }

    REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
    return QPL_STS_OK;
}

REF_INLINE qpl_status own_select(const uint32_t* const source_ptr, uint32_t number_of_elements,
                                 const uint32_t* const mask_ptr, uint32_t* const destination_ptr,
                                 uint32_t* const destination_length_ptr) {
    // Current position in destination_ptr
    uint32_t index = 0U;

    for (uint32_t i = 0U; i < number_of_elements; ++i) {
        if (mask_ptr[i]) { destination_ptr[index++] = source_ptr[i]; }
    }

    // Save destination length
    (*destination_length_ptr) = index;

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_select_output_to_format(const uint32_t* const source_ptr, uint32_t number_of_elements,
                                                  uint32_t source_bit_width, qpl_job* const qpl_job_ptr) {

    // Destination vector
    uint8_t* destination_ptr = qpl_job_ptr->next_out_ptr;

    // End of the destination vector
    const uint8_t* destination_end_ptr = destination_ptr + qpl_job_ptr->available_out;

    // Number of bytes available in destination_ptr
    uint32_t output_bytes = qpl_job_ptr->available_out;

    // Output format qpl_ow_nom|8|16|32
    qpl_out_format output_format = qpl_job_ptr->out_bit_width;

    // Number of bits required in destination buffer for particular fmt
    uint32_t destination_bits = ref_fmt_2_bits(output_format, source_bit_width);

    // Bit index in destination_ptr
    uint32_t element_index = qpl_job_ptr->initial_output_index;

    // Input LE or BE
    bool input_be = (bool)(qpl_p_be_packed_array == qpl_job_ptr->parser);

    // Output LE or BE
    bool output_be = (bool)(qpl_job_ptr->flags & QPL_FLAG_OUT_BE);

    // Check if destination length has enough bytes
    REF_BAD_ARG_RET((REF_BIT_2_BYTE((uint64_t)number_of_elements * (uint64_t)destination_bits) > output_bytes),
                    QPL_STS_DST_IS_SHORT_ERR);

    // Store result
    qpl_status status = ref_store_values(source_ptr, number_of_elements, source_bit_width, destination_ptr,
                                         destination_end_ptr, output_be, output_format, &element_index);

    if (QPL_STS_OK != status) { return status; }

    // Update required fields in job structure
    uint32_t output_elements = element_index - qpl_job_ptr->initial_output_index;
    status                   = ref_get_output_bytes(&qpl_job_ptr->last_bit_offset, output_elements, source_bit_width,
                                                    qpl_job_ptr->available_out, output_format, &output_bytes);

    if (QPL_STS_OK != status) { return status; }

    // Update required fields in Job structure
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
    REF_BAD_PTR3_RET(qpl_job_ptr->next_in_ptr, qpl_job_ptr->next_src2_ptr, qpl_job_ptr->next_out_ptr);
    REF_BAD_SIZE_RET(qpl_job_ptr->available_in);
    REF_BAD_SIZE_RET(qpl_job_ptr->available_src2);
    REF_BAD_SIZE_RET(qpl_job_ptr->available_out);
    REF_BAD_SIZE_RET(qpl_job_ptr->num_input_elements);
    REF_BAD_ARG_RET(
            (((QPL_ONE_32U > qpl_job_ptr->src1_bit_width) || (REF_MAX_BIT_WIDTH < qpl_job_ptr->src1_bit_width)) &&
             (qpl_p_parquet_rle != qpl_job_ptr->parser)),
            QPL_STS_BIT_WIDTH_ERR);
    REF_BAD_ARG_RET((QPL_ONE_32U != qpl_job_ptr->src2_bit_width), QPL_STS_BIT_WIDTH_ERR);
    REF_BAD_ARG_RET((qpl_op_select != qpl_job_ptr->op), QPL_STS_OPERATION_ERR);
    REF_BAD_ARG_RET((qpl_p_parquet_rle < qpl_job_ptr->parser), QPL_STS_PARSER_ERR);
    REF_BAD_ARG_RET((qpl_job_ptr->available_in < qpl_job_ptr->drop_initial_bytes), QPL_STS_SIZE_ERR);

    // Update job's fields
    qpl_job_ptr->next_in_ptr += qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->available_in -= qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->total_in        = qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->last_bit_offset = 0;

    return QPL_STS_OK;
}
