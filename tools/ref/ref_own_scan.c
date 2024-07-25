/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/07/2018
 * Contains an implementation of the @ref ref_compare function
 *
 */

#include "ref_scan.h"

qpl_status ref_store_result(uint32_t dst_buf, uint32_t bit_idx, uint8_t** pp_dst, uint8_t* dst_end_ptr,
                            uint32_t* idx_ptr, uint32_t o_format) {
    switch (o_format) {
        case qpl_ow_nom: { // nominal bit vector - store 1 bit
            // "divide by 8" - index of byte in dst_ptr where to update/store bit
            uint32_t tIdx    = bit_idx >> REF_BIT_LEN_2_BYTE;
            uint8_t* dst_ptr = (uint8_t*)*pp_dst;                               // destination pointer
            uint8_t  r_bit   = REF_LOW_BIT_MASK << (bit_idx & REF_MAX_BIT_IDX); // result bit mask
            // as we store only 1 bit - dst_buf can have only 2 values - 0 or 1
            // we already have correct dst byte - so mask index with position inside this byte
            dst_ptr[tIdx] = (0U < dst_buf) ? dst_ptr[tIdx] | r_bit : dst_ptr[tIdx] & (~r_bit);
            break;
        }
        case qpl_ow_nom | QPL_FLAG_OUT_BE: {
            // "divide by 8" - index of byte in dst_ptr where to update/store bit
            uint32_t tIdx    = bit_idx >> REF_BIT_LEN_2_BYTE;
            uint8_t* dst_ptr = (uint8_t*)*pp_dst;                  // destination pointer
            uint8_t  r_bit   = REF_HIGH_BIT_MASK >> (bit_idx & 7); // result bit mask
            // as we store only 1 bit - dst_buf can have only 2 values - 0 or 1
            // we already have correct dst byte - so mask index with position inside this byte
            dst_ptr[tIdx] = (0U < dst_buf) ? dst_ptr[tIdx] | r_bit : dst_ptr[tIdx] & (~r_bit);
            break;
        }
        case qpl_ow_8:                            // output modification: store 8u idexes of non-zero bits in LE
        case qpl_ow_8 | QPL_FLAG_OUT_BE: {        // or BE (same as LE for bytes)
            uint8_t* dst_ptr = (uint8_t*)*pp_dst; // destination pointer
            if (0U < dst_buf) {                   // skip zero values
                if (*idx_ptr > UINT8_MAX) {       // 8u data type - index can't exceed max 8u
                    return QPL_STS_OUTPUT_OVERFLOW_ERR;
                }
                // we don't know initially required dst buf size - so check here
                REF_CHECK_PTR_END(dst_ptr, dst_end_ptr, sizeof(uint8_t), QPL_STS_DST_IS_SHORT_ERR);
                *dst_ptr = (uint8_t)*idx_ptr; // put index to dst
                (*pp_dst)++;                  // update dst pointer
            }
            (*idx_ptr)++; // inc index
            break;
        }
        case qpl_ow_16: { // output modification: index of non-zero element in 16u data type (LE)
            uint16_t* dst_ptr = (uint16_t*)*pp_dst; // dst pointer
            if (0U < dst_buf) {                     // skip zero index
                if (*idx_ptr > UINT16_MAX) {        // can't exceed max 16u
                    return QPL_STS_OUTPUT_OVERFLOW_ERR;
                }
                REF_CHECK_PTR_END((uint8_t*)dst_ptr, dst_end_ptr, sizeof(uint16_t),
                                  QPL_STS_DST_IS_SHORT_ERR); // check if dst is available
                *dst_ptr = (uint16_t)*idx_ptr;               // put index to dst
                (*pp_dst) += sizeof(uint16_t);               // update dst pointer
            }
            (*idx_ptr)++; // inc index
            break;
        }
        case qpl_ow_16 | QPL_FLAG_OUT_BE: { // the same as above, but BE format
            uint16_t* dst_ptr = (uint16_t*)*pp_dst;
            union {
                uint16_t u_int;
                uint8_t  u_byte[2];
            } x, y;
            if (0U < dst_buf) {
                if (*idx_ptr > UINT16_MAX) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                REF_CHECK_PTR_END((uint8_t*)dst_ptr, dst_end_ptr, sizeof(uint16_t), QPL_STS_DST_IS_SHORT_ERR);
                x.u_int     = (uint16_t)*idx_ptr; // swap bytes for BE format
                y.u_byte[0] = x.u_byte[1];
                y.u_byte[1] = x.u_byte[0];
                *dst_ptr    = y.u_int;
                (*pp_dst) += sizeof(uint16_t);
            }
            (*idx_ptr)++;
            break;
        }
        case qpl_ow_32: { // output modification - indexes of non-zero elements in 32u data type (LE)
            uint32_t* dst_ptr = (uint32_t*)*pp_dst; // destination pointer
            if (0U < dst_buf) {                     // skip zero elements
                REF_CHECK_PTR_END((uint8_t*)dst_ptr, dst_end_ptr, sizeof(uint32_t),
                                  QPL_STS_DST_IS_SHORT_ERR); // check if there is enough place in dst
                *dst_ptr = *idx_ptr;                         // put index to dst
                (*pp_dst) += sizeof(uint32_t);               // update dst pointer
            }
            if (UINT32_MAX > *idx_ptr) { // we can't exceed 32u max
                (*idx_ptr)++;            // update index
            } else {
                return QPL_STS_OUTPUT_OVERFLOW_ERR;
            }
            break;
        }
        case qpl_ow_32 | QPL_FLAG_OUT_BE: { // the same as above, but for BE store format
            uint32_t* dst_ptr = (uint32_t*)*pp_dst;
            union {
                uint32_t u_int;
                uint8_t  u_byte[4];
            } x, y;
            if (0U < dst_buf) {
                REF_CHECK_PTR_END((uint8_t*)dst_ptr, dst_end_ptr, sizeof(uint32_t), QPL_STS_DST_IS_SHORT_ERR);
                x.u_int     = (uint32_t)*idx_ptr; // swap bytes for BE
                y.u_byte[0] = x.u_byte[3];
                y.u_byte[1] = x.u_byte[2];
                y.u_byte[2] = x.u_byte[1];
                y.u_byte[3] = x.u_byte[0];
                *dst_ptr    = y.u_int;
                (*pp_dst) += sizeof(uint32_t);
            }
            if (UINT32_MAX > *idx_ptr) {
                (*idx_ptr)++;
            } else {
                return QPL_STS_OUTPUT_OVERFLOW_ERR;
            }
            break;
        }
        default: break;
    }
    return QPL_STS_OK;
}

