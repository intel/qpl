/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

 /**
  * @brief Contains implementation of all functions for find_unique analytics operation
  * @date 04/12/2021
  *
  * @details Function list:
  *          - @ref qplc_find_unique_8u
  *          - @ref qplc_find_unique_16u8u
  *          - @ref qplc_find_unique_32u8u
  */

#ifndef OWN_FIND_UBIQUE_H
#define OWN_FIND_UBIQUE_H

#include "own_qplc_defs.h"
#include "immintrin.h"

  /******** out-of-place select functions ********/
OWN_OPT_FUN(void, k0_qplc_find_unique_8u, (const uint8_t* src_ptr,
    uint8_t* dst_ptr,
    uint32_t length,
    uint32_t shift,
    uint32_t mask)) {
    __m512i     z_data;
    __m512i     z_ones = _mm512_set1_epi64(1);
    __m512i     z_rslt = _mm512_setzero_si512();
    __m256i     y_rslt;
    __m128i     x_shft = _mm_cvtsi32_si128((int)shift);
    __m128i     x_data;
    __m128i     x_mask;
    uint32_t    remind = length & 15;
    __mmask64   msk;
    __mmask16   msk16;

    mask &= 0xff >> shift;
    length -= remind;
    x_mask = _mm_set1_epi8((char)mask);

    if (mask < 64) {
        for (uint32_t idx = 0u; idx < length; idx += 16) {
            x_data = _mm_loadu_si128((const __m128i*)(src_ptr + idx));          /* x_data = s15 .. s0       */
            x_data = _mm_srl_epi16(x_data, x_shft);                             /* x_data = s >> shft       */
            x_data = _mm_and_si128(x_data, x_mask);                             /* x_data = indx15 .. indx0 */
            z_data = _mm512_cvtepu8_epi64(x_data);                              /* z_data = indx7 .. indx0  */
            z_data = _mm512_sllv_epi64(z_ones, z_data);                         /* z_data = bit7 .. bit0    */
            z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
            x_data = _mm_srli_si128(x_data, 8);                                 /* x_data = indx15 .. indx8 */
            z_data = _mm512_cvtepu8_epi64(x_data);                              /* z_data = indx7 .. indx0  */
            z_data = _mm512_sllv_epi64(z_ones, z_data);                        /* z_data = bit7 .. bit0    */
            z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
        }
        if (remind) {
            msk16 = (__mmask16)_bzhi_u32(0xffff, remind);
            x_data = _mm_maskz_loadu_epi8(msk16, (const __m128i*)(src_ptr + length));  /* x_data = s15 .. s0       */
            x_data = _mm_srl_epi16(x_data, x_shft);                             /* x_data = s >> shft       */
            x_data = _mm_and_si128(x_data, x_mask);                             /* x_data = indx15 .. indx0 */
            z_data = _mm512_cvtepu8_epi64(x_data);                              /* z_data = indx7 .. indx0  */
            z_data = _mm512_maskz_sllv_epi64((__mmask8)msk16, z_ones, z_data);  /* z_data = bit7 .. bit0    */
            msk16 >>= 8;
            z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
            x_data = _mm_srli_si128(x_data, 8);                                 /* x_data = indx15 .. indx8 */
            z_data = _mm512_cvtepu8_epi64(x_data);                              /* z_data = indx7 .. indx0  */
            z_data = _mm512_maskz_sllv_epi64((__mmask8)msk16, z_ones, z_data);  /* z_data = bit7 .. bit0    */
            z_rslt = _mm512_or_si512(z_rslt, z_data);                           /* z_rslt = r7 .. r0        */
        }
        y_rslt = _mm512_extracti32x8_epi32(z_rslt, 1);
        y_rslt = _mm256_or_si256(y_rslt, _mm512_castsi512_si256(z_rslt));       /* y_rslt = r3 r2 r1 r0 */
        x_data = _mm256_extractf128_si256(y_rslt, 1);
        x_data = _mm_or_si128(x_data, _mm256_castsi256_si128(y_rslt));          /* x_data = r1 r0 */
        x_data = _mm_or_si128(x_data, _mm_srli_si128(x_data, 8));               /* x_data = r0 */
        z_ones = _mm512_set1_epi8(1);
        msk = (__mmask64)_mm_cvtsi128_si64(x_data);
        _mm512_mask_storeu_epi8((void*)dst_ptr, msk, z_ones);
        return;
    }

    if (mask < 128) {
        __m512i z_slct = _mm512_slli_epi64(z_ones, 6);                          /* z_slct = 64 */
        __m512i z_dat1;
        __m512i z_rsl1 = _mm512_setzero_si512();
        __m256i y_rsl1;
        __m128i x_dat1;
        for (uint32_t idx = 0u; idx < length; idx += 16) {
            x_data = _mm_loadu_si128((const __m128i*)(src_ptr + idx));          /* x_data = s15 .. s0       */
            x_data = _mm_srl_epi16(x_data, x_shft);                             /* x_data = s >> shft       */
            x_data = _mm_and_si128(x_data, x_mask);                             /* x_data = indx15 .. indx0 */
            z_data = _mm512_cvtepu8_epi64(x_data);                              /* z_data = indx7 .. indx0  */
            z_dat1 = _mm512_xor_si512(z_data, z_slct);
            z_data = _mm512_sllv_epi64(z_ones, z_data);                         /* z_data = bit7 .. bit0    */
            z_dat1 = _mm512_sllv_epi64(z_ones, z_dat1);                         /* z_data = bit7 .. bit0    */
            z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
            z_rsl1 = _mm512_or_si512(z_rsl1, z_dat1);                          /* z_rslt = r7 .. r0        */
            x_data = _mm_srli_si128(x_data, 8);                                 /* x_data = indx15 .. indx8 */
            z_data = _mm512_cvtepu8_epi64(x_data);                              /* z_data = indx7 .. indx0  */
            z_dat1 = _mm512_xor_si512(z_data, z_slct);
            z_data = _mm512_sllv_epi64(z_ones, z_data);                         /* z_data = bit7 .. bit0    */
            z_dat1 = _mm512_sllv_epi64(z_ones, z_dat1);                         /* z_data = bit7 .. bit0    */
            z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
            z_rsl1 = _mm512_or_si512(z_rsl1, z_dat1);                          /* z_rslt = r7 .. r0        */
        }
        if (remind) {
            msk16 = (__mmask16)_bzhi_u32(0xffff, remind);
            x_data = _mm_maskz_loadu_epi8(msk16, (const __m128i*)(src_ptr + length));  /* x_data = s15 .. s0       */
            x_data = _mm_srl_epi16(x_data, x_shft);                             /* x_data = s >> shft       */
            x_data = _mm_and_si128(x_data, x_mask);                             /* x_data = indx15 .. indx0 */
            z_data = _mm512_cvtepu8_epi64(x_data);                              /* z_data = indx7 .. indx0  */
            z_dat1 = _mm512_xor_si512(z_data, z_slct);
            z_data = _mm512_maskz_sllv_epi64((__mmask8)msk16, z_ones, z_data);  /* z_data = bit7 .. bit0    */
            z_dat1 = _mm512_maskz_sllv_epi64((__mmask8)msk16, z_ones, z_dat1);  /* z_data = bit7 .. bit0    */
            msk16 >>= 8;
            z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
            z_rsl1 = _mm512_or_si512(z_rsl1, z_dat1);                          /* z_rslt = r7 .. r0        */
            x_data = _mm_srli_si128(x_data, 8);                                 /* x_data = indx15 .. indx8 */
            z_data = _mm512_cvtepu8_epi64(x_data);                              /* z_data = indx7 .. indx0  */
            z_dat1 = _mm512_xor_si512(z_data, z_slct);
            z_data = _mm512_maskz_sllv_epi64((__mmask8)msk16, z_ones, z_data);  /* z_data = bit7 .. bit0    */
            z_dat1 = _mm512_maskz_sllv_epi64((__mmask8)msk16, z_ones, z_dat1);  /* z_data = bit7 .. bit0    */
            z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
            z_rsl1 = _mm512_or_si512(z_rsl1, z_dat1);                          /* z_rslt = r7 .. r0        */
        }
        y_rslt = _mm512_extracti32x8_epi32(z_rslt, 1);
        y_rsl1 = _mm512_extracti32x8_epi32(z_rsl1, 1);
        y_rslt = _mm256_or_si256(y_rslt, _mm512_castsi512_si256(z_rslt));       /* y_rslt = r3 r2 r1 r0 */
        y_rsl1 = _mm256_or_si256(y_rsl1, _mm512_castsi512_si256(z_rsl1));       /* y_rslt = r3 r2 r1 r0 */
        x_data = _mm256_extractf128_si256(y_rslt, 1);
        x_dat1 = _mm256_extractf128_si256(y_rsl1, 1);
        x_data = _mm_or_si128(x_data, _mm256_castsi256_si128(y_rslt));          /* x_data = r1 r0 */
        x_dat1 = _mm_or_si128(x_dat1, _mm256_castsi256_si128(y_rsl1));          /* x_data = r1 r0 */
        x_data = _mm_or_si128(x_data, _mm_srli_si128(x_data, 8));               /* x_data = r0 */
        x_dat1 = _mm_or_si128(x_dat1, _mm_srli_si128(x_dat1, 8));               /* x_data = r0 */
        z_ones = _mm512_set1_epi8(1);
        msk = (__mmask64)_mm_cvtsi128_si64(x_data);
        _mm512_mask_storeu_epi8((void*)dst_ptr, msk, z_ones);
        msk = (__mmask64)_mm_cvtsi128_si64(x_dat1);
        _mm512_mask_storeu_epi8((void*)(dst_ptr + 64), msk, z_ones);
        return;
    }

    {
        __m512i z_slc1 = _mm512_slli_epi64(z_ones, 6);                          /* z_slct = 64 */
        __m512i z_slc2 = _mm512_add_epi64(z_slc1, z_slc1);                      /* z_slct = 128 */
        __m512i z_slc3 = _mm512_add_epi64(z_slc1, z_slc2);                      /* z_slct = 192*/
        __m512i z_dat1, z_dat2, z_dat3;
        __m512i z_rsl1 = _mm512_setzero_si512();
        __m512i z_rsl2 = _mm512_setzero_si512();
        __m512i z_rsl3 = _mm512_setzero_si512();
        __m256i y_rsl1, y_rsl2, y_rsl3;
        __m128i x_dat1, x_dat2, x_dat3;
        for (uint32_t idx = 0u; idx < length; idx += 16) {
            x_data = _mm_loadu_si128((const __m128i*)(src_ptr + idx));          /* x_data = s15 .. s0       */
            x_data = _mm_srl_epi16(x_data, x_shft);                             /* x_data = s >> shft       */
            x_data = _mm_and_si128(x_data, x_mask);                             /* x_data = indx15 .. indx0 */
            z_data = _mm512_cvtepu8_epi64(x_data);                              /* z_data = indx7 .. indx0  */
            z_dat1 = _mm512_xor_si512(z_data, z_slc1);
            z_dat2 = _mm512_xor_si512(z_data, z_slc2);
            z_dat3 = _mm512_xor_si512(z_data, z_slc3);
            z_data = _mm512_sllv_epi64(z_ones, z_data);                         /* z_data = bit7 .. bit0    */
            z_dat1 = _mm512_sllv_epi64(z_ones, z_dat1);                         /* z_data = bit7 .. bit0    */
            z_dat2 = _mm512_sllv_epi64(z_ones, z_dat2);                         /* z_data = bit7 .. bit0    */
            z_dat3 = _mm512_sllv_epi64(z_ones, z_dat3);                         /* z_data = bit7 .. bit0    */
            z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
            z_rsl1 = _mm512_or_si512(z_rsl1, z_dat1);                          /* z_rslt = r7 .. r0        */
            z_rsl2 = _mm512_or_si512(z_rsl2, z_dat2);                          /* z_rslt = r7 .. r0        */
            z_rsl3 = _mm512_or_si512(z_rsl3, z_dat3);                          /* z_rslt = r7 .. r0        */
            x_data = _mm_srli_si128(x_data, 8);                                 /* x_data = indx15 .. indx8 */
            z_data = _mm512_cvtepu8_epi64(x_data);                              /* z_data = indx15 .. indx0  */
            z_dat1 = _mm512_xor_si512(z_data, z_slc1);
            z_dat2 = _mm512_xor_si512(z_data, z_slc2);
            z_dat3 = _mm512_xor_si512(z_data, z_slc3);
            z_data = _mm512_sllv_epi64(z_ones, z_data);                         /* z_data = bit7 .. bit0    */
            z_dat1 = _mm512_sllv_epi64(z_ones, z_dat1);                         /* z_data = bit7 .. bit0    */
            z_dat2 = _mm512_sllv_epi64(z_ones, z_dat2);                         /* z_data = bit7 .. bit0    */
            z_dat3 = _mm512_sllv_epi64(z_ones, z_dat3);                         /* z_data = bit7 .. bit0    */
            z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
            z_rsl1 = _mm512_or_si512(z_rsl1, z_dat1);                          /* z_rslt = r7 .. r0        */
            z_rsl2 = _mm512_or_si512(z_rsl2, z_dat2);                          /* z_rslt = r7 .. r0        */
            z_rsl3 = _mm512_or_si512(z_rsl3, z_dat3);                          /* z_rslt = r7 .. r0        */
        }
        if (remind) {
            msk16 = (__mmask16)_bzhi_u32(0xffff, remind);
            x_data = _mm_maskz_loadu_epi8(msk16, (const __m128i*)(src_ptr + length));  /* x_data = s15 .. s0       */
            x_data = _mm_srl_epi16(x_data, x_shft);                             /* x_data = s >> shft       */
            x_data = _mm_and_si128(x_data, x_mask);                             /* x_data = indx15 .. indx0 */
            z_data = _mm512_cvtepu8_epi64(x_data);                              /* z_data = indx7 .. indx0  */
            z_dat1 = _mm512_xor_si512(z_data, z_slc1);
            z_dat2 = _mm512_xor_si512(z_data, z_slc2);
            z_dat3 = _mm512_xor_si512(z_data, z_slc3);
            z_data = _mm512_maskz_sllv_epi64((__mmask8)msk16, z_ones, z_data);  /* z_data = bit7 .. bit0    */
            z_dat1 = _mm512_maskz_sllv_epi64((__mmask8)msk16, z_ones, z_dat1);  /* z_data = bit7 .. bit0    */
            z_dat2 = _mm512_maskz_sllv_epi64((__mmask8)msk16, z_ones, z_dat2);  /* z_data = bit7 .. bit0    */
            z_dat3 = _mm512_maskz_sllv_epi64((__mmask8)msk16, z_ones, z_dat3);  /* z_data = bit7 .. bit0    */
            msk16 >>= 8;
            z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
            z_rsl1 = _mm512_or_si512(z_rsl1, z_dat1);                          /* z_rslt = r7 .. r0        */
            z_rsl2 = _mm512_or_si512(z_rsl2, z_dat2);                          /* z_rslt = r7 .. r0        */
            z_rsl3 = _mm512_or_si512(z_rsl3, z_dat3);                          /* z_rslt = r7 .. r0        */
            x_data = _mm_srli_si128(x_data, 8);                                 /* x_data = indx15 .. indx8 */
            z_data = _mm512_cvtepu8_epi64(x_data);                              /* z_data = indx7 .. indx0  */
            z_dat1 = _mm512_xor_si512(z_data, z_slc1);
            z_dat2 = _mm512_xor_si512(z_data, z_slc2);
            z_dat3 = _mm512_xor_si512(z_data, z_slc3);
            z_data = _mm512_maskz_sllv_epi64((__mmask8)msk16, z_ones, z_data);  /* z_data = bit7 .. bit0    */
            z_dat1 = _mm512_maskz_sllv_epi64((__mmask8)msk16, z_ones, z_dat1);  /* z_data = bit7 .. bit0    */
            z_dat2 = _mm512_maskz_sllv_epi64((__mmask8)msk16, z_ones, z_dat2);  /* z_data = bit7 .. bit0    */
            z_dat3 = _mm512_maskz_sllv_epi64((__mmask8)msk16, z_ones, z_dat3);  /* z_data = bit7 .. bit0    */
            z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
            z_rsl1 = _mm512_or_si512(z_rsl1, z_dat1);                          /* z_rslt = r7 .. r0        */
            z_rsl2 = _mm512_or_si512(z_rsl2, z_dat2);                          /* z_rslt = r7 .. r0        */
            z_rsl3 = _mm512_or_si512(z_rsl3, z_dat3);                          /* z_rslt = r7 .. r0        */
        }
        y_rslt = _mm512_extracti32x8_epi32(z_rslt, 1);
        y_rsl1 = _mm512_extracti32x8_epi32(z_rsl1, 1);
        y_rsl2 = _mm512_extracti32x8_epi32(z_rsl2, 1);
        y_rsl3 = _mm512_extracti32x8_epi32(z_rsl3, 1);
        y_rslt = _mm256_or_si256(y_rslt, _mm512_castsi512_si256(z_rslt));       /* y_rslt = r3 r2 r1 r0 */
        y_rsl1 = _mm256_or_si256(y_rsl1, _mm512_castsi512_si256(z_rsl1));       /* y_rslt = r3 r2 r1 r0 */
        y_rsl2 = _mm256_or_si256(y_rsl2, _mm512_castsi512_si256(z_rsl2));       /* y_rslt = r3 r2 r1 r0 */
        y_rsl3 = _mm256_or_si256(y_rsl3, _mm512_castsi512_si256(z_rsl3));       /* y_rslt = r3 r2 r1 r0 */
        x_data = _mm256_extractf128_si256(y_rslt, 1);
        x_dat1 = _mm256_extractf128_si256(y_rsl1, 1);
        x_dat2 = _mm256_extractf128_si256(y_rsl2, 1);
        x_dat3 = _mm256_extractf128_si256(y_rsl3, 1);
        x_data = _mm_or_si128(x_data, _mm256_castsi256_si128(y_rslt));          /* x_data = r1 r0 */
        x_dat1 = _mm_or_si128(x_dat1, _mm256_castsi256_si128(y_rsl1));          /* x_data = r1 r0 */
        x_dat2 = _mm_or_si128(x_dat2, _mm256_castsi256_si128(y_rsl2));          /* x_data = r1 r0 */
        x_dat3 = _mm_or_si128(x_dat3, _mm256_castsi256_si128(y_rsl3));          /* x_data = r1 r0 */
        x_data = _mm_or_si128(x_data, _mm_srli_si128(x_data, 8));               /* x_data = r0 */
        x_dat1 = _mm_or_si128(x_dat1, _mm_srli_si128(x_dat1, 8));               /* x_data = r0 */
        x_dat2 = _mm_or_si128(x_dat2, _mm_srli_si128(x_dat2, 8));               /* x_data = r0 */
        x_dat3 = _mm_or_si128(x_dat3, _mm_srli_si128(x_dat3, 8));               /* x_data = r0 */
        z_ones = _mm512_set1_epi8(1);
        msk = (__mmask64)_mm_cvtsi128_si64(x_data);
        _mm512_mask_storeu_epi8((void*)dst_ptr, msk, z_ones);
        msk = (__mmask64)_mm_cvtsi128_si64(x_dat1);
        _mm512_mask_storeu_epi8((void*)(dst_ptr + 64), msk, z_ones);
        msk = (__mmask64)_mm_cvtsi128_si64(x_dat2);
        _mm512_mask_storeu_epi8((void*)(dst_ptr + 128), msk, z_ones);
        msk = (__mmask64)_mm_cvtsi128_si64(x_dat3);
        _mm512_mask_storeu_epi8((void*)(dst_ptr + 192), msk, z_ones);
    }
}

