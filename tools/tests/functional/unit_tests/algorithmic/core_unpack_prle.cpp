/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/
#include <algorithm>
#include <array>

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
    uint8_t* dst_ptr       = (uint8_t*)*pp_dst;
    uint8_t* src_ptr       = (uint8_t*)*pp_src;
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
    uint8_t* dst_ptr       = (uint8_t*)*pp_dst;
    uint8_t* src_ptr       = (uint8_t*)*pp_src;
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
    uint8_t* dst_ptr       = (uint8_t*)*pp_dst;
    uint8_t* src_ptr       = (uint8_t*)*pp_src;
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
        value      = (uint32_t)*value_ptr;
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
} // namespace qpl::test
