/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_ISAL_IGZIP_HUFFMAN_H
#define QPL_SOURCES_ISAL_IGZIP_HUFFMAN_H

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "igzip_lib.h"
#include "unaligned.h"

#if __x86_64__ || __i386__ || _M_X64 || _M_IX86
#ifdef _MSC_VER
#include <intrin.h>
#define inline __inline
#else
#include <x86intrin.h>
#endif
#else
#define inline __inline
#endif //__x86_64__  || __i386__ || _M_X64 || _M_IX86

// Avoid getting GCC compiler warnings with implicit fallthrough
#if (defined(__GNUC__) || defined(__GNUG__)) && !defined(__clang__)
#define ATTRIBUTE_FALLTHROUGH __attribute__((fallthrough))
#else
#define ATTRIBUTE_FALLTHROUGH ((void)0)
#endif

/**
 * @brief Calculate the bit offset of the msb.
 * @param val 32-bit unsigned integer input
 *
 * @returns bit offset of msb starting at 1 for first bit
 */
static inline uint32_t bsr(uint32_t val) {
    uint32_t msb = 0U;
#if defined(_MSC_VER)
    unsigned long ret = 0U;
    if (val != 0U) {
        _BitScanReverse(&ret, val);
        msb = ret + 1U;
    } else
        msb = 0;
#elif defined(__LZCNT__)
    msb = 32 - __lzcnt32(val);
#elif defined(__x86_64__) || defined(__aarch64__)
    msb = (val == 0) ? 0 : 32 - __builtin_clz(val);
#else
    for (msb = 0U; val > 0U; val >>= 1U)
        msb++;
#endif
    return msb;
}

static inline uint32_t tzbytecnt(uint64_t val) {
    uint32_t cnt = 0U;

#ifdef __BMI__
    cnt = __tzcnt_u64(val);
    cnt = cnt / 8;
#elif defined(__x86_64__) || defined(__aarch64__)

    cnt = (val == 0) ? 64 : __builtin_ctzll(val);
    cnt = cnt / 8;

#else
    for (cnt = 8U; val > 0U; val <<= 8U)
        cnt -= 1U;
#endif
    return cnt;
}

static void compute_dist_code(struct isal_hufftables* hufftables, uint16_t dist, uint64_t* p_code, uint64_t* p_len) {
    assert(dist > IGZIP_DIST_TABLE_SIZE);

    dist -= 1U;
    uint32_t msb            = 0U;
    uint32_t num_extra_bits = 0U;
    uint32_t extra_bits     = 0U;
    uint32_t sym            = 0U;
    uint32_t len            = 0U;
    uint32_t code           = 0U;

    msb = bsr(dist);
    assert(msb >= 2U);
    num_extra_bits = msb - 2U;
    extra_bits     = dist & ((1 << num_extra_bits) - 1);
    dist >>= num_extra_bits;
    sym = dist + 2U * num_extra_bits;
    assert(sym < 30U);
    code    = hufftables->dcodes[sym - IGZIP_DECODE_OFFSET];
    len     = hufftables->dcodes_sizes[sym - IGZIP_DECODE_OFFSET];
    *p_code = code | (extra_bits << len);
    *p_len  = len + num_extra_bits;
}

static inline void get_dist_code(struct isal_hufftables* hufftables, uint32_t dist, uint64_t* code, uint64_t* len) {
    if (dist < 1U) dist = 0U;
    assert(dist >= 1U);
    assert(dist <= 32768U);
    if (dist <= IGZIP_DIST_TABLE_SIZE) {
        uint64_t code_len = hufftables->dist_table[dist - 1U];
        *code             = code_len >> 5;
        *len              = code_len & 0x1F;
    } else {
        compute_dist_code(hufftables, dist, code, len);
    }
}

static inline void get_len_code(struct isal_hufftables* hufftables, uint32_t length, uint64_t* code, uint64_t* len) {
    assert(length >= 3U);
    assert(length <= 258U);

    uint64_t code_len = hufftables->len_table[length - 3U];
    *code             = code_len >> 5;
    *len              = code_len & 0x1F;
}

static inline void get_lit_code(struct isal_hufftables* hufftables, uint32_t lit, uint64_t* code, uint64_t* len) {
    assert(lit <= 256U);

    *code = hufftables->lit_table[lit];
    *len  = hufftables->lit_table_sizes[lit];
}

static void compute_dist_icf_code(uint32_t dist, uint32_t* code, uint32_t* extra_bits) {
    dist -= 1U;
    uint32_t msb = bsr(dist);
    assert(msb >= 2U);
    uint32_t num_extra_bits = msb - 2U;
    *extra_bits             = dist & ((1 << num_extra_bits) - 1);
    dist >>= num_extra_bits;
    *code = dist + 2U * num_extra_bits;
    assert(*code < 30U);
}

