/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/
#include "array"

#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"
#include "random_generator.h"
#include "../t_common.hpp"
#include "check_result.hpp"

#include "qplc_api.h"
#include "dispatcher/dispatcher.hpp"

qplc_rle_burst_t_ptr qplc_rle_burst(uint32_t index) {
    static const auto &table = qpl::ml::dispatcher::kernels_dispatcher::get_instance().get_expand_rle_table();

    return (qplc_rle_burst_t_ptr) table[index];
}

static qplc_status_t ref_qplc_pack_rle_8u(const uint8_t* src1_ptr,
    uint32_t flag_be,
    const uint8_t* src2_ptr,
    uint32_t src_length,
    uint8_t** pp_dst,
    uint8_t* dst_end_ptr,
    uint32_t* count_ptr,
    uint32_t* index_ptr)
{
    qplc_status_t status = QPLC_STS_OK;
    uint32_t      count;
    uint32_t      max_count;
    uint8_t* dst_ptr = *pp_dst;


    if (*count_ptr) {
        if ((dst_ptr + *count_ptr) > dst_end_ptr) {
            max_count = (uint32_t)(dst_end_ptr - dst_ptr);
            *count_ptr -= max_count;
            for (uint32_t idx = 0; idx < max_count; idx++) {
                dst_ptr[idx] = src2_ptr[*index_ptr];
            }
            *pp_dst = dst_end_ptr;
            return QPLC_STS_DST_IS_SHORT_ERR;
        }
        else {
            for (uint32_t idx = 0; idx < *count_ptr; idx++) {
                dst_ptr[idx] = src2_ptr[*index_ptr];
            }
            dst_ptr += *count_ptr;
            *count_ptr = 0u;
            (*index_ptr)++;
        }
    }
    for (; *index_ptr < src_length; (*index_ptr)++) {
        count = src1_ptr[*index_ptr];
        if (count) {
            if ((dst_ptr + count) > dst_end_ptr) {
                status = QPLC_STS_DST_IS_SHORT_ERR;
                max_count = (uint32_t)(dst_end_ptr - dst_ptr);
                *count_ptr = count - max_count;
                for (uint32_t idx = 0; idx < max_count; idx++) {
                    dst_ptr[idx] = src2_ptr[*index_ptr];
                }
                dst_ptr = dst_end_ptr;
                break;
            }
            for (uint32_t idx = 0; idx < count; idx++) {
                dst_ptr[idx] = src2_ptr[*index_ptr];
            }
            dst_ptr += count;
        }
    }
    *pp_dst = dst_ptr;
    return status;
}

static qplc_status_t ref_qplc_pack_rle_8u16u(const uint8_t* src1_ptr,
    uint32_t flag_be,
    const uint8_t* src2_ptr,
    uint32_t src_length,
    uint8_t** pp_dst,
    uint8_t* dst_end_ptr,
    uint32_t* count_ptr,
    uint32_t* index_ptr)
{
    qplc_status_t status = QPLC_STS_OK;
    uint32_t      count;
    uint32_t      max_count;
    uint16_t* dst_16u_ptr = (uint16_t*)*pp_dst;
    uint16_t* src2_16u_ptr = (uint16_t*)src2_ptr;

    if (*count_ptr) {
        if ((dst_16u_ptr + *count_ptr) > (uint16_t*)dst_end_ptr) {
            max_count = (uint32_t)((uint16_t*)dst_end_ptr - dst_16u_ptr);
            *count_ptr -= max_count;
            CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, max_count);
            *pp_dst = dst_end_ptr;
            return QPLC_STS_DST_IS_SHORT_ERR;
        }
        else {
            CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, *count_ptr);
            dst_16u_ptr += *count_ptr;
            *count_ptr = 0u;
            (*index_ptr)++;
        }
    }
    for (; *index_ptr < src_length; (*index_ptr)++) {
        count = src1_ptr[*index_ptr];
        if (count) {
            if ((dst_16u_ptr + count) > (uint16_t*) dst_end_ptr) {
                status = QPLC_STS_DST_IS_SHORT_ERR;
                max_count = (uint32_t)((uint16_t*)dst_end_ptr - dst_16u_ptr);
                *count_ptr = count - max_count;
                CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, max_count);
                dst_16u_ptr = (uint16_t*)dst_end_ptr;
                break;
            }
            CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, count);
            dst_16u_ptr += count;
        }
    }
    *pp_dst = (uint8_t*)dst_16u_ptr;
    return status;
}

static qplc_status_t ref_qplc_pack_rle_8u32u(const uint8_t* src1_ptr,
    uint32_t flag_be,
    const uint8_t* src2_ptr,
    uint32_t src_length,
    uint8_t** pp_dst,
    uint8_t* dst_end_ptr,
    uint32_t* count_ptr,
    uint32_t* index_ptr)
{
    qplc_status_t status = QPLC_STS_OK;
    uint32_t      count;
    uint32_t      max_count;
    uint32_t* dst_32u_ptr = (uint32_t*)*pp_dst;
    uint32_t* src2_32u_ptr = (uint32_t*)src2_ptr;

    if (*count_ptr) {
        if ((dst_32u_ptr + *count_ptr) > (uint32_t*)dst_end_ptr) {
            max_count = (uint32_t)((uint32_t*)dst_end_ptr - dst_32u_ptr);
            *count_ptr -= max_count;
            CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, max_count);
            *pp_dst = dst_end_ptr;
            return QPLC_STS_DST_IS_SHORT_ERR;
        }
        else {
            CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, *count_ptr);
            dst_32u_ptr += *count_ptr;
            *count_ptr = 0u;
            (*index_ptr)++;
        }
    }
    for (; *index_ptr < src_length; (*index_ptr)++) {
        count = src1_ptr[*index_ptr];
        if (count) {
            if ((dst_32u_ptr + count) > (uint32_t*) dst_end_ptr) {
                status = QPLC_STS_DST_IS_SHORT_ERR;
                max_count = (uint32_t)((uint32_t*)dst_end_ptr - dst_32u_ptr);
                *count_ptr = count - max_count;
                CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, max_count);
                dst_32u_ptr = (uint32_t*)dst_end_ptr;
                break;
            }
            CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, count);
            dst_32u_ptr += count;
        }
    }
    *pp_dst = (uint8_t*)dst_32u_ptr;
    return status;
}

