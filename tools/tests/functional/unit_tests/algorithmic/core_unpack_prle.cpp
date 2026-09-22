/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/
#include <algorithm>
#include <array>
#include <cstddef>

#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"
#include "random_generator.h"
#include "t_common.hpp"
#include "util.hpp"

#define QPL_TEST_STS_OK                  QPLC_STS_OK
#define QPL_TEST_STS_OUTPUT_OVERFLOW_ERR QPL_STS_OUTPUT_OVERFLOW_ERR
#define QPL_TEST_STS_DST_IS_SHORT_ERR    QPL_STS_DST_IS_SHORT_ERR
#define QPL_TEST_STS_SRC_IS_SHORT_ERR    QPL_STS_SRC_IS_SHORT_ERR

#define QPL_TEST_PARQUET_WIDTH 8U
#define QPL_TEST_MIN(a, b)     (((a) < (b)) ? (a) : (b))

#define QPL_TEST_LOW_BIT_MASK  1U
#define QPL_TEST_5_BIT_MASK    0x1FU
#define QPL_TEST_7_BIT_MASK    0x7FU
#define QPL_TEST_HIGH_BIT_MASK 0x80U

#define QPL_TEST_PRLE_COUNT(x)    (((x) & QPL_TEST_7_BIT_MASK) >> 1U)
#define QPL_TEST_VARINT_BYTE_1(x) (((x) & QPL_TEST_7_BIT_MASK) << 6U)
#define QPL_TEST_VARINT_BYTE_2(x) (((x) & QPL_TEST_7_BIT_MASK) << 13U)
#define QPL_TEST_VARINT_BYTE_3(x) (((x) & QPL_TEST_7_BIT_MASK) << 20U)
#define QPL_TEST_VARINT_BYTE_4(x) (((x) & QPL_TEST_5_BIT_MASK) << 27U)

#define QPL_TEST_LITERAL_OCTA_GROUP 1U

#define QPL_TEST_RETURN_ERROR(expression, error_code) \
    {                                                 \
        if (expression) { return (error_code); }      \
    }

#define QPL_TEST_3_BYTE_WIDTH 24U
#define QPL_TEST_BYTE_WIDTH   8U
#define QPL_TEST_WORD_WIDTH   16U

#include "dispatcher.hpp"
#include "qplc_api.h"

static inline qplc_unpack_bits_t_ptr qplc_unpack_bits(uint32_t index) {
    static const auto& table = qpl::core_sw::dispatcher::kernels_dispatcher::get_instance().get_unpack_table();

    return (qplc_unpack_bits_t_ptr)table[index];
}

qplc_unpack_prle_t_ptr qplc_unpack_prle(uint32_t index) {
    static const auto& table = qpl::core_sw::dispatcher::kernels_dispatcher::get_instance().get_unpack_prle_table();

    return (qplc_unpack_prle_t_ptr)table[index];
}

static void ref_qplc_set_8u(uint8_t value, uint8_t* p_dst, uint32_t length) {
    for (uint32_t indx = 0U; indx < length; indx++) {
        p_dst[indx] = value;
    }
}

static void ref_qplc_set_16u(uint16_t value, uint16_t* p_dst, uint32_t length) {
    for (uint32_t indx = 0U; indx < length; indx++) {
        p_dst[indx] = value;
    }
}

static void ref_qplc_set_32u(uint32_t value, uint32_t* p_dst, uint32_t length) {
    for (uint32_t indx = 0U; indx < length; indx++) {
        p_dst[indx] = value;
    }
}

static void ref_qplc_copy_8u(const uint8_t* p_src, uint8_t* p_dst, uint32_t length) {
    for (uint32_t indx = 0U; indx < length; indx++) {
        p_dst[indx] = p_src[indx];
    }
}

static void ref_qplc_copy_16u(const uint16_t* p_src, uint16_t* p_dst, uint32_t length) {
    for (uint32_t indx = 0U; indx < length; indx++) {
        p_dst[indx] = p_src[indx];
    }
}

static void ref_qplc_copy_32u(const uint32_t* p_src, uint32_t* p_dst, uint32_t length) {
    for (uint32_t indx = 0U; indx < length; indx++) {
        p_dst[indx] = p_src[indx];
    }
}

static uint32_t ownc_decode_prle_header(uint8_t** pp_src, const uint8_t* src_stop_ptr, uint32_t* format_ptr,
                                        uint32_t* count_ptr) {
    QPL_TEST_RETURN_ERROR((*pp_src >= src_stop_ptr), QPL_TEST_STS_SRC_IS_SHORT_ERR);
    uint32_t value = (uint32_t)(*(*pp_src)++);
    *format_ptr    = value & QPL_TEST_LOW_BIT_MASK;
    *count_ptr     = QPL_TEST_PRLE_COUNT(value);
    if (value & QPL_TEST_HIGH_BIT_MASK) {
        QPL_TEST_RETURN_ERROR((*pp_src >= src_stop_ptr), QPL_TEST_STS_SRC_IS_SHORT_ERR);
        value = (uint32_t)(*(*pp_src)++);
        *count_ptr |= QPL_TEST_VARINT_BYTE_1(value);
        if (value & QPL_TEST_HIGH_BIT_MASK) {
            QPL_TEST_RETURN_ERROR((*pp_src >= src_stop_ptr), QPL_TEST_STS_SRC_IS_SHORT_ERR);
            value = (uint32_t)(*(*pp_src)++);
            *count_ptr |= QPL_TEST_VARINT_BYTE_2(value);
            if (value & QPL_TEST_HIGH_BIT_MASK) {
                QPL_TEST_RETURN_ERROR((*pp_src >= src_stop_ptr), QPL_TEST_STS_SRC_IS_SHORT_ERR);
                value = (uint32_t)(*(*pp_src)++);
                *count_ptr |= QPL_TEST_VARINT_BYTE_3(value);
                if (value & QPL_TEST_HIGH_BIT_MASK) {
                    QPL_TEST_RETURN_ERROR((*pp_src >= src_stop_ptr), QPL_TEST_STS_SRC_IS_SHORT_ERR);
                    value = (uint32_t)(*(*pp_src)++);
                    *count_ptr |= QPL_TEST_VARINT_BYTE_4(value);
                }
            }
        }
    }
    QPL_TEST_RETURN_ERROR((*count_ptr > (uint32_t)INT32_MAX), QPL_TEST_STS_SRC_IS_SHORT_ERR);
    return QPL_TEST_STS_OK;
}

static uint32_t ownc_octa_part_8u(uint8_t** pp_src, const uint8_t* src_stop_ptr, uint8_t** pp_dst,
                                  const uint8_t* dst_stop_ptr) {
    const uint32_t max_count_src = (uint32_t)(src_stop_ptr - *pp_src);
    const uint32_t max_count_dst = (uint32_t)(dst_stop_ptr - *pp_dst);
    const uint32_t max_count     = std::min(max_count_src, max_count_dst);
    if (0U == max_count) {
        return 0U;
    } else {
        ref_qplc_copy_8u(*pp_src, *pp_dst, max_count);
        *pp_src += max_count;
        *pp_dst += max_count;
        return (QPL_TEST_PARQUET_WIDTH - max_count);
    }
}

