/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/09/2018
 * Contains an implementation of the @ref ref_expand function
 */

#include "ref_checksums.h"
#include "ref_convert.h"
#include "ref_count.h"
#include "ref_mask.h"
#include "ref_prle.h"
#include "ref_store.h"
#include "stdbool.h"

/**
 * @defgroup REFERENCE_EXPAND Expand
 * @ingroup REFERENCE_PRIVATE
 * @{
 * @brief Contains helper functions for the @ref ref_expand
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
REF_INLINE qpl_status own_expand_le_be(qpl_job* const qpl_job_ptr);

/**
 * @todo
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_expand_prle(qpl_job* const qpl_job_ptr);

/**
 * @todo
 * @param source_ptr
 * @param mask_ptr
 * @param number_of_bits
 * @param number_of_elements
 * @param destination_ptr
 * @return
 */
REF_INLINE qpl_status own_expand(const uint32_t* const source_ptr, const uint32_t* const mask_ptr,
                                 uint32_t number_of_bits, uint32_t number_of_elements, uint32_t* const destination_ptr);

/**
 * @todo
 * @param source_ptr
 * @param number_of_elements
 * @param source_bit_width
 * @param available_bytes
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_expand_output_to_format(const uint32_t* const source_ptr, uint32_t number_of_elements,
                                                  uint32_t source_bit_width, uint32_t available_bytes,
                                                  qpl_job* const qpl_job_ptr);

/** @} */

qpl_status ref_expand(qpl_job* const qpl_job_ptr) {
    REF_CHECK_FUNC_STS(own_prepare_job(qpl_job_ptr));

    switch (qpl_job_ptr->parser) {
        case qpl_p_be_packed_array: {
            return own_expand_le_be(qpl_job_ptr);
        }
        case qpl_p_le_packed_array: {
            return own_expand_le_be(qpl_job_ptr);
        }
        case qpl_p_parquet_rle: {
            return own_expand_prle(qpl_job_ptr);
        }
        default: {
            return QPL_STS_PARSER_ERR;
        }
    }
}

REF_INLINE qpl_status own_expand_le_be(qpl_job* const qpl_job_ptr) {
    // Not enough data available in source buffer
    REF_BAD_ARG_RET((qpl_job_ptr->available_src2 < REF_BIT_2_BYTE(qpl_job_ptr->num_input_elements)),
                    QPL_STS_SRC_IS_SHORT_ERR);

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

    // Src2 input format - LE (0) or BE (>0)
    uint32_t mask_be = qpl_job_ptr->flags & QPL_FLAG_SRC2_BE;

    // Bit length of number of elements
    uint64_t bit_length = (uint64_t)number_of_elements * (uint64_t)source_bit_width;

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
    status = own_expand(extracted_ptr, extracted_mask_ptr, number_of_elements, number_of_elements, results_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
        return status;
    }

    // Update crc and xor checksum fields
    update_checksums(qpl_job_ptr);

    // Store result
    status = own_expand_output_to_format(results_ptr, number_of_elements, source_bit_width, available_bytes,
                                         qpl_job_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
        return status;
    }

    REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
    return QPL_STS_OK;
}

REF_INLINE qpl_status own_expand_prle(qpl_job* const qpl_job_ptr) {

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

    // Getting number of elements
    qpl_status status = ref_count_elements_prle(source_ptr, source_end_ptr, &number_of_elements, available_bytes);

    if (QPL_STS_OK != status) { return status; }

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

    // Extract mask bits
    status = ref_extract_mask_bits(source_mask_ptr, source_length, mask_be, extracted_mask_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
        return status;
    }

    // Main action
    status = own_expand(extracted_ptr, extracted_mask_ptr, source_length, number_of_elements, results_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
        return status;
    }

    // Update crc and xor checksum fields
    update_checksums(qpl_job_ptr);

    // Store result
    status = own_expand_output_to_format(results_ptr, source_length, source_bit_width, available_bytes, qpl_job_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
        return status;
    }

    REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
    return QPL_STS_OK;
}

