/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/19/2018
 * Contains an internal functions for reference implementation of the qpl_extract/Copy
 */

#include "ref_copy.h"

void ref_own_copy_8u(uint8_t* src_ptr, uint8_t* dst_ptr, uint32_t len) {
    for (uint32_t i = 0U; i < len; i++) {
        dst_ptr[i] = src_ptr[i]; // simple memcpy
    }
}

qpl_status ref_copy_le_le_nu(uint8_t* src_ptr, uint32_t src_bit_offset, uint8_t** pp_dst, uint32_t i_offset,
                             uint32_t s_bit, uint32_t len, qpl_out_format o_fmt) {
    uint8_t  src_mask = (uint8_t)REF_LOW_BIT_MASK; // first bit position is 0 in LE notation
    uint8_t  dst_mask = (uint8_t)REF_LOW_BIT_MASK;
    uint8_t  src_bit  = 0U;
    uint32_t idx      = i_offset;

    src_mask <<= src_bit_offset; // first src bit position
    if (qpl_ow_nom == o_fmt) {
        uint8_t* dst_ptr = *pp_dst;

        for (uint32_t n = 0U; n < len; n++) {             // through all mask elements
            for (uint32_t i = 0U; i < s_bit; i++) {       // copy bit by bit
                src_bit = (uint8_t)(*src_ptr & src_mask); // get source bit value
                src_mask <<= 1;                           // shift mask to the next bit position
                if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                    src_mask = (uint8_t)REF_LOW_BIT_MASK; // than init mask to the next byte initial bit position
                    src_ptr++;                            // and shift source pointer to the next byte
                }
                // put current bit value to proper dst position
                *dst_ptr = (src_bit) ? (*dst_ptr | dst_mask) : (*dst_ptr & (~dst_mask));
                dst_mask <<= 1;                           // direction is the same - down -> up (in LE notation)
                if (!dst_mask) {                          // if dst mask is beyond the byte boundaries
                    dst_mask = (uint8_t)REF_LOW_BIT_MASK; // set it to initial bit of the byte
                    dst_ptr++;                            // and shift pointer to the next byte
                }
            } // for s_bit
        }     // for len
        *pp_dst = dst_ptr;
        if (REF_LOW_BIT_MASK < dst_mask) {
            (**pp_dst) &= (dst_mask - 1); // zero unused bits in the last byte
            (*pp_dst)++;
        }
    } // if qpl_ow_nom
    else if (qpl_ow_8 == o_fmt) {
        uint8_t* dst_ptr = *pp_dst;
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint8_t val = 0U;

            for (uint32_t i = 0U; i < s_bit; i++) {       // copy bit by bit
                src_bit = (uint8_t)(*src_ptr & src_mask); // get source bit value
                src_mask <<= 1;                           // shift mask to the next bit position
                if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                    src_mask = (uint8_t)REF_LOW_BIT_MASK; // than init mask to the next byte initial bit position
                    src_ptr++;                            // and shift source pointer to the next byte
                }
                // put current bit value to proper dst position
                val = (src_bit) ? (val | (1 << i)) : val;
            }                  // for s_bit
            if (1U == s_bit) { // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT8_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst_ptr = (uint8_t)idx;
                    dst_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst_ptr = (uint8_t)val;
                dst_ptr++;
            }
        } // for len
        *pp_dst = dst_ptr;
    } else if (qpl_ow_16 == o_fmt) {
        uint16_t* dst_ptr = (uint16_t*)*pp_dst;
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint16_t val = 0U;

            for (uint32_t i = 0U; i < s_bit; i++) {       // copy bit by bit
                src_bit = (uint8_t)(*src_ptr & src_mask); // get source bit value
                src_mask <<= 1;                           // shift mask to the next bit position
                if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                    src_mask = (uint8_t)REF_LOW_BIT_MASK; // than init mask to the next byte initial bit position
                    src_ptr++;                            // and shift source pointer to the next byte
                }
                // put current bit value to proper dst position
                val = (src_bit) ? (val | (1 << i)) : val;
            }                  // for s_bit
            if (1U == s_bit) { // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT16_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst_ptr = (uint16_t)idx;
                    dst_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst_ptr = (uint16_t)val;
                dst_ptr++;
            }
        } // for len
        *pp_dst = (uint8_t*)dst_ptr;
    } else if (qpl_ow_32 == o_fmt) {
        uint32_t* dst_ptr = (uint32_t*)*pp_dst;
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint32_t val = 0U;

            for (uint32_t i = 0U; i < s_bit; i++) {       // copy bit by bit
                src_bit = (uint8_t)(*src_ptr & src_mask); // get source bit value
                src_mask <<= 1;                           // shift mask to the next bit position
                if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                    src_mask = (uint8_t)REF_LOW_BIT_MASK; // than init mask to the next byte initial bit position
                    src_ptr++;                            // and shift source pointer to the next byte
                }
                // put current bit value to proper dst position
                val = (src_bit) ? (val | (1 << i)) : val;
            }                  // for s_bit
            if (1U == s_bit) { // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT32_MAX == idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst_ptr = (uint32_t)idx;
                    dst_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst_ptr = (uint32_t)val;
                dst_ptr++;
            }
        } // for len

        *pp_dst = (uint8_t*)dst_ptr;
    }

    return QPL_STS_OK;
}

qpl_status ref_copy_le_be_nu(uint8_t* src_ptr, uint32_t src_bit_offset, uint8_t** pp_dst, uint32_t i_offset,
                             uint32_t s_bit, uint32_t len, qpl_out_format o_fmt) {
    uint8_t  src_mask        = (uint8_t)(REF_LOW_BIT_MASK << src_bit_offset); // init src bit mask
    uint8_t  dst_mask        = 0U;
    uint8_t  src_bit         = 0U;
    uint8_t* current_dst_ptr = NULL; // ptr to current destination byte
    uint64_t dst_bit_offset  = 0U;   // init dst offset position
    uint32_t idx             = i_offset;

    if (qpl_ow_nom == o_fmt) {
        uint8_t* dst_ptr = *pp_dst;
        for (uint32_t n = 0U; n < len; n++) { // for all mask elements
            dst_bit_offset += s_bit;          // first bit of n-bit BE number in some dst_ptr byte

            // pointer to the actual destination byte;  "-1" - because we need the last bit of actual value,
            // not the next one
            current_dst_ptr = dst_ptr + ((dst_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);

            // bit mask to actual position (actual offset of the 1st bit in the actual destination byte)
            dst_mask = REF_HIGH_BIT_MASK >> ((dst_bit_offset - 1) & REF_MAX_BIT_IDX);
            for (uint32_t i = 0U; i < s_bit; i++) {       // copy bit by bit
                src_bit = (uint8_t)(*src_ptr & src_mask); // get source bit value
                src_mask <<= 1;                           // shift mask to the next bit position
                if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                    src_mask = (uint8_t)REF_LOW_BIT_MASK; // than init mask to the next byte initial bit position
                    src_ptr++;                            // and shift source pointer to the next byte
                }
                // put current bit value to proper dst position
                *current_dst_ptr = (src_bit) ? (*current_dst_ptr | dst_mask) : (*current_dst_ptr & (~dst_mask));

                // direction is the same - down -> up (in LE notation)
                dst_mask <<= 1;
                if (!dst_mask) {                          // if dst mask is beyond the byte boundaries
                    dst_mask = (uint8_t)REF_LOW_BIT_MASK; // set it to initial bit of the byte
                    current_dst_ptr--;                    // and shift pointer to the next byte
                }
            } // for s_bit
        }     // for len
        *pp_dst += ((dst_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
        dst_mask = (uint8_t) ~((REF_HIGH_BIT_MASK >> ((dst_bit_offset - 1) & REF_MAX_BIT_IDX)) - 1);
        if (0U != dst_mask) {
            (**pp_dst) &= dst_mask; // zero unused bits in the last byte
            (*pp_dst)++;
        }
    } else if (qpl_ow_8 == o_fmt) { // if qpl_ow_nom
        uint8_t* dst_ptr = *pp_dst;
        uint8_t  val     = 0U;

        for (uint32_t n = 0U; n < len; n++) { // for all mask elements
            val = 0U;
            for (uint32_t i = 0U; i < s_bit; i++) {       // copy bit by bit
                src_bit = (uint8_t)(*src_ptr & src_mask); // get source bit value
                src_mask <<= 1;                           // shift mask to the next bit position
                if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                    src_mask = (uint8_t)REF_LOW_BIT_MASK; // than init mask to the next byte initial bit position
                    src_ptr++;                            // and shift source pointer to the next byte
                }
                // put current bit value to proper dst position
                val = (src_bit) ? (val | (1 << i)) : val;
            }                  // for s_bit
            if (1U == s_bit) { // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT8_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst_ptr = (uint8_t)idx;
                    dst_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst_ptr = (uint8_t)val;
                dst_ptr++;
            }
        } // for len
        *pp_dst = dst_ptr;
    } else if (qpl_ow_16 == o_fmt) {
        uint16_t* dst16_ptr = (uint16_t*)(*pp_dst);
        uint16_t  val       = 0U;

        for (uint32_t n = 0U; n < len; n++) { // for all mask elements
            val = 0U;
            for (uint32_t i = 0U; i < s_bit; i++) {       // copy bit by bit
                src_bit = (uint8_t)(*src_ptr & src_mask); // get source bit value
                src_mask <<= 1;                           // shift mask to the next bit position
                if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                    src_mask = (uint8_t)REF_LOW_BIT_MASK; // than init mask to the next byte initial bit position
                    src_ptr++;                            // and shift source pointer to the next byte
                }
                // put current bit value to proper dst position
                val = (src_bit) ? (val | (1 << i)) : val;
            }                  // for s_bit
            if (1U == s_bit) { // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT16_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst16_ptr = (uint16_t)ref_cvt_le_2_be_16u(idx);
                    dst16_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst16_ptr = (uint16_t)ref_cvt_le_2_be_16u(val);
                dst16_ptr++;
            }
        } // for len
        *pp_dst = (uint8_t*)dst16_ptr;
    } else if (qpl_ow_32 == o_fmt) {
        uint32_t* dst32_ptr = (uint32_t*)(*pp_dst);
        uint32_t  val       = 0U;

        for (uint32_t n = 0U; n < len; n++) { // for all mask elements
            val = 0U;
            for (uint32_t i = 0U; i < s_bit; i++) {       // copy bit by bit
                src_bit = (uint8_t)(*src_ptr & src_mask); // get source bit value
                src_mask <<= 1;                           // shift mask to the next bit position
                if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                    src_mask = (uint8_t)REF_LOW_BIT_MASK; // than init mask to the next byte initial bit position
                    src_ptr++;                            // and shift source pointer to the next byte
                }
                // put current bit value to proper dst position
                val = (src_bit) ? (val | (1 << i)) : val;
            }                  // for s_bit
            if (1U == s_bit) { // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT32_MAX == idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst32_ptr = (uint32_t)ref_cvt_le_2_be_32u(idx);
                    dst32_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst32_ptr = (uint32_t)ref_cvt_le_2_be_32u(val);
                dst32_ptr++;
            }
        } // for len
        *pp_dst = (uint8_t*)dst32_ptr;
    }
    return QPL_STS_OK;
}

