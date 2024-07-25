/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 10/30/2018
 * Contains an implementation of the @ref ref_crc64 function
 */

#include "qpl_api_ref.h"

/**
 * @defgroup REFERENCE_CRC64 CRC-64
 * @ingroup REFERENCE_PRIVATE
 * @{
 * @brief Contains helper functions for the @ref ref_crc64
 */

/**
 * @brief CRC accumulator
 */
typedef struct {
    uint32_t h32; /**< high 32bit of registry*/
    uint64_t l64; /**< low  64bit of registry*/
} crc_reg;

// clang-format off

/**
 * @brief Auxiliary table to perform bits reversing in byte
 */
static const uint8_t bit_rev_8[0x100] = {
        0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
        0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
        0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
        0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
        0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
        0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
        0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
        0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
        0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
        0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
        0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
        0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
        0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
        0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
        0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
        0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF,
};

// clang-format on

/**
 * @brief Helper for reversing bits in byte
 * @todo
 */
static uint8_t bit_ref_8(uint8_t x) {
    return bit_rev_8[x];
}

/**
 * @brief set of helpers bits/bytes reflecting
 * @todo
 */
static uint32_t bit_ref_32(uint32_t x) {
    uint32_t y = 0U;

    y = bit_ref_8(x >> 24);
    y = (y << 8) | bit_ref_8(x >> 16);
    y = (y << 8) | bit_ref_8(x >> 8);
    y = (y << 8) | bit_ref_8(x >> 0);
    return y;
}

/**
 * @todo
 * @param x
 * @return
 */
static uint64_t bit_ref_64(uint64_t x) {
    uint64_t y = 0U;

    y = bit_ref_32(x >> 32);
    y = (y << 32) | bit_ref_32(x >> 0);
    return y;
}

/**
 * @todo
 * @param x
 * @return
 */
static uint32_t byte_ref_32(uint32_t x) {
    return ((x >> 24) | ((x >> 8) & 0x0000FF00) | ((x << 8) & 0x00FF0000) | (x << 24));
}

/**
 * @todo
 * @param x
 * @return
 */
static uint64_t byte_ref_64(uint64_t x) {
    return (((uint64_t)byte_ref_32((uint32_t)x)) << 32) | byte_ref_32((uint32_t)(x >> 32));
}

/**
 * @todo
 * @param x
 * @return
 */
static uint64_t bit_byte_swap_64(uint64_t x) {
    return byte_ref_64(bit_ref_64(x));
}

/**
 * @brief set of helpers for CRC64 processing
 * @todo
 */
static uint64_t reduce(uint64_t poly, uint32_t n) {
    uint64_t data = poly;

    for (uint32_t i = 0U; i < n; i++) {
        if (data & 0x8000000000000000ULL) {
            data = (data << 1) ^ poly;
        } else {
            data = (data << 1);
        }
    }
    return data;
}

/**
 * @todo
 * @param a
 * @param b
 * @return
 */
static uint64_t clmul_32_x_32(uint64_t a, uint64_t b) {
    uint64_t r = 0U;

    for (uint32_t i = 0U; i < 32; i++) {
        if (a & 1) { r ^= b; }
        a >>= 1;
        b <<= 1;
    }
    return r;
}

/**
 * @todo
 * @param res
 * @param a
 * @param b
 */
static void clmul_32_x_64(crc_reg* res, uint32_t a, uint64_t b) {
    uint64_t x1 = 0U;
    uint64_t x2 = 0U;
    x1          = clmul_32_x_32(a, b & 0xFFFFFFFF);
    x2          = clmul_32_x_32(a, b >> 32);
    res->h32    = (uint32_t)(x2 >> 32);
    res->l64    = x1 ^ (x2 << 32);
}

/**
 * @todo
 * @param k
 * @param crc_accum
 * @param data
 */
static void proc_dword(uint64_t k, crc_reg* crc_accum, uint32_t data) {
    uint32_t x = 0U;
    crc_reg  tmp;

    x = data ^ crc_accum->h32;
    clmul_32_x_64(&tmp, x, k);
    crc_accum->h32 = tmp.h32 ^ (uint32_t)(crc_accum->l64 >> 32);
    crc_accum->l64 = tmp.l64 ^ (crc_accum->l64 << 32);
}

