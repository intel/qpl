/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/13/2018
 * Contains a reference implementation of the ref_min_max_sum function
 *
 */

#include "ref_bit_rev.h"
#include "ref_min_max_sum.h"

qpl_status ref_min_max_sum(const uint8_t* src_ptr, uint32_t len, uint32_t last_bit_offset, qpl_out_format elem_format,
                           uint8_t value_bit_width, uint32_t input_be, uint32_t initial_output_index,
                           uint32_t* first_idx_min_value_ptr, uint32_t* last_idx_max_value_ptr,
                           uint32_t* sum_value_ptr) {
    qpl_status status = QPL_STS_OK;

    // input len in bits
    uint64_t src_bit_len =
            last_bit_offset > 0 ? ((len - 1) * REF_BYTE_BIT_LEN) + last_bit_offset : len * REF_BYTE_BIT_LEN;
    uint64_t bit_buf = 0; // temporal bit-buffer for packed input
    // bit mask to extract 1 source element
    uint32_t elem_bit_width = (qpl_ow_nom == elem_format ? value_bit_width : 4 << elem_format);
    uint64_t s_bit_mask     = ((QPL_ONE_64U << elem_bit_width) - 1);
    uint32_t n_bits_in_buf  = 0; // number of valid (unprocessed) bits in the bit_buf

    uint32_t num_of_elements = (uint32_t)(src_bit_len / elem_bit_width);

    uint32_t elem = 0; // storage for current element value from src_ptr

    uint64_t first_index_min_value = UINT32_MAX;
    uint64_t last_index_max_value  = 0;
    uint64_t sum_value             = 0;

    // used for input BE format only - number of bits which should be read from stream to form 1 element
    uint8_t bits_to_read        = 0;
    uint8_t left_bits_in_rb_buf = 8; // used for input BE format only - number of free bits in read byte buffer

    // main processing cycle, read elements and perform aggregate operations
    for (uint32_t curr_elem_ind = 0; curr_elem_ind < num_of_elements; curr_elem_ind++) {
        // check if circular bit-buffer has enough bits to read new element
        if (!input_be) {
            if (n_bits_in_buf <= REF_BIT_BUF_LEN_HALF) {
                // fill buffer until there is enough space in buffer to store new byte
                // and there are bytes in input array
                while (src_bit_len > 0 && REF_BYTE_BIT_LEN <= (REF_BIT_BUF_LEN - n_bits_in_buf)) {
                    // load byte, shift it to the proper position
                    // and put it to the circular buffer
                    bit_buf |= ((uint64_t)(*src_ptr)) << n_bits_in_buf;
                    src_ptr++;                         // shift source pointer to the next position
                    n_bits_in_buf += REF_BYTE_BIT_LEN; // update bit-counter for circular buffer
                    src_bit_len -= REF_BYTE_BIT_LEN;   // update remaining source bit-counter
                }
            }
            n_bits_in_buf -= elem_bit_width;                   // update bit-buffer counter
            elem = ((uint32_t)bit_buf) & (uint32_t)s_bit_mask; // extract input value
            bit_buf >>= elem_bit_width;                        // shift circular bit-buffer to the next position
        } else if (input_be) {
            // input is in BE format
            elem         = 0;
            bits_to_read = elem_bit_width;
            while (src_bit_len > 0 && bits_to_read > 0) {
                if (bits_to_read < left_bits_in_rb_buf) {
                    left_bits_in_rb_buf -= bits_to_read;
                    elem |= ((uint32_t)(*src_ptr)) >> left_bits_in_rb_buf;
                    bits_to_read = 0;
                } else {
                    bits_to_read -= left_bits_in_rb_buf;
                    elem |= ((uint32_t)(*src_ptr)) << bits_to_read;
                    left_bits_in_rb_buf = REF_BYTE_BIT_LEN;
                    src_ptr++;
                    src_bit_len -= REF_BYTE_BIT_LEN; // update remaining source bit-counter
                }
            }
            elem = ((uint32_t)elem) & (uint32_t)s_bit_mask; // extract input value
        }

        // now perform aggregate operations with received elem
        if (1 == elem_bit_width) {
            // it is regular bit vector
            if (elem & 1) {
                if (!sum_value) {
                    first_index_min_value = curr_elem_ind + initial_output_index;
#ifdef OWN_SATURATION_ENABLED
                    if (UINT32_MAX < first_index_min_value) {
                        //saturation
                        first_index_min_value = UINT32_MAX;
                    }
#endif
                }

                last_index_max_value = curr_elem_ind + initial_output_index;
#ifdef OWN_SATURATION_ENABLED
                if (UINT32_MAX < last_index_max_value) {
                    //saturation
                    last_index_max_value = UINT32_MAX;
                }
#endif

                sum_value++;
            }
        } else if (1 == value_bit_width && (qpl_ow_nom != elem_format)) {
            // it is vector of indexes of non 0 elements of bit vector
            if (!sum_value) {
                first_index_min_value = elem;
                sum_value             = num_of_elements;
            }

            last_index_max_value = elem;
        } else {
            // array of uints
            sum_value += elem;
#ifdef OWN_SATURATION_ENABLED
            if (UINT32_MAX < sum_value) { // check for uint32_t overflow and perform saturation
                sum_value = UINT32_MAX;
            }
#endif

            if (elem > last_index_max_value) { last_index_max_value = elem; }

            if (elem < first_index_min_value) { first_index_min_value = elem; }
        }
    }

    *first_idx_min_value_ptr = (uint32_t)first_index_min_value;
    *last_idx_max_value_ptr  = (uint32_t)last_index_max_value;
    *sum_value_ptr           = (uint32_t)sum_value;

    return status;
}