static inline void get_dist_icf_code(uint32_t dist, uint32_t* code, uint32_t* extra_bits) {
    assert(dist >= 1U);
    assert(dist <= 32768U);
    if (dist <= 2U) {
        *code       = dist - 1U;
        *extra_bits = 0U;
    } else {
        compute_dist_icf_code(dist, code, extra_bits);
    }
}

static inline void get_len_icf_code(uint32_t length, uint32_t* code) {
    assert(length >= 3U);
    assert(length <= 258U);

    *code = length + 254U;
}

static inline void get_lit_icf_code(uint32_t lit, uint32_t* code) {
    assert(lit <= 256U);

    *code = lit;
}

/**
 * @brief Returns a hash of the first 3 bytes of input data.
 */
static inline uint32_t compute_hash(uint32_t data) {
#ifdef __SSE4_2__

    return _mm_crc32_u32(0, data);

#else
    /* Use multiplication to create a hash, 0xBDD06057 is a prime number */
    uint64_t hash = data;
    hash *= 0xB2D06057;
    hash >>= 16;
    hash *= 0xB2D06057;
    hash >>= 16;
#ifdef QPL_LIB
    return (uint8_t)hash;
#else
    return hash;
#endif

#endif /* __SSE4_2__ */
}

#define PROD1 0xFFFFE84B
#define PROD2 0xFFFF97B1
static inline uint32_t compute_hash_mad(uint32_t data) {
    int16_t data_low  = 0;
    int16_t data_high = 0;

    data_low  = data;
    data_high = data >> 16;
    data      = PROD1 * data_low + PROD2 * data_high;

    data_low  = data;
    data_high = data >> 16;
    data      = PROD1 * data_low + PROD2 * data_high;

    return data;
}

static inline uint32_t compute_long_hash(uint64_t data) {
#ifdef QPL_LIB
    return compute_hash((uint32_t)(data >> 32)) ^ compute_hash((uint32_t)data);
#else
    return compute_hash(data >> 32) ^ compute_hash(data);
#endif
}

/**
 * @brief Returns how long str1 and str2 have the same symbols.
 * @param str1: First input string.
 * @param str2: Second input string.
 * @param max_length: length of the smaller string.
 */
static inline int compare258(uint8_t* str1, uint8_t* str2, uint32_t max_length) {
    uint32_t count = 0U;
    uint64_t test  = 0U;

    if (max_length > 258U) max_length = 258U;

    uint64_t loop_length = max_length & ~0x7;

    for (; count < loop_length; count += 8U) {
        test = load_u64(str1);
        test ^= load_u64(str2);
        if (test != 0U) return count + tzbytecnt(test);
        str1 += 8U;
        str2 += 8U;
    }

    switch (max_length % 8) {

        case 7:
            if (*str1++ != *str2++) return count;
            count++;
            ATTRIBUTE_FALLTHROUGH;
        case 6:
            if (*str1++ != *str2++) return count;
            count++;
            ATTRIBUTE_FALLTHROUGH;
        case 5:
            if (*str1++ != *str2++) return count;
            count++;
            ATTRIBUTE_FALLTHROUGH;
        case 4:
            if (*str1++ != *str2++) return count;
            count++;
            ATTRIBUTE_FALLTHROUGH;
        case 3:
            if (*str1++ != *str2++) return count;
            count++;
            ATTRIBUTE_FALLTHROUGH;
        case 2:
            if (*str1++ != *str2++) return count;
            count++;
            ATTRIBUTE_FALLTHROUGH;
        case 1:
            if (*str1 != *str2) return count;
            count++;
    }

    return count;
}

/**
 * @brief Returns how long str1 and str2 have the same symbols.
 * @param str1: First input string.
 * @param str2: Second input string.
 * @param max_length: length of the smaller string.
 */
static inline int compare(uint8_t* str1, uint8_t* str2, uint32_t max_length) {
    uint32_t count       = 0U;
    uint64_t test        = 0U;
    uint64_t loop_length = max_length & ~0x7;

    for (; count < loop_length; count += 8U) {
        test = load_u64(str1);
        test ^= load_u64(str2);
        if (test != 0U) return count + tzbytecnt(test);
        str1 += 8U;
        str2 += 8U;
    }

    switch (max_length % 8) {

        case 7:
            if (*str1++ != *str2++) return count;
            count++;
            ATTRIBUTE_FALLTHROUGH;
        case 6:
            if (*str1++ != *str2++) return count;
            count++;
            ATTRIBUTE_FALLTHROUGH;
        case 5:
            if (*str1++ != *str2++) return count;
            count++;
            ATTRIBUTE_FALLTHROUGH;
        case 4:
            if (*str1++ != *str2++) return count;
            count++;
            ATTRIBUTE_FALLTHROUGH;
        case 3:
            if (*str1++ != *str2++) return count;
            count++;
            ATTRIBUTE_FALLTHROUGH;
        case 2:
            if (*str1++ != *str2++) return count;
            count++;
            ATTRIBUTE_FALLTHROUGH;
        case 1:
            if (*str1 != *str2) return count;
            count++;
    }

    return count;
}

#endif