qpl_status ref_copy_be_le_nu(uint8_t* src_ptr, uint32_t src_bit_offset, uint8_t** pp_dst, uint32_t i_offset,
                             uint32_t s_bit, uint32_t len, qpl_out_format o_fmt) {
    uint8_t  src_mask        = (uint8_t)REF_LOW_BIT_MASK; // src bit mask
    uint8_t  dst_mask        = (uint8_t)REF_LOW_BIT_MASK; // init dst bit mask
    uint8_t  src_bit         = 0U;                        // current source bit
    uint32_t idx             = i_offset;                  // count for nominal bit-vector and output modification
    uint8_t* current_src_ptr = NULL;

    if (qpl_ow_nom == o_fmt) {
        uint8_t* dst_ptr = *pp_dst;

        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            src_bit_offset += s_bit;
            current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
            src_mask        = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);

            for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                src_mask <<= 1;                                   // shift mask to the next bit position (BE)
                if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                    src_mask = (uint8_t)REF_LOW_BIT_MASK; // than init mask to the next byte initial bit position
                    current_src_ptr--;                    // and shift source pointer to the next byte
                }
                *dst_ptr = (src_bit) ? (*dst_ptr | dst_mask)
                                     : (*dst_ptr & (~dst_mask)); // put current bit value to proper dst position
                dst_mask <<= 1;                                  // direction is the same - down -> up (in LE notation)
                if (!dst_mask) {                                 // if dst mask is beyond the byte boundaries
                    dst_mask = (uint8_t)REF_LOW_BIT_MASK;        // set it to initial bit of the byte
                    dst_ptr++;                                   // and shift pointer to the next byte
                }
            } // for s_bit
        }     // for len
        *pp_dst = dst_ptr;
        if (REF_LOW_BIT_MASK < dst_mask) {
            (**pp_dst) &= (dst_mask - 1); // zero unused bits in the last byte
            (*pp_dst)++;
        }
    } // if qpl_ow_nom
    else if (qpl_ow_8 == o_fmt) {
        uint8_t* dst_ptr = *pp_dst;

        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint8_t val = 0U;

            src_bit_offset += s_bit;
            current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
            src_mask        = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);

            for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                src_mask <<= 1;                                   // shift mask to the next bit position
                if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                    src_mask = (uint8_t)REF_LOW_BIT_MASK; // than init mask to the next byte initial bit position
                    current_src_ptr--;                    // and shift source pointer to the next byte
                }
                // put current bit value to proper dst position
                val = (src_bit) ? (val | (1 << i)) : val;
            }                  // for s_bit
            if (1U == s_bit) { // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT8_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst_ptr = (uint8_t)idx;
                    dst_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst_ptr = (uint8_t)val;
                dst_ptr++;
            }
        } // for len
        *pp_dst = dst_ptr;
    } else if (qpl_ow_16 == o_fmt) {
        uint16_t* dst16_ptr = (uint16_t*)(*pp_dst);
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint16_t val = 0U;

            src_bit_offset += s_bit;
            current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
            src_mask        = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);
            for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                src_mask <<= 1;                                   // shift mask to the next bit position
                if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                    src_mask = (uint8_t)REF_LOW_BIT_MASK; // than init mask to the next byte initial bit position
                    current_src_ptr--;                    // and shift source pointer to the next byte
                }
                // put current bit value to proper dst position
                val = (src_bit) ? (val | (1 << i)) : val;
            }                  // for s_bit
            if (1U == s_bit) { // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT16_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst16_ptr = (uint16_t)idx;
                    dst16_ptr++;
                }
                idx++;
            } else if (1U != s_bit) { // otherwise store all values itself
                *dst16_ptr = (uint16_t)val;
                dst16_ptr++;
            }
        } // for len
        *pp_dst = (uint8_t*)dst16_ptr;
    } else if (qpl_ow_32 == o_fmt) {
        uint32_t* dst32_ptr = (uint32_t*)(*pp_dst);
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint32_t val = 0U;

            src_bit_offset += s_bit;
            current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
            src_mask        = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);

            for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                src_mask <<= 1;                                   // shift mask to the next bit position
                if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                    src_mask = (uint8_t)REF_LOW_BIT_MASK; // than init mask to the next byte initial bit position
                    current_src_ptr--;                    // and shift source pointer to the next byte
                }
                // put current bit value to proper dst position
                val = (src_bit) ? (val | (1 << i)) : val;
            }                  // for s_bit
            if (1U == s_bit) { // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT32_MAX == idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst32_ptr = (uint32_t)idx;
                    dst32_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst32_ptr = (uint32_t)val;
                dst32_ptr++;
            }
        } // for len
        *pp_dst = (uint8_t*)dst32_ptr;
    }
    return QPL_STS_OK;
}

qpl_status ref_copy_be_be_nu(uint8_t* src_ptr, uint32_t src_bit_offset, uint8_t** pp_dst, uint32_t i_offset,
                             uint32_t s_bit, uint32_t len, qpl_out_format o_fmt) {
    uint8_t  src_mask        = REF_LOW_BIT_MASK; // src bit mask
    uint8_t  dst_mask        = REF_LOW_BIT_MASK; // dst bit mask
    uint8_t  src_bit         = 0U;               // current source bit
    uint32_t idx             = i_offset;         // count for nominal bit-vector and output modification
    uint64_t dst_bit_offset  = 0U;
    uint8_t* current_src_ptr = NULL;
    uint8_t* current_dst_ptr = NULL;

    if (qpl_ow_nom == o_fmt) {
        uint8_t* dst_ptr = *pp_dst;

        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            src_bit_offset += s_bit;
            current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
            src_mask        = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);
            dst_bit_offset += s_bit;
            current_dst_ptr = dst_ptr + ((dst_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
            dst_mask        = REF_HIGH_BIT_MASK >> ((dst_bit_offset - 1) & REF_MAX_BIT_IDX);
            for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                src_mask <<= 1;                                   // shift mask to the next bit position (BE)
                if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                    src_mask = (uint8_t)REF_LOW_BIT_MASK; // than init mask to the next byte initial bit position
                    current_src_ptr--;                    // and shift source pointer to the next byte
                }
                // put current bit value to proper dst position
                *current_dst_ptr = (src_bit) ? (*current_dst_ptr | dst_mask) : (*current_dst_ptr & (~dst_mask));
                dst_mask <<= 1;                           // direction is the same - down -> up (in LE notation)
                if (!dst_mask) {                          // if dst mask is beyond the byte boundaries
                    dst_mask = (uint8_t)REF_LOW_BIT_MASK; // set it to initial bit of the byte
                    current_dst_ptr--;                    // and shift pointer to the next byte
                }
            } // for s_bit
        }     // for len
        *pp_dst += ((dst_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
        dst_mask = (uint8_t) ~((REF_HIGH_BIT_MASK >> ((dst_bit_offset - 1) & REF_MAX_BIT_IDX)) - 1);
        if (0U != dst_mask) {
            (**pp_dst) &= dst_mask; // zero unused bits in the last byte
            (*pp_dst)++;
        }
    } else if (qpl_ow_8 == o_fmt) { // if qpl_ow_nom
        uint8_t* dst_ptr = *pp_dst;

        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint8_t val = 0U;

            src_bit_offset += s_bit;
            current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
            src_mask        = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);

            for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                src_mask <<= 1;                                   // shift mask to the next bit position
                if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                    src_mask = (uint8_t)REF_LOW_BIT_MASK; // than init mask to the next byte initial bit position
                    current_src_ptr--;                    // and shift source pointer to the next byte
                }
                // put current bit value to proper dst position
                val = (src_bit) ? (val | (1 << i)) : val;
            }                  // for s_bit
            if (1U == s_bit) { // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT8_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst_ptr = (uint8_t)idx;
                    dst_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst_ptr = (uint8_t)val;
                dst_ptr++;
            }
        } // for len
        *pp_dst = dst_ptr;
    } else if (qpl_ow_16 == o_fmt) {
        uint16_t* dst16_ptr = (uint16_t*)(*pp_dst);
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint16_t val = 0U;

            src_bit_offset += s_bit;
            current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
            src_mask        = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);

            for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                src_mask <<= 1;                                   // shift mask to the next bit position
                if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                    src_mask = (uint8_t)REF_LOW_BIT_MASK; // than init mask to the next byte initial bit position
                    current_src_ptr--;                    // and shift source pointer to the next byte
                }
                // put current bit value to proper dst position
                val = (src_bit) ? (val | (1 << i)) : val;
            }                  // for s_bit
            if (1U == s_bit) { // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT16_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst16_ptr = ref_cvt_le_2_be_16u(idx);
                    dst16_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst16_ptr = ref_cvt_le_2_be_16u(val);
                dst16_ptr++;
            }
        } // for len
        *pp_dst = (uint8_t*)dst16_ptr;
    } else if (qpl_ow_32 == o_fmt) {
        uint32_t* dst32_ptr = (uint32_t*)(*pp_dst);
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint32_t val = 0U;

            src_bit_offset += s_bit;
            current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
            src_mask        = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);

            for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                src_mask <<= 1;                                   // shift mask to the next bit position
                if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                    src_mask = (uint8_t)REF_LOW_BIT_MASK; // than init mask to the next byte initial bit position
                    current_src_ptr--;                    // and shift source pointer to the next byte
                }
                // put current bit value to proper dst position
                val = (src_bit) ? (val | (1 << i)) : val;
            }                  // for s_bit
            if (1U == s_bit) { // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT32_MAX == idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst32_ptr = ref_cvt_le_2_be_32u(idx);
                    dst32_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst32_ptr = ref_cvt_le_2_be_32u(val);
                dst32_ptr++;
            }
        } // for len
        *pp_dst = (uint8_t*)dst32_ptr;
    }
    return QPL_STS_OK;
}

