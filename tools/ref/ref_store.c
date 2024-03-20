/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/22/2019
 * Contains an implementation of the @ref ref_store_values function
 */

#include "ref_store.h"

/**
 * @todo
 * @param source_ptr
 * @param number_of_elements
 * @param source_bit_width
 * @param destination_ptr
 * @param destination_end_ptr
 * @param element_index_ptr
 * @return
 */
REF_INLINE qpl_status own_store_value_8(const uint32_t *const source_ptr,
                                        uint32_t number_of_elements,
                                        uint32_t source_bit_width,
                                        uint8_t *const destination_ptr,
                                        const uint8_t *const destination_end_ptr,
                                        uint32_t *element_index_ptr);

/**
 * @todo
 * @param source_ptr
 * @param number_of_elements
 * @param source_bit_width
 * @param destination_ptr
 * @param destination_end_ptr
 * @param output_be
 * @param element_index_ptr
 * @return
 */
REF_INLINE qpl_status own_store_value_16(const uint32_t *const source_ptr,
                                         uint32_t number_of_elements,
                                         uint32_t source_bit_width,
                                         uint8_t *const destination_ptr,
                                         const uint8_t *const destination_end_ptr,
                                         bool output_be,
                                         uint32_t *element_index_ptr);

/**
 * @todo
 * @param source_ptr
 * @param number_of_elements
 * @param source_bit_width
 * @param destination_ptr
 * @param destination_end_ptr
 * @param output_be
 * @param element_index_ptr
 * @return
 */
REF_INLINE qpl_status own_store_value_32(const uint32_t *const source_ptr,
                                         uint32_t number_of_elements,
                                         uint32_t source_bit_width,
                                         uint8_t *const destination_ptr,
                                         const uint8_t *const destination_end_ptr,
                                         bool output_be,
                                         uint32_t *element_index_ptr);

/**
 * @todo
 * @param source_ptr
 * @param number_of_elements
 * @param source_bit_width
 * @param destination_ptr
 * @param output_be
 * @param element_index_ptr
 * @return
 */
REF_INLINE qpl_status own_store_value_nom(const uint32_t *const source_ptr,
                                          uint32_t number_of_elements,
                                          uint32_t source_bit_width,
                                          uint8_t *const destination_ptr,
                                          bool output_be,
                                          uint32_t *element_index_ptr);

qpl_status ref_store_values(const uint32_t *const source_ptr,
                                    uint32_t number_of_elements,
                                    uint32_t source_bit_width,
                                    uint8_t *const destination_ptr,
                                    const uint8_t *const destination_end_ptr,
                                    bool output_be,
                                    qpl_out_format output_format,
                                    uint32_t *element_index_ptr) {
    REF_BAD_PTR_RET(destination_ptr)

    switch (output_format) {
        case qpl_ow_8: {
            return own_store_value_8(source_ptr,
                                     number_of_elements,
                                     source_bit_width,
                                     destination_ptr,
                                     destination_end_ptr,
                                     element_index_ptr);
        }
        case qpl_ow_16: {
            return own_store_value_16(source_ptr,
                                      number_of_elements,
                                      source_bit_width,
                                      destination_ptr,
                                      destination_end_ptr,
                                      output_be,
                                      element_index_ptr);
        }
        case qpl_ow_32: {
            return own_store_value_32(source_ptr,
                                      number_of_elements,
                                      source_bit_width,
                                      destination_ptr,
                                      destination_end_ptr,
                                      output_be,
                                      element_index_ptr);
        }
        case qpl_ow_nom: {
            return own_store_value_nom(source_ptr,
                                       number_of_elements,
                                       source_bit_width,
                                       destination_ptr,
                                       output_be,
                                       element_index_ptr);
        }
        default: {
            return QPL_STS_OUT_FORMAT_ERR;
        }
    }
}