qpl_status ref_store_1_bit(uint32_t destination_buffer, uint32_t bit_index, uint8_t** const pp_destination,
                           const uint8_t* const destination_end_ptr, uint32_t* const index_ptr,
                           const uint32_t output_format) {
    // Destination pointer
    uint8_t*  destination_8u_ptr  = NULL;
    uint16_t* destination_16u_ptr = NULL;
    uint32_t* destination_32u_ptr = NULL;

    // Result bit mask
    uint8_t result_bit_mask = 0U;

    // Index of byte in dst_ptr where to update/store bit
    uint32_t byte_index = 0U;

    switch (output_format) {
        // Nominal bit vector - store 1 bit
        case qpl_ow_nom: {
            byte_index         = (bit_index >> REF_BIT_LEN_2_BYTE);
            destination_8u_ptr = (uint8_t*)(*pp_destination);
            result_bit_mask    = (REF_LOW_BIT_MASK << (bit_index & REF_MAX_BIT_IDX));

            // As we store only 1 bit - destination_buffer can have only 2 values - 0 or 1
            // We already have correct destination byte - so mask index with position inside this byte
            destination_8u_ptr[byte_index] = (0U < destination_buffer)
                                                     ? (destination_8u_ptr[byte_index] | result_bit_mask)
                                                     : (destination_8u_ptr[byte_index] & (~result_bit_mask));
            break;
        } // The same as above, but BE output format
        case qpl_ow_nom | QPL_FLAG_OUT_BE: {
            byte_index         = (bit_index >> REF_BIT_LEN_2_BYTE);
            destination_8u_ptr = (uint8_t*)(*pp_destination);
            result_bit_mask    = (REF_HIGH_BIT_MASK >> (bit_index & REF_MAX_BIT_IDX));

            // Update destination byte
            destination_8u_ptr[byte_index] = (0U < destination_buffer)
                                                     ? (destination_8u_ptr[byte_index] | result_bit_mask)
                                                     : (destination_8u_ptr[byte_index] & (~result_bit_mask));
            break;
        }              // Output modification: store 8u idexes of non-zero bits in LE
        case qpl_ow_8: // Or BE (same as LE for bytes)
        case qpl_ow_8 | QPL_FLAG_OUT_BE: {
            destination_8u_ptr = (uint8_t*)(*pp_destination);

            // Skip zero values
            if (0U < destination_buffer) {
                // 8u data type - index can't exceed max 8u
                if ((*index_ptr) > UINT8_MAX) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }

                // We don't know initially required dst buf size - so check here
                REF_CHECK_PTR_END(destination_8u_ptr, destination_end_ptr, sizeof(uint8_t), QPL_STS_DST_IS_SHORT_ERR);

                // Put index to dst
                (*destination_8u_ptr) = (uint8_t)(*index_ptr);

                // Update dst pointer
                (*pp_destination)++;
            }

            // Inc index
            (*index_ptr)++;

            break;
        } // Output modification: index of non-zero element in 16u data type (LE)
        case qpl_ow_16: {
            destination_16u_ptr = (uint16_t*)(*pp_destination);

            // Skip zero index
            if (0U < destination_buffer) {
                // Can't exceed max 16u
                if ((*index_ptr) > UINT16_MAX) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }

                // Check if dst is available
                REF_CHECK_PTR_END((uint8_t*)(destination_16u_ptr), destination_end_ptr, sizeof(uint16_t),
                                  QPL_STS_DST_IS_SHORT_ERR);

                // Put index to dst
                (*destination_16u_ptr) = (uint16_t)(*index_ptr);

                // Update dst pointer
                (*pp_destination) += sizeof(uint16_t);
            }

            // Inc idx
            (*index_ptr)++;

            break;
        } // The same as above, but BE format
        case qpl_ow_16 | QPL_FLAG_OUT_BE: {
            destination_16u_ptr = (uint16_t*)(*pp_destination);

            union {
                uint16_t u_int;
                uint8_t  u_byte[2];
            } x, y;

            if (0U < destination_buffer) {
                if ((*index_ptr) > UINT16_MAX) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }

                REF_CHECK_PTR_END((uint8_t*)destination_16u_ptr, destination_end_ptr, sizeof(uint16_t),
                                  QPL_STS_DST_IS_SHORT_ERR);

                // Swap bytes for BE format
                x.u_int     = (uint16_t)(*index_ptr);
                y.u_byte[0] = x.u_byte[1];
                y.u_byte[1] = x.u_byte[0];

                (*destination_16u_ptr) = y.u_int;
                (*pp_destination) += sizeof(uint16_t);
            }

            (*index_ptr)++;

            break;
        } // Output modification - indexes of non-zero elements in 32u data type (LE)
        case qpl_ow_32: {
            destination_32u_ptr = (uint32_t*)(*pp_destination);

            // Skip zero elements
            if (0U < destination_buffer) {
                // Check if there is enough place in dst
                REF_CHECK_PTR_END((uint8_t*)destination_32u_ptr, destination_end_ptr, sizeof(uint32_t),
                                  QPL_STS_DST_IS_SHORT_ERR);

                // Put index to dst
                (*destination_32u_ptr) = (*index_ptr);

                // Update dst pointer
                (*pp_destination) += sizeof(uint32_t);
            }

            // We can't exceed 32u max
            if (UINT32_MAX > (*index_ptr)) {
                // Update index
                (*index_ptr)++;
            } else {
                return QPL_STS_OUTPUT_OVERFLOW_ERR;
            }
            break;
        } // The same as above, but for BE store format
        case qpl_ow_32 | QPL_FLAG_OUT_BE: {
            destination_32u_ptr = (uint32_t*)(*pp_destination);

            union {
                uint32_t u_int;
                uint8_t  u_byte[4];
            } x, y;

            if (0U < destination_buffer) {
                REF_CHECK_PTR_END((uint8_t*)destination_32u_ptr, destination_end_ptr, sizeof(uint32_t),
                                  QPL_STS_DST_IS_SHORT_ERR);

                // Swap bytes for BE
                x.u_int     = (uint32_t)(*index_ptr);
                y.u_byte[0] = x.u_byte[3];
                y.u_byte[1] = x.u_byte[2];
                y.u_byte[2] = x.u_byte[1];
                y.u_byte[3] = x.u_byte[0];

                (*destination_32u_ptr) = y.u_int;
                (*pp_destination) += sizeof(uint32_t);
            }

            // We can't exceed 32u max
            if (UINT32_MAX > (*index_ptr)) {
                (*index_ptr)++;
            } else {
                return QPL_STS_OUTPUT_OVERFLOW_ERR;
            }

            break;
        }
        default: break;
    }

    return QPL_STS_OK;
}