qpl_status ref_store_val(uint32_t value, uint8_t* dst_ptr, uint32_t* idx_ptr, uint32_t current_idx, uint32_t s_bit,
                         uint32_t flag_be, qpl_out_format o_fmt) {
    uint32_t idx = *idx_ptr;

    if ((flag_be) && (qpl_ow_nom == o_fmt)) { // if dst has BE format
        uint32_t dst_bit_offset = (idx + 1U) * s_bit - 1U;
        // get actual dst byte for given index idx of s_bit value
        uint8_t* current_dst_ptr = dst_ptr + (dst_bit_offset >> REF_BIT_LEN_2_BYTE);
        // get actual bit position for given idx of s_bit value
        uint8_t  dst_mask = REF_HIGH_BIT_MASK >> (dst_bit_offset & REF_MAX_BIT_IDX);
        uint8_t  z_mask   = ~(dst_mask - 1U);
        uint32_t src_bit  = 0U;

        if (0U != z_mask) {
            (*current_dst_ptr) &= dst_mask; // zero unused bits in the last byte
        }
        for (uint32_t i = 0U; i < s_bit; i++) {     // copy bit by bit
            src_bit = (uint32_t)(value & (1 << i)); // get source bit value
            // put current bit value to proper dst position
            *current_dst_ptr = (src_bit) ? (*current_dst_ptr | dst_mask) : (*current_dst_ptr & (~dst_mask));
            dst_mask <<= 1;                           // next bit position
            if (!dst_mask) {                          // if dst mask is beyond the byte boundaries
                dst_mask = (uint8_t)REF_LOW_BIT_MASK; // set it to initial bit of the next byte
                current_dst_ptr--;                    // and shift pointer to the next byte
            }
        } // for s_bit
        (*idx_ptr)++;
    } else if (qpl_ow_8 == o_fmt) {               // LE or BE doesn't matter for 8u data type
        uint8_t* current_dst_ptr = dst_ptr + idx; // get actual dst ptr for given index idx of 8u value
        if (1U < s_bit) {
            *current_dst_ptr = (uint8_t)value;
            (*idx_ptr)++;
        } else {
            if (0U < value) {
                if (UINT8_MAX <= current_idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                *current_dst_ptr = (uint8_t)current_idx;
                (*idx_ptr)++;
            }
        }
    } else if ((flag_be) && (qpl_ow_16 == o_fmt)) {
        // get actual dst ptr for given index idx of 16u value
        uint16_t* current_dst_ptr = (uint16_t*)dst_ptr + idx;
        if (1U < s_bit) {
            *current_dst_ptr = ref_cvt_le_2_be_16u(value);
            (*idx_ptr)++;
        } else {
            if (0U < value) {
                if (UINT16_MAX <= current_idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                *current_dst_ptr = (uint16_t)ref_cvt_le_2_be_16u(current_idx);
                (*idx_ptr)++;
            }
        }
    } else if ((flag_be) && (qpl_ow_32 == o_fmt)) {
        // get actual dst ptr for given index idx of 32u value
        uint32_t* current_dst_ptr = (uint32_t*)dst_ptr + idx;
        if (1U < s_bit) {
            *current_dst_ptr = ref_cvt_le_2_be_32u(value);
            (*idx_ptr)++;
        } else {
            if (0U < value) {
                if (UINT32_MAX == current_idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                *current_dst_ptr = (uint32_t)ref_cvt_le_2_be_32u(current_idx);
                (*idx_ptr)++;
            }
        }
    } else if ((!flag_be) && (qpl_ow_nom == o_fmt)) { // LE dst format
        // get actual dst byte for given index idx of s_bit value
        uint8_t* current_dst_ptr = dst_ptr + ((idx * s_bit) >> REF_BIT_LEN_2_BYTE);
        // get actual bit position for given idx of s_bit value
        uint8_t dst_mask = REF_LOW_BIT_MASK << ((idx * s_bit) & REF_MAX_BIT_IDX);

        for (uint32_t i = 0U; i < s_bit; i++) {              // copy bit by bit
            uint32_t src_bit = (uint32_t)(value & (1 << i)); // get source bit value
            // put current bit value to proper dst position
            *current_dst_ptr = (src_bit) ? (*current_dst_ptr | dst_mask) : (*current_dst_ptr & (~dst_mask));
            dst_mask <<= 1;                            // direction is down -> up (in LE notation)
            if (!dst_mask) {                           // if dst mask is beyond the byte boundaries
                dst_mask = (uint32_t)REF_LOW_BIT_MASK; // set it to initial bit of the byte
                current_dst_ptr++;                     // and shift pointer to the next byte
            }
        } // for s_bit
        if (REF_LOW_BIT_MASK != dst_mask) {
            *current_dst_ptr &= (dst_mask - 1); // zero unused bits
        }
        (*idx_ptr)++;
    } else if ((!flag_be) && (qpl_ow_16 == o_fmt)) {
        // get actual dst ptr for given index idx of 16u value
        uint16_t* current_dst_ptr = (uint16_t*)dst_ptr + idx;
        if (1U < s_bit) {
            *current_dst_ptr = value;
            (*idx_ptr)++;
        } else {
            if (0U < value) {
                if (UINT16_MAX <= current_idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                *current_dst_ptr = (uint16_t)current_idx;
                (*idx_ptr)++;
            }
        }
    } else if ((!flag_be) && (qpl_ow_32 == o_fmt)) {
        // get actual dst ptr for given index idx of 32u value
        uint32_t* current_dst_ptr = (uint32_t*)dst_ptr + idx;
        if (1U < s_bit) {
            *current_dst_ptr = value;
            (*idx_ptr)++;
        } else {
            if (0U < value) {
                if (UINT32_MAX == current_idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                *current_dst_ptr = (uint32_t)current_idx;
                (*idx_ptr)++;
            }
        }
    }
    return QPL_STS_OK;
}

qpl_status ref_copy_mask_le_le_nu(uint8_t* src_ptr, uint8_t* mask_ptr, uint32_t mask_be, uint8_t** pp_dst,
                                  uint32_t i_offset, uint64_t* output_bits_ptr, uint32_t s_bit, uint32_t len,
                                  qpl_out_format o_fmt) {
    uint8_t  src_mask = (uint8_t)REF_LOW_BIT_MASK;                                            // init src bit mask
    uint8_t  dst_mask = (uint8_t)REF_LOW_BIT_MASK;                                            // init dst bit mask
    uint8_t  src_bit  = 0U;                                                                   // current source bit
    uint8_t  msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK); // init mask bit position
    uint32_t m_bit  = 0U;        // current src2 bit (mask bit)
    uint8_t  m_byte = *mask_ptr; // get mask byte
    uint32_t idx    = i_offset;  // count for nominal bit-vector and output modification
    uint8_t* current_src_ptr = NULL;
    uint64_t src_bit_offset  = 0U;
    if (qpl_ow_nom == o_fmt) {
        uint8_t* dst_ptr = *pp_dst;
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            m_bit = m_byte & msk;             // get mask bit
            // shift mask bit-mask to the next position
            msk = (mask_be) ? msk >> 1 : msk << 1;
            if (!msk) {     // if bit-mask is beyond byte boundaries
                mask_ptr++; // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get next mask byte
            }
            if (m_bit) {                                                            // if 1 - store src s_bit value
                current_src_ptr = src_ptr + (src_bit_offset >> REF_BIT_LEN_2_BYTE); // current src byte
                src_mask        = REF_LOW_BIT_MASK << (src_bit_offset & REF_MAX_BIT_IDX); // current src mask
                if (s_bit > *output_bits_ptr) {                                           // control dst buffer
                    return QPL_STS_DST_IS_SHORT_ERR; // if upper boundary reached - than return error
                }
                *output_bits_ptr -= s_bit;                            // for dst upper boundary check
                for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                                   // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr++; // and shift source pointer to the next byte
                    }
                    *dst_ptr = (src_bit) ? (*dst_ptr | dst_mask)
                                         : (*dst_ptr & (~dst_mask)); // put current bit value to proper dst position
                    dst_mask <<= 1;                           // direction is the same - down -> up (in LE notation)
                    if (!dst_mask) {                          // if dst mask is beyond the byte boundaries
                        dst_mask = (uint8_t)REF_LOW_BIT_MASK; // set it to initial bit of the byte
                        dst_ptr++;                            // and shift pointer to the next byte
                    }
                }                    // for s_bit
            }                        // if m_bit
            src_bit_offset += s_bit; // next src value
        }                            // for len
        *pp_dst = dst_ptr;
        if (REF_LOW_BIT_MASK < dst_mask) {
            (**pp_dst) &= (dst_mask - 1); // zero unused bits in the last byte
            (*pp_dst)++;
        }
    } else if (qpl_ow_8 == o_fmt) { // if qpl_ow_nom
        uint8_t* dst_ptr = *pp_dst;

        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint8_t val = 0U;

            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask bit-mask to the next position
            if (!msk) {                              // if bit-mask is beyond byte boundaries
                mask_ptr++;                          // and load next mask byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK)
                                   : ((uint8_t)REF_LOW_BIT_MASK); // init mask bit for the new byte
                m_byte = *mask_ptr;                               // get next mask byte
            }
            if (m_bit) { // if not 0 - store src s_bit value (if it is not a bit vector)
                current_src_ptr = src_ptr + (src_bit_offset >> REF_BIT_LEN_2_BYTE);       // current src byte
                src_mask        = REF_LOW_BIT_MASK << (src_bit_offset & REF_MAX_BIT_IDX); // current src mask
                for (uint32_t i = 0U; i < s_bit; i++) {                                   // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask);                     // get source bit value
                    src_mask <<= 1;  // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr++; // and shift source pointer to the next byte
                    }
                    // put current bit value to proper dst position
                    val = (src_bit) ? (val | (1 << i)) : val;
                }                  // for s_bit
                if (1U == s_bit) { // store non-zero value index for bit vector
                    if (0U < val) {
                        if (UINT8_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                        if (REF_8U_BITS > *output_bits_ptr) { // control dst buffer
                            return QPL_STS_DST_IS_SHORT_ERR;  // if upper boundary reached - than return error
                        }
                        *dst_ptr = (uint8_t)idx;
                        dst_ptr++;
                        *output_bits_ptr -= REF_8U_BITS; // for dst upper boundary check
                    }
                    idx++;
                } else {                                  // otherwise store all values itself
                    if (REF_8U_BITS > *output_bits_ptr) { // control dst buffer
                        return QPL_STS_DST_IS_SHORT_ERR;  // if upper boundary reached - than return error
                    }
                    *dst_ptr = (uint8_t)val;
                    dst_ptr++;
                    *output_bits_ptr -= REF_8U_BITS; // for dst upper boundary check
                }
            }                        // if m_bit
            src_bit_offset += s_bit; // next src value
        }                            // for len
        *pp_dst = dst_ptr;
    } else if (qpl_ow_16 == o_fmt) {
        uint16_t* dst16_ptr = (uint16_t*)(*pp_dst);
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint16_t val = 0U;

            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask bit-mask to the next position
            if (!msk) {                              // if bit-mask is beyond byte boundaries
                mask_ptr++;                          // and load next mask byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK)
                                   : ((uint8_t)REF_LOW_BIT_MASK); // init mask bit for the new byte
                m_byte = *mask_ptr;                               // get next mask byte
            }
            if (m_bit) { // if not 0 - store src s_bit value (if it is not a bit vector)
                current_src_ptr = src_ptr + (src_bit_offset >> REF_BIT_LEN_2_BYTE);       // current src byte
                src_mask        = REF_LOW_BIT_MASK << (src_bit_offset & REF_MAX_BIT_IDX); // current src mask
                for (uint32_t i = 0U; i < s_bit; i++) {                                   // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask);                     // get source bit value
                    src_mask <<= 1;  // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr++; // and shift source pointer to the next byte
                    }
                    // put current bit value to proper dst position
                    val = (src_bit) ? (val | (1 << i)) : val;
                }                  // for s_bit
                if (1U == s_bit) { // store non-zero value index for bit vector
                    if (0U < val) {
                        if (UINT16_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                        if (REF_16U_BITS > *output_bits_ptr) { // control dst buffer
                            return QPL_STS_DST_IS_SHORT_ERR;   // if upper boundary reached - than return error
                        }
                        *dst16_ptr = (uint16_t)idx;
                        dst16_ptr++;
                        *output_bits_ptr -= REF_16U_BITS; // for dst upper boundary check
                    }
                    idx++;
                } else {                                   // otherwise store all values itself
                    if (REF_16U_BITS > *output_bits_ptr) { // control dst buffer
                        return QPL_STS_DST_IS_SHORT_ERR;   // if upper boundary reached - than return error
                    }
                    *dst16_ptr = (uint16_t)val;
                    dst16_ptr++;
                    *output_bits_ptr -= REF_16U_BITS; // for dst upper boundary check
                }
            }                        // if m_bit
            src_bit_offset += s_bit; // next src value
        }                            // for len
        *pp_dst = (uint8_t*)dst16_ptr;
    } else if (qpl_ow_32 == o_fmt) {
        uint32_t* dst32_ptr = (uint32_t*)(*pp_dst);
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint32_t val = 0U;

            m_bit = m_byte & msk; // get mask bit
            // shift mask bit-mask to the next position
            msk = (mask_be) ? msk >> 1 : msk << 1;
            if (!msk) {     // if bit-mask is beyond byte boundaries
                mask_ptr++; // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // grt next mask byte
            }
            if (m_bit) { // if not 0 - store src s_bit value (if it is not a bit vector)
                current_src_ptr = src_ptr + (src_bit_offset >> REF_BIT_LEN_2_BYTE);       // current src byte
                src_mask        = REF_LOW_BIT_MASK << (src_bit_offset & REF_MAX_BIT_IDX); // current src mask
                for (uint32_t i = 0U; i < s_bit; i++) {                                   // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask);                     // get source bit value
                    src_mask <<= 1;  // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr++; // and shift source pointer to the next byte
                    }
                    // put current bit value to proper dst position
                    val = (src_bit) ? (val | (1 << i)) : val;
                }                  // for s_bit
                if (1U == s_bit) { // store non-zero value index for bit vector
                    if (0U < val) {
                        if (UINT32_MAX == idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                        if (REF_32U_BITS > *output_bits_ptr) { // control dst buffer
                            return QPL_STS_DST_IS_SHORT_ERR;   // if upper boundary reached - than return error
                        }

                        *dst32_ptr = (uint32_t)idx;
                        dst32_ptr++;
                        *output_bits_ptr -= REF_32U_BITS; // for dst upper boundary check
                    }
                    idx++;
                } else {                                   // otherwise store all values itself
                    if (REF_32U_BITS > *output_bits_ptr) { // control dst buffer
                        return QPL_STS_DST_IS_SHORT_ERR;   // if upper boundary reached - than return error
                    }
                    *dst32_ptr = (uint32_t)val;
                    dst32_ptr++;
                    *output_bits_ptr -= REF_32U_BITS; // for dst upper boundary check
                }
            }                        // if m_bit
            src_bit_offset += s_bit; // next src value
        }                            // for len
        *pp_dst = (uint8_t*)dst32_ptr;
    }
    return QPL_STS_OK;
}