static qplc_status_t ref_qplc_pack_rle_16u8u(const uint8_t* src1_ptr,
    uint32_t flag_be,
    const uint8_t* src2_ptr,
    uint32_t src_length,
    uint8_t** pp_dst,
    uint8_t* dst_end_ptr,
    uint32_t* count_ptr,
    uint32_t* index_ptr)
{
    qplc_status_t status = QPLC_STS_OK;
    uint32_t      count;
    uint32_t      max_count;
    uint8_t* dst_ptr = *pp_dst;
    uint16_t* src1_16u_ptr = (uint16_t*)src1_ptr;

    if (*count_ptr) {
        if ((dst_ptr + *count_ptr) > dst_end_ptr) {
            max_count = (uint32_t)(dst_end_ptr - dst_ptr);
            *count_ptr -= max_count;
            CALL_CORE_FUN(qplc_set_8u)(src2_ptr[*index_ptr], dst_ptr, max_count);
            *pp_dst = dst_end_ptr;
            return QPLC_STS_DST_IS_SHORT_ERR;
        }
        else {
            CALL_CORE_FUN(qplc_set_8u)(src2_ptr[*index_ptr], dst_ptr, *count_ptr);
            dst_ptr += *count_ptr;
            *count_ptr = 0u;
            (*index_ptr)++;
        }
    }
    for (; *index_ptr < src_length; (*index_ptr)++) {
        count = src1_16u_ptr[*index_ptr];
        if (flag_be) {
            count = ((count << 8u) & 0xff00) | ((count >> 8u) & 0xff);
        }
        if (count) {
            if ((dst_ptr + count) > dst_end_ptr) {
                status = QPLC_STS_DST_IS_SHORT_ERR;
                max_count = (uint32_t)(dst_end_ptr - dst_ptr);
                *count_ptr = count - max_count;
                CALL_CORE_FUN(qplc_set_8u)(src2_ptr[*index_ptr], dst_ptr, max_count);
                dst_ptr = dst_end_ptr;
                break;
            }
            CALL_CORE_FUN(qplc_set_8u)(src2_ptr[*index_ptr], dst_ptr, count);
            dst_ptr += count;
        }
    }
    *pp_dst = dst_ptr;
    return status;
}

static qplc_status_t ref_qplc_pack_rle_16u(const uint8_t* src1_ptr,
    uint32_t flag_be,
    const uint8_t* src2_ptr,
    uint32_t src_length,
    uint8_t** pp_dst,
    uint8_t* dst_end_ptr,
    uint32_t* count_ptr,
    uint32_t* index_ptr)
{
    qplc_status_t status = QPLC_STS_OK;
    uint32_t      count;
    uint32_t      max_count;
    uint16_t* dst_16u_ptr = (uint16_t*)*pp_dst;
    uint16_t* src2_16u_ptr = (uint16_t*)src2_ptr;
    uint16_t* src1_16u_ptr = (uint16_t*)src1_ptr;

    if (*count_ptr) {
        if ((dst_16u_ptr + *count_ptr) > (uint16_t*)dst_end_ptr) {
            max_count = (uint32_t)((uint16_t*)dst_end_ptr - dst_16u_ptr);
            *count_ptr -= max_count;
            CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, max_count);
            *pp_dst = dst_end_ptr;
            return QPLC_STS_DST_IS_SHORT_ERR;
        }
        else {
            CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, *count_ptr);
            dst_16u_ptr += *count_ptr;
            *count_ptr = 0u;
            (*index_ptr)++;
        }
    }
    for (; *index_ptr < src_length; (*index_ptr)++) {
        count = src1_16u_ptr[*index_ptr];
        if (flag_be) {
            count = ((count << 8u) & 0xff00) | ((count >> 8u) & 0xff);
        }
        if (count) {
            if ((dst_16u_ptr + count) > (uint16_t*) dst_end_ptr) {
                status = QPLC_STS_DST_IS_SHORT_ERR;
                max_count = (uint32_t)((uint16_t*)dst_end_ptr - dst_16u_ptr);
                *count_ptr = count - max_count;
                CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, max_count);
                dst_16u_ptr = (uint16_t*)dst_end_ptr;
                break;
            }
            CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, count);
            dst_16u_ptr += count;
        }
    }
    *pp_dst = (uint8_t*)dst_16u_ptr;
    return status;
}