REF_INLINE qpl_status own_expand(const uint32_t* const source_ptr, const uint32_t* const mask_ptr,
                                 uint32_t number_of_bits, uint32_t number_of_elements,
                                 uint32_t* const destination_ptr) {
    uint32_t element_index = 0U;

    for (uint32_t i = 0U; i < number_of_bits; ++i) {
        if (!mask_ptr[i]) {
            // Store zero
            destination_ptr[i] = 0U;
        } else {
            // Check length
            if (number_of_elements <= element_index) { return QPL_STS_SRC_IS_SHORT_ERR; }

            // Store value
            destination_ptr[i] = source_ptr[element_index++];
        }
    }

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_expand_output_to_format(const uint32_t* const source_ptr, uint32_t number_of_elements,
                                                  uint32_t source_bit_width, uint32_t available_bytes,
                                                  qpl_job* const qpl_job_ptr) {
    // Current destination vector
    uint8_t* destination_ptr = qpl_job_ptr->next_out_ptr;

    // End of the destination vector
    const uint8_t* destination_end_ptr = destination_ptr + qpl_job_ptr->available_out;

    // Element index in destination vector
    uint32_t element_index = qpl_job_ptr->initial_output_index;

    // Input LE or BE
    bool input_be = (bool)(qpl_p_be_packed_array == qpl_job_ptr->parser);

    // Output LE or BE
    bool output_be = (bool)(qpl_job_ptr->flags & QPL_FLAG_OUT_BE);

    // Output format
    qpl_out_format output_format = (qpl_out_format)qpl_job_ptr->out_bit_width;

    // Number of output bytes
    uint32_t output_bytes = 0U;

    // Store result
    qpl_status status = ref_store_values(source_ptr, number_of_elements, source_bit_width, destination_ptr,
                                         destination_end_ptr, output_be, output_format, &element_index);

    if (QPL_STS_OK != status) { return status; }

    // Update required fields in job structure
    uint32_t output_elements = element_index - qpl_job_ptr->initial_output_index;
    status                   = ref_get_output_bytes(&qpl_job_ptr->last_bit_offset, output_elements, source_bit_width,
                                                    qpl_job_ptr->available_out, output_format, &output_bytes);

    if (QPL_STS_OK != status) { return status; }

    qpl_job_ptr->total_in += qpl_job_ptr->available_in;
    qpl_job_ptr->total_out   = output_bytes;
    qpl_job_ptr->next_in_ptr = (uint8_t*)(source_ptr + (qpl_job_ptr->available_in) - available_bytes);
    qpl_job_ptr->next_out_ptr += output_bytes;
    qpl_job_ptr->available_in = available_bytes;
    qpl_job_ptr->available_out -= output_bytes;

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_prepare_job(qpl_job* const qpl_job_ptr) {
    REF_BAD_PTR_RET(qpl_job_ptr);
    REF_BAD_PTR3_RET(qpl_job_ptr->next_in_ptr, qpl_job_ptr->next_src2_ptr, qpl_job_ptr->next_out_ptr);
    REF_BAD_SIZE_RET(qpl_job_ptr->available_src2);
    REF_BAD_SIZE_RET(qpl_job_ptr->available_out);
    REF_BAD_SIZE_RET(qpl_job_ptr->num_input_elements);
    REF_BAD_ARG_RET(
            (((QPL_ONE_32U > qpl_job_ptr->src1_bit_width) || (REF_MAX_BIT_WIDTH < qpl_job_ptr->src1_bit_width)) &&
             (qpl_p_parquet_rle != qpl_job_ptr->parser)),
            QPL_STS_BIT_WIDTH_ERR);
    REF_BAD_ARG_RET((qpl_job_ptr->available_in < qpl_job_ptr->drop_initial_bytes), QPL_STS_SIZE_ERR);
    REF_BAD_ARG_RET((QPL_ONE_32U != qpl_job_ptr->src2_bit_width), QPL_STS_BIT_WIDTH_ERR);
    REF_BAD_ARG_RET((qpl_op_expand != qpl_job_ptr->op), QPL_STS_OPERATION_ERR);
    REF_BAD_ARG_RET((qpl_p_parquet_rle < qpl_job_ptr->parser), QPL_STS_PARSER_ERR);

    // Update job's fields
    qpl_job_ptr->next_in_ptr += qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->available_in -= qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->total_in        = qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->last_bit_offset = 0;

    return QPL_STS_OK;
}