qpl_status ref_copy_mask_le_be_nu(uint8_t* src_ptr, uint8_t* mask_ptr, uint32_t mask_be, uint8_t** pp_dst,
                                  uint32_t i_offset, uint64_t* output_bits_ptr, uint32_t s_bit, uint32_t len,
                                  qpl_out_format o_fmt) {
    uint8_t src_mask = (uint8_t)REF_LOW_BIT_MASK; // init src bit mask
    uint8_t dst_mask = 0U;
    uint8_t src_bit  = 0U;
    // init mask bit-mask
    uint8_t  msk             = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
    uint32_t m_bit           = 0U;
    uint8_t* current_dst_ptr = NULL;      // ptr to current destination byte
    uint64_t dst_bit_offset  = 0U;        // init dst offset position
    uint8_t  m_byte          = *mask_ptr; // get 1st mask byte
    uint32_t idx             = i_offset;
    uint8_t* current_src_ptr = NULL;
    uint64_t src_bit_offset  = 0U;

    if (qpl_ow_nom == o_fmt) {
        uint8_t* dst_ptr = *pp_dst;
        for (uint32_t n = 0U; n < len; n++) { // for all mask elements
            m_bit = m_byte & msk;             // get mask bit
            // shift mask to the next position
            msk = (mask_be) ? msk >> 1 : msk << 1;
            if (!msk) {     // if mask is beyond byte boundaries
                mask_ptr++; // shift mask ptr to the next position
                // init mask bit-mask
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get new mask byte
            }
            if (m_bit) { // if mask bit is not zero - put src s_bit value to dst
                current_src_ptr = src_ptr + (src_bit_offset >> REF_BIT_LEN_2_BYTE);       // current src byte
                src_mask        = REF_LOW_BIT_MASK << (src_bit_offset & REF_MAX_BIT_IDX); // current src mask
                dst_bit_offset += s_bit; // first bit of n-bit BE number in some dst_ptr byte
                // pointer to the actual destination byte;  "-1" - because we need the last bit of actual value,
                // not the next one
                current_dst_ptr = dst_ptr + ((dst_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
                if (s_bit > *output_bits_ptr) {      // control dst buffer
                    return QPL_STS_DST_IS_SHORT_ERR; // if reached - than error
                }
                *output_bits_ptr -= s_bit; // for dst upper boundary check
                // bit mask to actual position (actual offset of the 1st bit in the actual destination byte)
                dst_mask = REF_HIGH_BIT_MASK >> ((dst_bit_offset - 1) & REF_MAX_BIT_IDX);
                for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                                   // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr++; // and shift source pointer to the next byte
                    }
                    // put current bit value to proper dst position
                    *current_dst_ptr = (src_bit) ? (*current_dst_ptr | dst_mask) : (*current_dst_ptr & (~dst_mask));
                    dst_mask <<= 1;                           // direction is the same - down -> up (in LE notation)
                    if (!dst_mask) {                          // if dst mask is beyond the byte boundaries
                        dst_mask = (uint8_t)REF_LOW_BIT_MASK; // set it to initial bit of the byte
                        current_dst_ptr--;                    // and shift pointer to the next byte
                    }
                }                    // for s_bit
            }                        // if m_bit
            src_bit_offset += s_bit; // next src value
        }                            // for len
        if (0U < dst_bit_offset) {
            *pp_dst += ((dst_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
            dst_mask = (uint8_t) ~((REF_HIGH_BIT_MASK >> ((dst_bit_offset - 1) & REF_MAX_BIT_IDX)) - 1);
            if (0U != dst_mask) {
                (**pp_dst) &= dst_mask; // zero unused bits in the last byte
                (*pp_dst)++;
            }
        }
    } else if (qpl_ow_8 == o_fmt) { // if qpl_ow_nom
        uint8_t* dst_ptr = *pp_dst;

        for (uint32_t n = 0U; n < len; n++) { // for all mask elements
            uint8_t val = 0U;

            m_bit = m_byte & msk; // get mask bit
            // shift mask to the next position
            msk = (mask_be) ? msk >> 1 : msk << 1;
            if (!msk) {     // if mask is beyond byte boundaries
                mask_ptr++; // shift mask ptr to the next position
                // init mask bit-mask
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get new mask byte
            }
            if (m_bit) { // if mask bit is non-zero - get s_bit value
                current_src_ptr = src_ptr + (src_bit_offset >> REF_BIT_LEN_2_BYTE);       // current src byte
                src_mask        = REF_LOW_BIT_MASK << (src_bit_offset & REF_MAX_BIT_IDX); // current src mask
                for (uint32_t i = 0U; i < s_bit; i++) {                                   // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask);                     // get source bit value
                    src_mask <<= 1;  // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        src_mask = (uint8_t)REF_LOW_BIT_MASK; // than init mask to the next byte initial bit position
                        current_src_ptr++;                    // and shift source pointer to the next byte
                    }
                    // put current bit value to proper dst position
                    val = (src_bit) ? (val | (1 << i)) : val;
                }                  // for s_bit
                if (1U == s_bit) { // store non-zero value index for bit vector
                    if (0U < val) {
                        if (UINT8_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                        if (REF_8U_BITS > *output_bits_ptr) { // control dst buffer
                            return QPL_STS_DST_IS_SHORT_ERR;  // if upper boundary reached - than return error
                        }
                        *dst_ptr = (uint8_t)idx;
                        dst_ptr++;
                        *output_bits_ptr -= REF_8U_BITS; // for dst upper boundary check
                    }
                    idx++;
                } else {                                  // otherwise store all values itself
                    if (REF_8U_BITS > *output_bits_ptr) { // control dst buffer
                        return QPL_STS_DST_IS_SHORT_ERR;  // if upper boundary reached - than return error
                    }
                    *dst_ptr = (uint8_t)val;
                    dst_ptr++;
                    *output_bits_ptr -= REF_8U_BITS; // for dst upper boundary check
                }
            }                        // if m_bit
            src_bit_offset += s_bit; // next src value
        }                            // for len
        *pp_dst = dst_ptr;
    } else if (qpl_ow_16 == o_fmt) {
        uint16_t* dst16_ptr = (uint16_t*)(*pp_dst);

        for (uint32_t n = 0U; n < len; n++) { // for all mask elements
            uint16_t val = 0U;
            m_bit        = m_byte & msk; // get mask bit
            // shift mask to the next position
            msk = (mask_be) ? msk >> 1 : msk << 1;
            if (!msk) {     // if mask is beyond byte boundaries
                mask_ptr++; // shift mask ptr to the next position
                // init mask bit-mask
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get new mask byte
            }
            if (m_bit) { // if not 0 - store src s_bit value (if it is not a bit vector)
                current_src_ptr = src_ptr + (src_bit_offset >> REF_BIT_LEN_2_BYTE);       // current src byte
                src_mask        = REF_LOW_BIT_MASK << (src_bit_offset & REF_MAX_BIT_IDX); // current src mask
                for (uint32_t i = 0U; i < s_bit; i++) {                                   // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask);                     // get source bit value
                    src_mask <<= 1;  // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr++; // and shift source pointer to the next byte
                    }
                    // put current bit value to proper dst position
                    val = (src_bit) ? (val | (1 << i)) : val;
                }                  // for s_bit
                if (1U == s_bit) { // store non-zero value index for bit vector
                    if (0U < val) {
                        if (UINT16_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                        if (REF_16U_BITS > *output_bits_ptr) { // control dst buffer
                            return QPL_STS_DST_IS_SHORT_ERR;   // if upper boundary reached - than return error
                        }
                        *dst16_ptr = (uint16_t)ref_cvt_le_2_be_16u(idx);
                        dst16_ptr++;
                        *output_bits_ptr -= REF_16U_BITS; // for dst upper boundary check
                    }
                    idx++;
                } else {                                   // otherwise store all values itself
                    if (REF_16U_BITS > *output_bits_ptr) { // control dst buffer
                        return QPL_STS_DST_IS_SHORT_ERR;   // if upper boundary reached - than return error
                    }
                    *dst16_ptr = (uint16_t)ref_cvt_le_2_be_16u(val);
                    dst16_ptr++;
                    *output_bits_ptr -= REF_16U_BITS; // for dst upper boundary check
                }
            }                        // if m_bit
            src_bit_offset += s_bit; // next src value
        }                            // for len
        *pp_dst = (uint8_t*)dst16_ptr;
    } else if (qpl_ow_32 == o_fmt) {
        uint32_t* dst32_ptr = (uint32_t*)(*pp_dst);

        for (uint32_t n = 0U; n < len; n++) { // for all mask elements
            uint32_t val = 0U;

            m_bit = m_byte & msk; // get mask bit
            // shift mask to the next position
            msk = (mask_be) ? msk >> 1 : msk << 1;
            if (!msk) {     // if mask is beyond byte boundaries
                mask_ptr++; // shift mask ptr to the next position
                // init mask bit-mask
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get new mask byte
            }
            if (m_bit) { // if not 0 - store src s_bit value (if it is not a bit vector)
                current_src_ptr = src_ptr + (src_bit_offset >> REF_BIT_LEN_2_BYTE);       // current src byte
                src_mask        = REF_LOW_BIT_MASK << (src_bit_offset & REF_MAX_BIT_IDX); // current src mask
                for (uint32_t i = 0U; i < s_bit; i++) {                                   // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask);                     // get source bit value
                    src_mask <<= 1;  // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr++; // and shift source pointer to the next byte
                    }
                    val = (src_bit) ? (val | (1 << i)) : val; // put current bit value to proper dst position
                }                                             // for s_bit
                if (1U == s_bit) {                            // store non-zero value index for bit vector
                    if (0U < val) {
                        if (UINT32_MAX == idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                        if (REF_32U_BITS > *output_bits_ptr) { // control dst buffer
                            return QPL_STS_DST_IS_SHORT_ERR;   // if upper boundary reached - than return error
                        }
                        *dst32_ptr = (uint32_t)ref_cvt_le_2_be_32u(idx);
                        dst32_ptr++;
                        *output_bits_ptr -= REF_32U_BITS; // for dst upper boundary check
                    }
                    idx++;
                } else {                                   // otherwise store all values itself
                    if (REF_32U_BITS > *output_bits_ptr) { // control dst buffer
                        return QPL_STS_DST_IS_SHORT_ERR;   // if upper boundary reached - than return error
                    }
                    *dst32_ptr = (uint32_t)ref_cvt_le_2_be_32u(val);
                    dst32_ptr++;
                    *output_bits_ptr -= REF_32U_BITS; // for dst upper boundary check
                }
            }                        // if !m_bit
            src_bit_offset += s_bit; // next src value
        }                            // for len
        *pp_dst = (uint8_t*)dst32_ptr;
    }
    return QPL_STS_OK;
}

qpl_status ref_copy_mask_be_le__nu(uint8_t* src_ptr, uint8_t* mask_ptr, uint32_t mask_be, uint8_t** pp_dst,
                                   uint32_t i_offset, uint64_t* output_bits_ptr, uint32_t s_bit, uint32_t len,
                                   qpl_out_format o_fmt) {
    uint8_t src_mask = (uint8_t)REF_LOW_BIT_MASK; // src bit mask
    uint8_t dst_mask = (uint8_t)REF_LOW_BIT_MASK; // init dst bit mask
    uint8_t src_bit  = 0U;                        // current source bit
    // init mask bit position
    uint8_t  msk             = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
    uint32_t m_bit           = 0U;        // current src2 bit (mask bit)
    uint8_t  m_byte          = *mask_ptr; // get mask byte
    uint32_t idx             = i_offset;  // count for nominal bit-vector and output modification
    uint64_t src_bit_offset  = 0U;
    uint8_t* current_src_ptr = NULL;

    if (qpl_ow_nom == o_fmt) {
        uint8_t* dst_ptr = *pp_dst;

        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            m_bit = m_byte & msk;             // get mask bit
            // shift mask bit-mask to the next position
            msk = (mask_be) ? msk >> 1 : msk << 1;
            if (!msk) {     // if bit-mask is beyond byte boundaries
                mask_ptr++; // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get next mask byte
            }
            src_bit_offset += s_bit;
            if (m_bit) { // if !0 - store src s_bit value
                current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
                src_mask        = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);
                if (s_bit > *output_bits_ptr) {      // control dst buffer
                    return QPL_STS_DST_IS_SHORT_ERR; // if upper boundary reached - than return error
                }
                *output_bits_ptr -= s_bit;                            // for dst upper boundary check
                for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                                   // shift mask to the next bit position (BE)
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr--; // and shift source pointer to the next byte
                    }
                    // put current bit value to proper dst position
                    *dst_ptr = (src_bit) ? (*dst_ptr | dst_mask) : (*dst_ptr & (~dst_mask));
                    dst_mask <<= 1;                           // direction is the same - down -> up (in LE notation)
                    if (!dst_mask) {                          // if dst mask is beyond the byte boundaries
                        dst_mask = (uint8_t)REF_LOW_BIT_MASK; // set it to initial bit of the byte
                        dst_ptr++;                            // and shift pointer to the next byte
                    }
                } // for s_bit
            }     // if !m_bit
        }         // for len
        *pp_dst = dst_ptr;
        if (REF_LOW_BIT_MASK < dst_mask) {
            (**pp_dst) &= (dst_mask - 1); // zero unused bits in the last byte
            (*pp_dst)++;
        }
    } else if (qpl_ow_8 == o_fmt) { // if qpl_ow_nom
        uint8_t* dst_ptr = *pp_dst;

        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint8_t val = 0U;

            m_bit = m_byte & msk; // get mask bit
            // shift mask bit-mask to the next position
            msk = (mask_be) ? msk >> 1 : msk << 1;
            if (!msk) {     // if bit-mask is beyond byte boundaries
                mask_ptr++; // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get next mask byte
            }
            src_bit_offset += s_bit;
            if (m_bit) { // if !0 - store src s_bit value (if it is not a bit vector)
                current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
                src_mask        = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);

                for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                                   // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr--; // and shift source pointer to the next byte
                    }
                    // put current bit value to proper dst position
                    val = (src_bit) ? (val | (1 << i)) : val;
                }                  // for s_bit
                if (1U == s_bit) { // store non-zero value index for bit vector
                    if (0U < val) {
                        if (UINT8_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                        if (REF_8U_BITS > *output_bits_ptr) { // control dst buffer
                            return QPL_STS_DST_IS_SHORT_ERR;  // if upper boundary reached - than return error
                        }
                        *dst_ptr = (uint8_t)idx;
                        dst_ptr++;
                        *output_bits_ptr -= REF_8U_BITS; // for dst upper boundary check
                    }
                    idx++;
                } else {                                  // otherwise store all values itself
                    if (REF_8U_BITS > *output_bits_ptr) { // control dst buffer
                        return QPL_STS_DST_IS_SHORT_ERR;  // if upper boundary reached - than return error
                    }
                    *dst_ptr = (uint8_t)val;
                    dst_ptr++;
                    *output_bits_ptr -= REF_8U_BITS; // for dst upper boundary check
                }
            } // if m_bit
        }     // for len
        *pp_dst = dst_ptr;
    } else if (qpl_ow_16 == o_fmt) {
        uint16_t* dst16_ptr = (uint16_t*)(*pp_dst);
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint16_t val = 0U;

            m_bit = m_byte & msk; // get mask bit
            // shift mask bit-mask to the next position
            msk = (mask_be) ? msk >> 1 : msk << 1;
            if (!msk) {     // if bit-mask is beyond byte boundaries
                mask_ptr++; // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get next mask byte
            }
            src_bit_offset += s_bit;
            if (m_bit) { // if !0 - store src s_bit value
                current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
                src_mask        = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);
                for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                                   // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr--; // and shift source pointer to the next byte
                    }
                    // put current bit value to proper dst position
                    val = (src_bit) ? (val | (1 << i)) : val;
                }                  // for s_bit
                if (1U == s_bit) { // store non-zero value index for bit vector
                    if (0U < val) {
                        if (UINT16_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                        if (REF_16U_BITS > *output_bits_ptr) { // control dst buffer
                            return QPL_STS_DST_IS_SHORT_ERR;   // if upper boundary reached - than return error
                        }
                        *dst16_ptr = (uint16_t)idx;
                        dst16_ptr++;
                        *output_bits_ptr -= REF_16U_BITS; // for dst upper boundary check
                    }
                    idx++;
                } else {                                   // otherwise store all values itself
                    if (REF_16U_BITS > *output_bits_ptr) { // control dst buffer
                        return QPL_STS_DST_IS_SHORT_ERR;   // if upper boundary reached - than return error
                    }
                    *dst16_ptr = (uint16_t)val;
                    dst16_ptr++;
                    *output_bits_ptr -= REF_16U_BITS; // for dst upper boundary check
                }
            } // if !m_bit
        }     // for len
        *pp_dst = (uint8_t*)dst16_ptr;
    } else if (qpl_ow_32 == o_fmt) {
        uint32_t* dst32_ptr = (uint32_t*)(*pp_dst);
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint32_t val = 0U;

            m_bit = m_byte & msk; // get mask bit
            // shift mask bit-mask to the next position
            msk = (mask_be) ? msk >> 1 : msk << 1;
            if (!msk) {     // if bit-mask is beyond byte boundaries
                mask_ptr++; // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // grt next mask byte
            }
            src_bit_offset += s_bit;
            if (m_bit) { // if !0 - store src s_bit value
                current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
                src_mask        = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);

                for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                                   // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr--; // and shift source pointer to the next byte
                    }
                    // put current bit value to proper dst position
                    val = (src_bit) ? (val | (1 << i)) : val;
                }                  // for s_bit
                if (1U == s_bit) { // store non-zero value index for bit vector
                    if (0U < val) {
                        if (UINT32_MAX == idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                        if (REF_32U_BITS > *output_bits_ptr) { // control dst buffer
                            return QPL_STS_DST_IS_SHORT_ERR;   // if upper boundary reached - than return error
                        }

                        *dst32_ptr = (uint32_t)idx;
                        dst32_ptr++;
                        *output_bits_ptr -= REF_32U_BITS; // for dst upper boundary check
                    }
                    idx++;
                } else {                                   // otherwise store all values itself
                    if (REF_32U_BITS > *output_bits_ptr) { // control dst buffer
                        return QPL_STS_DST_IS_SHORT_ERR;   // if upper boundary reached - than return error
                    }
                    *dst32_ptr = (uint32_t)val;
                    dst32_ptr++;
                    *output_bits_ptr -= REF_32U_BITS; // for dst upper boundary check
                }
            } // if !m_bit
        }     // for len
        *pp_dst = (uint8_t*)dst32_ptr;
    }
    return QPL_STS_OK;
}