static uint32_t ownc_octa_part_16u(uint8_t** pp_src, const uint8_t* src_stop_ptr, uint8_t** pp_dst,
                                   const uint8_t* dst_stop_ptr) {
    const uint32_t max_count_src = (uint32_t)(src_stop_ptr - *pp_src) / sizeof(uint16_t);
    const uint32_t max_count_dst = (uint32_t)(dst_stop_ptr - *pp_dst) / sizeof(uint16_t);
    const uint32_t max_count     = std::min(max_count_src, max_count_dst);
    if (0U == max_count) {
        return 0U;
    } else {
        ref_qplc_copy_16u((const uint16_t*)*pp_src, (uint16_t*)*pp_dst, max_count);
        *pp_src += max_count * sizeof(uint16_t);
        *pp_dst += max_count * sizeof(uint16_t);
        return (QPL_TEST_PARQUET_WIDTH - max_count);
    }
}

static uint32_t ownc_octa_part_32u(uint8_t** pp_src, const uint8_t* src_stop_ptr, uint8_t** pp_dst,
                                   const uint8_t* dst_stop_ptr) {
    const uint32_t max_count_src = (uint32_t)(src_stop_ptr - *pp_src) / sizeof(uint32_t);
    const uint32_t max_count_dst = (uint32_t)(dst_stop_ptr - *pp_dst) / sizeof(uint32_t);
    const uint32_t max_count     = std::min(max_count_src, max_count_dst);
    if (0U == max_count) {
        return 0U;
    } else {
        ref_qplc_copy_32u((const uint32_t*)*pp_src, (uint32_t*)*pp_dst, max_count);
        *pp_src += max_count * sizeof(uint32_t);
        *pp_dst += max_count * sizeof(uint32_t);
        return (QPL_TEST_PARQUET_WIDTH - max_count);
    }
}

static uint32_t ref_qplc_unpack_prle_8u(uint8_t** pp_src, uint32_t src_length, uint32_t bit_width, uint8_t** pp_dst,
                                        uint32_t dst_length, int32_t* count_ptr, uint32_t* value_ptr) {
    uint32_t count         = 0U;
    uint32_t format        = 0U;
    uint8_t  value         = 0U;
    uint32_t max_count     = 0U;
    uint32_t max_count_src = 0U;
    uint32_t max_count_dst = 0U;
    uint32_t src_step      = 0U;
    uint32_t dst_step      = 0U;
    uint8_t* kept_src_ptr  = nullptr;
    uint8_t* dst_ptr       = (*pp_dst);
    uint8_t* src_ptr       = (*pp_src);
    uint8_t* src_stop_ptr  = src_ptr + src_length;
    uint8_t* dst_stop_ptr  = dst_ptr + dst_length;
    uint32_t status        = QPL_TEST_STS_OK;

    if (0 < *count_ptr) {
        count      = std::min((uint32_t)*count_ptr, dst_length);
        *count_ptr = *count_ptr - (int32_t)count;
        value      = (uint8_t)*value_ptr;
        ref_qplc_set_8u(value, dst_ptr, count);
        dst_ptr += count;
        status = (0 != *count_ptr) ? QPL_TEST_STS_DST_IS_SHORT_ERR : status;
    }
    if (0 > *count_ptr) {
        max_count     = (uint32_t)(-*count_ptr);
        max_count_dst = dst_length / QPL_TEST_PARQUET_WIDTH;
        max_count_src = src_length / bit_width;
        count         = QPL_TEST_MIN(max_count_src, max_count_dst);
        status        = (count == max_count_src) ? QPL_TEST_STS_SRC_IS_SHORT_ERR : QPL_TEST_STS_DST_IS_SHORT_ERR;
        count         = QPL_TEST_MIN(max_count, count);
        *count_ptr    = *count_ptr + (int32_t)count;
        qplc_unpack_bits(bit_width - 1U)(src_ptr, count * QPL_TEST_PARQUET_WIDTH, 0U, dst_ptr);
        dst_ptr += count * QPL_TEST_PARQUET_WIDTH;
        src_ptr += count * bit_width;
        if (0 != *count_ptr) {
            if (8U == bit_width) {
                *value_ptr = ownc_octa_part_8u(&src_ptr, src_stop_ptr, &dst_ptr, dst_stop_ptr);
            } else {
                const uint32_t max_count_lit_src =
                        (uint32_t)(src_stop_ptr - src_ptr) / bit_width * QPL_TEST_PARQUET_WIDTH;
                const uint32_t max_count_lit_dst = (uint32_t)(dst_stop_ptr - dst_ptr);
                const uint32_t max_count_lit     = QPL_TEST_MIN(max_count_lit_src, max_count_lit_dst);
                qplc_unpack_bits(bit_width - 1U)(src_ptr, max_count_lit, 0U, dst_ptr);
                src_ptr += max_count_lit * (bit_width / QPL_TEST_PARQUET_WIDTH);
                dst_ptr += max_count_lit;
            }
            *pp_dst = dst_ptr;
            *pp_src = src_ptr;
            return status;
        }
        status = QPL_TEST_STS_OK;
    }
    while ((src_ptr < src_stop_ptr) && (dst_ptr < dst_stop_ptr)) {
        kept_src_ptr = src_ptr;
        // Extract format and counter
        status = ownc_decode_prle_header(&src_ptr, src_stop_ptr, &format, &count);
        if (status != QPL_TEST_STS_OK) {
            src_ptr = kept_src_ptr;
            break;
        }
        if (QPL_TEST_LITERAL_OCTA_GROUP == format) {
            // This is a set of qplc_packed bit_width-integers (octa-groups)
            src_step = count * bit_width;
            dst_step = count * QPL_TEST_PARQUET_WIDTH;
            if (((src_ptr + src_step) > src_stop_ptr) || ((dst_ptr + dst_step) > dst_stop_ptr)) {
                max_count_src = (uint32_t)(src_stop_ptr - src_ptr) / bit_width;
                max_count_dst = (uint32_t)((dst_stop_ptr - dst_ptr) / QPL_TEST_PARQUET_WIDTH);
                max_count     = std::min(max_count_src, max_count_dst);
                status = (max_count == max_count_src) ? QPL_TEST_STS_SRC_IS_SHORT_ERR : QPL_TEST_STS_DST_IS_SHORT_ERR;
                *count_ptr = -(int32_t)(count - max_count);
                count      = max_count;
                qplc_unpack_bits(bit_width - 1U)(src_ptr, count * QPL_TEST_PARQUET_WIDTH, 0U, dst_ptr);
                src_ptr += count * bit_width;
                dst_ptr += count * QPL_TEST_PARQUET_WIDTH;
                break;
            }

            if (count > 0U) { qplc_unpack_bits(bit_width - 1U)(src_ptr, count * QPL_TEST_PARQUET_WIDTH, 0U, dst_ptr); }

            src_ptr += src_step;
            dst_ptr += dst_step;
        } else {
            // This is a set of RLE-qplc_packed bit_width-integers
            if ((src_ptr + sizeof(uint8_t)) > src_stop_ptr) {
                status  = QPL_TEST_STS_SRC_IS_SHORT_ERR;
                src_ptr = kept_src_ptr;
                break;
            }
            value = *src_ptr++;
            if ((dst_ptr + count) > dst_stop_ptr) {
                max_count  = (uint32_t)(dst_stop_ptr - dst_ptr);
                *count_ptr = (int32_t)(count - max_count);
                count      = max_count;
                *value_ptr = value;
                status     = QPL_TEST_STS_DST_IS_SHORT_ERR;
            }
            ref_qplc_set_8u(value, dst_ptr, count);
            dst_ptr += count;
        }
    }
    *pp_dst = dst_ptr;
    *pp_src = src_ptr;
    return status;
}

