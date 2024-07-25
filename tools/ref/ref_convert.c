/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 10/03/2019
 * @todo
 *
 */

#include "ref_convert.h"

/**
 * @todo describe union
 */
typedef union {
    uint32_t u_int;     /**< @todo */
    uint8_t  u_byte[4]; /**< @todo */
} own_rle_val;

qpl_status ref_convert_to_32u_le_be(const uint8_t* const source_ptr, uint32_t source_bit_offset,
                                    uint32_t source_bit_width, uint32_t number_of_elements,
                                    uint32_t* const destination_ptr, qpl_parser parser) {
    REF_BAD_PTR_RET(source_ptr);
    REF_BAD_PTR_RET(destination_ptr);

    // For BE we'll take bits from the top of the buffer
    uint32_t shift_bits = REF_BIT_BUF_LEN - source_bit_width;

    // Bits in buffer counter
    uint32_t number_of_bits_in_buffer = 0U;

    // source_ptr vector's length in bits
    int64_t bit_length  = (int64_t)number_of_elements * source_bit_width;
    int64_t byte_length = REF_BIT_2_BYTE(bit_length);

    // Current position in source_ptr
    const uint8_t* current_ptr = source_ptr;
    const uint8_t* end_ptr     = source_ptr + byte_length;

    // Temporary buffer to store bits
    uint64_t temp_buffer = (uint64_t)((*current_ptr++) >> source_bit_offset);

    // Mask for extracting value bits
    uint64_t source_bit_mask = (QPL_ONE_64U << source_bit_width) - QPL_ONE_64U;

    // Shift bit mask to the top position for BE
    uint64_t shift_bit_mask = source_bit_mask << shift_bits;

    // Main buffer
    uint64_t bit_buffer = 0U;

    for (uint32_t i = 0U; i < number_of_elements; ++i) {
        if (REF_BIT_BUF_LEN_HALF >= number_of_bits_in_buffer) {
            while (0U < bit_length) {
                if (REF_BYTE_BIT_LEN > (REF_BIT_BUF_LEN - number_of_bits_in_buffer)) { break; }

                if (qpl_p_be_packed_array == parser) {
                    temp_buffer <<= REF_BIT_BUF_LEN - number_of_bits_in_buffer - REF_BYTE_BIT_LEN;
                } else {
                    temp_buffer <<= number_of_bits_in_buffer;
                }

                number_of_bits_in_buffer += REF_BYTE_BIT_LEN;
                bit_length -= REF_BYTE_BIT_LEN;
                bit_buffer |= temp_buffer;

                if (current_ptr != end_ptr) {
                    temp_buffer = (uint64_t)(*current_ptr);
                    ++current_ptr;
                }
            }
        }

        if (qpl_p_be_packed_array == parser) {
            destination_ptr[i] = (uint32_t)((bit_buffer & shift_bit_mask) >> shift_bits);
            bit_buffer <<= source_bit_width;
        } else {
            destination_ptr[i] = (uint32_t)(bit_buffer & source_bit_mask);
            bit_buffer >>= source_bit_width;
        }

        number_of_bits_in_buffer -= source_bit_width;
    }

    return QPL_STS_OK;
}

qpl_status ref_convert_to_32u_prle(const uint8_t* const source_ptr, const uint8_t* const source_end_ptr,
                                   uint32_t* const destination_ptr, uint32_t* const available_bytes_ptr) {
    REF_BAD_PTR_RET(source_ptr);
    REF_BAD_PTR_RET(source_end_ptr);
    REF_BAD_PTR_RET(destination_ptr);
    REF_BAD_PTR_RET(available_bytes_ptr);

    const uint8_t* current_ptr = source_ptr;

    //Extract bit width and shift current_ptr to the first element
    uint32_t source_bit_width = *current_ptr++;

    // Current index in destination_ptr vector
    uint32_t index = 0U;

    // Format ( octa-group or RLE )
    uint32_t format = 0U;

    // Number of repetitions of each element
    uint32_t repetitions = 0U;

    // Bits in buffer counter
    uint32_t number_of_bits_in_buffer = 0U;

    // Mask for extracting value bits
    uint64_t source_bit_mask = (QPL_ONE_64U << source_bit_width) - QPL_ONE_64U;

    // Main buffer
    uint64_t bit_buffer = 0U;

    // Temporary buffer to store bits
    uint64_t temp_buffer = 0U;

    // Check source bit width
    REF_BAD_ARG_RET((REF_MAX_BIT_WIDTH < source_bit_width), QPL_STS_BIT_WIDTH_ERR);

    // This is 'bit_width' byte
    (*available_bytes_ptr)--;

    // Check if PRLE format is correct
    REF_BAD_ARG_RET((0U == (*available_bytes_ptr)), QPL_STS_SRC_IS_SHORT_ERR);

    while (current_ptr < source_end_ptr) {
        // Get format and count
        REF_CHECK_FUNC_STS(ref_get_format_and_count(&current_ptr, &format, &repetitions, available_bytes_ptr));

        // Check if repetitions is equal to zero
        REF_BAD_ARG_RET((0U == repetitions), QPL_STS_PRLE_FORMAT_ERR);

        // Packed set of octa-groups
        if (0U < format) {
            number_of_bits_in_buffer = 0U;
            bit_buffer               = 0U;

            for (uint32_t i = 0U; i < repetitions; ++i) {
                for (uint32_t j = 0U; j < REF_OCTA_GROUP_SIZE; ++j) {
                    while (source_bit_width > number_of_bits_in_buffer) {
                        if (0U == (*available_bytes_ptr)) { return QPL_STS_PRLE_FORMAT_ERR; }

                        temp_buffer = (uint64_t)(*current_ptr);
                        temp_buffer <<= number_of_bits_in_buffer;
                        bit_buffer |= temp_buffer;
                        number_of_bits_in_buffer += REF_BYTE_BIT_LEN;

                        --(*available_bytes_ptr);
                        ++current_ptr;
                    }

                    destination_ptr[index++] = (uint32_t)(bit_buffer & source_bit_mask);
                    number_of_bits_in_buffer -= source_bit_width;
                    bit_buffer >>= source_bit_width;
                }
            }
        } else // RLE
        {
            own_rle_val rle_val;

            uint32_t value_size = (source_bit_width + REF_MAX_BIT_IDX) >> REF_BIT_LEN_2_BYTE;

            // If the next src byte portion is not available
            if ((*available_bytes_ptr) < value_size) { return QPL_STS_PRLE_FORMAT_ERR; }

            rle_val.u_byte[0] = current_ptr[0];
            rle_val.u_byte[1] = (8U < source_bit_width) ? current_ptr[1] : 0;
            rle_val.u_byte[2] = (16U < source_bit_width) ? current_ptr[2] : 0;
            rle_val.u_byte[3] = (24U < source_bit_width) ? current_ptr[3] : 0;

            (*available_bytes_ptr) -= value_size;
            current_ptr += value_size;

            for (uint32_t j = 0U; j < repetitions; ++j) {
                destination_ptr[index++] = rle_val.u_int;
            }
        }
    }

    return QPL_STS_OK;
}