/**
 * @todo
 * @param poly
 * @param h
 * @param l
 * @param n
 * @return
 */
static uint64_t final_reduce(uint64_t poly, uint64_t h, uint32_t l, uint32_t n) {
    uint64_t p = 0U;

    for (uint32_t i = 0U; i < n; i++) {
        if (h & 0x8000000000000000ULL) {
            p = poly;
        } else {
            p = 0U;
        }
        h = p ^ (h << 1) ^ (l >> 31);
        l <<= 1;
    }
    return h;
}

/** @} */

qpl_status ref_crc64(qpl_job* const qpl_job_ptr) {
    if (!qpl_job_ptr) { return QPL_STS_NULL_PTR_ERR; }
    if (!qpl_job_ptr->next_in_ptr) { return QPL_STS_NULL_PTR_ERR; }
    if (!qpl_job_ptr->available_in) { return QPL_STS_SIZE_ERR; }
    if (qpl_job_ptr->crc64_poly == 0U) { return QPL_STS_CRC64_BAD_POLYNOM; }

    qpl_status status = QPL_STS_OK;

    uint8_t* src       = (uint8_t*)qpl_job_ptr->next_in_ptr;
    uint32_t len       = qpl_job_ptr->available_in;
    uint32_t data      = 0U;
    uint64_t crc       = 0U;
    uint64_t init_crc  = 0U;
    uint64_t crc_high  = 0U;
    uint32_t crc_low   = 0U;
    uint32_t i         = 0U;
    uint64_t poly      = qpl_job_ptr->crc64_poly;
    uint8_t  big_end   = 0U;
    uint8_t  inv_crc   = 0U;
    crc_reg  crc_accum = {0};
    uint64_t poly_red  = 0U;

    if (qpl_job_ptr->flags & QPL_FLAG_CRC64_BE) { big_end = 1U; }
    if (qpl_job_ptr->flags & QPL_FLAG_CRC64_INV) { inv_crc = 1U; }

    poly_red = reduce(poly, 32);

    if (inv_crc) {
        init_crc = poly;
        init_crc |= init_crc << 1;
        init_crc |= init_crc << 2;
        init_crc |= init_crc << 4;
        init_crc |= init_crc << 8;
        init_crc |= init_crc << 16;
        init_crc |= init_crc << 32;
    }

    crc_accum.h32 = (uint32_t)(init_crc >> 32);
    crc_accum.l64 = init_crc << 32;

    if (len > 8U) {
        for (i = 0U; i < len - 8U; i += 4) {
            data = *(uint32_t*)(src + i);
            data = byte_ref_32(data);
            if (big_end) { data = bit_ref_32(data); }
            proc_dword(poly_red, &crc_accum, data);
        }
    } else {
        i = 0U;
    }

    crc_high = (((uint64_t)crc_accum.h32) << 32) | (crc_accum.l64 >> 32);
    crc_low  = (uint32_t)crc_accum.l64;

    len = (len - i);
    if (len < 4U) {
        data = 0U;
        for (uint32_t idx = 0U; idx < len; idx++) {
            data |= (*(src + i + idx)) << (idx * 8);
        }
    } else {
        data = *(uint32_t*)(src + i);
    }
    data = byte_ref_32(data);
    if (big_end) { data = bit_ref_32(data); }
    crc_high ^= ((uint64_t)data) << 32;
    if (len > 4U) {
        if (len < 8U) {
            data = 0U;
            for (uint32_t idx = 0U; idx < len - 4U; idx++) {
                data |= (*(src + i + idx + 4)) << (idx * 8);
            }
        } else {
            data = *(uint32_t*)(src + i + 4U);
        }
        data = byte_ref_32(data);
        if (big_end) { data = bit_ref_32(data); }
        crc_high ^= data;
    }

    crc = final_reduce(poly, crc_high, crc_low, 8 * len);

    crc ^= init_crc;

    if (big_end) { crc = bit_byte_swap_64(crc); }

    qpl_job_ptr->crc64 = crc;

    return status;
}