static uint32_t ref_qplc_unpack_prle_16u(uint8_t** pp_src, uint32_t src_length, uint32_t bit_width, uint8_t** pp_dst,
                                         uint32_t dst_length, int32_t* count_ptr, uint32_t* value_ptr) {
    uint32_t count         = 0U;
    uint32_t format        = 0U;
    uint16_t value         = 0U;
    uint32_t max_count     = 0U;
    uint32_t max_count_src = 0U;
    uint32_t max_count_dst = 0U;
    uint8_t* dst_ptr       = (*pp_dst);
    uint8_t* src_ptr       = (*pp_src);
    uint8_t* src_stop_ptr  = src_ptr + src_length;
    // dst_length is length in unpacked elements;
    dst_length *= sizeof(uint16_t);
    uint8_t* dst_stop_ptr = dst_ptr + dst_length;
    uint8_t* kept_src_ptr = nullptr;
    uint32_t src_step     = 0U;
    uint32_t dst_step     = 0U;
    uint32_t status       = QPL_TEST_STS_OK;

    if (0 < *count_ptr) {
        count      = std::min(static_cast<uint32_t>(*count_ptr), static_cast<uint32_t>(dst_length / sizeof(uint16_t)));
        *count_ptr = *count_ptr - (int32_t)count;
        value      = (uint16_t)*value_ptr;
        ref_qplc_set_16u(value, (uint16_t*)dst_ptr, count);
        dst_ptr += count * sizeof(uint16_t);
        status = (0 != *count_ptr) ? QPL_TEST_STS_DST_IS_SHORT_ERR : status;
    }
    if (0 > *count_ptr) {
        max_count     = (uint32_t)(-*count_ptr);
        max_count_dst = dst_length / (QPL_TEST_PARQUET_WIDTH * sizeof(uint16_t));
        max_count_src = src_length / bit_width;
        count         = QPL_TEST_MIN(max_count_src, max_count_dst);
        status        = (count == max_count_src) ? QPL_TEST_STS_SRC_IS_SHORT_ERR : QPL_TEST_STS_DST_IS_SHORT_ERR;
        count         = QPL_TEST_MIN(max_count, count);
        *count_ptr    = *count_ptr + (int32_t)count;
        qplc_unpack_bits(bit_width - 1U)(src_ptr, count * QPL_TEST_PARQUET_WIDTH, 0U, dst_ptr);
        dst_ptr += count * QPL_TEST_PARQUET_WIDTH * sizeof(uint16_t);
        src_ptr += count * bit_width;
        if (0 != *count_ptr) {
            if (16U == bit_width) {
                *value_ptr = ownc_octa_part_16u(&src_ptr, src_stop_ptr, &dst_ptr, dst_stop_ptr);
            } else {
                const uint32_t max_count_lit_src =
                        (uint32_t)(src_stop_ptr - src_ptr) / bit_width * QPL_TEST_PARQUET_WIDTH;
                const uint32_t max_count_lit_dst = (uint32_t)(dst_stop_ptr - dst_ptr) / sizeof(uint16_t);
                const uint32_t max_count_lit     = QPL_TEST_MIN(max_count_lit_src, max_count_lit_dst);
                qplc_unpack_bits(bit_width - 1U)(src_ptr, max_count_lit, 0U, dst_ptr);
                src_ptr += max_count_lit * (bit_width / QPL_TEST_PARQUET_WIDTH);
                dst_ptr += max_count_lit * sizeof(uint16_t);
            }
            *pp_dst = dst_ptr;
            *pp_src = src_ptr;
            return status;
        }
        status = QPL_TEST_STS_OK;
    }
    while ((src_ptr < src_stop_ptr) && (dst_ptr < dst_stop_ptr)) {
        kept_src_ptr = src_ptr;
        // Extract format and counter
        status = ownc_decode_prle_header(&src_ptr, src_stop_ptr, &format, &count);
        if (status != QPL_TEST_STS_OK) {
            src_ptr = kept_src_ptr;
            break;
        }
        if (QPL_TEST_LITERAL_OCTA_GROUP == format) {
            // This is a set of qplc_packed bit_width-integers (octa-groups)
            src_step = count * bit_width;
            dst_step = count * QPL_TEST_PARQUET_WIDTH * sizeof(uint16_t);
            if (((src_ptr + src_step) > src_stop_ptr) || ((dst_ptr + dst_step) > dst_stop_ptr)) {
                max_count_src = (uint32_t)(src_stop_ptr - src_ptr) / bit_width;
                max_count_dst = (uint32_t)((dst_stop_ptr - dst_ptr) / (QPL_TEST_PARQUET_WIDTH * sizeof(uint16_t)));
                max_count     = std::min(max_count_src, max_count_dst);
                status = (max_count == max_count_src) ? QPL_TEST_STS_SRC_IS_SHORT_ERR : QPL_TEST_STS_DST_IS_SHORT_ERR;
                *count_ptr = -(int32_t)(count - max_count);
                count      = max_count;
                qplc_unpack_bits(bit_width - 1U)(src_ptr, count * QPL_TEST_PARQUET_WIDTH, 0U, dst_ptr);
                src_ptr += count * bit_width;
                dst_ptr += count * QPL_TEST_PARQUET_WIDTH * sizeof(uint16_t);
                break;
            }
            qplc_unpack_bits(bit_width - 1U)(src_ptr, count * QPL_TEST_PARQUET_WIDTH, 0U, dst_ptr);
            src_ptr += src_step;
            dst_ptr += dst_step;
        } else {
            // This is a set of RLE-qplc_packed bit_width-integers
            if ((src_ptr + sizeof(uint16_t)) > src_stop_ptr) {
                status  = QPL_TEST_STS_SRC_IS_SHORT_ERR;
                src_ptr = kept_src_ptr;
                break;
            }
            value = *(uint16_t*)src_ptr;
            src_ptr += sizeof(uint16_t);
            if ((dst_ptr + count * sizeof(uint16_t)) > dst_stop_ptr) {
                max_count  = (uint32_t)((dst_stop_ptr - dst_ptr) / sizeof(uint16_t));
                *count_ptr = (int32_t)(count - max_count);
                count      = max_count;
                *value_ptr = value;
                status     = QPL_TEST_STS_DST_IS_SHORT_ERR;
            }
            ref_qplc_set_16u(value, (uint16_t*)dst_ptr, count);
            dst_ptr += count * sizeof(uint16_t);
        }
    }
    *pp_dst = dst_ptr;
    *pp_src = src_ptr;
    return status;
}