REF_INLINE qpl_status own_store_value_8(const uint32_t *const source_ptr,
                                        uint32_t number_of_elements,
                                        uint32_t source_bit_width,
                                        uint8_t *const destination_ptr,
                                        const uint8_t *const destination_end_ptr,
                                        uint32_t *element_index_ptr) {
    REF_BAD_ARG_RET((8U < source_bit_width), QPL_STS_OUTPUT_OVERFLOW_ERR);

    uint8_t *current_destination_ptr = destination_ptr;

    // Total processed elements
    uint32_t last_element_index = (*element_index_ptr);

    // Current index to store in case 1 bit output bit width
    uint32_t index = last_element_index;

    for (uint32_t i = 0U; i < number_of_elements; ++i) {
        uint8_t value = (uint8_t) source_ptr[i];

        if (QPL_ONE_32U == source_bit_width) {
            if (0U < value) {
                if (UINT8_MAX < index) {
                    return QPL_STS_OUTPUT_OVERFLOW_ERR;
                }

                REF_CHECK_PTR_END((uint8_t *) current_destination_ptr,
                                  destination_end_ptr,
                                  sizeof(uint8_t),
                                  QPL_STS_DST_IS_SHORT_ERR);

                (*current_destination_ptr) = (uint8_t) index;
                ++current_destination_ptr;
                ++last_element_index;
            }
            ++index;
        } else {
            REF_CHECK_PTR_END((uint8_t *) current_destination_ptr,
                              destination_end_ptr,
                              sizeof(uint8_t),
                              QPL_STS_DST_IS_SHORT_ERR);

            (*current_destination_ptr) = (uint8_t) value;
            ++current_destination_ptr;
            ++last_element_index;
        }
    }

    (*element_index_ptr) = (last_element_index);

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_store_value_16(const uint32_t *const source_ptr,
                                         uint32_t number_of_elements,
                                         uint32_t source_bit_width,
                                         uint8_t *const destination_ptr,
                                         const uint8_t *const destination_end_ptr,
                                         bool output_be,
                                         uint32_t *element_index_ptr) {
    REF_BAD_ARG_RET((16U < source_bit_width), QPL_STS_OUTPUT_OVERFLOW_ERR);

    uint16_t *current_destination_ptr = (uint16_t *) destination_ptr;

    // Total processed elements
    uint32_t last_element_index = (*element_index_ptr);

    // Current index to store in case 1 bit output bit width
    uint32_t index = last_element_index;

    for (uint32_t i = 0U; i < number_of_elements; ++i) {
        uint16_t value = (uint16_t) source_ptr[i];

        if (QPL_ONE_32U == source_bit_width) {
            if (0U < value) {
                if (UINT16_MAX < index) {
                    return QPL_STS_OUTPUT_OVERFLOW_ERR;
                }

                REF_CHECK_PTR_END((uint8_t *) current_destination_ptr,
                                  destination_end_ptr,
                                  sizeof(uint16_t),
                                  QPL_STS_DST_IS_SHORT_ERR);

                (*current_destination_ptr) = (output_be)
                                             ? ((uint16_t) ref_cvt_le_2_be_16u(index))
                                             : ((uint16_t) index);
                ++current_destination_ptr;
                ++last_element_index;
            }
            ++index;
        } else {
            REF_CHECK_PTR_END((uint8_t *) current_destination_ptr,
                              destination_end_ptr,
                              sizeof(uint16_t),
                              QPL_STS_DST_IS_SHORT_ERR);

            (*current_destination_ptr) = (output_be)
                                         ? ((uint16_t) ref_cvt_le_2_be_16u(value))
                                         : ((uint16_t) value);
            ++current_destination_ptr;
            ++last_element_index;
        }
    }

    (*element_index_ptr) = last_element_index;

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_store_value_32(const uint32_t *const source_ptr,
                                         uint32_t number_of_elements,
                                         uint32_t source_bit_width,
                                         uint8_t *const destination_ptr,
                                         const uint8_t *const destination_end_ptr,
                                         bool output_be,
                                         uint32_t *element_index_ptr) {
    REF_BAD_ARG_RET((32U < source_bit_width), QPL_STS_OUTPUT_OVERFLOW_ERR);

    uint32_t *current_destination_ptr = (uint32_t *) destination_ptr;

    // Total processed elements
    uint32_t last_element_index = (*element_index_ptr);

    // Current index to store in case 1 bit output bit width
    uint32_t index = last_element_index;

    for (uint32_t i = 0U; i < number_of_elements; ++i) {
        uint32_t value = (uint32_t) source_ptr[i];

        if (QPL_ONE_32U == source_bit_width) {
            if (0U < value) {
                if (UINT32_MAX < index) {
                    return QPL_STS_OUTPUT_OVERFLOW_ERR;
                }

                REF_CHECK_PTR_END((uint8_t *) current_destination_ptr,
                                  destination_end_ptr,
                                  sizeof(uint32_t),
                                  QPL_STS_DST_IS_SHORT_ERR);

                (*current_destination_ptr) = (output_be)
                                             ? ((uint32_t) ref_cvt_le_2_be_32u(index))
                                             : ((uint32_t) index);
                ++current_destination_ptr;
                ++last_element_index;
            }
            ++index;
        } else {
            REF_CHECK_PTR_END((uint8_t *) current_destination_ptr,
                              destination_end_ptr,
                              sizeof(uint32_t),
                              QPL_STS_DST_IS_SHORT_ERR);

            (*current_destination_ptr) = (output_be)
                                         ? ((uint32_t) ref_cvt_le_2_be_32u(value))
                                         : ((uint32_t) value);
            ++current_destination_ptr;
            ++last_element_index;
        }
    }

    (*element_index_ptr) = last_element_index;

    return QPL_STS_OK;
}

REF_INLINE qpl_status own_store_value_nom(const uint32_t *const source_ptr,
                                          uint32_t number_of_elements,
                                          uint32_t source_bit_width,
                                          uint8_t *const destination_ptr,
                                          bool output_be,
                                          uint32_t *element_index_ptr) {
    // Current position in destination_ptr
    uint8_t *current_destination_ptr = destination_ptr;

    // Mask to store one bit
    uint8_t destination_mask = 0U;

    // Destination offset in bits
    uint64_t destination_bit_offset = 0U;

    // Ouput is in BE format
    if (output_be) {
        // For all mask elements
        for (uint32_t i = 0U; i < number_of_elements; i++) {
            // First bit of n-bit BE number in some current_destination_ptr byte
            destination_bit_offset += source_bit_width;

            // Pointer to the actual destination byte;  "-1" - because we need the last bit of actual value,
            // not the next one
            current_destination_ptr = destination_ptr + ((destination_bit_offset - QPL_ONE_8U) >> REF_BIT_LEN_2_BYTE);

            // Bit mask to actual position (actual offset of the 1st bit in the actual destination byte)
            destination_mask = REF_HIGH_BIT_MASK >> ((destination_bit_offset - QPL_ONE_8U) & REF_MAX_BIT_IDX);

            // Zero unused bits in the last byte
            (*current_destination_ptr) &= ~(destination_mask - QPL_ONE_8U);

            // COpy bit by bit
            for (uint32_t j = 0U; j < source_bit_width; j++) {
                // Get source bit value
                uint32_t sourceBit = (uint32_t) (source_ptr[i] & (QPL_ONE_32U << j));

                // Store bit
                (*current_destination_ptr) = (sourceBit)
                                             ? ((*current_destination_ptr) | destination_mask)
                                             : ((*current_destination_ptr) & (~destination_mask));

                // Direction is the same - down -> up (in LE notation)
                destination_mask <<= QPL_ONE_8U;

                // If dst mask is beyond the byte boundaries
                if (!destination_mask) {
                    // Set it to initial bit of the byte
                    destination_mask = (uint8_t) REF_LOW_BIT_MASK;

                    // And shift pointer to the next byte
                    current_destination_ptr--;
                }
            }
        }
    }
        // Output is in LE format
    else {
        destination_mask = (uint8_t) REF_LOW_BIT_MASK;

        // Through all mask elements
        for (uint32_t i = 0U; i < number_of_elements; i++) {
            // Copy bit by bit
            for (uint32_t j = 0U; j < source_bit_width; j++) {
                // Get source bit value
                uint32_t sourceBit = (uint32_t) (source_ptr[i] & (QPL_ONE_32U << j));

                // Store bit
                (*current_destination_ptr) = (sourceBit)
                                             ? ((*current_destination_ptr) | destination_mask)
                                             : ((*current_destination_ptr) & (~destination_mask));

                // Direction is the same - down -> up (in LE notation)
                destination_mask <<= QPL_ONE_8U;

                // If dst mask is beyond the byte boundaries
                if (!destination_mask) {
                    // Set it to initial bit of the byte
                    destination_mask = (uint8_t) REF_LOW_BIT_MASK;

                    // And shift pointer to the next byte
                    current_destination_ptr++;
                }
            }
        }

        if (REF_LOW_BIT_MASK != destination_mask) {
            // Zero unused bits in the last byte
            (*current_destination_ptr) &= (destination_mask - QPL_ONE_8U);
        }
    }

    // Store element index
    (*element_index_ptr) = number_of_elements;

    return QPL_STS_OK;
}

qpl_status ref_set_bit_by_index(uint32_t bit_index, uint8_t *vector_ptr, uint32_t o_format) {
    switch (o_format) {
        case qpl_ow_nom | QPL_FLAG_OUT_BE: {    // the same as above, but BE output format
            // "divide by 8" - index of byte in dst_ptr where to update/store bit
            uint32_t byte_index = bit_index >> REF_BIT_LEN_2_BYTE;
            uint8_t  r_bit      = REF_HIGH_BIT_MASK >> (bit_index & REF_MAX_BIT_IDX);    // result bit mask
            vector_ptr[byte_index] = vector_ptr[byte_index] | r_bit;
            break;
        }
        default: {
            // "divide by 8" - index of byte in dst_ptr where to update/store bit
            uint32_t byte_index = bit_index >> REF_BIT_LEN_2_BYTE;
            uint8_t  r_bit      = REF_LOW_BIT_MASK << (bit_index & REF_MAX_BIT_IDX);    // result bit mask

            // we already have correct dst byte - so mask index with position inside this byte
            vector_ptr[byte_index] = vector_ptr[byte_index] | r_bit;
            break;
        }
    }

    return QPL_STS_OK;
}
