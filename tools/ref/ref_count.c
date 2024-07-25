/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 01/31/2020
 * Contains a reference implementation of @ref ref_count_elements_prle and  @ref ref_count_non_zero_elements_32u functions
 *
 */

#include "ref_count.h"

qpl_status ref_count_elements_prle(const uint8_t* const source_ptr, const uint8_t* const source_end_ptr,
                                   uint32_t* const number_of_elements_ptr, uint32_t available_bytes) {
    REF_BAD_PTR_RET(source_ptr);
    REF_BAD_PTR_RET(source_end_ptr);
    REF_BAD_PTR_RET(number_of_elements_ptr);

    // Result of the operation
    qpl_status status = QPL_STS_OK;

    // Current position in source_ptr
    const uint8_t* current_ptr = source_ptr;

    //Extract bit width and shift current_ptr to the first element
    uint32_t source_bit_width = *current_ptr++;

    // Number of unpacked elements
    uint32_t number_of_elements = 0U;

    // Format ( octa-group or RLE )
    uint32_t format = 0U;

    // Number of repetitions of each element
    uint32_t repetitions = 0U;

    // Bits in buffer counter
    uint32_t number_of_bits_in_buffer = 0U;

    // Main buffer
    uint64_t bit_buffer = 0U;

    // Temporary buffer to store bits
    uint64_t temp_buffer = 0U;

    // Check source bit width
    REF_BAD_ARG_RET((REF_MAX_BIT_WIDTH < source_bit_width), QPL_STS_BIT_WIDTH_ERR);

    // This is 'bit_width' bit
    available_bytes--;

    // Check if PRLE format is correct
    REF_BAD_ARG_RET((0U == available_bytes), QPL_STS_SRC_IS_SHORT_ERR);

    while (current_ptr < source_end_ptr) {
        // Get format and count
        status = ref_get_format_and_count(&current_ptr, &format, &repetitions, &available_bytes);

        if (QPL_STS_OK != status) { return status; }

        // Check if repetitions is equal to zero
        REF_BAD_ARG_RET((0U == repetitions), QPL_STS_PRLE_FORMAT_ERR);

        // Packed set of octa-groups
        if (0U < format) {
            number_of_bits_in_buffer = 0U;
            bit_buffer               = 0U;

            // Through all octa-groups
            for (uint32_t i = 0U; i < repetitions; ++i) {
                // One octa-group
                for (uint32_t j = 0U; j < REF_OCTA_GROUP_SIZE; ++j) {
                    // Extract an element
                    while (source_bit_width > number_of_bits_in_buffer) {
                        // Checks if we have enough bytes
                        if (0U == available_bytes) { return QPL_STS_PRLE_FORMAT_ERR; }

                        temp_buffer = (uint64_t)(*current_ptr);
                        temp_buffer <<= number_of_bits_in_buffer;
                        bit_buffer |= temp_buffer;
                        number_of_bits_in_buffer += REF_BYTE_BIT_LEN;

                        --available_bytes;
                        ++current_ptr;
                    }

                    // Got one element
                    number_of_elements++;
                    number_of_bits_in_buffer -= source_bit_width;
                    bit_buffer >>= source_bit_width;
                }
            }
        }
        // RLE
        else {
            // Size of an element in bytes
            uint32_t value_size = REF_BIT_2_BYTE(source_bit_width);

            // If the next src byte portion is not available
            if (available_bytes < value_size) { return QPL_STS_PRLE_FORMAT_ERR; }

            // Got multiple number of elements
            number_of_elements += repetitions;
            available_bytes -= value_size;
            current_ptr += value_size;
        }
    }

    // Save number of unpacked elements
    (*number_of_elements_ptr) = number_of_elements;

    return QPL_STS_OK;
}

uint32_t ref_count_non_zero_elements_32u(const uint32_t* const source_ptr, uint32_t number_of_elements) {
    // Current position in source_ptr vector
    const uint32_t* current_ptr = source_ptr;

    // Result of the operation
    uint32_t number_of_non_zero_elements = 0U;

    // Main action
    for (uint32_t i = 0U; i < number_of_elements; ++i) {
        if (0U != (*current_ptr)) { number_of_non_zero_elements++; }

        current_ptr++;
    }

    return number_of_non_zero_elements;
}