qpl_status ref_copy_mask_be_be_nu(uint8_t* src_ptr, uint8_t* mask_ptr, uint32_t mask_be, uint8_t** pp_dst,
                                  uint32_t i_offset, uint64_t* output_bits_ptr, uint32_t s_bit, uint32_t len,
                                  qpl_out_format o_fmt) {
    uint8_t src_mask = REF_LOW_BIT_MASK; // src bit mask
    uint8_t dst_mask = REF_LOW_BIT_MASK; // dst bit mask
    uint8_t src_bit  = 0U;               // current source bit
    // init mask bit position
    uint8_t  msk             = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
    uint32_t m_bit           = 0U;        // current src2 bit (mask bit)
    uint8_t  m_byte          = *mask_ptr; // get mask byte
    uint32_t idx             = i_offset;  // count for nominal bit-vector and output modification
    uint64_t src_bit_offset  = 0U;
    uint64_t dst_bit_offset  = 0U;
    uint8_t* current_src_ptr = NULL;
    uint8_t* current_dst_ptr = NULL;

    if (qpl_ow_nom == o_fmt) {
        uint8_t* dst_ptr = *pp_dst;

        for (uint32_t n = 0U; n < len; n++) {        // through all mask elements
            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask bit-mask to the next position
            if (!msk) {                              // if bit-mask is beyond byte boundaries
                mask_ptr++;                          // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get next mask byte
            }
            src_bit_offset += s_bit;
            if (m_bit) { // if !0 - store src s_bit value
                dst_bit_offset += s_bit;
                current_dst_ptr = dst_ptr + ((dst_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
                dst_mask        = REF_HIGH_BIT_MASK >> ((dst_bit_offset - 1) & REF_MAX_BIT_IDX);

                current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
                src_mask        = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);
                if (s_bit > *output_bits_ptr) {      // control dst buffer
                    return QPL_STS_DST_IS_SHORT_ERR; // if upper boundary reached - than return error
                }
                *output_bits_ptr -= s_bit;                            // for dst upper boundary check
                for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                                   // shift mask to the next bit position (BE)
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr--; // and shift source pointer to the next byte
                    }
                    // put current bit value to proper dst position
                    *current_dst_ptr = (src_bit) ? (*current_dst_ptr | dst_mask) : (*current_dst_ptr & (~dst_mask));
                    dst_mask <<= 1;                           // direction is the same - down -> up (in LE notation)
                    if (!dst_mask) {                          // if dst mask is beyond the byte boundaries
                        dst_mask = (uint8_t)REF_LOW_BIT_MASK; // set it to initial bit of the byte
                        current_dst_ptr--;                    // and shift pointer to the next byte
                    }
                } // for s_bit
            }     // if !m_bit
        }         // for len
        if (0U < dst_bit_offset) {
            *pp_dst += ((dst_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
            dst_mask = (uint8_t) ~((REF_HIGH_BIT_MASK >> ((dst_bit_offset - 1) & REF_MAX_BIT_IDX)) - 1);
            if (0U != dst_mask) {
                (**pp_dst) &= dst_mask; // zero unused bits in the last byte
                (*pp_dst)++;
            }
        }
    } else if (qpl_ow_8 == o_fmt) { // if qpl_ow_nom
        uint8_t* dst_ptr = *pp_dst;

        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint8_t val = 0U;

            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask bit-mask to the next position
            if (!msk) {                              // if bit-mask is beyond byte boundaries
                mask_ptr++;                          // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get next mask byte
            }
            src_bit_offset += s_bit;
            if (m_bit) { // if 0 - skip this element, if !0 - copy to dst
                current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
                src_mask        = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);

                for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                                   // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr--; // and shift source pointer to the next byte
                    }
                    // put current bit value to proper dst position
                    val = (src_bit) ? (val | (1 << i)) : val;
                }                  // for s_bit
                if (1U == s_bit) { // store non-zero value index for bit vector
                    if (0 < val) {
                        if (UINT8_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                        if (REF_8U_BITS > *output_bits_ptr) { // control dst buffer
                            return QPL_STS_DST_IS_SHORT_ERR;  // if upper boundary reached - than return error
                        }
                        *dst_ptr = (uint8_t)idx;
                        dst_ptr++;
                        *output_bits_ptr -= REF_8U_BITS; // for dst upper boundary check
                    }
                    idx++;
                } else {                                  // otherwise store all values itself
                    if (REF_8U_BITS > *output_bits_ptr) { // control dst buffer
                        return QPL_STS_DST_IS_SHORT_ERR;  // if upper boundary reached - than return error
                    }
                    *dst_ptr = (uint8_t)val;
                    dst_ptr++;
                    *output_bits_ptr -= REF_8U_BITS; // for dst upper boundary check
                }
            } // if !m_bit
        }     // for len
        *pp_dst = dst_ptr;
    } else if (qpl_ow_16 == o_fmt) {
        uint16_t* dst16_ptr = (uint16_t*)(*pp_dst);
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint16_t val = 0U;

            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask bit-mask to the next position
            if (!msk) {                              // if bit-mask is beyond byte boundaries
                mask_ptr++;                          // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get next mask byte
            }
            src_bit_offset += s_bit;
            if (m_bit) { // if !0 - store src s_bit value
                current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
                src_mask        = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);

                for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                                   // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr--; // and shift source pointer to the next byte
                    }
                    // put current bit value to proper dst position
                    val = (src_bit) ? (val | (1 << i)) : val;
                }                  // for s_bit
                if (1U == s_bit) { // store non-zero value index for bit vector
                    if (0U < val) {
                        if (UINT16_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                        if (REF_16U_BITS > *output_bits_ptr) { // control dst buffer
                            return QPL_STS_DST_IS_SHORT_ERR;   // if upper boundary reached - than return error
                        }
                        *dst16_ptr = ref_cvt_le_2_be_16u(idx);
                        dst16_ptr++;
                        *output_bits_ptr -= REF_16U_BITS; // for dst upper boundary check
                    }
                    idx++;
                } else {                                   // otherwise store all values itself
                    if (REF_16U_BITS > *output_bits_ptr) { // control dst buffer
                        return QPL_STS_DST_IS_SHORT_ERR;   // if upper boundary reached - than return error
                    }
                    *dst16_ptr = ref_cvt_le_2_be_16u(val);
                    dst16_ptr++;
                    *output_bits_ptr -= REF_16U_BITS; // for dst upper boundary check
                }
            } // if !m_bit
        }     // for len
        *pp_dst = (uint8_t*)dst16_ptr;
    } else if (qpl_ow_32 == o_fmt) {
        uint32_t* dst32_ptr = (uint32_t*)(*pp_dst);
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint32_t val = 0U;

            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask bit-mask to the next position
            if (!msk) {                              // if bit-mask is beyond byte boundaries
                mask_ptr++;                          // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // grt next mask byte
            }
            src_bit_offset += s_bit;
            if (m_bit) { // if !0 - store src s_bit value
                current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
                src_mask        = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);

                for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                                   // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr--; // and shift source pointer to the next byte
                    }
                    // put current bit value to proper dst position
                    val = (src_bit) ? (val | (1 << i)) : val;
                }                  // for s_bit
                if (1U == s_bit) { // store non-zero value index for bit vector
                    if (0U < val) {
                        if (UINT32_MAX == idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                        if (REF_32U_BITS > *output_bits_ptr) { // control dst buffer
                            return QPL_STS_DST_IS_SHORT_ERR;   // if upper boundary reached - than return error
                        }
                        *dst32_ptr = ref_cvt_le_2_be_32u(idx);
                        dst32_ptr++;
                        *output_bits_ptr -= REF_32U_BITS; // for dst upper boundary check
                    }
                    idx++;
                } else {                                   // otherwise store all values itself
                    if (REF_32U_BITS > *output_bits_ptr) { // control dst buffer
                        return QPL_STS_DST_IS_SHORT_ERR;   // if upper boundary reached - than return error
                    }
                    *dst32_ptr = ref_cvt_le_2_be_32u(val);
                    dst32_ptr++;
                    *output_bits_ptr -= REF_32U_BITS; // for dst upper boundary check
                }
            } // if !m_bit
        }     // for len
        *pp_dst = (uint8_t*)dst32_ptr;
    }
    return QPL_STS_OK;
}