static uint32_t ref_qplc_unpack_prle_32u(uint8_t** pp_src, uint32_t src_length, uint32_t bit_width, uint8_t** pp_dst,
                                         uint32_t dst_length, int32_t* count_ptr, uint32_t* value_ptr) {
    uint32_t count         = 0U;
    uint32_t format        = 0U;
    uint32_t value         = 0U;
    uint32_t max_count     = 0U;
    uint32_t max_count_src = 0U;
    uint32_t max_count_dst = 0U;
    uint8_t* dst_ptr       = (*pp_dst);
    uint8_t* src_ptr       = (*pp_src);
    uint8_t* src_stop_ptr  = src_ptr + src_length;
    // dst_length is length in unpacked elements;
    dst_length *= sizeof(uint32_t);
    uint8_t* dst_stop_ptr = dst_ptr + dst_length;
    uint8_t* kept_src_ptr = nullptr;
    uint32_t src_step     = 0U;
    uint32_t dst_step     = 0U;
    uint32_t status       = QPL_TEST_STS_OK;
    // Using a fixed-width of round-up-to-next-byte(bit-width) - value may take 3 or 4 bytes
    const uint32_t value_width = (QPL_TEST_3_BYTE_WIDTH < bit_width) ? 4U : 3U;
    const uint32_t value_mask  = (QPL_TEST_3_BYTE_WIDTH < bit_width) ? UINT32_MAX : UINT32_MAX >> QPL_TEST_BYTE_WIDTH;

    if (0 < *count_ptr) {
        count      = std::min(static_cast<uint32_t>(*count_ptr), static_cast<uint32_t>(dst_length / sizeof(uint32_t)));
        *count_ptr = *count_ptr - (int32_t)count;
        value      = (*value_ptr);
        ref_qplc_set_32u(value, (uint32_t*)dst_ptr, count);
        dst_ptr += count * sizeof(uint32_t);
        status = (0 != *count_ptr) ? QPL_TEST_STS_DST_IS_SHORT_ERR : status;
    }
    if (0 > *count_ptr) {
        max_count     = (uint32_t)(-*count_ptr);
        max_count_dst = dst_length / (QPL_TEST_PARQUET_WIDTH * sizeof(uint32_t));
        max_count_src = src_length / bit_width;
        count         = QPL_TEST_MIN(max_count_src, max_count_dst);
        status        = (count == max_count_src) ? QPL_TEST_STS_SRC_IS_SHORT_ERR : QPL_TEST_STS_DST_IS_SHORT_ERR;
        count         = QPL_TEST_MIN(max_count, count);
        *count_ptr    = *count_ptr + (int32_t)count;
        qplc_unpack_bits(bit_width - 1U)(src_ptr, count * QPL_TEST_PARQUET_WIDTH, 0U, dst_ptr);
        dst_ptr += count * QPL_TEST_PARQUET_WIDTH * sizeof(uint32_t);
        src_ptr += count * bit_width;
        if (0 != *count_ptr) {
            if (32U == bit_width) {
                *value_ptr = ownc_octa_part_32u(&src_ptr, src_stop_ptr, &dst_ptr, dst_stop_ptr);
            } else {
                const uint32_t max_count_lit_src =
                        (uint32_t)(src_stop_ptr - src_ptr) / bit_width * QPL_TEST_PARQUET_WIDTH;
                const uint32_t max_count_lit_dst = (uint32_t)(dst_stop_ptr - dst_ptr) / sizeof(uint32_t);
                const uint32_t max_count_lit     = QPL_TEST_MIN(max_count_lit_src, max_count_lit_dst);
                qplc_unpack_bits(bit_width - 1U)(src_ptr, max_count_lit, 0U, dst_ptr);
                src_ptr += max_count_lit * (bit_width / QPL_TEST_PARQUET_WIDTH);
                dst_ptr += max_count_lit * sizeof(uint32_t);
            }
            *pp_dst = dst_ptr;
            *pp_src = src_ptr;
            return status;
        }
        status = QPL_TEST_STS_OK;
    }
    while ((src_ptr < src_stop_ptr) && (dst_ptr < dst_stop_ptr)) {
        kept_src_ptr = src_ptr;
        // Extract format and counter
        status = ownc_decode_prle_header(&src_ptr, src_stop_ptr, &format, &count);
        if (status != QPL_TEST_STS_OK) {
            src_ptr = kept_src_ptr;
            break;
        }
        if (QPL_TEST_LITERAL_OCTA_GROUP == format) {
            // This is a set of qplc_packed bit_width-integers (octa-groups)
            src_step = count * bit_width;
            dst_step = count * QPL_TEST_PARQUET_WIDTH * sizeof(uint32_t);
            if (((src_ptr + src_step) > src_stop_ptr) || ((dst_ptr + dst_step) > dst_stop_ptr)) {
                max_count_src = (uint32_t)(src_stop_ptr - src_ptr) / bit_width;
                max_count_dst = (uint32_t)((dst_stop_ptr - dst_ptr) / (QPL_TEST_PARQUET_WIDTH * sizeof(uint32_t)));
                max_count     = std::min(max_count_src, max_count_dst);
                status = (max_count == max_count_src) ? QPL_TEST_STS_SRC_IS_SHORT_ERR : QPL_TEST_STS_DST_IS_SHORT_ERR;
                *count_ptr = -(int32_t)(count - max_count);
                count      = max_count;
                qplc_unpack_bits(bit_width - 1U)(src_ptr, count * QPL_TEST_PARQUET_WIDTH, 0U, dst_ptr);
                src_ptr += count * bit_width;
                dst_ptr += count * QPL_TEST_PARQUET_WIDTH * sizeof(uint32_t);
                break;
            }
            qplc_unpack_bits(bit_width - 1U)(src_ptr, count * QPL_TEST_PARQUET_WIDTH, 0U, dst_ptr);
            src_ptr += src_step;
            dst_ptr += dst_step;
        } else {
            // This is a set of RLE-qplc_packed bit_width-integers
            if ((src_ptr + value_width) > src_stop_ptr) {
                status  = QPL_TEST_STS_SRC_IS_SHORT_ERR;
                src_ptr = kept_src_ptr;
                break;
            }
            if ((src_ptr + 3U) < src_stop_ptr) {
                value = (*(uint32_t*)src_ptr) & value_mask;
            } else {
                value = *(uint16_t*)src_ptr;
                value |= (*(src_ptr + 2U)) << QPL_TEST_WORD_WIDTH;
                value &= value_mask;
            }

            src_ptr += value_width;
            if ((dst_ptr + count * sizeof(uint32_t)) > dst_stop_ptr) {
                max_count  = (uint32_t)((dst_stop_ptr - dst_ptr) / sizeof(uint32_t));
                *count_ptr = (int32_t)(count - max_count);
                count      = max_count;
                *value_ptr = value;
                status     = QPL_TEST_STS_DST_IS_SHORT_ERR;
            }
            ref_qplc_set_32u(value, (uint32_t*)dst_ptr, count);
            dst_ptr += count * sizeof(uint32_t);
        }
    }
    *pp_dst = dst_ptr;
    *pp_src = src_ptr;
    return status;
}

