/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

 /**
  * @brief Contains implementation of functions for vector packing byte integers indexes
  * @date 03/02/2021
  *
  * @details Function list:
  *          - @ref qplc_set_membership
  *
  */
#ifndef OWN_SET_MEMBERSHIP_H
#define OWN_SET_MEMBERSHIP_H

#include "own_qplc_defs.h"
#include "immintrin.h"

// ********************** 8u ****************************** //
OWN_OPT_FUN(void, k0_qplc_set_membership_8u_i, (uint8_t* src_dst_ptr,
    const uint8_t* src2_ptr,
    uint32_t      length,
    uint32_t      shift,
    uint32_t      mask)) {

    __m512i     z_data;
    __m512i     z_mask;
    __m128i     x_data;
    __m128i     x_val_shift = _mm_cvtsi32_si128((int)shift);
    uint32_t    tabl_len = ((0xff >> shift) & mask) + 1;
    uint32_t    rem_len;
    uint32_t    len_crn;
    __mmask16   msk16;

    if (tabl_len <= 16) {
        __m512i     z_tabl;
        __mmask64   msk64;

        mask &= 0xff >> shift;
        z_mask = _mm512_set1_epi8((char)mask);

        /* set table */
        msk16 = (__mmask16)_bzhi_u32(0xffff, tabl_len);
        x_data = _mm_maskz_loadu_epi8(msk16, (__m128i const*)src2_ptr);

        /* alignment of src_dst_ptr */
        z_tabl = _mm512_broadcast_i32x4(x_data);
        rem_len = (uint64_t)((uint64_t)(src_dst_ptr)) & 0x3f;
        if (rem_len) {
            rem_len = 64 - rem_len;
            if (rem_len > length)
                rem_len = length;
            msk64 = (__mmask64)_bzhi_u64((uint64_t)((int64_t)(-1)), rem_len);
            z_data = _mm512_maskz_loadu_epi8(msk64, (void const*)src_dst_ptr);
            z_data = _mm512_srl_epi16(z_data, x_val_shift);
            z_data = _mm512_and_si512(z_data, z_mask);
            z_data = _mm512_shuffle_epi8(z_tabl, z_data);
            _mm512_mask_storeu_epi8((void*)src_dst_ptr, msk64, z_data);
            src_dst_ptr += rem_len;
            length -= rem_len;
        }

        rem_len = length & 0x3f;
        len_crn = length - rem_len;
        for (uint32_t i = 0; i < len_crn; i += 64) {
            z_data = _mm512_load_si512((void const *)(src_dst_ptr + i));
            z_data = _mm512_srl_epi16(z_data, x_val_shift);
            z_data = _mm512_and_si512(z_data, z_mask);
            z_data = _mm512_shuffle_epi8(z_tabl, z_data);
            _mm512_store_si512((void*)(src_dst_ptr + i), z_data);
        }
        if (rem_len) {
            msk64 = (__mmask64)_bzhi_u64((uint64_t)((int64_t)(-1)), rem_len);
            z_data = _mm512_maskz_loadu_epi8(msk64, (void const*)(src_dst_ptr + len_crn));
            z_data = _mm512_srl_epi16(z_data, x_val_shift);
            z_data = _mm512_and_si512(z_data, z_mask);
            z_data = _mm512_shuffle_epi8(z_tabl, z_data);
            _mm512_mask_storeu_epi8((void*)(src_dst_ptr + len_crn), msk64, z_data);
        }
    } else {
        uint32_t    tabl32u[256];

        z_mask = _mm512_set1_epi32((int)mask);
        rem_len = tabl_len & 0xf;
        len_crn = tabl_len - rem_len;
        for (uint32_t i = 0; i < len_crn; i += 16) {
            z_data = _mm512_cvtepu8_epi32(_mm_loadu_si128((__m128i const*)(src2_ptr + i)));
            _mm512_storeu_si512((void*)(tabl32u + i), z_data);
        }
        if (rem_len) {
            msk16 = (__mmask16)_bzhi_u32(0xffff, rem_len);
            z_data = _mm512_maskz_cvtepu8_epi32(msk16, _mm_maskz_loadu_epi8(msk16, (__m128i const*)(src2_ptr + len_crn)));
            _mm512_storeu_si512((void*)(tabl32u + len_crn), z_data);
        }

        rem_len = length & 0xf;
        len_crn = length - rem_len;
        for (uint32_t i = 0; i < len_crn; i += 16) {
            z_data = _mm512_cvtepu8_epi32(_mm_loadu_si128((__m128i const*)(src_dst_ptr + i)));
            z_data = _mm512_srl_epi32(z_data, x_val_shift);
            z_data = _mm512_and_epi32(z_data, z_mask);
            z_data = _mm512_i32gather_epi32(z_data, (void const*)tabl32u, 4);
            x_data = _mm512_cvtepi32_epi8(z_data);
            _mm_storeu_si128((__m128i*)(src_dst_ptr + i), x_data);
        }
        if (rem_len) {
            msk16 = (__mmask16)_bzhi_u32(0xffff, rem_len);
            z_data = _mm512_cvtepu8_epi32(_mm_maskz_loadu_epi8(msk16, (__m128i const*)(src_dst_ptr + len_crn)));
            z_data = _mm512_srl_epi32(z_data, x_val_shift);
            z_data = _mm512_and_epi32(z_data, z_mask);
            z_data = _mm512_i32gather_epi32(z_data, (void const*)tabl32u, 4);
            x_data = _mm512_cvtepi32_epi8(z_data);
            _mm_mask_storeu_epi8((__m128i*)(src_dst_ptr + len_crn), msk16, x_data);
        }
    }
}