qpl_status ref_expand_mask_le_le_nu(uint8_t* src_ptr, uint8_t* mask_ptr, uint32_t mask_be, uint8_t** pp_dst,
                                    uint32_t i_offset, uint32_t* i_bytes_ptr, uint32_t s_bit, uint32_t len,
                                    qpl_out_format o_fmt) {
    uint8_t src_mask = (uint8_t)REF_LOW_BIT_MASK; // init src bit mask
    uint8_t dst_mask = (uint8_t)REF_LOW_BIT_MASK; // init dst bit mask
    uint8_t src_bit  = 0U;                        // current source bit
    // init mask bit position
    uint8_t  msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
    uint32_t m_bit  = 0U;        // current src2 bit (mask bit)
    uint8_t  m_byte = *mask_ptr; // get mask byte
    uint32_t idx    = i_offset;  // count for nominal bit-vector and output modification
    if (qpl_ow_nom == o_fmt) {
        uint8_t* dst_ptr = *pp_dst;
        for (uint32_t n = 0U; n < len; n++) {        // through all mask elements
            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask bit-mask to the next position
            if (!msk) {                              // if bit-mask is beyond byte boundaries
                mask_ptr++;                          // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get next mask byte
            }
            if (!m_bit) {                                     // if 0 - store zero s_bit value
                for (uint32_t i = 0U; i < s_bit; i++) {       // set zero bit by bit
                    *dst_ptr = *dst_ptr & (~dst_mask);        // put zero bit value to proper dst position
                    dst_mask <<= 1;                           // direction is the same - down -> up (in LE notation)
                    if (!dst_mask) {                          // if dst mask is beyond the byte boundaries
                        dst_mask = (uint8_t)REF_LOW_BIT_MASK; // set it to initial bit of the byte
                        dst_ptr++;                            // and shift pointer to the next byte
                    }
                }
            } else {                                     // for "1" mask bit - copy src s_bit value to dst
                for (uint32_t i = 0U; i < s_bit; i++) {  // copy bit by bit
                    if (0 == *i_bytes_ptr) {             // control src buffer
                        return QPL_STS_SRC_IS_SHORT_ERR; // if upper boundary reached - than return error
                    }
                    src_bit = (uint8_t)(*src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                           // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        src_ptr++;        // and shift source pointer to the next byte
                        (*i_bytes_ptr)--; // for src upper boundary check
                    }
                    // put current bit value to proper dst position
                    *dst_ptr = (src_bit) ? (*dst_ptr | dst_mask) : (*dst_ptr & (~dst_mask));
                    dst_mask <<= 1;                           // direction is the same - down -> up (in LE notation)
                    if (!dst_mask) {                          // if dst mask is beyond the byte boundaries
                        dst_mask = (uint8_t)REF_LOW_BIT_MASK; // set it to initial bit of the byte
                        dst_ptr++;                            // and shift pointer to the next byte
                    }
                } // for s_bit
            }     // if !m_bit
        }         // for len
        *pp_dst = dst_ptr;
        if (REF_LOW_BIT_MASK < dst_mask) {
            (**pp_dst) &= (dst_mask - 1); // zero unused bits in the last byte
            (*pp_dst)++;
        }
    } else if (qpl_ow_8 == o_fmt) { // if qpl_ow_nom
        uint8_t* dst_ptr = *pp_dst;

        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint8_t val = 0U;

            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask bit-mask to the next position
            if (!msk) {                              // if bit-mask is beyond byte boundaries
                mask_ptr++;                          // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get next mask byte
            }
            if (!m_bit) {          // if 0 - store zero s_bit value (if it is not a bit vector)
                val = (uint8_t)0U; // put zero bit value to proper dst position
            } else {               // for "1" mask bit - copy src s_bit value to dst
                val = 0U;
                for (uint32_t i = 0U; i < s_bit; i++) {  // copy bit by bit
                    if (0U == *i_bytes_ptr) {            // control src buffer
                        return QPL_STS_SRC_IS_SHORT_ERR; // if upper boundary reached - than return error
                    }
                    src_bit = (uint8_t)(*src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                           // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        src_ptr++;        // and shift source pointer to the next byte
                        (*i_bytes_ptr)--; // for src upper boundary check
                    }
                    // put current bit value to proper dst position
                    val = (src_bit) ? (val | (1 << i)) : val;
                }              // for s_bit
            }                  // if !m_bit
            if (1U == s_bit) { // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT8_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst_ptr = (uint8_t)idx;
                    dst_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst_ptr = (uint8_t)val;
                dst_ptr++;
            }
        } // for len
        *pp_dst = dst_ptr;
    } else if (qpl_ow_16 == o_fmt) {
        uint16_t* dst16_ptr = (uint16_t*)(*pp_dst);
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint16_t val = 0U;

            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask bit-mask to the next position
            if (!msk) {                              // if bit-mask is beyond byte boundaries
                mask_ptr++;                          // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get next mask byte
            }
            if (!m_bit) {          // if 0 - store zero s_bit value
                val = (uint16_t)0; // put zero bit value to proper dst position
            } else {               // for "1" mask bit - copy src s_bit value to dst
                val = 0U;
                for (uint32_t i = 0U; i < s_bit; i++) {  // copy bit by bit
                    if (0U == *i_bytes_ptr) {            // control src buffer
                        return QPL_STS_SRC_IS_SHORT_ERR; // if upper boundary reached - than return error
                    }
                    src_bit = (uint8_t)(*src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                           // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        src_ptr++;        // and shift source pointer to the next byte
                        (*i_bytes_ptr)--; // for src upper boundary check
                    }
                    // put current bit value to proper dst position
                    val = (src_bit) ? (val | (1 << i)) : val;
                }              // for s_bit
            }                  // if !m_bit
            if (1U == s_bit) { // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT16_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst16_ptr = (uint16_t)idx;
                    dst16_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst16_ptr = (uint16_t)val;
                dst16_ptr++;
            }
        } // for len
        *pp_dst = (uint8_t*)dst16_ptr;
    } else if (qpl_ow_32 == o_fmt) {
        uint32_t* dst32_ptr = (uint32_t*)(*pp_dst);
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint32_t val = 0U;

            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask bit-mask to the next position
            if (!msk) {                              // if bit-mask is beyond byte boundaries
                mask_ptr++;                          // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // grt next mask byte
            }
            if (!m_bit) {           // if 0 - store zero s_bit value
                val = (uint32_t)0U; // put zero bit value to proper dst position
            } else {                // for "1" mask bit - copy src s_bit value to dst
                val = 0U;
                for (uint32_t i = 0U; i < s_bit; i++) {  // copy bit by bit
                    if (0U == *i_bytes_ptr) {            // control src buffer
                        return QPL_STS_SRC_IS_SHORT_ERR; // if upper boundary reached - than return error
                    }
                    src_bit = (uint8_t)(*src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                           // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        src_ptr++;        // and shift source pointer to the next byte
                        (*i_bytes_ptr)--; // for src upper boundary check
                    }
                    val = (src_bit) ? (val | (1 << i)) : val; // put current bit value to proper dst position
                }                                             // for s_bit
            }                                                 // if !m_bit
            if (1U == s_bit) {                                // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT32_MAX == idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst32_ptr = (uint32_t)idx;
                    dst32_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst32_ptr = (uint32_t)val;
                dst32_ptr++;
            }
        } // for len
        *pp_dst = (uint8_t*)dst32_ptr;
    }
    if ((0U < *i_bytes_ptr) && (REF_LOW_BIT_MASK < src_mask)) { (*i_bytes_ptr)--; }

    return QPL_STS_OK;
}

