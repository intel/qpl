/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/09/2018
 * Contains an implementation of the @ref ref_expand_rle function
 */

#include "own_ref_defs.h"
#include "qpl_api_ref.h"
#include "ref_checksums.h"
#include "ref_convert.h"
#include "ref_count.h"
#include "ref_prle.h"
#include "ref_store.h"
#include "stdbool.h"
#include "stdint.h"

/**
 * @defgroup REFERENCE_EXPAND_RLE Expand RLE
 * @ingroup REFERENCE_PRIVATE
 * @{
 * @brief Contains helper functions for the @ref ref_expand_rle
 */

#define OWN_BYTE_BIT_LEN     8                                     /**< @todo */
#define OWN_BIT_BUF_LEN      (sizeof(uint64_t) * OWN_BYTE_BIT_LEN) /**< @todo */
#define OWN_BIT_BUF_LEN_HALF (OWN_BIT_BUF_LEN >> 1)                /**< @todo */
#define OWN_BUFL             OWN_BIT_BUF_LEN_HALF                  /**< @todo */

/**
 * @todo
 */
#define OWN_BIT_SIZEOF(a) (sizeof(a) * OWN_BYTE_BIT_LEN)

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
REF_INLINE qpl_status own_expand_rle_le_be(qpl_job* const qpl_job_ptr);

/**
 * @todo
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_expand_rle_prle(qpl_job* const qpl_job_ptr);

/**
 * @todo
 * @param extracted_source_ptr
 * @param source_length
 * @param initial_bit_width
 * @return
 */
REF_INLINE uint32_t own_get_count_expand_rle(const uint32_t* const extracted_source_ptr, uint32_t source_length,
                                             uint32_t initial_bit_width);

/**
 * @todo
 * @param source_ptr
 * @param mask_ptr
 * @param number_of_elements
 * @param destination_ptr
 * @param destination_length
 * @param initial_bit_width
 * @return
 */
REF_INLINE qpl_status own_expand_rle(const uint32_t* const source_ptr, const uint32_t* const mask_ptr,
                                     uint32_t number_of_elements, uint32_t* const destination_ptr,
                                     uint32_t destination_length, uint32_t initial_bit_width);

/**
 * @todo
 * @param source_ptr
 * @param number_of_output_elements
 * @param source_bit_width
 * @param qpl_job_ptr
 * @return
 */
REF_INLINE qpl_status own_expand_rle_output_to_format(const uint32_t* const source_ptr,
                                                      uint32_t number_of_output_elements, uint32_t source_bit_width,
                                                      qpl_job* const qpl_job_ptr);

/**
 * @todo
 * @param source_ptr
 * @param source_length
 * @return
 */
REF_INLINE uint32_t own_get_number_of_repetitions_prle(const uint8_t* const source_ptr, uint32_t source_length);

/** @} */

qpl_status ref_expand_rle(qpl_job* const qpl_job_ptr) {
    REF_CHECK_FUNC_STS(own_prepare_job(qpl_job_ptr));

    switch (qpl_job_ptr->parser) {
        case qpl_p_be_packed_array: {
            return own_expand_rle_le_be(qpl_job_ptr);
        }
        case qpl_p_le_packed_array: {
            return own_expand_rle_le_be(qpl_job_ptr);
        }
        case qpl_p_parquet_rle: {
            return own_expand_rle_prle(qpl_job_ptr);
        }
        default: {
            return QPL_STS_PARSER_ERR;
        }
    }
}