static qplc_status_t ref_qplc_pack_rle_16u32u(const uint8_t* src1_ptr,
    uint32_t flag_be,
    const uint8_t* src2_ptr,
    uint32_t src_length,
    uint8_t** pp_dst,
    uint8_t* dst_end_ptr,
    uint32_t* count_ptr,
    uint32_t* index_ptr)
{
    qplc_status_t status = QPLC_STS_OK;
    uint32_t      count;
    uint32_t      max_count;
    uint32_t* dst_32u_ptr = (uint32_t*)*pp_dst;
    uint32_t* src2_32u_ptr = (uint32_t*)src2_ptr;
    uint16_t* src1_16u_ptr = (uint16_t*)src1_ptr;

    if (*count_ptr) {
        if ((dst_32u_ptr + *count_ptr) > (uint32_t*)dst_end_ptr) {
            max_count = (uint32_t)((uint32_t*)dst_end_ptr - dst_32u_ptr);
            *count_ptr -= max_count;
            CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, max_count);
            *pp_dst = dst_end_ptr;
            return QPLC_STS_DST_IS_SHORT_ERR;
        }
        else {
            CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, *count_ptr);
            dst_32u_ptr += *count_ptr;
            *count_ptr = 0u;
            (*index_ptr)++;
        }
    }
    for (; *index_ptr < src_length; (*index_ptr)++) {
        count = src1_16u_ptr[*index_ptr];
        if (flag_be) {
            count = ((count << 8u) & 0xff00) | ((count >> 8u) & 0xff);
        }
        if (count) {
            if ((dst_32u_ptr + count) > (uint32_t*) dst_end_ptr) {
                status = QPLC_STS_DST_IS_SHORT_ERR;
                max_count = (uint32_t)((uint32_t*)dst_end_ptr - dst_32u_ptr);
                *count_ptr = count - max_count;
                CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, max_count);
                dst_32u_ptr = (uint32_t*)dst_end_ptr;
                break;
            }
            CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, count);
            dst_32u_ptr += count;
        }
    }
    *pp_dst = (uint8_t*)dst_32u_ptr;
    return status;
}

#define TEST_RLE_BURST_MAX_COUNT   65535u

static qplc_status_t ref_qplc_pack_rle_32u8u(const uint8_t* src1_ptr,
    uint32_t flag_be,
    const uint8_t* src2_ptr,
    uint32_t src_length,
    uint8_t** pp_dst,
    uint8_t* dst_end_ptr,
    uint32_t* count_ptr,
    uint32_t* index_ptr)
{
    qplc_status_t status = QPLC_STS_OK;
    uint32_t      count;
    uint32_t      count_current;
    uint32_t      count_next;
    uint32_t      max_count;
    uint8_t* dst_ptr = *pp_dst;
    uint32_t* src1_32u_ptr = (uint32_t*)src1_ptr;

    if (*count_ptr) {
        if ((dst_ptr + *count_ptr) > dst_end_ptr) {
            max_count = (uint32_t)(dst_end_ptr - dst_ptr);
            *count_ptr -= max_count;
            CALL_CORE_FUN(qplc_set_8u)(src2_ptr[*index_ptr], dst_ptr, max_count);
            *pp_dst = dst_end_ptr;
            return QPLC_STS_DST_IS_SHORT_ERR;
        }
        else {
            CALL_CORE_FUN(qplc_set_8u)(src2_ptr[*index_ptr], dst_ptr, *count_ptr);
            dst_ptr += *count_ptr;
            *count_ptr = 0u;
            (*index_ptr)++;
        }
    }
    count_current = src1_32u_ptr[*index_ptr];
    if (flag_be) {
        count_current = ((count_current << 24u) & 0xff000000) |
            ((count_current << 8u) & 0xff0000) |
            ((count_current >> 8u) & 0xff00) |
            ((count_current >> 24u) & 0xff);
    }
    for (; *index_ptr < src_length; (*index_ptr)++) {
        count_next = src1_32u_ptr[*index_ptr + 1u];
        if (flag_be) {
            count_next = ((count_current << 24u) & 0xff000000) |
            ((count_next << 8u) & 0xff0000) |
            ((count_next >> 8u) & 0xff00) |
            ((count_next >> 24u) & 0xff);
        }
        count = count_next - count_current;
        if (TEST_RLE_BURST_MAX_COUNT < count) {
            return QPLC_STS_INVALID_RLE_COUNT;
        }
        count_current = count_next;
        if (count) {
            if ((dst_ptr + count) > dst_end_ptr) {
                status = QPLC_STS_DST_IS_SHORT_ERR;
                max_count = (uint32_t)(dst_end_ptr - dst_ptr);
                *count_ptr = count - max_count;
                CALL_CORE_FUN(qplc_set_8u)(src2_ptr[*index_ptr], dst_ptr, max_count);
                dst_ptr = dst_end_ptr;
                break;
            }
            CALL_CORE_FUN(qplc_set_8u)(src2_ptr[*index_ptr], dst_ptr, count);
            dst_ptr += count;
        }
    }
    *pp_dst = dst_ptr;
    return status;
}