qpl_status ref_expand_mask_le_be_nu(uint8_t* src_ptr, uint8_t* mask_ptr, uint32_t mask_be, uint8_t** pp_dst,
                                    uint32_t i_offset, uint32_t* i_bytes_ptr, uint32_t s_bit, uint32_t len,
                                    qpl_out_format o_fmt) {
    uint8_t src_mask = (uint8_t)REF_LOW_BIT_MASK; // init src bit mask
    uint8_t dst_mask = 0U;
    uint8_t src_bit  = 0U;
    // init mask bit-mask
    uint8_t  msk             = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
    uint32_t m_bit           = 0U;
    uint8_t* current_dst_ptr = NULL;      // ptr to current destination byte
    uint64_t dst_bit_offset  = 0U;        // init dst offset position
    uint8_t  m_byte          = *mask_ptr; // get 1st mask byte
    uint32_t idx             = i_offset;

    if (qpl_ow_nom == o_fmt) {
        uint8_t* dst_ptr = *pp_dst;
        for (uint32_t n = 0U; n < len; n++) { // for all mask elements
            dst_bit_offset += s_bit;          // first bit of n-bit BE number in some dst_ptr byte
            // pointer to the actual destination byte;  "-1" - because we need the last bit of actual value,
            // not the next one
            current_dst_ptr = dst_ptr + ((dst_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
            // bit mask to actual position (actual offset of the 1st bit in the actual destination byte)
            dst_mask = REF_HIGH_BIT_MASK >> ((dst_bit_offset - 1) & REF_MAX_BIT_IDX);
            m_bit    = m_byte & msk;                    // get mask bit
            msk      = (mask_be) ? msk >> 1 : msk << 1; // shift mask to the next position
            if (!msk) {                                 // if mask is beyond byte boundaries
                mask_ptr++;                             // shift mask ptr to the next position
                // init mask bit-mask
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get new mask byte
            }
            if (!m_bit) {                               // if mask bit is zero - put zero s_bit value to dst
                for (uint32_t i = 0U; i < s_bit; i++) { // copy zero bit by bit
                    *current_dst_ptr = *current_dst_ptr & (~dst_mask); // put zero bit value to proper dst position
                    dst_mask <<= 1;                           // direction is the same - down -> up (in LE notation)
                    if (!dst_mask) {                          // if dst mask is beyond the byte boundaries
                        dst_mask = (uint8_t)REF_LOW_BIT_MASK; // set it to initial bit of the byte
                        current_dst_ptr--;                    // and shift pointer to the next byte
                    }
                }
            } else {                                     // if mask bit == 1 - than copy src s_bit value to dst
                for (uint32_t i = 0U; i < s_bit; i++) {  // copy bit by bit
                    if (0U == *i_bytes_ptr) {            // src buffer upper boundary control
                        return QPL_STS_SRC_IS_SHORT_ERR; // if reached - than error
                    }
                    src_bit = (uint8_t)(*src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                           // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        src_mask = (uint8_t)REF_LOW_BIT_MASK; // than init mask to the next byte initial bit position
                        src_ptr++;                            // and shift source pointer to the next byte
                        (*i_bytes_ptr)--;                     // for src upper boundary check
                    }
                    // put current bit value to proper dst position
                    *current_dst_ptr = (src_bit) ? (*current_dst_ptr | dst_mask) : (*current_dst_ptr & (~dst_mask));
                    dst_mask <<= 1;                           // direction is the same - down -> up (in LE notation)
                    if (!dst_mask) {                          // if dst mask is beyond the byte boundaries
                        dst_mask = (uint8_t)REF_LOW_BIT_MASK; // set it to initial bit of the byte
                        current_dst_ptr--;                    // and shift pointer to the next byte
                    }
                } // for s_bit
            }     // if !m_bit
        }         // for len
        *pp_dst += ((dst_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
        dst_mask = (uint8_t) ~((REF_HIGH_BIT_MASK >> ((dst_bit_offset - 1) & REF_MAX_BIT_IDX)) - 1);
        if (0U != dst_mask) {
            (**pp_dst) &= dst_mask; // zero unused bits in the last byte
            (*pp_dst)++;
        }
    } else if (qpl_ow_8 == o_fmt) { // if qpl_ow_nom
        uint8_t* dst_ptr = *pp_dst;
        uint8_t  val     = 0U;

        for (uint32_t n = 0U; n < len; n++) {        // for all mask elements
            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask to the next position
            if (!msk) {                              // if mask is beyond byte boundaries
                mask_ptr++;                          // shift mask ptr to the next position
                // init mask bit-mask
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get new mask byte
            }
            if (!m_bit) { // if mask bit is zero - put zero s_bit value to dst
                val = (uint8_t)0U;
            } else { // if mask bit == 1 - than copy src s_bit value to dst
                val = 0U;
                for (uint32_t i = 0U; i < s_bit; i++) {  // copy bit by bit
                    if (0U == *i_bytes_ptr) {            // src buffer upper boundary control
                        return QPL_STS_SRC_IS_SHORT_ERR; // if reached - than error
                    }
                    src_bit = (uint8_t)(*src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                           // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        src_ptr++;        // and shift source pointer to the next byte
                        (*i_bytes_ptr)--; // for src upper boundary check
                    }
                    // put current bit value to proper dst position
                    val = (src_bit) ? (val | (1 << i)) : val;
                }              // for s_bit
            }                  // if !m_bit
            if (1U == s_bit) { // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT8_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst_ptr = (uint8_t)idx;
                    dst_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst_ptr = (uint8_t)val;
                dst_ptr++;
            }
        } // for len
        *pp_dst = dst_ptr;
    } else if (qpl_ow_16 == o_fmt) {
        uint16_t* dst16_ptr = (uint16_t*)(*pp_dst);
        uint16_t  val       = 0U;

        for (uint32_t n = 0U; n < len; n++) {        // for all mask elements
            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask to the next position
            if (!msk) {                              // if mask is beyond byte boundaries
                mask_ptr++;                          // shift mask ptr to the next position
                // init mask bit-mask
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get new mask byte
            }
            if (!m_bit) { // if mask bit is zero - put zero s_bit value to dst
                val = (uint16_t)0U;
            } else { // if mask bit == 1 - than copy src s_bit value to dst
                val = 0U;
                for (uint32_t i = 0U; i < s_bit; i++) {  // copy bit by bit
                    if (0U == *i_bytes_ptr) {            // src buffer upper boundary control
                        return QPL_STS_SRC_IS_SHORT_ERR; // if reached - than error
                    }
                    src_bit = (uint8_t)(*src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                           // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        src_ptr++;        // and shift source pointer to the next byte
                        (*i_bytes_ptr)--; // for src upper boundary check
                    }
                    val = (src_bit) ? (val | (1 << i)) : val; // put current bit value to proper dst position
                }                                             // for s_bit
            }                                                 // if !m_bit
            if (1U == s_bit) {                                // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT16_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst16_ptr = (uint16_t)ref_cvt_le_2_be_16u(idx);
                    dst16_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst16_ptr = (uint16_t)ref_cvt_le_2_be_16u(val);
                dst16_ptr++;
            }
        } // for len
        *pp_dst = (uint8_t*)dst16_ptr;
    } else if (qpl_ow_32 == o_fmt) {
        uint32_t* dst32_ptr = (uint32_t*)(*pp_dst);
        uint32_t  val       = 0U;

        for (uint32_t n = 0U; n < len; n++) {        // for all mask elements
            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask to the next position
            if (!msk) {                              // if mask is beyond byte boundaries
                mask_ptr++;                          // shift mask ptr to the next position
                // init mask bit-mask
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get new mask byte
            }
            if (!m_bit) { // if mask bit is zero - put zero s_bit value to dst
                val = (uint32_t)0;
            } else { // if mask bit == 1 - than copy src s_bit value to dst
                val = 0U;
                for (uint32_t i = 0U; i < s_bit; i++) {  // copy bit by bit
                    if (0U == *i_bytes_ptr) {            // src buffer upper boundary control
                        return QPL_STS_SRC_IS_SHORT_ERR; // if reached - than error
                    }
                    src_bit = (uint8_t)(*src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                           // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        src_ptr++;        // and shift source pointer to the next byte
                        (*i_bytes_ptr)--; // for src upper boundary check
                    }
                    val = (src_bit) ? (val | (1 << i)) : val; // put current bit value to proper dst position
                }                                             // for s_bit
            }                                                 // if !m_bit
            if (1U == s_bit) {                                // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT32_MAX == idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst32_ptr = (uint32_t)ref_cvt_le_2_be_32u(idx);
                    dst32_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst32_ptr = (uint32_t)ref_cvt_le_2_be_32u(val);
                dst32_ptr++;
            }
        } // for len
        *pp_dst = (uint8_t*)dst32_ptr;
    }
    if ((0U < *i_bytes_ptr) && (REF_LOW_BIT_MASK < src_mask)) { (*i_bytes_ptr)--; }
    return QPL_STS_OK;
}

qpl_status ref_expand_mask_be_le_nu(uint8_t* src_ptr, uint8_t* mask_ptr, uint32_t mask_be, uint8_t** pp_dst,
                                    uint32_t i_offset, uint32_t* i_bytes_ptr, uint32_t s_bit, uint32_t len,
                                    qpl_out_format o_fmt) {
    uint8_t src_mask = (uint8_t)REF_LOW_BIT_MASK; // src bit mask
    uint8_t dst_mask = (uint8_t)REF_LOW_BIT_MASK; // init dst bit mask
    uint8_t src_bit  = 0U;                        // current source bit
    // init mask bit position
    uint8_t  msk             = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
    uint32_t m_bit           = 0U;        // current src2 bit (mask bit)
    uint8_t  m_byte          = *mask_ptr; // get mask byte
    uint32_t idx             = i_offset;  // count for nominal bit-vector and output modification
    uint64_t src_bit_offset  = 0U;
    uint8_t* current_src_ptr = NULL;

    if (qpl_ow_nom == o_fmt) {
        uint8_t* dst_ptr = *pp_dst;

        for (uint32_t n = 0U; n < len; n++) {        // through all mask elements
            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask bit-mask to the next position
            if (!msk) {                              // if bit-mask is beyond byte boundaries
                mask_ptr++;                          // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get next mask byte
            }
            if (!m_bit) {                                     // if 0 - store zero s_bit value
                for (uint32_t i = 0U; i < s_bit; i++) {       // set zero bit by bit
                    *dst_ptr = *dst_ptr & (~dst_mask);        // put zero bit value to proper dst position
                    dst_mask <<= 1;                           // direction is the same - down -> up (in LE notation)
                    if (!dst_mask) {                          // if dst mask is beyond the byte boundaries
                        dst_mask = (uint8_t)REF_LOW_BIT_MASK; // set it to initial bit of the byte
                        dst_ptr++;                            // and shift pointer to the next byte
                    }
                }
            } else { // for "1" mask bit - copy src s_bit value to dst
                src_bit_offset += s_bit;
                current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
                if ((*i_bytes_ptr + src_ptr) < current_src_ptr) { // control src buffer
                    return QPL_STS_SRC_IS_SHORT_ERR;              // if upper boundary reached - than return error
                }
                src_mask = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);

                for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                                   // shift mask to the next bit position (BE)
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr--; // and shift source pointer to the next byte
                    }
                    // put current bit value to proper dst position
                    *dst_ptr = (src_bit) ? (*dst_ptr | dst_mask) : (*dst_ptr & (~dst_mask));
                    dst_mask <<= 1;                           // direction is the same - down -> up (in LE notation)
                    if (!dst_mask) {                          // if dst mask is beyond the byte boundaries
                        dst_mask = (uint8_t)REF_LOW_BIT_MASK; // set it to initial bit of the byte
                        dst_ptr++;                            // and shift pointer to the next byte
                    }
                } // for s_bit
            }     // if !m_bit
        }         // for len
        *pp_dst = dst_ptr;
        if (REF_LOW_BIT_MASK < dst_mask) {
            (**pp_dst) &= (dst_mask - 1); // zero unused bits in the last byte
            (*pp_dst)++;
        }
    } else if (qpl_ow_8 == o_fmt) { // if qpl_ow_nom
        uint8_t* dst_ptr = *pp_dst;

        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint8_t val = 0U;

            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask bit-mask to the next position
            if (!msk) {                              // if bit-mask is beyond byte boundaries
                mask_ptr++;                          // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get next mask byte
            }
            if (!m_bit) {         // if 0 - store zero s_bit value (if it is not a bit vector)
                val = (uint8_t)0; // put zero bit value to proper dst position
            } else {              // for "1" mask bit - copy src s_bit value to dst
                val = 0U;
                src_bit_offset += s_bit;
                current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
                if ((src_ptr + *i_bytes_ptr) < current_src_ptr) { // control src buffer
                    return QPL_STS_SRC_IS_SHORT_ERR;              // if upper boundary reached - than return error
                }
                src_mask = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);

                for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                                   // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr--; // and shift source pointer to the next byte
                    }
                    // put current bit value to proper dst position
                    val = (src_bit) ? (val | (1 << i)) : val;
                }              // for s_bit
            }                  // if !m_bit
            if (1U == s_bit) { // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT8_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst_ptr = (uint8_t)idx;
                    dst_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst_ptr = (uint8_t)val;
                dst_ptr++;
            }
        } // for len
        *pp_dst = dst_ptr;
    } else if (qpl_ow_16 == o_fmt) {
        uint16_t* dst16_ptr = (uint16_t*)(*pp_dst);
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint16_t val = 0U;

            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask bit-mask to the next position
            if (!msk) {                              // if bit-mask is beyond byte boundaries
                mask_ptr++;                          // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get next mask byte
            }
            if (!m_bit) {          // if 0 - store zero s_bit value
                val = (uint16_t)0; // put zero bit value to proper dst position
            } else {               // for "1" mask bit - copy src s_bit value to dst
                val = 0U;
                src_bit_offset += s_bit;
                current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
                if ((src_ptr + *i_bytes_ptr) < current_src_ptr) { // control src buffer
                    return QPL_STS_SRC_IS_SHORT_ERR;              // if upper boundary reached - than return error
                }
                src_mask = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);
                for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                                   // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr--; // and shift source pointer to the next byte
                    }
                    val = (src_bit) ? (val | (1 << i)) : val; // put current bit value to proper dst position
                }                                             // for s_bit
            }                                                 // if !m_bit
            if (1U == s_bit) {                                // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT16_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst16_ptr = (uint16_t)idx;
                    dst16_ptr++;
                }
                idx++;
            } else if (1U != s_bit) { // otherwise store all values itself
                *dst16_ptr = (uint16_t)val;
                dst16_ptr++;
            }
        } // for len
        *pp_dst = (uint8_t*)dst16_ptr;
    } else if (qpl_ow_32 == o_fmt) {
        uint32_t* dst32_ptr = (uint32_t*)(*pp_dst);
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint32_t val = 0U;

            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask bit-mask to the next position
            if (!msk) {                              // if bit-mask is beyond byte boundaries
                mask_ptr++;                          // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // grt next mask byte
            }
            if (!m_bit) {          // if 0 - store zero s_bit value
                val = (uint32_t)0; // put zero bit value to proper dst position
            } else {               // for "1" mask bit - copy src s_bit value to dst
                val = 0U;
                src_bit_offset += s_bit;
                current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
                if ((src_ptr + *i_bytes_ptr) < current_src_ptr) { // control src buffer
                    return QPL_STS_SRC_IS_SHORT_ERR;              // if upper boundary reached - than return error
                }
                src_mask = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);

                for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                                   // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr--; // and shift source pointer to the next byte
                    }
                    val = (src_bit) ? (val | (1 << i)) : val; // put current bit value to proper dst position
                }                                             // for s_bit
            }                                                 // if !m_bit
            if (1U == s_bit) {                                // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT32_MAX == idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst32_ptr = (uint32_t)idx;
                    dst32_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst32_ptr = (uint32_t)val;
                dst32_ptr++;
            }
        } // for len
        *pp_dst = (uint8_t*)dst32_ptr;
    }
    *i_bytes_ptr -= (uint32_t)REF_BIT_2_BYTE(src_bit_offset);

    return QPL_STS_OK;
}