/***************************************************/
OWN_OPT_FUN(void, k0_qplc_find_unique_16u8u, (const uint8_t* src_ptr,
    uint8_t* dst_ptr,
    uint32_t length,
    uint32_t shift,
    uint32_t mask)) {
    uint16_t* src_16u_ptr = (uint16_t*)src_ptr;

    mask &= 0xffff >> shift;
    if (mask >= 256) {
        uint32_t index;
        for (uint32_t idx = 0u; idx < length; idx++) {
            index = (src_16u_ptr[idx] >> shift) & mask;
            dst_ptr[index] = 1u;
        }
        return;
    }

    {
        __m512i     z_data;
        __m512i     z_ones = _mm512_set1_epi64(1);
        __m512i     z_rslt = _mm512_setzero_si512();
        __m256i     y_rslt;
        __m128i     x_shft = _mm_cvtsi32_si128((int)shift);
        __m128i     x_data;
        __m128i     x_mask;
        uint32_t    remind = length & 7;
        __mmask64   msk;
        __mmask8    msk8;

        length -= remind;
        x_mask = _mm_set1_epi16((short)mask);

        if (mask < 64) {
            for (uint32_t idx = 0u; idx < length; idx += 8) {
                x_data = _mm_loadu_si128((const __m128i*)(src_16u_ptr + idx));     /* x_data = s7 .. s0       */
                x_data = _mm_srl_epi16(x_data, x_shft);                            /* x_data = s >> shft       */
                x_data = _mm_and_si128(x_data, x_mask);                            /* x_data = indx7 .. indx0 */
                z_data = _mm512_cvtepu16_epi64(x_data);                            /* z_data = indx7 .. indx0  */
                z_data = _mm512_sllv_epi64(z_ones, z_data);                        /* z_data = bit7 .. bit0    */
                z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
            }
            if (remind) {
                msk8 = (__mmask8)_bzhi_u32(0xff, remind);
                x_data = _mm_maskz_loadu_epi16(msk8, (const __m128i*)(src_16u_ptr + length));  /* x_data = s7 .. s0       */
                x_data = _mm_srl_epi16(x_data, x_shft);                            /* x_data = s >> shft       */
                x_data = _mm_and_si128(x_data, x_mask);                            /* x_data = indx7 .. indx0 */
                z_data = _mm512_cvtepu16_epi64(x_data);                            /* z_data = indx7 .. indx0  */
                z_data = _mm512_maskz_sllv_epi64((__mmask8)msk8, z_ones, z_data);  /* z_data = bit7 .. bit0    */
                z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
            }
            y_rslt = _mm512_extracti32x8_epi32(z_rslt, 1);
            y_rslt = _mm256_or_si256(y_rslt, _mm512_castsi512_si256(z_rslt));       /* y_rslt = r3 r2 r1 r0 */
            x_data = _mm256_extractf128_si256(y_rslt, 1);
            x_data = _mm_or_si128(x_data, _mm256_castsi256_si128(y_rslt));          /* x_data = r1 r0 */
            x_data = _mm_or_si128(x_data, _mm_srli_si128(x_data, 8));               /* x_data = r0 */
            z_ones = _mm512_set1_epi8(1);
            msk = (__mmask64)_mm_cvtsi128_si64(x_data);
            _mm512_mask_storeu_epi8((void*)dst_ptr, msk, z_ones);
            return;
        }

        if (mask < 128) {
            __m512i z_slct = _mm512_slli_epi64(z_ones, 6);                          /* z_slct = 64 */
            __m512i z_dat1;
            __m512i z_rsl1 = _mm512_setzero_si512();
            __m256i y_rsl1;
            __m128i x_dat1;
            for (uint32_t idx = 0u; idx < length; idx += 8) {
                x_data = _mm_loadu_si128((const __m128i*)(src_16u_ptr + idx));     /* x_data = s7 .. s0       */
                x_data = _mm_srl_epi16(x_data, x_shft);                            /* x_data = s >> shft       */
                x_data = _mm_and_si128(x_data, x_mask);                            /* x_data = indx7 .. indx0 */
                z_data = _mm512_cvtepu16_epi64(x_data);                            /* z_data = indx7 .. indx0  */
                z_dat1 = _mm512_xor_si512(z_data, z_slct);
                z_data = _mm512_sllv_epi64(z_ones, z_data);                        /* z_data = bit7 .. bit0    */
                z_dat1 = _mm512_sllv_epi64(z_ones, z_dat1);                        /* z_data = bit7 .. bit0    */
                z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
                z_rsl1 = _mm512_or_si512(z_rsl1, z_dat1);                          /* z_rslt = r7 .. r0        */
            }
            if (remind) {
                msk8 = (__mmask8)_bzhi_u32(0xff, remind);
                x_data = _mm_maskz_loadu_epi16(msk8, (const __m128i*)(src_16u_ptr + length));  /* x_data = s7 .. s0       */
                x_data = _mm_srl_epi16(x_data, x_shft);                             /* x_data = s >> shft       */
                x_data = _mm_and_si128(x_data, x_mask);                             /* x_data = indx7 .. indx0 */
                z_data = _mm512_cvtepu16_epi64(x_data);                             /* z_data = indx7 .. indx0  */
                z_dat1 = _mm512_xor_si512(z_data, z_slct);
                z_data = _mm512_maskz_sllv_epi64(msk8, z_ones, z_data);            /* z_data = bit7 .. bit0    */
                z_dat1 = _mm512_maskz_sllv_epi64(msk8, z_ones, z_dat1);            /* z_data = bit7 .. bit0    */
                z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
                z_rsl1 = _mm512_or_si512(z_rsl1, z_dat1);                          /* z_rslt = r7 .. r0        */
            }
            y_rslt = _mm512_extracti32x8_epi32(z_rslt, 1);
            y_rsl1 = _mm512_extracti32x8_epi32(z_rsl1, 1);
            y_rslt = _mm256_or_si256(y_rslt, _mm512_castsi512_si256(z_rslt));       /* y_rslt = r3 r2 r1 r0 */
            y_rsl1 = _mm256_or_si256(y_rsl1, _mm512_castsi512_si256(z_rsl1));       /* y_rslt = r3 r2 r1 r0 */
            x_data = _mm256_extractf128_si256(y_rslt, 1);
            x_dat1 = _mm256_extractf128_si256(y_rsl1, 1);
            x_data = _mm_or_si128(x_data, _mm256_castsi256_si128(y_rslt));          /* x_data = r1 r0 */
            x_dat1 = _mm_or_si128(x_dat1, _mm256_castsi256_si128(y_rsl1));          /* x_data = r1 r0 */
            x_data = _mm_or_si128(x_data, _mm_srli_si128(x_data, 8));               /* x_data = r0 */
            x_dat1 = _mm_or_si128(x_dat1, _mm_srli_si128(x_dat1, 8));               /* x_data = r0 */
            z_ones = _mm512_set1_epi8(1);
            msk = (__mmask64)_mm_cvtsi128_si64(x_data);
            _mm512_mask_storeu_epi8((void*)dst_ptr, msk, z_ones);
            msk = (__mmask64)_mm_cvtsi128_si64(x_dat1);
            _mm512_mask_storeu_epi8((void*)(dst_ptr + 64), msk, z_ones);
            return;
        }

        {
            __m512i z_slc1 = _mm512_slli_epi64(z_ones, 6);                          /* z_slct = 64 */
            __m512i z_slc2 = _mm512_add_epi64(z_slc1, z_slc1);                      /* z_slct = 128 */
            __m512i z_slc3 = _mm512_add_epi64(z_slc1, z_slc2);                      /* z_slct = 192*/
            __m512i z_dat1, z_dat2, z_dat3;
            __m512i z_rsl1 = _mm512_setzero_si512();
            __m512i z_rsl2 = _mm512_setzero_si512();
            __m512i z_rsl3 = _mm512_setzero_si512();
            __m256i y_rsl1, y_rsl2, y_rsl3;
            __m128i x_dat1, x_dat2, x_dat3;
            for (uint32_t idx = 0u; idx < length; idx += 8) {
                x_data = _mm_loadu_si128((const __m128i*)(src_16u_ptr + idx));      /* x_data = s7 .. s0       */
                x_data = _mm_srl_epi16(x_data, x_shft);                             /* x_data = s >> shft       */
                x_data = _mm_and_si128(x_data, x_mask);                             /* x_data = indx7 .. indx0 */
                z_data = _mm512_cvtepu16_epi64(x_data);                             /* z_data = indx7 .. indx0  */
                z_dat1 = _mm512_xor_si512(z_data, z_slc1);
                z_dat2 = _mm512_xor_si512(z_data, z_slc2);
                z_dat3 = _mm512_xor_si512(z_data, z_slc3);
                z_data = _mm512_sllv_epi64(z_ones, z_data);                         /* z_data = bit7 .. bit0    */
                z_dat1 = _mm512_sllv_epi64(z_ones, z_dat1);                         /* z_data = bit7 .. bit0    */
                z_dat2 = _mm512_sllv_epi64(z_ones, z_dat2);                         /* z_data = bit7 .. bit0    */
                z_dat3 = _mm512_sllv_epi64(z_ones, z_dat3);                         /* z_data = bit7 .. bit0    */
                z_rslt = _mm512_or_si512(z_rslt, z_data);                           /* z_rslt = r7 .. r0        */
                z_rsl1 = _mm512_or_si512(z_rsl1, z_dat1);                           /* z_rslt = r7 .. r0        */
                z_rsl2 = _mm512_or_si512(z_rsl2, z_dat2);                           /* z_rslt = r7 .. r0        */
                z_rsl3 = _mm512_or_si512(z_rsl3, z_dat3);                           /* z_rslt = r7 .. r0        */
            }
            if (remind) {
                msk8 = (__mmask8)_bzhi_u32(0xff, remind);
                x_data = _mm_maskz_loadu_epi16(msk8, (const __m128i*)(src_16u_ptr + length));  /* x_data = s7 .. s0       */
                x_data = _mm_srl_epi16(x_data, x_shft);                             /* x_data = s >> shft       */
                x_data = _mm_and_si128(x_data, x_mask);                             /* x_data = indx7 .. indx0 */
                z_data = _mm512_cvtepu16_epi64(x_data);                             /* z_data = indx7 .. indx0  */
                z_dat1 = _mm512_xor_si512(z_data, z_slc1);
                z_dat2 = _mm512_xor_si512(z_data, z_slc2);
                z_dat3 = _mm512_xor_si512(z_data, z_slc3);
                z_data = _mm512_maskz_sllv_epi64((__mmask8)msk8, z_ones, z_data);  /* z_data = bit7 .. bit0    */
                z_dat1 = _mm512_maskz_sllv_epi64((__mmask8)msk8, z_ones, z_dat1);  /* z_data = bit7 .. bit0    */
                z_dat2 = _mm512_maskz_sllv_epi64((__mmask8)msk8, z_ones, z_dat2);  /* z_data = bit7 .. bit0    */
                z_dat3 = _mm512_maskz_sllv_epi64((__mmask8)msk8, z_ones, z_dat3);  /* z_data = bit7 .. bit0    */
                z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
                z_rsl1 = _mm512_or_si512(z_rsl1, z_dat1);                          /* z_rslt = r7 .. r0        */
                z_rsl2 = _mm512_or_si512(z_rsl2, z_dat2);                          /* z_rslt = r7 .. r0        */
                z_rsl3 = _mm512_or_si512(z_rsl3, z_dat3);                          /* z_rslt = r7 .. r0        */
            }
            y_rslt = _mm512_extracti32x8_epi32(z_rslt, 1);
            y_rsl1 = _mm512_extracti32x8_epi32(z_rsl1, 1);
            y_rsl2 = _mm512_extracti32x8_epi32(z_rsl2, 1);
            y_rsl3 = _mm512_extracti32x8_epi32(z_rsl3, 1);
            y_rslt = _mm256_or_si256(y_rslt, _mm512_castsi512_si256(z_rslt));       /* y_rslt = r3 r2 r1 r0 */
            y_rsl1 = _mm256_or_si256(y_rsl1, _mm512_castsi512_si256(z_rsl1));       /* y_rslt = r3 r2 r1 r0 */
            y_rsl2 = _mm256_or_si256(y_rsl2, _mm512_castsi512_si256(z_rsl2));       /* y_rslt = r3 r2 r1 r0 */
            y_rsl3 = _mm256_or_si256(y_rsl3, _mm512_castsi512_si256(z_rsl3));       /* y_rslt = r3 r2 r1 r0 */
            x_data = _mm256_extractf128_si256(y_rslt, 1);
            x_dat1 = _mm256_extractf128_si256(y_rsl1, 1);
            x_dat2 = _mm256_extractf128_si256(y_rsl2, 1);
            x_dat3 = _mm256_extractf128_si256(y_rsl3, 1);
            x_data = _mm_or_si128(x_data, _mm256_castsi256_si128(y_rslt));          /* x_data = r1 r0 */
            x_dat1 = _mm_or_si128(x_dat1, _mm256_castsi256_si128(y_rsl1));          /* x_data = r1 r0 */
            x_dat2 = _mm_or_si128(x_dat2, _mm256_castsi256_si128(y_rsl2));          /* x_data = r1 r0 */
            x_dat3 = _mm_or_si128(x_dat3, _mm256_castsi256_si128(y_rsl3));          /* x_data = r1 r0 */
            x_data = _mm_or_si128(x_data, _mm_srli_si128(x_data, 8));               /* x_data = r0 */
            x_dat1 = _mm_or_si128(x_dat1, _mm_srli_si128(x_dat1, 8));               /* x_data = r0 */
            x_dat2 = _mm_or_si128(x_dat2, _mm_srli_si128(x_dat2, 8));               /* x_data = r0 */
            x_dat3 = _mm_or_si128(x_dat3, _mm_srli_si128(x_dat3, 8));               /* x_data = r0 */
            z_ones = _mm512_set1_epi8(1);
            msk = (__mmask64)_mm_cvtsi128_si64(x_data);
            _mm512_mask_storeu_epi8((void*)dst_ptr, msk, z_ones);
            msk = (__mmask64)_mm_cvtsi128_si64(x_dat1);
            _mm512_mask_storeu_epi8((void*)(dst_ptr + 64), msk, z_ones);
            msk = (__mmask64)_mm_cvtsi128_si64(x_dat2);
            _mm512_mask_storeu_epi8((void*)(dst_ptr + 128), msk, z_ones);
            msk = (__mmask64)_mm_cvtsi128_si64(x_dat3);
            _mm512_mask_storeu_epi8((void*)(dst_ptr + 192), msk, z_ones);
        }
    }
}