static qplc_status_t ref_qplc_pack_rle_32u16u(const uint8_t* src1_ptr,
    uint32_t flag_be,
    const uint8_t* src2_ptr,
    uint32_t src_length,
    uint8_t** pp_dst,
    uint8_t* dst_end_ptr,
    uint32_t* count_ptr,
    uint32_t* index_ptr)
{
    qplc_status_t status = QPLC_STS_OK;
    uint32_t      count;
    uint32_t      count_current;
    uint32_t      count_next;
    uint32_t      max_count;
    uint16_t* dst_16u_ptr = (uint16_t*)*pp_dst;
    uint16_t* src2_16u_ptr = (uint16_t*)src2_ptr;
    uint32_t* src1_32u_ptr = (uint32_t*)src1_ptr;

    if (*count_ptr) {
        if ((dst_16u_ptr + *count_ptr) > (uint16_t*)dst_end_ptr) {
            max_count = (uint32_t)((uint16_t*)dst_end_ptr - dst_16u_ptr);
            *count_ptr -= max_count;
            CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, max_count);
            *pp_dst = dst_end_ptr;
            return QPLC_STS_DST_IS_SHORT_ERR;
        }
        else {
            CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, *count_ptr);
            dst_16u_ptr += *count_ptr;
            *count_ptr = 0u;
            (*index_ptr)++;
        }
    }
    count_current = src1_32u_ptr[*index_ptr];
    if (flag_be) {
        count_current = ((count_current << 24u) & 0xff000000) |
            ((count_current << 8u) & 0xff0000) |
            ((count_current >> 8u) & 0xff00) |
            ((count_current >> 24u) & 0xff);
    }
    for (; *index_ptr < src_length; (*index_ptr)++) {
        count_next = src1_32u_ptr[*index_ptr + 1u];
        if (flag_be) {
            count_next = ((count_current << 24u) & 0xff000000) |
                ((count_next << 8u) & 0xff0000) |
                ((count_next >> 8u) & 0xff00) |
                ((count_next >> 24u) & 0xff);
        }
        count = count_next - count_current;
        if (TEST_RLE_BURST_MAX_COUNT < count) {
            return QPLC_STS_INVALID_RLE_COUNT;
        }
        count_current = count_next;
        if (count) {
            if ((dst_16u_ptr + count) > (uint16_t*) dst_end_ptr) {
                status = QPLC_STS_DST_IS_SHORT_ERR;
                max_count = (uint32_t)((uint16_t*)dst_end_ptr - dst_16u_ptr);
                *count_ptr = count - max_count;
                CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, max_count);
                dst_16u_ptr = (uint16_t*)dst_end_ptr;
                break;
            }
            CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, count);
            dst_16u_ptr += count;
        }
    }
    *pp_dst = (uint8_t*)dst_16u_ptr;
    return status;
}

static qplc_status_t ref_qplc_pack_rle_32u(const uint8_t* src1_ptr,
    uint32_t flag_be,
    const uint8_t* src2_ptr,
    uint32_t src_length,
    uint8_t** pp_dst,
    uint8_t* dst_end_ptr,
    uint32_t* count_ptr,
    uint32_t* index_ptr)
{
    qplc_status_t status = QPLC_STS_OK;
    uint32_t      count;
    uint32_t      count_current;
    uint32_t      count_next;
    uint32_t      max_count;
    uint32_t* dst_32u_ptr = (uint32_t*)*pp_dst;
    uint32_t* src2_32u_ptr = (uint32_t*)src2_ptr;
    uint32_t* src1_32u_ptr = (uint32_t*)src1_ptr;

    if (*count_ptr) {
        if ((dst_32u_ptr + *count_ptr) > (uint32_t*)dst_end_ptr) {
            max_count = (uint32_t)((uint32_t*)dst_end_ptr - dst_32u_ptr);
            *count_ptr -= max_count;
            CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, max_count);
            *pp_dst = dst_end_ptr;
            return QPLC_STS_DST_IS_SHORT_ERR;
        }
        else {
            CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, *count_ptr);
            dst_32u_ptr += *count_ptr;
            *count_ptr = 0u;
            (*index_ptr)++;
        }
    }
    count_current = src1_32u_ptr[*index_ptr];
    if (flag_be) {
        count_current = ((count_current << 24u) & 0xff000000) |
            ((count_current << 8u) & 0xff0000) |
            ((count_current >> 8u) & 0xff00) |
            ((count_current >> 24u) & 0xff);
    }
    for (; *index_ptr < src_length; (*index_ptr)++) {
        count_next = src1_32u_ptr[*index_ptr + 1u];
        if (flag_be) {
            count_next = ((count_current << 24u) & 0xff000000) |
                ((count_next << 8u) & 0xff0000) |
                ((count_next >> 8u) & 0xff00) |
                ((count_next >> 24u) & 0xff);
        }
        count = count_next - count_current;
        if (TEST_RLE_BURST_MAX_COUNT < count) {
            return QPLC_STS_INVALID_RLE_COUNT;
        }
        count_current = count_next;
        if (count) {
            if ((dst_32u_ptr + count) > (uint32_t*) dst_end_ptr) {
                status = QPLC_STS_DST_IS_SHORT_ERR;
                max_count = (uint32_t)((uint32_t*)dst_end_ptr - dst_32u_ptr);
                *count_ptr = count - max_count;
                CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, max_count);
                dst_32u_ptr = (uint32_t*)dst_end_ptr;
                break;
            }
            CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, count);
            dst_32u_ptr += count;
        }
    }
    *pp_dst = (uint8_t*)dst_32u_ptr;
    return status;
}

constexpr uint32_t fun_indx_rle_burst_8u     = 0;
constexpr uint32_t fun_indx_rle_burst_8u16u  = 1;
constexpr uint32_t fun_indx_rle_burst_8u32u  = 2;
constexpr uint32_t fun_indx_rle_burst_16u8u  = 3;
constexpr uint32_t fun_indx_rle_burst_16u    = 4;
constexpr uint32_t fun_indx_rle_burst_16u32u = 5;
constexpr uint32_t fun_indx_rle_burst_32u8u  = 6;
constexpr uint32_t fun_indx_rle_burst_32u16u = 7;
constexpr uint32_t fun_indx_rle_burst_32u    = 8;

constexpr uint32_t TEST_BUFFER_SIZE = 128u;