qpl_status ref_expand_mask_be_be_nu(uint8_t* src_ptr, uint8_t* mask_ptr, uint32_t mask_be, uint8_t** pp_dst,
                                    uint32_t i_offset, uint32_t* i_bytes_ptr, uint32_t s_bit, uint32_t len,
                                    qpl_out_format o_fmt) {
    uint8_t src_mask = REF_LOW_BIT_MASK; // src bit mask
    uint8_t dst_mask = REF_LOW_BIT_MASK; // dst bit mask
    uint8_t src_bit  = 0U;               // current source bit

    // init mask bit position
    uint8_t  msk             = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
    uint32_t m_bit           = 0U;        // current src2 bit (mask bit)
    uint8_t  m_byte          = *mask_ptr; // get mask byte
    uint32_t idx             = i_offset;  // count for nominal bit-vector and output modification
    uint64_t src_bit_offset  = 0U;
    uint64_t dst_bit_offset  = 0U;
    uint8_t* current_src_ptr = NULL;
    uint8_t* current_dst_ptr = NULL;

    if (qpl_ow_nom == o_fmt) {
        uint8_t* dst_ptr = *pp_dst;

        for (uint32_t n = 0U; n < len; n++) {        // through all mask elements
            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask bit-mask to the next position
            if (!msk) {                              // if bit-mask is beyond byte boundaries
                mask_ptr++;                          // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get next mask byte
            }
            dst_bit_offset += s_bit;
            current_dst_ptr = dst_ptr + ((dst_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
            dst_mask        = REF_HIGH_BIT_MASK >> ((dst_bit_offset - 1) & REF_MAX_BIT_IDX);
            if (!m_bit) {                                              // if 0 - store zero s_bit value
                for (uint32_t i = 0U; i < s_bit; i++) {                // set zero bit by bit
                    *current_dst_ptr = *current_dst_ptr & (~dst_mask); // put zero bit value to proper dst position
                    dst_mask <<= 1;                           // direction is the same - down -> up (in LE notation)
                    if (!dst_mask) {                          // if dst mask is beyond the byte boundaries
                        dst_mask = (uint8_t)REF_LOW_BIT_MASK; // set it to initial bit of the byte
                        current_dst_ptr--;                    // and shift pointer to the next byte
                    }
                }
            } else { // for "1" mask bit - copy src s_bit value to dst
                src_bit_offset += s_bit;
                current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
                if ((src_ptr + *i_bytes_ptr) < current_src_ptr) { // control src buffer
                    return QPL_STS_SRC_IS_SHORT_ERR;              // if upper boundary reached - than return error
                }
                src_mask = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);

                for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                                   // shift mask to the next bit position (BE)
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr--; // and shift source pointer to the next byte
                    }
                    // put current bit value to proper dst position
                    *current_dst_ptr = (src_bit) ? (*current_dst_ptr | dst_mask) : (*current_dst_ptr & (~dst_mask));
                    dst_mask <<= 1;                           // direction is the same - down -> up (in LE notation)
                    if (!dst_mask) {                          // if dst mask is beyond the byte boundaries
                        dst_mask = (uint8_t)REF_LOW_BIT_MASK; // set it to initial bit of the byte
                        current_dst_ptr--;                    // and shift pointer to the next byte
                    }
                } // for s_bit
            }     // if !m_bit
        }         // for len
        *pp_dst += ((dst_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
        dst_mask = (uint8_t) ~((REF_HIGH_BIT_MASK >> ((dst_bit_offset - 1) & REF_MAX_BIT_IDX)) - 1);
        if (0 != dst_mask) {
            (**pp_dst) &= dst_mask; // zero unused bits in the last byte
            (*pp_dst)++;
        }
    } else if (qpl_ow_8 == o_fmt) { // if qpl_ow_nom
        uint8_t* dst_ptr = *pp_dst;

        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint8_t val = 0U;

            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask bit-mask to the next position
            if (!msk) {                              // if bit-mask is beyond byte boundaries
                mask_ptr++;                          // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get next mask byte
            }
            if (!m_bit) {         // if 0 - store zero s_bit value (if it is not a bit vector)
                val = (uint8_t)0; // put zero bit value to proper dst position
            } else {              // for "1" mask bit - copy src s_bit value to dst
                val = 0U;
                src_bit_offset += s_bit;
                current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
                if ((src_ptr + *i_bytes_ptr) < current_src_ptr) { // control src buffer
                    return QPL_STS_SRC_IS_SHORT_ERR;              // if upper boundary reached - than return error
                }
                src_mask = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);

                for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                                   // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr--; // and shift source pointer to the next byte
                    }
                    val = (src_bit) ? (val | (1 << i)) : val; // put current bit value to proper dst position
                }                                             // for s_bit
            }                                                 // if !m_bit
            if (1U == s_bit) {                                // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT8_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst_ptr = (uint8_t)idx;
                    dst_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst_ptr = (uint8_t)val;
                dst_ptr++;
            }
        } // for len
        *pp_dst = dst_ptr;
    } else if (qpl_ow_16 == o_fmt) {
        uint16_t* dst16_ptr = (uint16_t*)(*pp_dst);
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint16_t val = 0U;

            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask bit-mask to the next position
            if (!msk) {                              // if bit-mask is beyond byte boundaries
                mask_ptr++;                          // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // get next mask byte
            }
            if (!m_bit) {          // if 0 - store zero s_bit value
                val = (uint16_t)0; // put zero bit value to proper dst position
            } else {               // for "1" mask bit - copy src s_bit value to dst
                val = 0U;
                src_bit_offset += s_bit;
                current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
                if ((src_ptr + *i_bytes_ptr) < current_src_ptr) { // control src buffer
                    return QPL_STS_SRC_IS_SHORT_ERR;              // if upper boundary reached - than return error
                }
                src_mask = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);

                for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                                   // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr--; // and shift source pointer to the next byte
                    }
                    val = (src_bit) ? (val | (1 << i)) : val; // put current bit value to proper dst position
                }                                             // for s_bit
            }                                                 // if !m_bit
            if (1U == s_bit) {                                // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT16_MAX < idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst16_ptr = ref_cvt_le_2_be_16u(idx);
                    dst16_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst16_ptr = ref_cvt_le_2_be_16u(val);
                dst16_ptr++;
            }
        } // for len
        *pp_dst = (uint8_t*)dst16_ptr;
    } else if (qpl_ow_32 == o_fmt) {
        uint32_t* dst32_ptr = (uint32_t*)(*pp_dst);
        for (uint32_t n = 0U; n < len; n++) { // through all mask elements
            uint32_t val = 0U;

            m_bit = m_byte & msk;                    // get mask bit
            msk   = (mask_be) ? msk >> 1 : msk << 1; // shift mask bit-mask to the next position
            if (!msk) {                              // if bit-mask is beyond byte boundaries
                mask_ptr++;                          // and load next mask byte
                // init mask bit for the new byte
                msk    = (mask_be) ? ((uint8_t)REF_HIGH_BIT_MASK) : ((uint8_t)REF_LOW_BIT_MASK);
                m_byte = *mask_ptr; // grt next mask byte
            }
            if (!m_bit) {          // if 0 - store zero s_bit value
                val = (uint32_t)0; // put zero bit value to proper dst position
            } else {               // for "1" mask bit - copy src s_bit value to dst
                val = 0U;
                src_bit_offset += s_bit;
                current_src_ptr = src_ptr + ((src_bit_offset - 1) >> REF_BIT_LEN_2_BYTE);
                if ((src_ptr + *i_bytes_ptr) < current_src_ptr) { // control src buffer
                    return QPL_STS_SRC_IS_SHORT_ERR;              // if upper boundary reached - than return error
                }
                src_mask = REF_HIGH_BIT_MASK >> ((src_bit_offset - 1) & REF_MAX_BIT_IDX);

                for (uint32_t i = 0U; i < s_bit; i++) {               // copy bit by bit
                    src_bit = (uint8_t)(*current_src_ptr & src_mask); // get source bit value
                    src_mask <<= 1;                                   // shift mask to the next bit position
                    if (!src_mask) { // if src mask is beyond byte boundaries (we finish with this byte)
                        // than init mask to the next byte initial bit position
                        src_mask = (uint8_t)REF_LOW_BIT_MASK;
                        current_src_ptr--; // and shift source pointer to the next byte
                    }
                    val = (src_bit) ? (val | (1 << i)) : val; // put current bit value to proper dst position
                }                                             // for s_bit
            }                                                 // if !m_bit
            if (1U == s_bit) {                                // store non-zero value index for bit vector
                if (0U < val) {
                    if (UINT32_MAX == idx) { return QPL_STS_OUTPUT_OVERFLOW_ERR; }
                    *dst32_ptr = ref_cvt_le_2_be_32u(idx);
                    dst32_ptr++;
                }
                idx++;
            } else { // otherwise store all values itself
                *dst32_ptr = ref_cvt_le_2_be_32u(val);
                dst32_ptr++;
            }
        } // for len
        *pp_dst = (uint8_t*)dst32_ptr;
    }
    *i_bytes_ptr -= (uint32_t)REF_BIT_2_BYTE(src_bit_offset);
    return QPL_STS_OK;
}