constexpr uint32_t fun_indx_unpack_prle_8u  = 0U;
constexpr uint32_t fun_indx_unpack_prle_16u = 1U;
constexpr uint32_t fun_indx_unpack_prle_32u = 2U;

constexpr uint32_t TEST_BUFFER_SIZE = 128U;

namespace qpl::test {
using randomizer = qpl::test::random;

/*
 * Header count limits.
 *
 * A Parquet header is a 32-bit value (count << 1 | format), so the kernels
 * reject a decoded count above INT32_MAX as a short source (as hardware does). That also
 * keeps every remainder within the int32_t resume state (*count_ptr): positive
 * for RLE values left, negative for literal octa-groups left.
 *
 * These cases need only small stack buffers: the large counts live in the 5-byte
 * header and the kernel stops at the small destination.
 */
constexpr uint32_t PRLE_HEADER_SIZE = 5U;
constexpr uint32_t PRLE_RLE_FORMAT  = 0U;
constexpr uint32_t PRLE_MAX_COUNT   = (uint32_t)INT32_MAX;
constexpr uint32_t PRLE_DST_LENGTH  = 64U;

// Encodes a PRLE header of `format` (literal octa-group or RLE) for `count`,
// inverting the layout that QPL_TEST_PRLE_COUNT / QPL_TEST_VARINT_BYTE_1..4
// decode. Always emits the full 5 bytes so the encoded length does not depend on
// `count`.
static void encode_prle_header(uint32_t format, uint32_t count, uint8_t* dst_ptr) {
    dst_ptr[0] = (uint8_t)(format | ((count & 0x3FU) << 1U));
    dst_ptr[1] = (uint8_t)((count >> 6U) & QPL_TEST_7_BIT_MASK);
    dst_ptr[2] = (uint8_t)((count >> 13U) & QPL_TEST_7_BIT_MASK);
    dst_ptr[3] = (uint8_t)((count >> 20U) & QPL_TEST_7_BIT_MASK);
    dst_ptr[4] = (uint8_t)((count >> 27U) & QPL_TEST_5_BIT_MASK);
    for (uint32_t i = 0U; i < PRLE_HEADER_SIZE - 1U; i++) {
        dst_ptr[i] |= (uint8_t)QPL_TEST_HIGH_BIT_MASK;
    }
}

// Number of source bytes holding one RLE value: 32u kernels use 3 bytes for bit
// widths up to 24, else 4; 8u and 16u kernels use the element size.
static uint32_t rle_value_bytes(uint32_t bit_width, uint32_t element_size) {
    if (sizeof(uint32_t) != element_size) { return element_size; }
    return (24U < bit_width) ? 4U : 3U;
}

// Writes `value` as an RLE value at `dst_ptr`; returns the bytes written.
static uint32_t encode_rle_value(uint32_t value, uint32_t bit_width, uint32_t element_size, uint8_t* dst_ptr) {
    const uint32_t value_bytes = rle_value_bytes(bit_width, element_size);
    for (uint32_t i = 0U; i < value_bytes; i++) {
        dst_ptr[i] = (uint8_t)(value >> (8U * i));
    }
    return value_bytes;
}

static uint32_t read_element(const uint8_t* buffer, uint32_t index, uint32_t element_size) {
    switch (element_size) {
        case sizeof(uint8_t): return buffer[index];
        case sizeof(uint16_t): return ((const uint16_t*)buffer)[index];
        default: return ((const uint32_t*)buffer)[index];
    }
}

/*
 * Literal octa-groups at the INT32_MAX limit are accepted, and the resume state
 * holds the exact negated number of groups left.
 */
static testing::AssertionResult check_literal_max_count(uint32_t fun_index, uint32_t bit_width, uint32_t element_size) {
    constexpr uint32_t src_length = 40U;

    std::array<uint8_t, 64U>                                source {};
    std::array<uint8_t, PRLE_DST_LENGTH * sizeof(uint32_t)> destination {};
    uint32_t                                                value_state = 0U;
    int32_t                                                 count_state = 0;
    uint8_t*                                                p_src       = source.data();
    uint8_t*                                                p_dst       = destination.data();

    encode_prle_header(QPL_TEST_LITERAL_OCTA_GROUP, PRLE_MAX_COUNT, source.data());
    const uint32_t status = qplc_unpack_prle(fun_index)(&p_src, src_length, bit_width, &p_dst, PRLE_DST_LENGTH,
                                                        &count_state, &value_state);

    if (QPL_TEST_STS_SRC_IS_SHORT_ERR != status && QPL_TEST_STS_DST_IS_SHORT_ERR != status) {
        return testing::AssertionFailure() << "literal: expected a short src/dst status, got " << status;
    }
    // Each decoded octa-group advances the source by bit_width bytes past the header.
    const ptrdiff_t src_consumed = p_src - source.data() - (ptrdiff_t)PRLE_HEADER_SIZE;
    if (src_consumed < 0 || 0 != src_consumed % bit_width) {
        return testing::AssertionFailure() << "literal: source advanced " << src_consumed << " bytes past the header";
    }
    const int32_t expected_state = -(int32_t)(PRLE_MAX_COUNT - (uint32_t)(src_consumed / bit_width));
    if (expected_state != count_state) {
        return testing::AssertionFailure()
               << "literal: *count_ptr = " << count_state << ", expected " << expected_state;
    }
    const ptrdiff_t written = p_dst - destination.data();
    if (written < 0 || (size_t)written > (size_t)PRLE_DST_LENGTH * element_size) {
        return testing::AssertionFailure() << "literal: destination advanced " << written << " bytes, capacity is "
                                           << (size_t)PRLE_DST_LENGTH * element_size;
    }
    return testing::AssertionSuccess();
}

/*
 * An RLE run at the INT32_MAX limit is accepted: the resume state holds the exact
 * number of values left, and a resumed call keeps repeating the same value
 * without reading the source.
 */
static testing::AssertionResult check_rle_max_count(uint32_t fun_index, uint32_t bit_width, uint32_t element_size) {
    const uint32_t rle_value = (uint32_t)((1ULL << bit_width) - 1U);

    // Bytes after the value would decode as non-zero literals if misread.
    std::array<uint8_t, 64U> source {};
    source.fill(0x5AU);
    encode_prle_header(PRLE_RLE_FORMAT, PRLE_MAX_COUNT, source.data());
    encode_rle_value(rle_value, bit_width, element_size, source.data() + PRLE_HEADER_SIZE);

    std::array<uint8_t, PRLE_DST_LENGTH * sizeof(uint32_t)> destination {};
    uint32_t                                                value_state = 0U;
    int32_t                                                 count_state = 0;
    uint8_t*                                                p_src       = source.data();

    for (uint32_t call = 1U; call <= 2U; call++) {
        destination.fill(0U);
        uint8_t*       p_dst         = destination.data();
        uint8_t* const call_src      = p_src;
        const auto     call_src_size = (uint32_t)(source.size() - (size_t)(p_src - source.data()));
        const uint32_t status = qplc_unpack_prle(fun_index)(&p_src, call_src_size, bit_width, &p_dst, PRLE_DST_LENGTH,
                                                            &count_state, &value_state);

        if (QPL_TEST_STS_DST_IS_SHORT_ERR != status) {
            return testing::AssertionFailure() << "rle call " << call << ": expected dst-short status, got " << status;
        }
        const auto expected_state = (int32_t)(PRLE_MAX_COUNT - call * PRLE_DST_LENGTH);
        if (expected_state != count_state) {
            return testing::AssertionFailure()
                   << "rle call " << call << ": *count_ptr = " << count_state << ", expected " << expected_state;
        }
        if ((size_t)(p_dst - destination.data()) != (size_t)PRLE_DST_LENGTH * element_size) {
            return testing::AssertionFailure()
                   << "rle call " << call << ": destination advanced " << (p_dst - destination.data()) << " bytes";
        }
        if (2U == call && p_src != call_src) {
            return testing::AssertionFailure() << "rle: resumed call consumed " << (p_src - call_src) << " bytes";
        }
        for (uint32_t i = 0U; i < PRLE_DST_LENGTH; i++) {
            const uint32_t element = read_element(destination.data(), i, element_size);
            if (rle_value != element) {
                return testing::AssertionFailure()
                       << "rle call " << call << ": element " << i << " = " << element << ", expected " << rle_value;
            }
        }
    }
    return testing::AssertionSuccess();
}

/*
 * A count above INT32_MAX, in either format, is rejected as a short source even
 * though source bytes remain. Output decoded before the bad header is kept, the
 * source stops at the bad header, and the resume state stays 0. The status alone
 * cannot tell rejection from an unsatisfiable literal count, so the source
 * position and resume state are what show the header was not consumed.
 */
static testing::AssertionResult check_count_above_max_rejected(uint32_t fun_index, uint32_t bit_width,
                                                               uint32_t element_size, uint32_t format) {
    constexpr uint32_t good_count = 3U;
    const uint32_t     rle_value  = (uint32_t)((1ULL << bit_width) - 1U);

    // A valid 3-element RLE run (1-byte header), then the bad header.
    std::array<uint8_t, 64U> source {};
    source.fill(0x5AU);
    source[0]              = (uint8_t)((good_count << 1U) | PRLE_RLE_FORMAT);
    const uint32_t bad_pos = 1U + encode_rle_value(rle_value, bit_width, element_size, source.data() + 1U);
    encode_prle_header(format, PRLE_MAX_COUNT + 1U, source.data() + bad_pos);

    std::array<uint8_t, PRLE_DST_LENGTH * sizeof(uint32_t)> destination {};
    uint32_t                                                value_state = 0U;
    int32_t                                                 count_state = 0;
    uint8_t*                                                p_src       = source.data();
    uint8_t*                                                p_dst       = destination.data();

    const uint32_t status = qplc_unpack_prle(fun_index)(&p_src, (uint32_t)source.size(), bit_width, &p_dst,
                                                        PRLE_DST_LENGTH, &count_state, &value_state);

    if (QPL_TEST_STS_SRC_IS_SHORT_ERR != status) {
        return testing::AssertionFailure() << "format " << format << ": expected src-short status, got " << status;
    }
    if (p_src != source.data() + bad_pos) {
        return testing::AssertionFailure() << "format " << format << ": source stopped at offset "
                                           << (p_src - source.data()) << ", expected " << bad_pos;
    }
    if (0 != count_state) {
        return testing::AssertionFailure() << "format " << format << ": *count_ptr = " << count_state;
    }
    if ((size_t)(p_dst - destination.data()) != (size_t)good_count * element_size) {
        return testing::AssertionFailure()
               << "format " << format << ": destination advanced " << (p_dst - destination.data()) << " bytes";
    }
    for (uint32_t i = 0U; i < good_count; i++) {
        if (rle_value != read_element(destination.data(), i, element_size)) {
            return testing::AssertionFailure() << "format " << format << ": element " << i << " was not kept";
        }
    }
    return testing::AssertionSuccess();
}

static void check_header_count_limits(uint32_t fun_index, uint32_t max_bit_width, uint32_t element_size) {
    for (uint32_t bit_width = 1U; bit_width <= max_bit_width; bit_width++) {
        SCOPED_TRACE(testing::Message() << "bit_width " << bit_width);
        EXPECT_TRUE(check_literal_max_count(fun_index, bit_width, element_size));
        EXPECT_TRUE(check_rle_max_count(fun_index, bit_width, element_size));
        EXPECT_TRUE(check_count_above_max_rejected(fun_index, bit_width, element_size, QPL_TEST_LITERAL_OCTA_GROUP));
        EXPECT_TRUE(check_count_above_max_rejected(fun_index, bit_width, element_size, PRLE_RLE_FORMAT));
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_unpack_prle_8u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)>  source {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> value {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> ref_value {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)>  destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)>  reference {};
    const uint64_t                                           seed = util::TestEnvironment::GetInstance().GetSeed();
    const randomizer                                         random_value(0U, static_cast<double>(UINT8_MAX), seed);

    {
        uint8_t*  p_source_8u     = (uint8_t*)source.data();
        uint32_t* p_value_32u     = (uint32_t*)value.data();
        uint32_t* p_ref_value_32u = (uint32_t*)ref_value.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_8u[indx]     = 1U;
            p_value_32u[indx]     = 1U;
            p_ref_value_32u[indx] = 1U;
        }
    }
    for (uint32_t bit_width = 7U; bit_width <= 8U; bit_width++) {
        for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
            for (uint32_t length_dst = 1U; length_dst <= TEST_BUFFER_SIZE; length_dst++) {
                for (int32_t count = -1; count <= (int32_t)TEST_BUFFER_SIZE; count++) {
                    destination.fill(0);
                    reference.fill(0);
                    uint8_t*       p_src_8u     = source.data();
                    uint8_t*       p_ref_src_8u = source.data();
                    uint8_t*       p_dst_8u     = destination.data();
                    uint8_t*       p_ref_dst_8u = reference.data();
                    uint32_t*      p_value      = (uint32_t*)value.data();
                    uint32_t*      p_ref_value  = (uint32_t*)ref_value.data();
                    const uint32_t index        = 0U;
                    const uint32_t ref_index    = 0U;

                    int32_t        count_current     = count;
                    int32_t        ref_count_current = count;
                    const uint32_t status            = qplc_unpack_prle(fun_indx_unpack_prle_8u)(
                            &p_src_8u, length, bit_width, &p_dst_8u, length_dst, &count_current, p_value);
                    const uint32_t ref_status = ref_qplc_unpack_prle_8u(&p_ref_src_8u, length, bit_width, &p_ref_dst_8u,
                                                                        length_dst, &ref_count_current, p_ref_value);

                    ASSERT_EQ(status, ref_status);
                    ASSERT_EQ(p_src_8u, p_ref_src_8u);
                    ASSERT_EQ(count_current, ref_count_current);
                    ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
                    ASSERT_EQ(((uint8_t*)p_value - (uint8_t*)value.data()),
                              ((uint8_t*)p_ref_value - (uint8_t*)ref_value.data()));
                    ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(),
                                                destination.end(), "FAIL qplc_unpack_prle_8u!!! "));
                }
            }
        }
    }

    {
        uint8_t*  p_source_8u     = (uint8_t*)source.data();
        uint32_t* p_value_32u     = (uint32_t*)value.data();
        uint32_t* p_ref_value_32u = (uint32_t*)ref_value.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_8u[indx]     = 0x8FU;
            p_value_32u[indx]     = 0x8FU;
            p_ref_value_32u[indx] = 0x8FU;
        }
    }
    for (uint32_t bit_width = 7U; bit_width <= 8U; bit_width++) {
        for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
            for (uint32_t length_dst = 1U; length_dst <= TEST_BUFFER_SIZE; length_dst++) {
                for (int32_t count = -1; count <= (int32_t)TEST_BUFFER_SIZE; count++) {
                    destination.fill(0);
                    reference.fill(0);
                    uint8_t*       p_src_8u     = source.data();
                    uint8_t*       p_ref_src_8u = source.data();
                    uint8_t*       p_dst_8u     = destination.data();
                    uint8_t*       p_ref_dst_8u = reference.data();
                    uint32_t*      p_value      = (uint32_t*)value.data();
                    uint32_t*      p_ref_value  = (uint32_t*)ref_value.data();
                    const uint32_t index        = 0U;
                    const uint32_t ref_index    = 0U;

                    int32_t        count_current     = count;
                    int32_t        ref_count_current = count;
                    const uint32_t status            = qplc_unpack_prle(fun_indx_unpack_prle_8u)(
                            &p_src_8u, length, bit_width, &p_dst_8u, length_dst, &count_current, p_value);
                    const uint32_t ref_status = ref_qplc_unpack_prle_8u(&p_ref_src_8u, length, bit_width, &p_ref_dst_8u,
                                                                        length_dst, &ref_count_current, p_ref_value);

                    ASSERT_EQ(status, ref_status);
                    ASSERT_EQ(p_src_8u, p_ref_src_8u);
                    ASSERT_EQ(count_current, ref_count_current);
                    ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
                    ASSERT_EQ(((uint8_t*)p_value - (uint8_t*)value.data()),
                              ((uint8_t*)p_ref_value - (uint8_t*)ref_value.data()));
                    ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(),
                                                destination.end(), "FAIL qplc_unpack_prle_8u!!! "));
                }
            }
        }
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_unpack_prle_16u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> source {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> value {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> ref_value {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> reference {};
    const uint64_t                                           seed = util::TestEnvironment::GetInstance().GetSeed();
    const randomizer                                         random_value(0U, static_cast<double>(UINT16_MAX), seed);

    {
        uint16_t* p_source_16u    = (uint16_t*)source.data();
        uint32_t* p_value_32u     = (uint32_t*)value.data();
        uint32_t* p_ref_value_32u = (uint32_t*)ref_value.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_16u[indx]    = 1U;
            p_value_32u[indx]     = 1U;
            p_ref_value_32u[indx] = 1U;
        }
    }
    for (uint32_t bit_width = 15U; bit_width <= 16U; bit_width++) {
        for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
            for (uint32_t length_dst = 1U; length_dst <= TEST_BUFFER_SIZE; length_dst++) {
                for (int32_t count = -1; count <= (int32_t)TEST_BUFFER_SIZE; count++) {
                    destination.fill(0);
                    reference.fill(0);
                    uint8_t*       p_src_8u     = source.data();
                    uint8_t*       p_ref_src_8u = source.data();
                    uint8_t*       p_dst_8u     = destination.data();
                    uint8_t*       p_ref_dst_8u = reference.data();
                    uint32_t*      p_value      = (uint32_t*)value.data();
                    uint32_t*      p_ref_value  = (uint32_t*)ref_value.data();
                    const uint32_t index        = 0U;
                    const uint32_t ref_index    = 0U;

                    int32_t        count_current     = count;
                    int32_t        ref_count_current = count;
                    const uint32_t status            = qplc_unpack_prle(fun_indx_unpack_prle_16u)(
                            &p_src_8u, length, bit_width, &p_dst_8u, length_dst, &count_current, p_value);
                    const uint32_t ref_status =
                            ref_qplc_unpack_prle_16u(&p_ref_src_8u, length, bit_width, &p_ref_dst_8u, length_dst,
                                                     &ref_count_current, p_ref_value);

                    ASSERT_EQ(status, ref_status);
                    ASSERT_EQ(p_src_8u, p_ref_src_8u);
                    ASSERT_EQ(count_current, ref_count_current);
                    ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
                    ASSERT_EQ(((uint8_t*)p_value - (uint8_t*)value.data()),
                              ((uint8_t*)p_ref_value - (uint8_t*)ref_value.data()));
                    ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(),
                                                destination.end(), "FAIL qplc_unpack_prle_16u!!! "));
                }
            }
        }
    }

    {
        uint16_t* p_source_16u    = (uint16_t*)source.data();
        uint32_t* p_value_32u     = (uint32_t*)value.data();
        uint32_t* p_ref_value_32u = (uint32_t*)ref_value.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_16u[indx]    = 0x8FU;
            p_value_32u[indx]     = 0x8FU;
            p_ref_value_32u[indx] = 0x8FU;
        }
    }
    for (uint32_t bit_width = 15U; bit_width <= 16U; bit_width++) {
        for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
            for (uint32_t length_dst = 1U; length_dst <= TEST_BUFFER_SIZE; length_dst++) {
                for (int32_t count = -1; count <= (int32_t)TEST_BUFFER_SIZE; count++) {
                    destination.fill(0);
                    reference.fill(0);
                    uint8_t*       p_src_8u     = source.data();
                    uint8_t*       p_ref_src_8u = source.data();
                    uint8_t*       p_dst_8u     = destination.data();
                    uint8_t*       p_ref_dst_8u = reference.data();
                    uint32_t*      p_value      = (uint32_t*)value.data();
                    uint32_t*      p_ref_value  = (uint32_t*)ref_value.data();
                    const uint32_t index        = 0U;
                    const uint32_t ref_index    = 0U;

                    int32_t        count_current     = count;
                    int32_t        ref_count_current = count;
                    const uint32_t status            = qplc_unpack_prle(fun_indx_unpack_prle_16u)(
                            &p_src_8u, length, bit_width, &p_dst_8u, length_dst, &count_current, p_value);
                    const uint32_t ref_status =
                            ref_qplc_unpack_prle_16u(&p_ref_src_8u, length, bit_width, &p_ref_dst_8u, length_dst,
                                                     &ref_count_current, p_ref_value);

                    ASSERT_EQ(status, ref_status);
                    ASSERT_EQ(p_src_8u, p_ref_src_8u);
                    ASSERT_EQ(count_current, ref_count_current);
                    ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
                    ASSERT_EQ(((uint8_t*)p_value - (uint8_t*)value.data()),
                              ((uint8_t*)p_ref_value - (uint8_t*)ref_value.data()));
                    ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(),
                                                destination.end(), "FAIL qplc_unpack_prle_16u!!! "));
                }
            }
        }
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_unpack_prle_32u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> source {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> value {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> ref_value {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> reference {};
    const uint64_t                                           seed = util::TestEnvironment::GetInstance().GetSeed();
    const randomizer                                         random_value(0U, static_cast<double>(UINT16_MAX), seed);
    const uint32_t                                           p_bit_witdh[3U] = {23U, 24U, 32U};

    {
        uint32_t* p_source_32u    = (uint32_t*)source.data();
        uint32_t* p_value_32u     = (uint32_t*)value.data();
        uint32_t* p_ref_value_32u = (uint32_t*)ref_value.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_32u[indx]    = 1U;
            p_value_32u[indx]     = 1U;
            p_ref_value_32u[indx] = 1U;
        }
    }
    for (uint32_t indx_bit_width = 0U; indx_bit_width < 3U; indx_bit_width++) {
        const uint32_t bit_width = p_bit_witdh[indx_bit_width];
        for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
            for (uint32_t length_dst = 1U; length_dst <= TEST_BUFFER_SIZE; length_dst++) {
                for (int32_t count = -1; count <= (int32_t)TEST_BUFFER_SIZE; count++) {
                    destination.fill(0);
                    reference.fill(0);
                    uint8_t*       p_src_8u     = source.data();
                    uint8_t*       p_ref_src_8u = source.data();
                    uint8_t*       p_dst_8u     = destination.data();
                    uint8_t*       p_ref_dst_8u = reference.data();
                    uint32_t*      p_value      = (uint32_t*)value.data();
                    uint32_t*      p_ref_value  = (uint32_t*)ref_value.data();
                    const uint32_t index        = 0U;
                    const uint32_t ref_index    = 0U;

                    int32_t        count_current     = count;
                    int32_t        ref_count_current = count;
                    const uint32_t status            = qplc_unpack_prle(fun_indx_unpack_prle_32u)(
                            &p_src_8u, length, bit_width, &p_dst_8u, length_dst, &count_current, p_value);
                    const uint32_t ref_status =
                            ref_qplc_unpack_prle_32u(&p_ref_src_8u, length, bit_width, &p_ref_dst_8u, length_dst,
                                                     &ref_count_current, p_ref_value);

                    ASSERT_EQ(status, ref_status);
                    ASSERT_EQ(p_src_8u, p_ref_src_8u);
                    ASSERT_EQ(count_current, ref_count_current);
                    ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
                    ASSERT_EQ(((uint8_t*)p_value - (uint8_t*)value.data()),
                              ((uint8_t*)p_ref_value - (uint8_t*)ref_value.data()));
                    ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(),
                                                destination.end(), "FAIL qplc_unpack_prle_32u!!! "));
                }
            }
        }
    }

    {
        uint32_t* p_source_32u    = (uint32_t*)source.data();
        uint32_t* p_value_32u     = (uint32_t*)value.data();
        uint32_t* p_ref_value_32u = (uint32_t*)ref_value.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_32u[indx]    = 0x8FU;
            p_value_32u[indx]     = 0x8FU;
            p_ref_value_32u[indx] = 0x8FU;
        }
    }
    for (uint32_t indx_bit_width = 0U; indx_bit_width < 3U; indx_bit_width++) {
        const uint32_t bit_width = p_bit_witdh[indx_bit_width];
        for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
            for (uint32_t length_dst = 1U; length_dst <= TEST_BUFFER_SIZE; length_dst++) {
                for (int32_t count = -1; count <= (int32_t)TEST_BUFFER_SIZE; count++) {
                    destination.fill(0);
                    reference.fill(0);
                    uint8_t*       p_src_8u     = source.data();
                    uint8_t*       p_ref_src_8u = source.data();
                    uint8_t*       p_dst_8u     = destination.data();
                    uint8_t*       p_ref_dst_8u = reference.data();
                    uint32_t*      p_value      = (uint32_t*)value.data();
                    uint32_t*      p_ref_value  = (uint32_t*)ref_value.data();
                    const uint32_t index        = 0U;
                    const uint32_t ref_index    = 0U;

                    int32_t        count_current     = count;
                    int32_t        ref_count_current = count;
                    const uint32_t status            = qplc_unpack_prle(fun_indx_unpack_prle_32u)(
                            &p_src_8u, length, bit_width, &p_dst_8u, length_dst, &count_current, p_value);
                    const uint32_t ref_status =
                            ref_qplc_unpack_prle_32u(&p_ref_src_8u, length, bit_width, &p_ref_dst_8u, length_dst,
                                                     &ref_count_current, p_ref_value);

                    ASSERT_EQ(status, ref_status);
                    ASSERT_EQ(p_src_8u, p_ref_src_8u);
                    ASSERT_EQ(count_current, ref_count_current);
                    ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
                    ASSERT_EQ(((uint8_t*)p_value - (uint8_t*)value.data()),
                              ((uint8_t*)p_ref_value - (uint8_t*)ref_value.data()));
                    ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(),
                                                destination.end(), "FAIL qplc_unpack_prle_32u!!! "));
                }
            }
        }
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_unpack_prle_8u, header_count_limits) {
    check_header_count_limits(fun_indx_unpack_prle_8u, QPL_TEST_BYTE_WIDTH, sizeof(uint8_t));
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_unpack_prle_16u, header_count_limits) {
    check_header_count_limits(fun_indx_unpack_prle_16u, QPL_TEST_WORD_WIDTH, sizeof(uint16_t));
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_unpack_prle_32u, header_count_limits) {
    check_header_count_limits(fun_indx_unpack_prle_32u, 32U, sizeof(uint32_t));
}
} // namespace qpl::test