/***************************************************/
OWN_OPT_FUN(void, k0_qplc_find_unique_32u8u, (const uint8_t* src_ptr,
    uint8_t* dst_ptr,
    uint32_t length,
    uint32_t shift,
    uint32_t mask)) {
    uint32_t* src_32u_ptr = (uint32_t*)src_ptr;

    mask &= (uint32_t)0xffffffff >> shift;
    if (mask >= 256) {
        uint32_t index;
        for (uint32_t idx = 0u; idx < length; idx++) {
            index = (src_32u_ptr[idx] >> shift) & mask;
            dst_ptr[index] = 1u;
        }
        return;
    }

    {
        __m512i     z_data;
        __m512i     z_ones = _mm512_set1_epi64(1);
        __m512i     z_rslt = _mm512_setzero_si512();
        __m256i     y_rslt;
        __m256i     y_data;
        __m256i     y_mask;
        __m128i     x_shft = _mm_cvtsi32_si128((int)shift);
        __m128i     x_data;
        uint32_t    remind = length & 7;
        __mmask64   msk;
        __mmask8    msk8;

        length -= remind;
        y_mask = _mm256_set1_epi32((int)mask);
        if (mask < 64) {
            for (uint32_t idx = 0u; idx < length; idx += 8) {
                y_data = _mm256_loadu_si256((const __m256i*)(src_32u_ptr + idx));  /* y_data = s7 .. s0       */
                y_data = _mm256_srl_epi32(y_data, x_shft);                         /* y_data = s >> shft       */
                y_data = _mm256_and_si256(y_data, y_mask);                         /* y_data = indx7 .. indx0 */
                z_data = _mm512_cvtepu32_epi64(y_data);                            /* z_data = indx7 .. indx0  */
                z_data = _mm512_sllv_epi64(z_ones, z_data);                        /* z_data = bit7 .. bit0    */
                z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
            }
            if (remind) {
                msk8 = (__mmask8)_bzhi_u32(0xff, remind);
                y_data = _mm256_maskz_loadu_epi32(msk8, (const __m256i*)(src_32u_ptr + length));  /* x_data = s15 .. s0       */
                y_data = _mm256_srl_epi32(y_data, x_shft);                         /* x_data = s >> shft       */
                y_data = _mm256_and_si256(y_data, y_mask);                         /* x_data = indx15 .. indx0 */
                z_data = _mm512_cvtepu32_epi64(y_data);                            /* z_data = indx7 .. indx0  */
                z_data = _mm512_maskz_sllv_epi64((__mmask8)msk8, z_ones, z_data);  /* z_data = bit7 .. bit0    */
                z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
            }
            y_rslt = _mm512_extracti32x8_epi32(z_rslt, 1);
            y_rslt = _mm256_or_si256(y_rslt, _mm512_castsi512_si256(z_rslt));       /* y_rslt = r3 r2 r1 r0 */
            x_data = _mm256_extractf128_si256(y_rslt, 1);
            x_data = _mm_or_si128(x_data, _mm256_castsi256_si128(y_rslt));          /* x_data = r1 r0 */
            x_data = _mm_or_si128(x_data, _mm_srli_si128(x_data, 8));               /* x_data = r0 */
            z_ones = _mm512_set1_epi8(1);
            msk = (__mmask64)_mm_cvtsi128_si64(x_data);
            _mm512_mask_storeu_epi8((void*)dst_ptr, msk, z_ones);
            return;
        }

        if (mask < 128) {
            __m512i z_slct = _mm512_slli_epi64(z_ones, 6);                          /* z_slct = 64 */
            __m512i z_dat1;
            __m512i z_rsl1 = _mm512_setzero_si512();
            __m256i y_rsl1;
            __m128i x_dat1;
            for (uint32_t idx = 0u; idx < length; idx += 8) {
                y_data = _mm256_loadu_si256((const __m256i*)(src_32u_ptr + idx));  /* y_data = s7 .. s0        */
                y_data = _mm256_srl_epi32(y_data, x_shft);                         /* y_data = s >> shft       */
                y_data = _mm256_and_si256(y_data, y_mask);                         /* y_data = indx7 .. indx0  */
                z_data = _mm512_cvtepu32_epi64(y_data);                            /* z_data = indx7 .. indx0  */
                z_dat1 = _mm512_xor_si512(z_data, z_slct);
                z_data = _mm512_sllv_epi64(z_ones, z_data);                        /* z_data = bit7 .. bit0    */
                z_dat1 = _mm512_sllv_epi64(z_ones, z_dat1);                        /* z_data = bit7 .. bit0    */
                z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
                z_rsl1 = _mm512_or_si512(z_rsl1, z_dat1);                          /* z_rslt = r7 .. r0        */
            }
            if (remind) {
                msk8 = (__mmask8)_bzhi_u32(0xff, remind);
                y_data = _mm256_maskz_loadu_epi32(msk8, (const __m256i*)(src_32u_ptr + length));  /* y_data = s7 .. s0       */
                y_data = _mm256_srl_epi32(y_data, x_shft);                         /* y_data = s >> shft       */
                y_data = _mm256_and_si256(y_data, y_mask);                         /* y_data = indx7 .. indx0  */
                z_data = _mm512_cvtepu32_epi64(y_data);                            /* z_data = indx7 .. indx0  */
                z_dat1 = _mm512_xor_si512(z_data, z_slct);
                z_data = _mm512_maskz_sllv_epi64(msk8, z_ones, z_data);  /* z_data = bit7 .. bit0    */
                z_dat1 = _mm512_maskz_sllv_epi64(msk8, z_ones, z_dat1);  /* z_data = bit7 .. bit0    */
                z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
                z_rsl1 = _mm512_or_si512(z_rsl1, z_dat1);                          /* z_rslt = r7 .. r0        */
            }
            y_rslt = _mm512_extracti32x8_epi32(z_rslt, 1);
            y_rsl1 = _mm512_extracti32x8_epi32(z_rsl1, 1);
            y_rslt = _mm256_or_si256(y_rslt, _mm512_castsi512_si256(z_rslt));       /* y_rslt = r3 r2 r1 r0 */
            y_rsl1 = _mm256_or_si256(y_rsl1, _mm512_castsi512_si256(z_rsl1));       /* y_rslt = r3 r2 r1 r0 */
            x_data = _mm256_extractf128_si256(y_rslt, 1);
            x_dat1 = _mm256_extractf128_si256(y_rsl1, 1);
            x_data = _mm_or_si128(x_data, _mm256_castsi256_si128(y_rslt));          /* x_data = r1 r0 */
            x_dat1 = _mm_or_si128(x_dat1, _mm256_castsi256_si128(y_rsl1));          /* x_data = r1 r0 */
            x_data = _mm_or_si128(x_data, _mm_srli_si128(x_data, 8));               /* x_data = r0 */
            x_dat1 = _mm_or_si128(x_dat1, _mm_srli_si128(x_dat1, 8));               /* x_data = r0 */
            z_ones = _mm512_set1_epi8(1);
            msk = (__mmask64)_mm_cvtsi128_si64(x_data);
            _mm512_mask_storeu_epi8((void*)dst_ptr, msk, z_ones);
            msk = (__mmask64)_mm_cvtsi128_si64(x_dat1);
            _mm512_mask_storeu_epi8((void*)(dst_ptr + 64), msk, z_ones);
            return;
        }

        {
            __m512i z_slc1 = _mm512_slli_epi64(z_ones, 6);                          /* z_slct = 64 */
            __m512i z_slc2 = _mm512_add_epi64(z_slc1, z_slc1);                      /* z_slct = 128 */
            __m512i z_slc3 = _mm512_add_epi64(z_slc1, z_slc2);                      /* z_slct = 192*/
            __m512i z_dat1, z_dat2, z_dat3;
            __m512i z_rsl1 = _mm512_setzero_si512();
            __m512i z_rsl2 = _mm512_setzero_si512();
            __m512i z_rsl3 = _mm512_setzero_si512();
            __m256i y_rsl1, y_rsl2, y_rsl3;
            __m128i x_dat1, x_dat2, x_dat3;
            for (uint32_t idx = 0u; idx < length; idx += 8) {
                y_data = _mm256_loadu_si256((const __m256i*)(src_32u_ptr + idx));   /* y_data = s7 .. s0        */
                y_data = _mm256_srl_epi32(y_data, x_shft);                          /* y_data = s >> shft       */
                y_data = _mm256_and_si256(y_data, y_mask);                          /* y_data = indx7 .. indx0  */
                z_data = _mm512_cvtepu32_epi64(y_data);                             /* z_data = indx7 .. indx0  */
                z_dat1 = _mm512_xor_si512(z_data, z_slc1);
                z_dat2 = _mm512_xor_si512(z_data, z_slc2);
                z_dat3 = _mm512_xor_si512(z_data, z_slc3);
                z_data = _mm512_sllv_epi64(z_ones, z_data);                         /* z_data = bit7 .. bit0    */
                z_dat1 = _mm512_sllv_epi64(z_ones, z_dat1);                         /* z_data = bit7 .. bit0    */
                z_dat2 = _mm512_sllv_epi64(z_ones, z_dat2);                         /* z_data = bit7 .. bit0    */
                z_dat3 = _mm512_sllv_epi64(z_ones, z_dat3);                         /* z_data = bit7 .. bit0    */
                z_rslt = _mm512_or_si512(z_rslt, z_data);                           /* z_rslt = r7 .. r0        */
                z_rsl1 = _mm512_or_si512(z_rsl1, z_dat1);                           /* z_rslt = r7 .. r0        */
                z_rsl2 = _mm512_or_si512(z_rsl2, z_dat2);                           /* z_rslt = r7 .. r0        */
                z_rsl3 = _mm512_or_si512(z_rsl3, z_dat3);                           /* z_rslt = r7 .. r0        */
            }
            if (remind) {
                msk8 = (__mmask8)_bzhi_u32(0xff, remind);
                y_data = _mm256_maskz_loadu_epi32(msk8, (const __m256i*)(src_32u_ptr + length));  /* y_data = s175 .. s0 */
                y_data = _mm256_srl_epi32(y_data, x_shft);                          /* y_data = s >> shft    */
                y_data = _mm256_and_si256(y_data, y_mask);                          /* y_data = indx7 .. indx0 */
                z_data = _mm512_cvtepu32_epi64(y_data);                             /* z_data = indx7 .. indx0  */
                z_dat1 = _mm512_xor_si512(z_data, z_slc1);
                z_dat2 = _mm512_xor_si512(z_data, z_slc2);
                z_dat3 = _mm512_xor_si512(z_data, z_slc3);
                z_data = _mm512_maskz_sllv_epi64((__mmask8)msk8, z_ones, z_data);  /* z_data = bit7 .. bit0    */
                z_dat1 = _mm512_maskz_sllv_epi64((__mmask8)msk8, z_ones, z_dat1);  /* z_data = bit7 .. bit0    */
                z_dat2 = _mm512_maskz_sllv_epi64((__mmask8)msk8, z_ones, z_dat2);  /* z_data = bit7 .. bit0    */
                z_dat3 = _mm512_maskz_sllv_epi64((__mmask8)msk8, z_ones, z_dat3);  /* z_data = bit7 .. bit0    */
                z_rslt = _mm512_or_si512(z_rslt, z_data);                          /* z_rslt = r7 .. r0        */
                z_rsl1 = _mm512_or_si512(z_rsl1, z_dat1);                          /* z_rslt = r7 .. r0        */
                z_rsl2 = _mm512_or_si512(z_rsl2, z_dat2);                          /* z_rslt = r7 .. r0        */
                z_rsl3 = _mm512_or_si512(z_rsl3, z_dat3);                          /* z_rslt = r7 .. r0        */
            }
            y_rslt = _mm512_extracti32x8_epi32(z_rslt, 1);
            y_rsl1 = _mm512_extracti32x8_epi32(z_rsl1, 1);
            y_rsl2 = _mm512_extracti32x8_epi32(z_rsl2, 1);
            y_rsl3 = _mm512_extracti32x8_epi32(z_rsl3, 1);
            y_rslt = _mm256_or_si256(y_rslt, _mm512_castsi512_si256(z_rslt));       /* y_rslt = r3 r2 r1 r0 */
            y_rsl1 = _mm256_or_si256(y_rsl1, _mm512_castsi512_si256(z_rsl1));       /* y_rslt = r3 r2 r1 r0 */
            y_rsl2 = _mm256_or_si256(y_rsl2, _mm512_castsi512_si256(z_rsl2));       /* y_rslt = r3 r2 r1 r0 */
            y_rsl3 = _mm256_or_si256(y_rsl3, _mm512_castsi512_si256(z_rsl3));       /* y_rslt = r3 r2 r1 r0 */
            x_data = _mm256_extractf128_si256(y_rslt, 1);
            x_dat1 = _mm256_extractf128_si256(y_rsl1, 1);
            x_dat2 = _mm256_extractf128_si256(y_rsl2, 1);
            x_dat3 = _mm256_extractf128_si256(y_rsl3, 1);
            x_data = _mm_or_si128(x_data, _mm256_castsi256_si128(y_rslt));          /* x_data = r1 r0 */
            x_dat1 = _mm_or_si128(x_dat1, _mm256_castsi256_si128(y_rsl1));          /* x_data = r1 r0 */
            x_dat2 = _mm_or_si128(x_dat2, _mm256_castsi256_si128(y_rsl2));          /* x_data = r1 r0 */
            x_dat3 = _mm_or_si128(x_dat3, _mm256_castsi256_si128(y_rsl3));          /* x_data = r1 r0 */
            x_data = _mm_or_si128(x_data, _mm_srli_si128(x_data, 8));               /* x_data = r0 */
            x_dat1 = _mm_or_si128(x_dat1, _mm_srli_si128(x_dat1, 8));               /* x_data = r0 */
            x_dat2 = _mm_or_si128(x_dat2, _mm_srli_si128(x_dat2, 8));               /* x_data = r0 */
            x_dat3 = _mm_or_si128(x_dat3, _mm_srli_si128(x_dat3, 8));               /* x_data = r0 */
            z_ones = _mm512_set1_epi8(1);
            msk = (__mmask64)_mm_cvtsi128_si64(x_data);
            _mm512_mask_storeu_epi8((void*)dst_ptr, msk, z_ones);
            msk = (__mmask64)_mm_cvtsi128_si64(x_dat1);
            _mm512_mask_storeu_epi8((void*)(dst_ptr + 64), msk, z_ones);
            msk = (__mmask64)_mm_cvtsi128_si64(x_dat2);
            _mm512_mask_storeu_epi8((void*)(dst_ptr + 128), msk, z_ones);
            msk = (__mmask64)_mm_cvtsi128_si64(x_dat3);
            _mm512_mask_storeu_epi8((void*)(dst_ptr + 192), msk, z_ones);
        }
    }
}

#endif // OWN_FIND_UBIQUE_H