// ********************** 16u8u ****************************** //
OWN_OPT_FUN(void, k0_qplc_set_membership_16u8u_i, (uint8_t* src_dst_ptr,
    const uint8_t* src2_ptr,
    uint32_t      length,
    uint32_t      shift,
    uint32_t      mask)) {

    uint16_t*   src_16u_ptr = (uint16_t*)src_dst_ptr;
    uint8_t*    dst_ptr = src_dst_ptr;
    __m512i     z_data;
    __m512i     z_mask;
    __m128i     x_data;
    __m128i     x_val_shift = _mm_cvtsi32_si128((int)shift);
    uint32_t    tabl_len = ((0xffff >> shift) & mask) + 1;
    uint32_t    rem_len;
    uint32_t    len_crn ;
    __mmask16   msk16;
 
    if (tabl_len <= 16) {
        __m512i     z_tabl;
        __m512i     z_data_l;
        __m512i     z_data_h;
        __mmask64   msk64;

        mask &= 0xffff >> shift;
        z_mask = _mm512_set1_epi8((char)mask);

        /* set table */
        msk16 = (__mmask16)_bzhi_u32(0xffff, tabl_len);
        x_data = _mm_maskz_loadu_epi8(msk16, (__m128i const*)src2_ptr);

        /* alignment of src_dst_ptr */
		z_tabl = _mm512_broadcast_i32x4(x_data);
        rem_len = (uint64_t)((uint64_t)(dst_ptr)) & 0x3f;
        if (rem_len) {
            rem_len = 64 - rem_len;
            if (rem_len > length)
                rem_len = length;
            msk64 = (__mmask64)_bzhi_u64((uint64_t)((int64_t)(-1)), rem_len);
            z_data_l = _mm512_maskz_loadu_epi16((__mmask32)msk64, (void const*)src_16u_ptr);
            z_data_h = _mm512_maskz_loadu_epi16((__mmask32)(msk64 >> 32), (void const*)(src_16u_ptr + 32));
            z_data_l = _mm512_srl_epi16(z_data_l, x_val_shift);
            z_data_h = _mm512_srl_epi16(z_data_h, x_val_shift);
            z_data = _mm512_castsi256_si512(_mm512_cvtepi16_epi8(z_data_l));
            z_data = _mm512_inserti64x4(z_data, _mm512_cvtepi16_epi8(z_data_h), 1);
            z_data = _mm512_and_si512(z_data, z_mask);
            z_data = _mm512_shuffle_epi8(z_tabl, z_data);
            _mm512_mask_storeu_epi8((void*)dst_ptr, msk64, z_data);
            src_16u_ptr += rem_len;
            dst_ptr += rem_len;
            length -= rem_len;
        }
        rem_len = length & 0x3f;
        len_crn = length - rem_len;
        for (uint32_t i = 0; i < len_crn; i += 64) {
            z_data_l = _mm512_loadu_si512((void const*)(src_16u_ptr + i));
            z_data_h = _mm512_loadu_si512((void const*)(src_16u_ptr + i + 32));
            z_data_l = _mm512_srl_epi16(z_data_l, x_val_shift);
            z_data_h = _mm512_srl_epi16(z_data_h, x_val_shift);
            z_data = _mm512_castsi256_si512(_mm512_cvtepi16_epi8(z_data_l));
            z_data = _mm512_inserti64x4(z_data, _mm512_cvtepi16_epi8(z_data_h), 1);
            z_data = _mm512_and_si512(z_data, z_mask);
            z_data = _mm512_shuffle_epi8(z_tabl, z_data);
            _mm512_store_si512((void*)(dst_ptr + i), z_data);
        }
        if (rem_len) {
            msk64 = (__mmask64)_bzhi_u64((uint64_t)((int64_t)(-1)), rem_len);
            z_data_l = _mm512_maskz_loadu_epi16((__mmask32)msk64, (void const*)(src_16u_ptr + len_crn));
            z_data_h = _mm512_maskz_loadu_epi16((__mmask32)(msk64 >> 32), (void const*)(src_16u_ptr + len_crn + 32));
            z_data_l = _mm512_srl_epi16(z_data_l, x_val_shift);
            z_data_h = _mm512_srl_epi16(z_data_h, x_val_shift);
            z_data = _mm512_castsi256_si512(_mm512_cvtepi16_epi8(z_data_l));
            z_data = _mm512_inserti64x4(z_data, _mm512_cvtepi16_epi8(z_data_h), 1);
            z_data = _mm512_and_si512(z_data, z_mask);
            z_data = _mm512_shuffle_epi8(z_tabl, z_data);
            _mm512_mask_storeu_epi8((void*)(dst_ptr + len_crn), msk64, z_data);
        }
        return;
    }

    z_mask = _mm512_set1_epi32((int)mask);

    if (tabl_len <= 256) {
        uint32_t    tabl32u[256];

        rem_len = tabl_len & 0xf;
        len_crn = tabl_len - rem_len;

        for (uint32_t i = 0; i < len_crn; i += 16) {
            z_data = _mm512_cvtepu8_epi32(_mm_loadu_si128((__m128i const*)(src2_ptr + i)));
            _mm512_storeu_si512((void*)(tabl32u + i), z_data);
        }
        if (rem_len) {
            msk16 = (__mmask16)_bzhi_u32(0xffff, rem_len);
            z_data = _mm512_maskz_cvtepu8_epi32(msk16, _mm_maskz_loadu_epi8(msk16, (__m128i const*)(src2_ptr + len_crn)));
            _mm512_storeu_si512((void*)(tabl32u + len_crn), z_data);
        }

        rem_len = length & 0xf;
        len_crn = length - rem_len;
        for (uint32_t i = 0; i < len_crn; i += 16) {
            z_data = _mm512_cvtepu16_epi32(_mm256_loadu_si256((__m256i const*)(src_16u_ptr + i)));
            z_data = _mm512_srl_epi32(z_data, x_val_shift);
            z_data = _mm512_and_epi32(z_data, z_mask);
            z_data = _mm512_i32gather_epi32(z_data, (void const*)tabl32u, 4);
            x_data = _mm512_cvtepi32_epi8(z_data);
            _mm_storeu_si128((__m128i*)(dst_ptr + i), x_data);
        }
        if (rem_len) {
            msk16 = (__mmask16)_bzhi_u32(0xffff, rem_len);
            z_data = _mm512_cvtepu16_epi32(_mm256_maskz_loadu_epi16(msk16, (void const*)(src_16u_ptr + len_crn)));
            z_data = _mm512_srl_epi32(z_data, x_val_shift);
            z_data = _mm512_and_epi32(z_data, z_mask);
            z_data = _mm512_i32gather_epi32(z_data, (void const*)tabl32u, 4);
            x_data = _mm512_cvtepi32_epi8(z_data);
            _mm_mask_storeu_epi8((__m128i*)(dst_ptr + len_crn), msk16, x_data);
        }
        return;
    }

    {
        uint32_t    tabl_len_save = tabl_len - 3;
        __m512i     z_3_32i = _mm512_set1_epi32(3);
        __m512i     z_tbl_len_save = _mm512_set1_epi32(tabl_len_save);
        __mmask16   msklen;

        rem_len = length & 0xf;
        len_crn = length - rem_len;

        for (uint32_t i = 0; i < len_crn; i += 16) {
            z_data = _mm512_cvtepu16_epi32(_mm256_loadu_si256((__m256i const*)(src_16u_ptr + i)));
            z_data = _mm512_srl_epi32(z_data, x_val_shift);
            z_data = _mm512_and_epi32(z_data, z_mask);
            msklen = _mm512_cmpgt_epi32_mask(z_data, z_tbl_len_save);
            z_data = _mm512_mask_sub_epi32(z_data, msklen, z_data, z_3_32i);
            z_data = _mm512_i32gather_epi32(z_data, (void const*)src2_ptr, 1);
            z_data = _mm512_mask_srli_epi32(z_data, msklen, z_data, 24);
            x_data = _mm512_cvtepi32_epi8(z_data);
            _mm_storeu_si128((__m128i*)(dst_ptr + i), x_data);
        }
        if (rem_len) {
            msk16 = (__mmask16)_bzhi_u32(0xffff, rem_len);
            z_data = _mm512_cvtepu16_epi32(_mm256_maskz_loadu_epi16(msk16, (void const*)(src_16u_ptr + len_crn)));
            z_data = _mm512_srl_epi32(z_data, x_val_shift);
            z_data = _mm512_and_epi32(z_data, z_mask);
            msklen = _mm512_cmpgt_epi32_mask(z_data, z_tbl_len_save);
            z_data = _mm512_mask_sub_epi32(z_data, msklen, z_data, z_3_32i);
            z_data = _mm512_i32gather_epi32(z_data, (void const*)src2_ptr, 1);
            z_data = _mm512_mask_srli_epi32(z_data, msklen, z_data, 24);
            x_data = _mm512_cvtepi32_epi8(z_data);
            _mm_mask_storeu_epi8((__m128i*)(dst_ptr + len_crn), msk16, x_data);
        }
    }
}