REF_INLINE qpl_status own_expand_rle_prle(qpl_job* const qpl_job_ptr) {
    // Status of the process
    qpl_status status = QPL_STS_OK;

    // Start of the source vector
    uint8_t* source_ptr     = qpl_job_ptr->next_in_ptr;
    uint8_t* source_end_ptr = qpl_job_ptr->next_in_ptr + qpl_job_ptr->available_in;

    // Start of the mask vector
    uint8_t* source_mask_ptr = qpl_job_ptr->next_src2_ptr;

    // First byte of PRLE stream format is one element bit width, extract it
    uint32_t source_bit_width = (uint32_t)(*source_ptr);

    // Bit width of the mask
    uint32_t mask_bit_width = qpl_job_ptr->src2_bit_width;

    // Number of elements in the source vector
    uint32_t number_of_elements = 0U;

    // Number of bytes available in source_ptr
    uint32_t available_bytes = qpl_job_ptr->available_in;

    // Number of bytes available in source_mask_ptr
    uint32_t available_bytes_mask = qpl_job_ptr->available_src2;

    // Src2 input format - LE (0) or BE (>0)
    uint32_t   mask_be     = qpl_job_ptr->flags & QPL_FLAG_SRC2_BE;
    qpl_parser mask_parser = (mask_be == 0U) ? qpl_p_le_packed_array : qpl_p_be_packed_array;

    status = ref_count_elements_prle(source_ptr, source_end_ptr, &number_of_elements, available_bytes);

    if (QPL_STS_OK != status) { return status; }

    // We should process qpl_job_ptr->num_input_elements, not less
    if (number_of_elements < qpl_job_ptr->num_input_elements) { return QPL_STS_SRC_IS_SHORT_ERR; }

    // Extracted elements from source_ptr vector
    uint32_t* extracted_ptr = (uint32_t*)malloc((uint64_t)number_of_elements * sizeof(uint32_t));

    // Convert source vector's elements to uint32_t format
    status = ref_convert_to_32u_prle(source_ptr, source_end_ptr, extracted_ptr, &available_bytes);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR(extracted_ptr);
        return status;
    }

    // Number of elements in the mask vector
    uint32_t number_of_elements_mask =
            (REF_MAX_BIT_WIDTH == source_bit_width) ? number_of_elements - 1U : number_of_elements;

    // Extracted mask elements from source_mask_ptr vector
    uint32_t* extracted_mask_ptr = (uint32_t*)malloc((uint64_t)number_of_elements_mask * sizeof(uint32_t));

    // Extract mask bits
    status = ref_convert_to_32u_le_be(source_mask_ptr, 0, mask_bit_width, number_of_elements_mask, extracted_mask_ptr,
                                      mask_parser);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, extracted_mask_ptr);
        return status;
    }

    // Extract number of output elements
    uint32_t number_of_output_elements = own_get_count_expand_rle(extracted_ptr, number_of_elements, source_bit_width);

    // Results of the operations
    uint32_t* results_ptr = (uint32_t*)malloc((uint64_t)number_of_output_elements * sizeof(uint32_t));

    // Main action
    status = own_expand_rle(extracted_ptr, extracted_mask_ptr, number_of_elements_mask, results_ptr,
                            number_of_output_elements, source_bit_width);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
        return status;
    }

    // Update crc and xor checksum fields
    update_checksums(qpl_job_ptr);

    // Store result
    status = own_expand_rle_output_to_format(results_ptr, number_of_output_elements, mask_bit_width, qpl_job_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
        return status;
    }

    REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);

    return QPL_STS_OK;
}

REF_INLINE uint32_t own_get_count_expand_rle(const uint32_t* const extracted_source_ptr, uint32_t source_length,
                                             uint32_t initial_bit_width) {
    // Number of repetitions stored in source
    uint32_t count_expand_rle = 0U;

    if (32U != initial_bit_width) {
        for (uint32_t i = 0U; i < source_length; i++) {
            count_expand_rle += extracted_source_ptr[i];
        }
    } else {
        count_expand_rle = extracted_source_ptr[source_length - 1];
    }

    return count_expand_rle;
}

