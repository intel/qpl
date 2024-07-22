/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @brief Contains implementation of functions for vector packing byte integers to 1...8-bit integers
 * @date 07/06/2020
 *
 * @details Function list:
 *          - @ref qplc_pack_8u1u
 *          - @ref qplc_pack_8u2u
 *          - @ref qplc_pack_8u3u
 *          - @ref qplc_pack_8u4u
 *          - @ref qplc_pack_8u5u
 *          - @ref qplc_pack_8u6u
 *          - @ref qplc_pack_8u7u
 *          - @ref qplc_pack_8u8u
 *          - @ref qplc_pack_8u16u
 *          - @ref qplc_pack_8u32u
 */
#include "own_qplc_defs.h"
#include "qplc_memop.h"

#if PLATFORM >= K0
#include "opt/qplc_pack_8u_k0.h"
#endif

// ********************** 1u ****************************** //

OWN_QPLC_FUN(void, qplc_pack_8u1u,
             (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {

#if PLATFORM >= K0
    CALL_OPT_FUNCTION(k0_qplc_pack_8u1u)(src_ptr, num_elements, dst_ptr, start_bit);
#else
    uint32_t i = 0U;

    dst_ptr[0] &= OWN_BIT_MASK(start_bit);
    while (0U != start_bit) {
        *dst_ptr |= *src_ptr << start_bit;
        num_elements--;
        src_ptr++;
        start_bit++;
        if (OWN_BYTE_WIDTH == start_bit) {
            dst_ptr++;
            break;
        }
        if (0 == num_elements) { return; }
    }
    while (num_elements > 64U) {
        uint64_t  bit_buf  = 0LLU;
        uint64_t* tmp_dst  = (uint64_t*)dst_ptr;
        uint64_t  bit_mask = 0U;

        for (i = 0; i < 64U; i++) {
            bit_mask = OWN_1_BIT_MASK & src_ptr[i];
            bit_mask = bit_mask << i;
            bit_buf |= bit_mask;
        }
        src_ptr += 64;
        *tmp_dst = bit_buf;
        dst_ptr += sizeof(uint64_t);
        num_elements -= 64U;
    }
    if (num_elements > 32U) {
        uint32_t  bit_buf  = 0U;
        uint32_t* tmp_dst  = (uint32_t*)dst_ptr;
        uint32_t  bit_mask = 0U;

        for (i = 0; i < 32U; i++) {
            bit_mask = OWN_1_BIT_MASK & src_ptr[i];
            bit_mask = bit_mask << i;
            bit_buf |= bit_mask;
        }
        src_ptr += 32U;
        *tmp_dst = bit_buf;
        dst_ptr += sizeof(uint32_t);
        num_elements -= 32U;
    }
    if (num_elements > 16U) {
        uint16_t  bit_buf  = 0U;
        uint16_t* tmp_dst  = (uint16_t*)dst_ptr;
        uint16_t  bit_mask = 0U;

        for (i = 0; i < 16; i++) {
            bit_mask = OWN_1_BIT_MASK & src_ptr[i];
            bit_mask = bit_mask << i;
            bit_buf |= bit_mask;
        }
        src_ptr += 16U;
        *tmp_dst = bit_buf;
        dst_ptr += sizeof(uint16_t);
        num_elements -= 16U;
    }
    if (0U < num_elements) {
        uint16_t  bit_buf  = 0U;
        uint16_t  bit_mask = 0U;
        uint16_t* tmp_dst  = (uint16_t*)dst_ptr;
        for (i = 0U; i < num_elements; i++) {
            bit_mask = OWN_1_BIT_MASK & src_ptr[i];
            bit_mask = bit_mask << i;
            bit_buf |= bit_mask;
        }
        if (OWN_BYTE_WIDTH >= i) {
            *dst_ptr = (uint8_t)bit_buf;
        } else {
            *tmp_dst = bit_buf;
        }
    }
#endif
}

// ********************** 2u ****************************** //

OWN_QPLC_FUN(void, qplc_pack_8u2u,
             (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
#if PLATFORM >= K0
    CALL_OPT_FUNCTION(k0_qplc_pack_8u2u)(src_ptr, num_elements, dst_ptr, start_bit);
#else
    uint32_t i = 0U;

    dst_ptr[0] &= OWN_BIT_MASK(start_bit);
    while (0U != start_bit) {
        *dst_ptr |= *src_ptr << start_bit;
        num_elements--;
        src_ptr++;
        start_bit += 2U;
        if (OWN_BYTE_WIDTH == start_bit) {
            dst_ptr++;
            break;
        }
        if (0U == num_elements) { return; }
    }
    while (num_elements > 32U) {
        uint64_t  bit_buf  = 0LLU;
        uint64_t* tmp_dst  = (uint64_t*)dst_ptr;
        uint64_t  bit_mask = 0U;

        for (i = 0U; i < 32U; i++) {
            bit_mask = OWN_2_BIT_MASK & src_ptr[i];
            bit_mask = bit_mask << 2U * i;
            bit_buf |= bit_mask;
        }
        src_ptr += 32U;
        *tmp_dst = bit_buf;
        dst_ptr += sizeof(uint64_t);
        num_elements -= 32U;
    }
    if (num_elements > 16U) {
        uint32_t  bit_buf  = 0U;
        uint32_t* tmp_dst  = (uint32_t*)dst_ptr;
        uint32_t  bit_mask = 0U;

        for (i = 0U; i < 16U; i++) {
            bit_mask = OWN_2_BIT_MASK & src_ptr[i];
            bit_mask = bit_mask << 2U * i;
            bit_buf |= bit_mask;
        }
        src_ptr += 16U;
        *tmp_dst = bit_buf;
        dst_ptr += sizeof(uint32_t);
        num_elements -= 16U;
    }
    if (num_elements > 8U) {
        uint16_t  bit_buf  = 0U;
        uint16_t* tmp_dst  = (uint16_t*)dst_ptr;
        uint16_t  bit_mask = 0U;

        for (i = 0U; i < 8U; i++) {
            bit_mask = OWN_2_BIT_MASK & src_ptr[i];
            bit_mask = bit_mask << 2U * i;
            bit_buf |= bit_mask;
        }
        src_ptr += 8U;
        *tmp_dst = bit_buf;
        dst_ptr += sizeof(uint16_t);
        num_elements -= 8U;
    }
    if (0U < num_elements) {
        uint16_t  bit_buf  = 0U;
        uint16_t  bit_mask = 0U;
        uint16_t* tmp_dst  = (uint16_t*)dst_ptr;
        for (i = 0U; i < num_elements; i++) {
            bit_mask = OWN_2_BIT_MASK & src_ptr[i];
            bit_mask = bit_mask << 2U * i;
            bit_buf |= bit_mask;
        }
        if (4U >= i) {
            *dst_ptr = (uint8_t)bit_buf;
        } else {
            *tmp_dst = bit_buf;
        }
    }
#endif
}

// ********************** 3u ****************************** //

OWN_QPLC_FUN(void, qplc_pack_8u3u,
             (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
#if PLATFORM >= K0
    CALL_OPT_FUNCTION(k0_qplc_pack_8u3u)(src_ptr, num_elements, dst_ptr, start_bit);
#else
    dst_ptr[0] &= OWN_BIT_MASK(start_bit);
    while (0U != start_bit) {
        *dst_ptr |= *src_ptr << start_bit;
        start_bit += 3U;
        if (OWN_BYTE_WIDTH < start_bit) {
            start_bit -= OWN_BYTE_WIDTH;
            dst_ptr++;
            *dst_ptr = *src_ptr >> (3U - start_bit);
        }
        num_elements--;
        src_ptr++;
        if (OWN_BYTE_WIDTH == start_bit) {
            dst_ptr++;
            break;
        }
        if (0U == num_elements) { return; }
    }
    while (num_elements > 32U) {
        uint64_t  bit_buf64 = 0U;
        uint32_t  bit_buf32 = 0U;
        uint64_t* dst64_ptr = (uint64_t*)dst_ptr;
        uint32_t* dst32_ptr = (uint32_t*)(dst_ptr + 8U);

        bit_buf64 = (uint64_t)src_ptr[0];
        bit_buf64 |= ((uint64_t)(src_ptr[1])) << 3U;
        bit_buf64 |= ((uint64_t)(src_ptr[2])) << 6U;
        bit_buf64 |= ((uint64_t)(src_ptr[3])) << 9U;
        bit_buf64 |= ((uint64_t)(src_ptr[4])) << 12U;
        bit_buf64 |= ((uint64_t)(src_ptr[5])) << 15U;
        bit_buf64 |= ((uint64_t)(src_ptr[6])) << 18U;
        bit_buf64 |= ((uint64_t)(src_ptr[7])) << 21U;
        bit_buf64 |= ((uint64_t)(src_ptr[8])) << 24U;
        bit_buf64 |= ((uint64_t)(src_ptr[9])) << 27U;
        bit_buf64 |= ((uint64_t)(src_ptr[10])) << 30U;
        bit_buf64 |= ((uint64_t)(src_ptr[11])) << 33U;
        bit_buf64 |= ((uint64_t)(src_ptr[12])) << 36U;
        bit_buf64 |= ((uint64_t)(src_ptr[13])) << 39U;
        bit_buf64 |= ((uint64_t)(src_ptr[14])) << 42U;
        bit_buf64 |= ((uint64_t)(src_ptr[15])) << 45U;
        bit_buf64 |= ((uint64_t)(src_ptr[16])) << 48U;
        bit_buf64 |= ((uint64_t)(src_ptr[17])) << 51U;
        bit_buf64 |= ((uint64_t)(src_ptr[18])) << 54U;
        bit_buf64 |= ((uint64_t)(src_ptr[19])) << 57U;
        bit_buf64 |= ((uint64_t)(src_ptr[20])) << 60U;
        bit_buf64 |= ((uint64_t)(src_ptr[21])) << 63U;
        bit_buf32 = ((uint32_t)(src_ptr[21])) >> 1U;
        bit_buf32 |= ((uint32_t)(src_ptr[22])) << 2U;
        bit_buf32 |= ((uint32_t)(src_ptr[23])) << 5U;
        bit_buf32 |= ((uint32_t)(src_ptr[24])) << 8U;
        bit_buf32 |= ((uint32_t)(src_ptr[25])) << 11U;
        bit_buf32 |= ((uint32_t)(src_ptr[26])) << 14U;
        bit_buf32 |= ((uint32_t)(src_ptr[27])) << 17U;
        bit_buf32 |= ((uint32_t)(src_ptr[28])) << 20U;
        bit_buf32 |= ((uint32_t)(src_ptr[29])) << 23U;
        bit_buf32 |= ((uint32_t)(src_ptr[30])) << 26U;
        bit_buf32 |= ((uint32_t)(src_ptr[31])) << 29U;
        *dst64_ptr = bit_buf64;
        *dst32_ptr = bit_buf32;
        src_ptr += 32U;
        dst_ptr += 12U;
        num_elements -= 32U;
    }
    while (num_elements > 16U) {
        uint16_t  bit_buf16 = 0U;
        uint32_t  bit_buf32 = 0U;
        uint16_t* dst16_ptr = (uint16_t*)(dst_ptr + 4U);
        uint32_t* dst32_ptr = (uint32_t*)(dst_ptr);

        bit_buf32 = (uint32_t)src_ptr[0];
        bit_buf32 |= ((uint32_t)(src_ptr[1])) << 3U;
        bit_buf32 |= ((uint32_t)(src_ptr[2])) << 6U;
        bit_buf32 |= ((uint32_t)(src_ptr[3])) << 9U;
        bit_buf32 |= ((uint32_t)(src_ptr[4])) << 12U;
        bit_buf32 |= ((uint32_t)(src_ptr[5])) << 15U;
        bit_buf32 |= ((uint32_t)(src_ptr[6])) << 18U;
        bit_buf32 |= ((uint32_t)(src_ptr[7])) << 21U;
        bit_buf32 |= ((uint32_t)(src_ptr[8])) << 24U;
        bit_buf32 |= ((uint32_t)(src_ptr[9])) << 27U;
        bit_buf32 |= ((uint32_t)(src_ptr[10])) << 30U;
        bit_buf16 = ((uint16_t)(src_ptr[10])) >> 2U;
        bit_buf16 |= ((uint16_t)(src_ptr[11])) << 1U;
        bit_buf16 |= ((uint16_t)(src_ptr[12])) << 4U;
        bit_buf16 |= ((uint16_t)(src_ptr[13])) << 7U;
        bit_buf16 |= ((uint16_t)(src_ptr[14])) << 10U;
        bit_buf16 |= ((uint16_t)(src_ptr[15])) << 13U;
        *dst32_ptr = bit_buf32;
        *dst16_ptr = bit_buf16;
        src_ptr += 16U;
        dst_ptr += 6U;
        num_elements -= 16U;
    }
    while (num_elements > 8U) {
        uint16_t  bit_buf16 = 0U;
        uint8_t   bit_buf8  = 0U;
        uint16_t* dst16_ptr = (uint16_t*)(dst_ptr);

        bit_buf16 = (uint16_t)src_ptr[0];
        bit_buf16 |= ((uint16_t)(src_ptr[1])) << 3U;
        bit_buf16 |= ((uint16_t)(src_ptr[2])) << 6U;
        bit_buf16 |= ((uint16_t)(src_ptr[3])) << 9U;
        bit_buf16 |= ((uint16_t)(src_ptr[4])) << 12U;
        bit_buf16 |= ((uint16_t)(src_ptr[5])) << 15U;
        bit_buf8 = src_ptr[5] >> 1U;
        bit_buf8 |= src_ptr[6] << 2U;
        bit_buf8 |= src_ptr[7] << 5U;
        *dst16_ptr = bit_buf16;
        dst_ptr += 2U;
        *dst_ptr = bit_buf8;
        src_ptr += 8U;
        dst_ptr++;
        num_elements -= 8U;
    }
    if (0U < num_elements) {
        uint32_t               bits_in_buf = 0U;
        qplc_bit_byte_pool32_t bit_byte_pool32;
        bit_byte_pool32.bit_buf = 0U;

        while (0U != num_elements) {
            bit_byte_pool32.bit_buf |= ((uint32_t)(*src_ptr)) << bits_in_buf;
            src_ptr++;
            bits_in_buf += 3U;
            num_elements--;
        }
        dst_ptr[0] = bit_byte_pool32.byte_buf[0];
        if (bits_in_buf > OWN_BYTE_WIDTH) { dst_ptr[1] = bit_byte_pool32.byte_buf[1]; }
        if (bits_in_buf > OWN_WORD_WIDTH) { dst_ptr[2] = bit_byte_pool32.byte_buf[2]; }
    }
#endif
}

// ********************** 4u ****************************** //

OWN_QPLC_FUN(void, qplc_pack_8u4u,
             (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
#if PLATFORM >= K0
    CALL_OPT_FUNCTION(k0_qplc_pack_8u4u)(src_ptr, num_elements, dst_ptr, start_bit);
#else
    uint32_t i = 0U;

    dst_ptr[0] &= OWN_BIT_MASK(start_bit);
    while (0U != start_bit) {
        *dst_ptr |= *src_ptr << start_bit;
        num_elements--;
        src_ptr++;
        start_bit += 4U;
        if (OWN_BYTE_WIDTH == start_bit) {
            dst_ptr++;
            break;
        }
        if (0U == num_elements) { return; }
    }
    while (num_elements > 32U) {
        uint64_t  bit_buf0  = 0LLU;
        uint64_t  bit_buf1  = 0LLU;
        uint64_t* tmp_dst   = (uint64_t*)dst_ptr;
        uint64_t  bit_mask0 = 0U;
        uint64_t  bit_mask1 = 0U;

        for (i = 0U; i < 16U; i++) {
            bit_mask0 = ((uint64_t)src_ptr[i]) << (4U * i);
            bit_buf0 |= bit_mask0;
            bit_mask1 = ((uint64_t)src_ptr[i + 16U]) << (4U * i);
            bit_buf1 |= bit_mask1;
        }
        src_ptr += 32U;
        *tmp_dst = bit_buf0;
        tmp_dst++;
        *tmp_dst = bit_buf1;
        dst_ptr += 2U * sizeof(uint64_t);
        num_elements -= 32U;
    }
    if (num_elements > 16U) {
        uint64_t  bit_buf0  = 0LLU;
        uint64_t* tmp_dst   = (uint64_t*)dst_ptr;
        uint64_t  bit_mask0 = 0U;

        for (i = 0U; i < 16U; i++) {
            bit_mask0 = ((uint64_t)src_ptr[i]) << (4U * i);
            bit_buf0 |= bit_mask0;
        }
        src_ptr += 16U;
        *tmp_dst = bit_buf0;
        dst_ptr += sizeof(uint64_t);
        num_elements -= 16U;
    }
    if (num_elements > 8U) {
        uint32_t  bit_buf0  = 0U;
        uint32_t* tmp_dst   = (uint32_t*)dst_ptr;
        uint32_t  bit_mask0 = 0U;

        for (i = 0U; i < 8U; i++) {
            bit_mask0 = ((uint32_t)src_ptr[i]) << (4U * i);
            bit_buf0 |= bit_mask0;
        }
        src_ptr += 8U;
        *tmp_dst = bit_buf0;
        dst_ptr += sizeof(uint32_t);
        num_elements -= 8U;
    }
    uint8_t bit_buf = 0U;
    for (i = 0U; i < num_elements; i++) {
        if (OWN_1_BIT_MASK & i) {
            bit_buf |= src_ptr[i] << 4U;
            *dst_ptr = bit_buf;
            dst_ptr++;
        } else {
            bit_buf  = src_ptr[i];
            *dst_ptr = bit_buf;
        }
    }
#endif
}

// ********************** 5u ****************************** //

OWN_QPLC_FUN(void, qplc_pack_8u5u,
             (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
#if PLATFORM >= K0
    CALL_OPT_FUNCTION(k0_qplc_pack_8u5u)(src_ptr, num_elements, dst_ptr, start_bit);
#else
    dst_ptr[0] &= OWN_BIT_MASK(start_bit);
    while (0U != start_bit) {
        *dst_ptr |= *src_ptr << start_bit;
        start_bit += 5U;
        if (OWN_BYTE_WIDTH < start_bit) {
            start_bit -= OWN_BYTE_WIDTH;
            dst_ptr++;
            *dst_ptr = *src_ptr >> (5U - start_bit);
        }
        num_elements--;
        src_ptr++;
        if (OWN_BYTE_WIDTH == start_bit) {
            dst_ptr++;
            break;
        }
        if (0U == num_elements) { return; }
    }
    while (num_elements > 32U) {
        uint64_t  bit_buf64_0 = 0U;
        uint64_t  bit_buf64_1 = 0U;
        uint32_t  bit_buf32   = 0U;
        uint64_t* dst64_ptr   = (uint64_t*)dst_ptr;
        uint32_t* dst32_ptr   = (uint32_t*)(dst_ptr + 2U * sizeof(uint64_t));

        bit_buf64_0 = (uint64_t)src_ptr[0];
        bit_buf64_0 |= ((uint64_t)(src_ptr[1])) << 5U;
        bit_buf64_0 |= ((uint64_t)(src_ptr[2])) << 10U;
        bit_buf64_0 |= ((uint64_t)(src_ptr[3])) << 15U;
        bit_buf64_0 |= ((uint64_t)(src_ptr[4])) << 20U;
        bit_buf64_0 |= ((uint64_t)(src_ptr[5])) << 25U;
        bit_buf64_0 |= ((uint64_t)(src_ptr[6])) << 30U;
        bit_buf64_0 |= ((uint64_t)(src_ptr[7])) << 35U;
        bit_buf64_0 |= ((uint64_t)(src_ptr[8])) << 40U;
        bit_buf64_0 |= ((uint64_t)(src_ptr[9])) << 45U;
        bit_buf64_0 |= ((uint64_t)(src_ptr[10])) << 50U;
        bit_buf64_0 |= ((uint64_t)(src_ptr[11])) << 55U;
        bit_buf64_0 |= ((uint64_t)(src_ptr[12])) << 60U;
        bit_buf64_1 = ((uint64_t)(src_ptr[12])) >> 4U;
        bit_buf64_1 |= ((uint64_t)(src_ptr[13])) << 1U;
        bit_buf64_1 |= ((uint64_t)(src_ptr[14])) << 6U;
        bit_buf64_1 |= ((uint64_t)(src_ptr[15])) << 11U;
        bit_buf64_1 |= ((uint64_t)(src_ptr[16])) << 16U;
        bit_buf64_1 |= ((uint64_t)(src_ptr[17])) << 21U;
        bit_buf64_1 |= ((uint64_t)(src_ptr[18])) << 26U;
        bit_buf64_1 |= ((uint64_t)(src_ptr[19])) << 31U;
        bit_buf64_1 |= ((uint64_t)(src_ptr[20])) << 36U;
        bit_buf64_1 |= ((uint64_t)(src_ptr[21])) << 41U;
        bit_buf64_1 |= ((uint64_t)(src_ptr[22])) << 46U;
        bit_buf64_1 |= ((uint64_t)(src_ptr[23])) << 51U;
        bit_buf64_1 |= ((uint64_t)(src_ptr[24])) << 56U;
        bit_buf64_1 |= ((uint64_t)(src_ptr[25])) << 61U;
        bit_buf32 = ((uint32_t)(src_ptr[25])) >> 3U;
        bit_buf32 |= ((uint32_t)(src_ptr[26])) << 2U;
        bit_buf32 |= ((uint32_t)(src_ptr[27])) << 7U;
        bit_buf32 |= ((uint32_t)(src_ptr[28])) << 12U;
        bit_buf32 |= ((uint32_t)(src_ptr[29])) << 17U;
        bit_buf32 |= ((uint32_t)(src_ptr[30])) << 22U;
        bit_buf32 |= ((uint32_t)(src_ptr[31])) << 27U;

        *dst64_ptr = bit_buf64_0;
        dst64_ptr++;
        *dst64_ptr = bit_buf64_1;
        *dst32_ptr = bit_buf32;
        src_ptr += 32U;
        dst_ptr += 20U;
        num_elements -= 32U;
    }
    while (num_elements > 16U) {
        uint64_t  bit_buf64 = 0U;
        uint16_t  bit_buf16 = 0U;
        uint64_t* dst64_ptr = (uint64_t*)dst_ptr;
        uint16_t* dst16_ptr = (uint16_t*)(dst_ptr + sizeof(uint64_t));

        bit_buf64 = (uint64_t)src_ptr[0];
        bit_buf64 |= ((uint64_t)(src_ptr[1])) << 5U;
        bit_buf64 |= ((uint64_t)(src_ptr[2])) << 10U;
        bit_buf64 |= ((uint64_t)(src_ptr[3])) << 15U;
        bit_buf64 |= ((uint64_t)(src_ptr[4])) << 20U;
        bit_buf64 |= ((uint64_t)(src_ptr[5])) << 25U;
        bit_buf64 |= ((uint64_t)(src_ptr[6])) << 30U;
        bit_buf64 |= ((uint64_t)(src_ptr[7])) << 35U;
        bit_buf64 |= ((uint64_t)(src_ptr[8])) << 40U;
        bit_buf64 |= ((uint64_t)(src_ptr[9])) << 45U;
        bit_buf64 |= ((uint64_t)(src_ptr[10])) << 50U;
        bit_buf64 |= ((uint64_t)(src_ptr[11])) << 55U;
        bit_buf64 |= ((uint64_t)(src_ptr[12])) << 60U;
        bit_buf16 = ((uint16_t)(src_ptr[12])) >> 4U;
        bit_buf16 |= ((uint16_t)(src_ptr[13])) << 1U;
        bit_buf16 |= ((uint16_t)(src_ptr[14])) << 6U;
        bit_buf16 |= ((uint16_t)(src_ptr[15])) << 11U;

        *dst64_ptr = bit_buf64;
        *dst16_ptr = bit_buf16;
        src_ptr += 16U;
        dst_ptr += 10U;
        num_elements -= 16U;
    }
    while (num_elements > 8U) {
        uint32_t  bit_buf32 = 0U;
        uint8_t   bit_buf8  = 0U;
        uint32_t* dst32_ptr = (uint32_t*)(dst_ptr);

        bit_buf32 = (uint32_t)src_ptr[0];
        bit_buf32 |= ((uint32_t)(src_ptr[1])) << 5U;
        bit_buf32 |= ((uint32_t)(src_ptr[2])) << 10U;
        bit_buf32 |= ((uint32_t)(src_ptr[3])) << 15U;
        bit_buf32 |= ((uint32_t)(src_ptr[4])) << 20U;
        bit_buf32 |= ((uint32_t)(src_ptr[5])) << 25U;
        bit_buf32 |= ((uint32_t)(src_ptr[6])) << 30U;
        bit_buf8 = src_ptr[6] >> 2U;
        bit_buf8 |= src_ptr[7] << 3U;
        *dst32_ptr = bit_buf32;
        dst_ptr += sizeof(uint32_t);
        *dst_ptr = bit_buf8;
        src_ptr += 8U;
        dst_ptr++;
        num_elements -= 8U;
    }
    if (0U < num_elements) {
        uint32_t bits_in_buf = 5U;
        uint16_t src         = (uint16_t)(*src_ptr);
        src_ptr++;
        num_elements--;
        while (0U < num_elements) {
            src = src | (((uint16_t)(*src_ptr)) << bits_in_buf);
            src_ptr++;
            num_elements--;
            bits_in_buf += 5U;
            if (8U <= bits_in_buf) {
                *dst_ptr = (uint8_t)(src);
                dst_ptr++;
                src = src >> OWN_BYTE_WIDTH;
                bits_in_buf -= OWN_BYTE_WIDTH;
            }
        }
        if (0U < bits_in_buf) { *dst_ptr = (uint8_t)(src); }
    }
#endif
}

// ********************** 6u - 8u ****************************** //

#if PLATFORM < K0

OWN_QPLC_INLINE(void, qplc_pack_8u_nu,
                (const uint8_t* src_ptr, uint32_t num_elements, uint32_t bit_width, uint8_t* dst_ptr,
                 uint32_t start_bit)) {
    uint32_t bits_in_buf = bit_width + start_bit;
    uint16_t src         = (uint16_t)(*dst_ptr) & OWN_BIT_MASK(start_bit);
    src |= ((uint16_t)(*src_ptr)) << start_bit;
    src_ptr++;
    num_elements--;
    while (0U < num_elements) {
        if (OWN_BYTE_WIDTH <= bits_in_buf) {
            *dst_ptr = (uint8_t)(src);
            dst_ptr++;
            src = src >> OWN_BYTE_WIDTH;
            bits_in_buf -= OWN_BYTE_WIDTH;
        }
        src = src | (((uint16_t)(*src_ptr)) << bits_in_buf);
        src_ptr++;
        num_elements--;
        bits_in_buf += bit_width;
    }
    *dst_ptr = (uint8_t)(src);
    if (OWN_BYTE_WIDTH < bits_in_buf) {
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
        *dst_ptr = (uint8_t)(src);
    }
}

#endif

OWN_QPLC_FUN(void, qplc_pack_8u6u,
             (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
#if PLATFORM >= K0
    CALL_OPT_FUNCTION(k0_qplc_pack_8u6u)(src_ptr, num_elements, dst_ptr, start_bit);
#else
    qplc_pack_8u_nu(src_ptr, num_elements, 6U, dst_ptr, start_bit);
#endif
}

OWN_QPLC_FUN(void, qplc_pack_8u7u,
             (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)) {
#if PLATFORM >= K0
    CALL_OPT_FUNCTION(k0_qplc_pack_8u7u)(src_ptr, num_elements, dst_ptr, start_bit);
#else
    qplc_pack_8u_nu(src_ptr, num_elements, 7U, dst_ptr, start_bit);
#endif
}

OWN_QPLC_FUN(void, qplc_pack_8u8u,
             (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr,
              uint32_t UNREFERENCED_PARAMETER(start_bit))) {
    CALL_CORE_FUN(qplc_copy_8u)(src_ptr, dst_ptr, num_elements);
}

OWN_QPLC_FUN(void, qplc_pack_8u16u,
             (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr,
              uint32_t UNREFERENCED_PARAMETER(start_bit))) {
#if PLATFORM >= K0
    CALL_OPT_FUNCTION(k0_qplc_pack_8u16u)(src_ptr, num_elements, dst_ptr);
#else
    uint16_t* dst_16u_ptr = (uint16_t*)dst_ptr;

    for (uint32_t i = 0U; i < num_elements; i++) {
        dst_16u_ptr[i] = src_ptr[i];
    }
#endif
}

OWN_QPLC_FUN(void, qplc_pack_8u32u,
             (const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr,
              uint32_t UNREFERENCED_PARAMETER(start_bit))) {
#if PLATFORM >= K0
    CALL_OPT_FUNCTION(k0_qplc_pack_8u32u)(src_ptr, num_elements, dst_ptr);
#else
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;

    for (uint32_t i = 0U; i < num_elements; i++) {
        dst_32u_ptr[i] = src_ptr[i];
    }
#endif
}