// ********************** 32u ****************************** //
OWN_OPT_FUN(void, k0_qplc_set_membership_32u8u_i, (uint8_t* src_dst_ptr,
    const uint8_t* src2_ptr,
    uint32_t      length,
    uint32_t      shift,
    uint32_t      mask)) {

    uint32_t* src_32u_ptr = (uint32_t*)src_dst_ptr;
    uint8_t* dst_ptr = src_dst_ptr;
    __m512i     z_data;
    __m512i     z_mask;
    __m128i     x_data;
    __m128i     x_val_shift = _mm_cvtsi32_si128((int)shift);
    uint32_t    tabl_len = (((uint32_t)0xffffffff >> shift) & mask) + 1;
    uint32_t    rem_len;
    uint32_t    len_crn;
    __mmask16   msk16;

    if (tabl_len <= 16) {
        __m512i     z_tabl;
        __m512i     z_data_0;
        __m512i     z_data_1;
        __m512i     z_data_2;
        __m512i     z_data_3;
        __mmask64   msk64;

        mask &= (uint32_t)0xffffffff >> shift;
        z_mask = _mm512_set1_epi8((char)mask);

        /* set table */
        msk16 = (__mmask16)_bzhi_u32(0xffff, tabl_len);
        x_data = _mm_maskz_loadu_epi8(msk16, (__m128i const*)src2_ptr);
		z_tabl = _mm512_broadcast_i32x4(x_data);

        /* alignment of src_dst_ptr */
        rem_len = (uint64_t)((uint64_t)(dst_ptr)) & 0x3f;
        if (rem_len) {
            rem_len = 64 - rem_len;
            if (rem_len > length)
                rem_len = length;
            msk64 = (__mmask64)_bzhi_u64((uint64_t)((int64_t)(-1)), rem_len);
            z_data_0 = _mm512_maskz_loadu_epi32((__mmask16)msk64, (void const*)src_32u_ptr);
            z_data_1 = _mm512_maskz_loadu_epi32((__mmask16)(msk64 >> 16), (void const*)(src_32u_ptr + 16));
            z_data_2 = _mm512_maskz_loadu_epi32((__mmask16)(msk64 >> 32), (void const*)(src_32u_ptr + 32));
            z_data_3 = _mm512_maskz_loadu_epi32((__mmask16)(msk64 >> 48), (void const*)(src_32u_ptr + 48));
            z_data_0 = _mm512_srl_epi16(z_data_0, x_val_shift);
            z_data_1 = _mm512_srl_epi16(z_data_1, x_val_shift);
            z_data_2 = _mm512_srl_epi16(z_data_2, x_val_shift);
            z_data_3 = _mm512_srl_epi16(z_data_3, x_val_shift);
            z_data   = _mm512_castsi128_si512(_mm512_cvtepi32_epi8(z_data_0));
            z_data_1 = _mm512_castsi128_si512(_mm512_cvtepi32_epi8(z_data_1));
            z_data_2 = _mm512_castsi128_si512(_mm512_cvtepi32_epi8(z_data_2));
            z_data_3 = _mm512_castsi128_si512(_mm512_cvtepi32_epi8(z_data_3));
            z_data = _mm512_inserti32x4(z_data, _mm512_castsi512_si128(z_data_1), 1);
            z_data_2 = _mm512_inserti32x4(z_data_2, _mm512_castsi512_si128(z_data_3), 1);
            z_data = _mm512_inserti64x4(z_data, _mm512_castsi512_si256(z_data_2), 1);
            z_data = _mm512_and_si512(z_data, z_mask);
            z_data = _mm512_shuffle_epi8(z_tabl, z_data);
            _mm512_mask_storeu_epi8((void*)dst_ptr, msk64, z_data);
            src_32u_ptr += rem_len;
            dst_ptr += rem_len;
            length -= rem_len;
        }
        rem_len = length & 0x3f;
        len_crn = length - rem_len;
        for (uint32_t i = 0; i < len_crn; i += 64) {
            z_data_0 = _mm512_loadu_si512((void const*)(src_32u_ptr + i));
            z_data_1 = _mm512_loadu_si512((void const*)(src_32u_ptr + i + 16));
            z_data_2 = _mm512_loadu_si512((void const*)(src_32u_ptr + i + 32));
            z_data_3 = _mm512_loadu_si512((void const*)(src_32u_ptr + i + 48));
            z_data_0 = _mm512_srl_epi16(z_data_0, x_val_shift);
            z_data_1 = _mm512_srl_epi16(z_data_1, x_val_shift);
            z_data_2 = _mm512_srl_epi16(z_data_2, x_val_shift);
            z_data_3 = _mm512_srl_epi16(z_data_3, x_val_shift);
            z_data = _mm512_castsi128_si512(_mm512_cvtepi32_epi8(z_data_0));
            z_data_1 = _mm512_castsi128_si512(_mm512_cvtepi32_epi8(z_data_1));
            z_data_2 = _mm512_castsi128_si512(_mm512_cvtepi32_epi8(z_data_2));
            z_data_3 = _mm512_castsi128_si512(_mm512_cvtepi32_epi8(z_data_3));
            z_data = _mm512_inserti32x4(z_data, _mm512_castsi512_si128(z_data_1), 1);
            z_data_2 = _mm512_inserti32x4(z_data_2, _mm512_castsi512_si128(z_data_3), 1);
            z_data = _mm512_inserti64x4(z_data, _mm512_castsi512_si256(z_data_2), 1);
            z_data = _mm512_and_si512(z_data, z_mask);
            z_data = _mm512_shuffle_epi8(z_tabl, z_data);
            _mm512_store_si512((void*)(dst_ptr + i), z_data);
        }
        if (rem_len) {
            msk64 = (__mmask64)_bzhi_u64((uint64_t)((int64_t)(-1)), rem_len);
            z_data_0 = _mm512_maskz_loadu_epi32((__mmask16)msk64, (void const*)(src_32u_ptr + len_crn));
            z_data_1 = _mm512_maskz_loadu_epi32((__mmask16)(msk64 >> 16), (void const*)(src_32u_ptr + len_crn + 16));
            z_data_2 = _mm512_maskz_loadu_epi32((__mmask16)(msk64 >> 32), (void const*)(src_32u_ptr + len_crn + 32));
            z_data_3 = _mm512_maskz_loadu_epi32((__mmask16)(msk64 >> 48), (void const*)(src_32u_ptr + len_crn + 48));
            z_data_0 = _mm512_srl_epi16(z_data_0, x_val_shift);
            z_data_1 = _mm512_srl_epi16(z_data_1, x_val_shift);
            z_data_2 = _mm512_srl_epi16(z_data_2, x_val_shift);
            z_data_3 = _mm512_srl_epi16(z_data_3, x_val_shift);
            z_data = _mm512_castsi128_si512(_mm512_cvtepi32_epi8(z_data_0));
            z_data_1 = _mm512_castsi128_si512(_mm512_cvtepi32_epi8(z_data_1));
            z_data_2 = _mm512_castsi128_si512(_mm512_cvtepi32_epi8(z_data_2));
            z_data_3 = _mm512_castsi128_si512(_mm512_cvtepi32_epi8(z_data_3));
            z_data = _mm512_inserti32x4(z_data, _mm512_castsi512_si128(z_data_1), 1);
            z_data_2 = _mm512_inserti32x4(z_data_2, _mm512_castsi512_si128(z_data_3), 1);
            z_data = _mm512_inserti64x4(z_data, _mm512_castsi512_si256(z_data_2), 1);
            z_data = _mm512_and_si512(z_data, z_mask);
            z_data = _mm512_shuffle_epi8(z_tabl, z_data);
            _mm512_mask_storeu_epi8((void*)(dst_ptr + len_crn), msk64, z_data);
        }
        return;
    }

    z_mask = _mm512_set1_epi32((int)mask);

    if (tabl_len <= 256) {
        uint32_t    tabl32u[256];

        rem_len = tabl_len & 0xf;
        len_crn = tabl_len - rem_len;

        for (uint32_t i = 0; i < len_crn; i += 16) {
            z_data = _mm512_cvtepu8_epi32(_mm_loadu_si128((__m128i const*)(src2_ptr + i)));
            _mm512_storeu_si512((void*)(tabl32u + i), z_data);
        }
        if (rem_len) {
            msk16 = (__mmask16)_bzhi_u32(0xffff, rem_len);
            z_data = _mm512_maskz_cvtepu8_epi32(msk16, _mm_maskz_loadu_epi8(msk16, (__m128i const*)(src2_ptr + len_crn)));
            _mm512_storeu_si512((void*)(tabl32u + len_crn), z_data);
        }

        rem_len = length & 0xf;
        len_crn = length - rem_len;
        for (uint32_t i = 0; i < len_crn; i += 16) {
            z_data = _mm512_loadu_si512((__m512i const*)(src_32u_ptr + i));
            z_data = _mm512_srl_epi32(z_data, x_val_shift);
            z_data = _mm512_and_epi32(z_data, z_mask);
            z_data = _mm512_i32gather_epi32(z_data, (void const*)tabl32u, 4);
            x_data = _mm512_cvtepi32_epi8(z_data);
            _mm_storeu_si128((__m128i*)(dst_ptr + i), x_data);
        }
        if (rem_len) {
            msk16 = (__mmask16)_bzhi_u32(0xffff, rem_len);
            z_data = _mm512_maskz_loadu_epi32(msk16, (void const*)(src_32u_ptr + len_crn));
            z_data = _mm512_srl_epi32(z_data, x_val_shift);
            z_data = _mm512_and_epi32(z_data, z_mask);
            z_data = _mm512_i32gather_epi32(z_data, (void const*)tabl32u, 4);
            x_data = _mm512_cvtepi32_epi8(z_data);
            _mm_mask_storeu_epi8((__m128i*)(dst_ptr + len_crn), msk16, x_data);
        }
        return;
    }

    {
        uint32_t    tabl_len_save = tabl_len - 3;
        __m512i     z_3_32i = _mm512_set1_epi32(3);
        __m512i     z_tbl_len_save = _mm512_set1_epi32(tabl_len_save);
        __mmask16   msklen;

        rem_len = length & 0xf;
        len_crn = length - rem_len;

        for (uint32_t i = 0; i < len_crn; i += 16) {
            z_data = _mm512_loadu_si512((__m512i const*)(src_32u_ptr + i));
            z_data = _mm512_srl_epi32(z_data, x_val_shift);
            z_data = _mm512_and_epi32(z_data, z_mask);
            msklen = _mm512_cmpgt_epi32_mask(z_data, z_tbl_len_save);
            z_data = _mm512_mask_sub_epi32(z_data, msklen, z_data, z_3_32i);
            z_data = _mm512_i32gather_epi32(z_data, (void const*)src2_ptr, 1);
            z_data = _mm512_mask_srli_epi32(z_data, msklen, z_data, 24);
            x_data = _mm512_cvtepi32_epi8(z_data);
            _mm_storeu_si128((__m128i*)(dst_ptr + i), x_data);
        }
        if (rem_len) {
            msk16 = (__mmask16)_bzhi_u32(0xffff, rem_len);
            z_data = _mm512_maskz_loadu_epi32(msk16, (void const*)(src_32u_ptr + len_crn));
            z_data = _mm512_srl_epi32(z_data, x_val_shift);
            z_data = _mm512_and_epi32(z_data, z_mask);
            msklen = _mm512_cmpgt_epi32_mask(z_data, z_tbl_len_save);
            z_data = _mm512_mask_sub_epi32(z_data, msklen, z_data, z_3_32i);
            z_data = _mm512_i32gather_epi32(z_data, (void const*)src2_ptr, 1);
            z_data = _mm512_mask_srli_epi32(z_data, msklen, z_data, 24);
            x_data = _mm512_cvtepi32_epi8(z_data);
            _mm_mask_storeu_epi8((__m128i*)(dst_ptr + len_crn), msk16, x_data);
        }
    }
}
#endif // OWN_SET_MEMBERSHIP_H