namespace qpl::test {
using randomizer = qpl::test::random;
QPL_UNIT_API_ALGORITHMIC_TEST(qplc_rle_burst_8u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> source1{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> source2{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT8_MAX), seed);

    {
        uint8_t* p_source1_8u = (uint8_t*)source1.data();
        uint8_t* p_source2_8u = (uint8_t*)source2.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source1_8u[indx] = 0x3f & static_cast<uint8_t>(random_value);
        }
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source2_8u[indx] = static_cast<uint8_t>(random_value);
        }
    }
    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        uint8_t* p_dst_8u = destination.data();
        uint8_t* p_ref_dst_8u = reference.data();
        uint32_t count = 128u + 64u;
        uint32_t ref_count = 128u + 64u;
        uint32_t index = 0;
        uint32_t ref_index = 0;
        uint32_t status;
        uint32_t ref_status;

        status = (uint32_t)qplc_rle_burst(fun_indx_rle_burst_8u)(source1.data(), 0, source2.data(),
            length, &p_dst_8u, destination.data() + destination.size(), &count, &index);
        ref_status = (uint32_t)ref_qplc_pack_rle_8u(source1.data(), 0, source2.data(),
            length, &p_ref_dst_8u, reference.data() + reference.size(), &ref_count, &ref_index);
        ASSERT_EQ(status, ref_status);
        ASSERT_EQ(index, ref_index);
        ASSERT_EQ(count, ref_count);
        ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_rle_burst_8u!!! "));
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        uint8_t* p_dst_8u = destination.data();
        uint8_t* p_ref_dst_8u = reference.data();
        uint32_t count = 0u;
        uint32_t ref_count = 0u;
        uint32_t index = 0;
        uint32_t ref_index = 0;
        uint32_t status;
        uint32_t ref_status;

        {
            uint8_t* p_source1_8u = (uint8_t*)source1.data();
            for (uint32_t indx = 0; indx < 4; indx++) {
                p_source1_8u[indx] = 0u;
            }
        }
        status = (uint32_t)qplc_rle_burst(fun_indx_rle_burst_8u)(source1.data(), 0, source2.data(),
            length, &p_dst_8u, destination.data() + destination.size(), &count, &index);
        ref_status = (uint32_t)ref_qplc_pack_rle_8u(source1.data(), 0, source2.data(),
            length, &p_ref_dst_8u, reference.data() + reference.size(), &ref_count, &ref_index);
        ASSERT_EQ(status, ref_status);
        ASSERT_EQ(index, ref_index);
        ASSERT_EQ(count, ref_count);
        ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_rle_burst_8u!!! "));
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_rle_burst_8u16u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> source1{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> source2{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT8_MAX), seed);

    {
        uint8_t* p_source1_8u = (uint8_t*)source1.data();
        uint16_t* p_source2_16u = (uint16_t*)source2.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source1_8u[indx] = 0x3f & static_cast<uint8_t>(random_value);
        }
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source2_16u[indx] = static_cast<uint16_t>(random_value);
        }
    }
    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        uint8_t* p_dst_8u = destination.data();
        uint8_t* p_ref_dst_8u = reference.data();
        uint32_t count = 128u + 64u;
        uint32_t ref_count = 128u + 64u;
        uint32_t index = 0;
        uint32_t ref_index = 0;
        uint32_t status;
        uint32_t ref_status;

        status = (uint32_t)qplc_rle_burst(fun_indx_rle_burst_8u16u)(source1.data(), 0, source2.data(),
            length, &p_dst_8u, destination.data() + destination.size(), &count, &index);
        ref_status = (uint32_t)ref_qplc_pack_rle_8u16u(source1.data(), 0, source2.data(),
            length, &p_ref_dst_8u, reference.data() + reference.size(), &ref_count, &ref_index);
        ASSERT_EQ(status, ref_status);
        ASSERT_EQ(index, ref_index);
        ASSERT_EQ(count, ref_count);
        ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_rle_burst_8u16u!!! "));
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        uint8_t* p_dst_8u = destination.data();
        uint8_t* p_ref_dst_8u = reference.data();
        uint32_t count = 0u;
        uint32_t ref_count = 0u;
        uint32_t index = 0;
        uint32_t ref_index = 0;
        uint32_t status;
        uint32_t ref_status;

        {
            uint8_t* p_source1_8u = (uint8_t*)source1.data();
            for (uint32_t indx = 0; indx < 4; indx++) {
                p_source1_8u[indx] = 0u;
            }
        }
        status = (uint32_t)qplc_rle_burst(fun_indx_rle_burst_8u16u)(source1.data(), 0, source2.data(),
            length, &p_dst_8u, destination.data() + destination.size(), &count, &index);
        ref_status = (uint32_t)ref_qplc_pack_rle_8u16u(source1.data(), 0, source2.data(),
            length, &p_ref_dst_8u, reference.data() + reference.size(), &ref_count, &ref_index);
        ASSERT_EQ(status, ref_status);
        ASSERT_EQ(index, ref_index);
        ASSERT_EQ(count, ref_count);
        ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_rle_burst_8u16u!!! "));
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_rle_burst_8u32u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> source1{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> source2{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT8_MAX), seed);

    {
        uint8_t* p_source1_8u = (uint8_t*)source1.data();
        uint32_t* p_source2_32u = (uint32_t*)source2.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source1_8u[indx] = 0x3f & static_cast<uint8_t>(random_value);
        }
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source2_32u[indx] = static_cast<uint32_t>(random_value);
        }
    }
    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        uint8_t* p_dst_8u = destination.data();
        uint8_t* p_ref_dst_8u = reference.data();
        uint32_t count = 128u + 64u;
        uint32_t ref_count = 128u + 64u;
        uint32_t index = 0;
        uint32_t ref_index = 0;
        uint32_t status;
        uint32_t ref_status;

        status = (uint32_t)qplc_rle_burst(fun_indx_rle_burst_8u32u)(source1.data(), 0, source2.data(),
            length, &p_dst_8u, destination.data() + destination.size(), &count, &index);
        ref_status = (uint32_t)ref_qplc_pack_rle_8u32u(source1.data(), 0, source2.data(),
            length, &p_ref_dst_8u, reference.data() + reference.size(), &ref_count, &ref_index);
        ASSERT_EQ(status, ref_status);
        ASSERT_EQ(index, ref_index);
        ASSERT_EQ(count, ref_count);
        ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_rle_burst_8u32u!!! "));
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        uint8_t* p_dst_8u = destination.data();
        uint8_t* p_ref_dst_8u = reference.data();
        uint32_t count = 0u;
        uint32_t ref_count = 0u;
        uint32_t index = 0;
        uint32_t ref_index = 0;
        uint32_t status;
        uint32_t ref_status;

        {
            uint8_t* p_source1_8u = (uint8_t*)source1.data();
            for (uint32_t indx = 0; indx < 4; indx++) {
                p_source1_8u[indx] = 0u;
            }
        }
        status = (uint32_t)qplc_rle_burst(fun_indx_rle_burst_8u32u)(source1.data(), 0, source2.data(),
            length, &p_dst_8u, destination.data() + destination.size(), &count, &index);
        ref_status = (uint32_t)ref_qplc_pack_rle_8u32u(source1.data(), 0, source2.data(),
            length, &p_ref_dst_8u, reference.data() + reference.size(), &ref_count, &ref_index);
        ASSERT_EQ(status, ref_status);
        ASSERT_EQ(index, ref_index);
        ASSERT_EQ(count, ref_count);
        ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_rle_burst_8u32u!!! "));
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_rle_burst_16u8u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> source1{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> source2{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT8_MAX), seed);

    {
        uint16_t* p_source1_16u = (uint16_t*)source1.data();
        uint8_t* p_source2_8u = (uint8_t*)source2.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source1_16u[indx] = 0x3f & static_cast<uint16_t>(random_value);
        }
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source2_8u[indx] = static_cast<uint8_t>(random_value);
        }
    }
    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        uint8_t* p_dst_8u = destination.data();
        uint8_t* p_ref_dst_8u = reference.data();
        uint32_t count = 128u + 64u;
        uint32_t ref_count = 128u + 64u;
        uint32_t index = 0;
        uint32_t ref_index = 0;
        uint32_t status;
        uint32_t ref_status;

        status = (uint32_t)qplc_rle_burst(fun_indx_rle_burst_16u8u)(source1.data(), 0, source2.data(),
            length, &p_dst_8u, destination.data() + destination.size(), &count, &index);
        ref_status = (uint32_t)ref_qplc_pack_rle_16u8u(source1.data(), 0, source2.data(),
            length, &p_ref_dst_8u, reference.data() + reference.size(), &ref_count, &ref_index);
        ASSERT_EQ(status, ref_status);
        ASSERT_EQ(index, ref_index);
        ASSERT_EQ(count, ref_count);
        ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_rle_burst_16u8u!!! "));
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        uint8_t* p_dst_8u = destination.data();
        uint8_t* p_ref_dst_8u = reference.data();
        uint32_t count = 0u;
        uint32_t ref_count = 0u;
        uint32_t index = 0;
        uint32_t ref_index = 0;
        uint32_t status;
        uint32_t ref_status;

        {
            uint16_t* p_source1_16u = (uint16_t*)source1.data();
            for (uint32_t indx = 0; indx < 4; indx++) {
                p_source1_16u[indx] = 0u;
            }
        }
        status = (uint32_t)qplc_rle_burst(fun_indx_rle_burst_16u8u)(source1.data(), 0, source2.data(),
            length, &p_dst_8u, destination.data() + destination.size(), &count, &index);
        ref_status = (uint32_t)ref_qplc_pack_rle_16u8u(source1.data(), 0, source2.data(),
            length, &p_ref_dst_8u, reference.data() + reference.size(), &ref_count, &ref_index);
        ASSERT_EQ(status, ref_status);
        ASSERT_EQ(index, ref_index);
        ASSERT_EQ(count, ref_count);
        ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_rle_burst_16u8u!!! "));
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_rle_burst_16u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> source1{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> source2{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT8_MAX), seed);

    {
        uint16_t* p_source1_16u = (uint16_t*)source1.data();
        uint16_t* p_source2_16u = (uint16_t*)source2.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source1_16u[indx] = 0x3f & static_cast<uint16_t>(random_value);
        }
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source2_16u[indx] = static_cast<uint16_t>(random_value);
        }
    }
    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        uint8_t* p_dst_8u = destination.data();
        uint8_t* p_ref_dst_8u = reference.data();
        uint32_t count = 128u + 64u;
        uint32_t ref_count = 128u + 64u;
        uint32_t index = 0;
        uint32_t ref_index = 0;
        uint32_t status;
        uint32_t ref_status;

        status = (uint32_t)qplc_rle_burst(fun_indx_rle_burst_16u)(source1.data(), 0, source2.data(),
            length, &p_dst_8u, destination.data() + destination.size(), &count, &index);
        ref_status = (uint32_t)ref_qplc_pack_rle_16u(source1.data(), 0, source2.data(),
            length, &p_ref_dst_8u, reference.data() + reference.size(), &ref_count, &ref_index);
        ASSERT_EQ(status, ref_status);
        ASSERT_EQ(index, ref_index);
        ASSERT_EQ(count, ref_count);
        ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_rle_burst_16u!!! "));
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        uint8_t* p_dst_8u = destination.data();
        uint8_t* p_ref_dst_8u = reference.data();
        uint32_t count = 0u;
        uint32_t ref_count = 0u;
        uint32_t index = 0;
        uint32_t ref_index = 0;
        uint32_t status;
        uint32_t ref_status;

        {
            uint16_t* p_source1_16u = (uint16_t*)source1.data();
            for (uint32_t indx = 0; indx < 4; indx++) {
                p_source1_16u[indx] = 0u;
            }
        }
        status = (uint32_t)qplc_rle_burst(fun_indx_rle_burst_16u)(source1.data(), 0, source2.data(),
            length, &p_dst_8u, destination.data() + destination.size(), &count, &index);
        ref_status = (uint32_t)ref_qplc_pack_rle_16u(source1.data(), 0, source2.data(),
            length, &p_ref_dst_8u, reference.data() + reference.size(), &ref_count, &ref_index);
        ASSERT_EQ(status, ref_status);
        ASSERT_EQ(index, ref_index);
        ASSERT_EQ(count, ref_count);
        ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_rle_burst_16u!!! "));
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_rle_burst_16u32u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> source1{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> source2{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT8_MAX), seed);

    {
        uint16_t* p_source1_16u = (uint16_t*)source1.data();
        uint32_t* p_source2_32u = (uint32_t*)source2.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source1_16u[indx] = 0x3f & static_cast<uint16_t>(random_value);
        }
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source2_32u[indx] = static_cast<uint32_t>(random_value);
        }
    }
    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        uint8_t* p_dst_8u = destination.data();
        uint8_t* p_ref_dst_8u = reference.data();
        uint32_t count = 128u + 64u;
        uint32_t ref_count = 128u + 64u;
        uint32_t index = 0;
        uint32_t ref_index = 0;
        uint32_t status;
        uint32_t ref_status;

        status = (uint32_t)qplc_rle_burst(fun_indx_rle_burst_16u32u)(source1.data(), 0, source2.data(),
            length, &p_dst_8u, destination.data() + destination.size(), &count, &index);
        ref_status = (uint32_t)ref_qplc_pack_rle_16u32u(source1.data(), 0, source2.data(),
            length, &p_ref_dst_8u, reference.data() + reference.size(), &ref_count, &ref_index);
        ASSERT_EQ(status, ref_status);
        ASSERT_EQ(index, ref_index);
        ASSERT_EQ(count, ref_count);
        ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_rle_burst_16u32u!!! "));
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        uint8_t* p_dst_8u = destination.data();
        uint8_t* p_ref_dst_8u = reference.data();
        uint32_t count = 0u;
        uint32_t ref_count = 0u;
        uint32_t index = 0;
        uint32_t ref_index = 0;
        uint32_t status;
        uint32_t ref_status;

        {
            uint16_t* p_source1_16u = (uint16_t*)source1.data();
            for (uint32_t indx = 0; indx < 4; indx++) {
                p_source1_16u[indx] = 0u;
            }
        }
        status = (uint32_t)qplc_rle_burst(fun_indx_rle_burst_16u32u)(source1.data(), 0, source2.data(),
            length, &p_dst_8u, destination.data() + destination.size(), &count, &index);
        ref_status = (uint32_t)ref_qplc_pack_rle_16u32u(source1.data(), 0, source2.data(),
            length, &p_ref_dst_8u, reference.data() + reference.size(), &ref_count, &ref_index);
        ASSERT_EQ(status, ref_status);
        ASSERT_EQ(index, ref_index);
        ASSERT_EQ(count, ref_count);
        ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_rle_burst_16u32u!!! "));
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_rle_burst_32u8u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> source1{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> source2{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT8_MAX), seed);

    {
        uint32_t* p_source1_32u = (uint32_t*)source1.data();
        uint8_t* p_source2_8u = (uint8_t*)source2.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source1_32u[indx] = 0x3f & static_cast<uint32_t>(random_value);
        }
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source2_8u[indx] = static_cast<uint8_t>(random_value);
        }
    }
    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        uint8_t* p_dst_8u = destination.data();
        uint8_t* p_ref_dst_8u = reference.data();
        uint32_t count = 128u + 64u;
        uint32_t ref_count = 128u + 64u;
        uint32_t index = 0;
        uint32_t ref_index = 0;
        uint32_t status;
        uint32_t ref_status;

        status = (uint32_t)qplc_rle_burst(fun_indx_rle_burst_32u8u)(source1.data(), 0, source2.data(),
            length, &p_dst_8u, destination.data() + destination.size(), &count, &index);
        ref_status = (uint32_t)ref_qplc_pack_rle_32u8u(source1.data(), 0, source2.data(),
            length, &p_ref_dst_8u, reference.data() + reference.size(), &ref_count, &ref_index);
        ASSERT_EQ(status, ref_status);
        ASSERT_EQ(index, ref_index);
        ASSERT_EQ(count, ref_count);
        ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_rle_burst_32u8u!!! "));
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        uint8_t* p_dst_8u = destination.data();
        uint8_t* p_ref_dst_8u = reference.data();
        uint32_t count = 0u;
        uint32_t ref_count = 0u;
        uint32_t index = 0;
        uint32_t ref_index = 0;
        uint32_t status;
        uint32_t ref_status;

        {
            uint32_t* p_source1_32u = (uint32_t*)source1.data();
            for (uint32_t indx = 0; indx < 4; indx++) {
                p_source1_32u[indx] = 0u;
            }
        }
        status = (uint32_t)qplc_rle_burst(fun_indx_rle_burst_32u8u)(source1.data(), 0, source2.data(),
            length, &p_dst_8u, destination.data() + destination.size(), &count, &index);
        ref_status = (uint32_t)ref_qplc_pack_rle_32u8u(source1.data(), 0, source2.data(),
            length, &p_ref_dst_8u, reference.data() + reference.size(), &ref_count, &ref_index);
        ASSERT_EQ(status, ref_status);
        ASSERT_EQ(index, ref_index);
        ASSERT_EQ(count, ref_count);
        ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_rle_burst_32u8u!!! "));
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_rle_burst_32u16u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> source1{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> source2{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT8_MAX), seed);

    {
        uint32_t* p_source1_32u = (uint32_t*)source1.data();
        uint16_t* p_source2_16u = (uint16_t*)source2.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source1_32u[indx] = 0x3f & static_cast<uint32_t>(random_value);
        }
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source2_16u[indx] = static_cast<uint16_t>(random_value);
        }
    }
    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        uint8_t* p_dst_8u = destination.data();
        uint8_t* p_ref_dst_8u = reference.data();
        uint32_t count = 128u + 64u;
        uint32_t ref_count = 128u + 64u;
        uint32_t index = 0;
        uint32_t ref_index = 0;
        uint32_t status;
        uint32_t ref_status;

        status = (uint32_t)qplc_rle_burst(fun_indx_rle_burst_32u16u)(source1.data(), 0, source2.data(),
            length, &p_dst_8u, destination.data() + destination.size(), &count, &index);
        ref_status = (uint32_t)ref_qplc_pack_rle_32u16u(source1.data(), 0, source2.data(),
            length, &p_ref_dst_8u, reference.data() + reference.size(), &ref_count, &ref_index);
        ASSERT_EQ(status, ref_status);
        ASSERT_EQ(index, ref_index);
        ASSERT_EQ(count, ref_count);
        ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_rle_burst_32u16u!!! "));
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        uint8_t* p_dst_8u = destination.data();
        uint8_t* p_ref_dst_8u = reference.data();
        uint32_t count = 0u;
        uint32_t ref_count = 0u;
        uint32_t index = 0;
        uint32_t ref_index = 0;
        uint32_t status;
        uint32_t ref_status;

        {
            uint32_t* p_source1_32u = (uint32_t*)source1.data();
            for (uint32_t indx = 0; indx < 4; indx++) {
                p_source1_32u[indx] = 0u;
            }
        }
        status = (uint32_t)qplc_rle_burst(fun_indx_rle_burst_32u16u)(source1.data(), 0, source2.data(),
            length, &p_dst_8u, destination.data() + destination.size(), &count, &index);
        ref_status = (uint32_t)ref_qplc_pack_rle_32u16u(source1.data(), 0, source2.data(),
            length, &p_ref_dst_8u, reference.data() + reference.size(), &ref_count, &ref_index);
        ASSERT_EQ(status, ref_status);
        ASSERT_EQ(index, ref_index);
        ASSERT_EQ(count, ref_count);
        ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_rle_burst_32u16u!!! "));
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_rle_burst_32u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> source1{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> source2{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT8_MAX), seed);

    {
        uint32_t* p_source1_32u = (uint32_t*)source1.data();
        uint32_t* p_source2_32u = (uint32_t*)source2.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source1_32u[indx] = 0x3f & static_cast<uint32_t>(random_value);
        }
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source2_32u[indx] = static_cast<uint32_t>(random_value);
        }
    }
    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        uint8_t* p_dst_8u = destination.data();
        uint8_t* p_ref_dst_8u = reference.data();
        uint32_t count = 128u + 64u;
        uint32_t ref_count = 128u + 64u;
        uint32_t index = 0;
        uint32_t ref_index = 0;
        uint32_t status;
        uint32_t ref_status;

        status = (uint32_t)qplc_rle_burst(fun_indx_rle_burst_32u)(source1.data(), 0, source2.data(),
            length, &p_dst_8u, destination.data() + destination.size(), &count, &index);
        ref_status = (uint32_t)ref_qplc_pack_rle_32u(source1.data(), 0, source2.data(),
            length, &p_ref_dst_8u, reference.data() + reference.size(), &ref_count, &ref_index);
        ASSERT_EQ(status, ref_status);
        ASSERT_EQ(index, ref_index);
        ASSERT_EQ(count, ref_count);
        ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_rle_burst_32u!!! "));
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        uint8_t* p_dst_8u = destination.data();
        uint8_t* p_ref_dst_8u = reference.data();
        uint32_t count = 0u;
        uint32_t ref_count = 0u;
        uint32_t index = 0;
        uint32_t ref_index = 0;
        uint32_t status;
        uint32_t ref_status;

        {
            uint32_t* p_source1_32u = (uint32_t*)source1.data();
            for (uint32_t indx = 0; indx < 4; indx++) {
                p_source1_32u[indx] = 0u;
            }
        }
        status = (uint32_t)qplc_rle_burst(fun_indx_rle_burst_32u)(source1.data(), 0, source2.data(),
            length, &p_dst_8u, destination.data() + destination.size(), &count, &index);
        ref_status = (uint32_t)ref_qplc_pack_rle_32u(source1.data(), 0, source2.data(),
            length, &p_ref_dst_8u, reference.data() + reference.size(), &ref_count, &ref_index);
        ASSERT_EQ(status, ref_status);
        ASSERT_EQ(index, ref_index);
        ASSERT_EQ(count, ref_count);
        ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_rle_burst_32u!!! "));
    }
}
}