REF_INLINE qpl_status own_prepare_job(qpl_job* const qpl_job_ptr) {
    REF_BAD_PTR_RET(qpl_job_ptr);
    REF_BAD_PTR3_RET(qpl_job_ptr->next_in_ptr, qpl_job_ptr->next_src2_ptr, qpl_job_ptr->next_out_ptr);
    REF_BAD_SIZE_RET(qpl_job_ptr->available_in);
    REF_BAD_ARG_RET((qpl_job_ptr->available_in < qpl_job_ptr->drop_initial_bytes), QPL_STS_SIZE_ERR);
    // qpl_job_ptr->available_out will be checked during input processing, because there can be situation
    // when all counters are 0
    REF_BAD_PTR_RET(qpl_job_ptr->next_src2_ptr);
    REF_BAD_SIZE_RET(qpl_job_ptr->available_src2);
    REF_BAD_SIZE_RET(qpl_job_ptr->num_input_elements);
    REF_BAD_ARG_RET(((REF_8U_BITS != qpl_job_ptr->src1_bit_width) && (REF_16U_BITS != qpl_job_ptr->src1_bit_width) &&
                     (REF_MAX_BIT_WIDTH != qpl_job_ptr->src1_bit_width)),
                    QPL_STS_BIT_WIDTH_ERR);
    REF_BAD_ARG_RET(((1U > qpl_job_ptr->src2_bit_width) || (REF_MAX_BIT_WIDTH < qpl_job_ptr->src2_bit_width)),
                    QPL_STS_BIT_WIDTH_ERR);
    // check that there is enough bits in src1 (counters)
    if (qpl_p_parquet_rle !=
        qpl_job_ptr->parser) // Check is disabled for PRLE, because unpacked (after PRLE) source length is unknown
    {
        REF_BAD_ARG_RET(((qpl_job_ptr->num_input_elements * qpl_job_ptr->src1_bit_width) >
                         OWN_BYTE_BIT_LEN * qpl_job_ptr->available_in),
                        QPL_STS_SRC_IS_SHORT_ERR);
    }
    // check that there is enough bits in src2 (elements)
    // for case when counters src1_bit_width is 32bit then src2 has 1 element less than qpl_job_ptr->num_input_elements
    REF_BAD_ARG_RET((((qpl_job_ptr->num_input_elements -
                       ((OWN_BIT_SIZEOF(uint32_t) == qpl_job_ptr->src1_bit_width) ? 1U : 0U)) *
                      qpl_job_ptr->src2_bit_width) > OWN_BYTE_BIT_LEN * qpl_job_ptr->available_src2),
                    QPL_STS_SRC_IS_SHORT_ERR);

    REF_BAD_ARG_RET((qpl_ow_32 < qpl_job_ptr->out_bit_width), QPL_STS_OUT_FORMAT_ERR);

    // Update job's fields
    qpl_job_ptr->next_in_ptr += qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->available_in -= qpl_job_ptr->drop_initial_bytes;
    qpl_job_ptr->total_in = qpl_job_ptr->drop_initial_bytes;

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_expand_rle_output_to_format(const uint32_t* const source_ptr, uint32_t number_of_elements,
                                                      uint32_t mask_bit_width, qpl_job* const qpl_job_ptr) {
    // Status of the operation
    qpl_status status = QPL_STS_OK;

    // Current destination vector
    uint8_t* destination_ptr = qpl_job_ptr->next_out_ptr;

    // End of the destination vector
    const uint8_t* destination_end_ptr = destination_ptr + qpl_job_ptr->available_out;

    // Element index in destination vector
    uint32_t element_index = qpl_job_ptr->initial_output_index;

    // Output LE or BE
    bool output_be = (bool)(qpl_job_ptr->flags & QPL_FLAG_OUT_BE);

    // Output format
    qpl_out_format output_format = (qpl_out_format)qpl_job_ptr->out_bit_width;

    // Number of output bytes
    uint32_t output_bytes = 0U;

    // Store result
    status = ref_store_values(source_ptr, number_of_elements, mask_bit_width, destination_ptr, destination_end_ptr,
                              output_be, output_format, &element_index);

    if (QPL_STS_OK != status) { return status; }

    // Update required fields in job structure
    uint32_t output_elements = element_index - qpl_job_ptr->initial_output_index;
    status                   = ref_get_output_bytes(&qpl_job_ptr->last_bit_offset, output_elements, mask_bit_width,
                                                    qpl_job_ptr->available_out, output_format, &output_bytes);

    if (QPL_STS_OK != status) { return status; }

    qpl_job_ptr->total_in += qpl_job_ptr->available_in;
    qpl_job_ptr->total_out   = output_bytes;
    qpl_job_ptr->next_in_ptr = (uint8_t*)(source_ptr + qpl_job_ptr->available_in);
    qpl_job_ptr->next_out_ptr += output_bytes;
    qpl_job_ptr->available_in = 0U;
    qpl_job_ptr->available_out -= output_bytes;

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_expand_rle_le_be(qpl_job* const qpl_job_ptr) {
    // Status of the process
    qpl_status status = QPL_STS_OK;

    // Start of the source vector
    uint8_t* source_ptr = qpl_job_ptr->next_in_ptr;

    // Start of the mask vector
    uint8_t* source_mask_ptr = qpl_job_ptr->next_src2_ptr;

    // Width of one element of the source vector
    uint32_t source_bit_width = qpl_job_ptr->src1_bit_width;

    // Bit width of the mask
    uint32_t mask_bit_width = qpl_job_ptr->src2_bit_width;

    // Number of elements in the source vector
    uint32_t number_of_elements = qpl_job_ptr->num_input_elements;

    // Number of elements in the mask vector
    uint32_t number_of_elements_mask = (32U == source_bit_width) ? number_of_elements - 1U : number_of_elements;

    // Number of bytes available in source_ptr
    uint32_t available_bytes = qpl_job_ptr->available_in;

    // Number of bytes available in source_mask_ptr
    uint32_t available_bytes_mask = qpl_job_ptr->available_src2;

    // Src2 input format - LE (0) or BE (>0)
    uint32_t   mask_be     = qpl_job_ptr->flags & QPL_FLAG_SRC2_BE;
    qpl_parser mask_parser = (0U == mask_be) ? qpl_p_le_packed_array : qpl_p_be_packed_array;

    // Bit length of number of elements
    uint64_t bit_length = (uint64_t)number_of_elements * (uint64_t)source_bit_width;

    // Check if source vector has enough bits
    REF_BAD_ARG_RET((available_bytes < REF_BIT_2_BYTE(bit_length)), QPL_STS_SRC_IS_SHORT_ERR);

    // Extracted elements from source_ptr vector
    uint32_t* extracted_ptr = (uint32_t*)malloc((uint64_t)number_of_elements * sizeof(uint32_t));

    // Extracted mask elements from source_mask_ptr vector
    uint32_t* extracted_mask_ptr = (uint32_t*)malloc((uint64_t)number_of_elements_mask * sizeof(uint32_t));

    // Convert source vector's elements to uint32_t format
    status = ref_convert_to_32u_le_be(source_ptr, 0, source_bit_width, number_of_elements, extracted_ptr,
                                      qpl_job_ptr->parser);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR2(extracted_ptr, extracted_mask_ptr);
        return status;
    }

    // Extract number of output elements
    uint32_t number_of_output_elements = own_get_count_expand_rle(extracted_ptr, number_of_elements, source_bit_width);

    // Results of the operations
    uint32_t* results_ptr = (uint32_t*)malloc((uint64_t)number_of_output_elements * sizeof(uint32_t));

    // Extract mask bits
    status = ref_convert_to_32u_le_be(source_mask_ptr, 0, mask_bit_width, number_of_elements_mask, extracted_mask_ptr,
                                      mask_parser);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
        return status;
    }

    // Main action
    status = own_expand_rle(extracted_ptr, extracted_mask_ptr, number_of_elements_mask, results_ptr,
                            number_of_output_elements, source_bit_width);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
        return status;
    }

    // Update crc and xor checksum fields
    update_checksums(qpl_job_ptr);

    // Store result
    status = own_expand_rle_output_to_format(results_ptr, number_of_output_elements, mask_bit_width, qpl_job_ptr);

    if (QPL_STS_OK != status) {
        REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);
        return status;
    }

    REF_FREE_PTR3(extracted_ptr, extracted_mask_ptr, results_ptr);

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_expand_rle(const uint32_t* const source_ptr, const uint32_t* const mask_ptr,
                                     uint32_t number_of_elements_mask, uint32_t* const destination_ptr,
                                     uint32_t number_of_output_elements, uint32_t initial_bit_width) {
    uint32_t numberOfRepetitions = 0U;
    uint32_t destinationIndex    = 0U;
    uint32_t elementToRepeat     = 0U;

    for (uint32_t sourceIndex = 0U; sourceIndex < number_of_elements_mask; sourceIndex++) {
        elementToRepeat = mask_ptr[sourceIndex];

        if (32U != initial_bit_width) {
            numberOfRepetitions = source_ptr[sourceIndex];
        } else {
            if (source_ptr[sourceIndex + 1] < source_ptr[sourceIndex]) { return QPL_STS_INTL_INVALID_COUNT; }

            numberOfRepetitions = source_ptr[sourceIndex + 1] - source_ptr[sourceIndex];
        }

        if (numberOfRepetitions > UINT16_MAX) { return QPL_STS_INTL_INVALID_COUNT; }

        for (uint32_t repetitionIndex = 0U; repetitionIndex < numberOfRepetitions; repetitionIndex++) {
            if (destinationIndex >= number_of_output_elements) { return QPL_STS_INTL_INVALID_COUNT; }

            destination_ptr[destinationIndex] = elementToRepeat;
            destinationIndex++;
        }
    }

    return QPL_STS_OK;
}
