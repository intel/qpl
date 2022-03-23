/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @brief Contains implementation of all functions for rle_burst analytics operation
 * @date 07/30/2020
 *
 * @details Function list:
 *          - @ref qplc_rle_burst_8u
 *          - @ref qplc_rle_burst_8u16u
 *          - @ref qplc_rle_burst_8u32u
 *          - @ref qplc_rle_burst_16u8u
 *          - @ref qplc_rle_burst_16u
 *          - @ref qplc_rle_burst_16u32u
 *          - @ref qplc_rle_burst_32u8u
 *          - @ref qplc_rle_burst_32u16u
 *          - @ref qplc_rle_burst_32u
 *
 */

#include "own_qplc_defs.h"
#include "qplc_memop.h"

/******** out-of-place rle_burst functions ********/

OWN_QPLC_FUN(qplc_status_t, qplc_rle_burst_8u, (const uint8_t *src1_ptr,
        uint32_t UNREFERENCED_PARAMETER(flag_be),
        const uint8_t *src2_ptr,
        uint32_t src_length,
        uint8_t **pp_dst,
        uint8_t *dst_end_ptr,
        uint32_t *count_ptr,
        uint32_t *index_ptr)) {
    qplc_status_t status   = QPLC_STS_OK;
    uint32_t      count;
    uint32_t      max_count;
    uint8_t       *dst_ptr = *pp_dst;

    if (*count_ptr) {
        if ((dst_ptr + *count_ptr) > dst_end_ptr) {
            max_count = (uint32_t) (dst_end_ptr - dst_ptr);
            *count_ptr -= max_count;
            CALL_CORE_FUN(qplc_set_8u)(src2_ptr[*index_ptr], dst_ptr, max_count);
            *pp_dst = dst_end_ptr;
            return QPLC_STS_DST_IS_SHORT_ERR;
        } else {
            CALL_CORE_FUN(qplc_set_8u)(src2_ptr[*index_ptr], dst_ptr, *count_ptr);
            dst_ptr += *count_ptr;
            *count_ptr = 0u;
            (*index_ptr)++;
        }
    }
    for (; *index_ptr < src_length; (*index_ptr)++) {
        count = src1_ptr[*index_ptr];
        if (count) {
            if ((dst_ptr + count) > dst_end_ptr) {
                status    = QPLC_STS_DST_IS_SHORT_ERR;
                max_count = (uint32_t) (dst_end_ptr - dst_ptr);
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

OWN_QPLC_FUN(qplc_status_t, qplc_rle_burst_8u16u, (const uint8_t *src1_ptr,
        uint32_t UNREFERENCED_PARAMETER(flag_be),
        const uint8_t *src2_ptr,
        uint32_t src_length,
        uint8_t **pp_dst,
        uint8_t *dst_end_ptr,
        uint32_t *count_ptr,
        uint32_t *index_ptr)) {
    qplc_status_t status        = QPLC_STS_OK;
    uint32_t      count;
    uint32_t      max_count;
    uint16_t      *dst_16u_ptr  = (uint16_t *) *pp_dst;
    uint16_t      *src2_16u_ptr = (uint16_t *) src2_ptr;

    if (*count_ptr) {
        if ((dst_16u_ptr + *count_ptr) > (uint16_t *) dst_end_ptr) {
            max_count = (uint32_t) ((uint16_t *) dst_end_ptr - dst_16u_ptr);
            *count_ptr -= max_count;
            CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, max_count);
            *pp_dst = dst_end_ptr;
            return QPLC_STS_DST_IS_SHORT_ERR;
        } else {
            CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, *count_ptr);
            dst_16u_ptr += *count_ptr;
            *count_ptr = 0u;
            (*index_ptr)++;
        }
    }
    for (; *index_ptr < src_length; (*index_ptr)++) {
        count = src1_ptr[*index_ptr];
        if (count) {
            if ((dst_16u_ptr + count) > (uint16_t *) dst_end_ptr) {
                status    = QPLC_STS_DST_IS_SHORT_ERR;
                max_count = (uint32_t) ((uint16_t *) dst_end_ptr - dst_16u_ptr);
                *count_ptr = count - max_count;
                CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, max_count);
                dst_16u_ptr = (uint16_t *) dst_end_ptr;
                break;
            }
            CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, count);
            dst_16u_ptr += count;
        }
    }
    *pp_dst = (uint8_t *) dst_16u_ptr;
    return status;
}

OWN_QPLC_FUN(qplc_status_t, qplc_rle_burst_8u32u, (const uint8_t *src1_ptr,
        uint32_t UNREFERENCED_PARAMETER(flag_be),
        const uint8_t *src2_ptr,
        uint32_t src_length,
        uint8_t **pp_dst,
        uint8_t *dst_end_ptr,
        uint32_t *count_ptr,
        uint32_t *index_ptr)) {
    qplc_status_t status        = QPLC_STS_OK;
    uint32_t      count;
    uint32_t      max_count;
    uint32_t      *dst_32u_ptr  = (uint32_t *) *pp_dst;
    uint32_t      *src2_32u_ptr = (uint32_t *) src2_ptr;

    if (*count_ptr) {
        if ((dst_32u_ptr + *count_ptr) > (uint32_t *) dst_end_ptr) {
            max_count = (uint32_t) ((uint32_t *) dst_end_ptr - dst_32u_ptr);
            *count_ptr -= max_count;
            CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, max_count);
            *pp_dst = dst_end_ptr;
            return QPLC_STS_DST_IS_SHORT_ERR;
        } else {
            CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, *count_ptr);
            dst_32u_ptr += *count_ptr;
            *count_ptr = 0u;
            (*index_ptr)++;
        }
    }
    for (; *index_ptr < src_length; (*index_ptr)++) {
        count = src1_ptr[*index_ptr];
        if (count) {
            if ((dst_32u_ptr + count) > (uint32_t *) dst_end_ptr) {
                status    = QPLC_STS_DST_IS_SHORT_ERR;
                max_count = (uint32_t) ((uint32_t *) dst_end_ptr - dst_32u_ptr);
                *count_ptr = count - max_count;
                CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, max_count);
                dst_32u_ptr = (uint32_t *) dst_end_ptr;
                break;
            }
            CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, count);
            dst_32u_ptr += count;
        }
    }
    *pp_dst = (uint8_t *) dst_32u_ptr;
    return status;
}

OWN_QPLC_FUN(qplc_status_t, qplc_rle_burst_16u8u, (const uint8_t *src1_ptr,
        uint32_t flag_be,
        const uint8_t *src2_ptr,
        uint32_t src_length,
        uint8_t **pp_dst,
        uint8_t *dst_end_ptr,
        uint32_t *count_ptr,
        uint32_t *index_ptr)) {
    qplc_status_t status        = QPLC_STS_OK;
    uint32_t      count;
    uint32_t      max_count;
    uint8_t       *dst_ptr      = *pp_dst;
    uint16_t      *src1_16u_ptr = (uint16_t *) src1_ptr;

    if (*count_ptr) {
        if ((dst_ptr + *count_ptr) > dst_end_ptr) {
            max_count = (uint32_t) (dst_end_ptr - dst_ptr);
            *count_ptr -= max_count;
            CALL_CORE_FUN(qplc_set_8u)(src2_ptr[*index_ptr], dst_ptr, max_count);
            *pp_dst = dst_end_ptr;
            return QPLC_STS_DST_IS_SHORT_ERR;
        } else {
            CALL_CORE_FUN(qplc_set_8u)(src2_ptr[*index_ptr], dst_ptr, *count_ptr);
            dst_ptr += *count_ptr;
            *count_ptr = 0u;
            (*index_ptr)++;
        }
    }
    for (; *index_ptr < src_length; (*index_ptr)++) {
        count = (flag_be) ? qplc_swap_bytes_16u(src1_16u_ptr[*index_ptr]) : src1_16u_ptr[*index_ptr];
        if (count) {
            if ((dst_ptr + count) > dst_end_ptr) {
                status    = QPLC_STS_DST_IS_SHORT_ERR;
                max_count = (uint32_t) (dst_end_ptr - dst_ptr);
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

OWN_QPLC_FUN(qplc_status_t, qplc_rle_burst_16u, (const uint8_t *src1_ptr,
        uint32_t flag_be,
        const uint8_t *src2_ptr,
        uint32_t src_length,
        uint8_t **pp_dst,
        uint8_t *dst_end_ptr,
        uint32_t *count_ptr,
        uint32_t *index_ptr)) {
    qplc_status_t status        = QPLC_STS_OK;
    uint32_t      count;
    uint32_t      max_count;
    uint16_t      *dst_16u_ptr  = (uint16_t *) *pp_dst;
    uint16_t      *src2_16u_ptr = (uint16_t *) src2_ptr;
    uint16_t      *src1_16u_ptr = (uint16_t *) src1_ptr;

    if (*count_ptr) {
        if ((dst_16u_ptr + *count_ptr) > (uint16_t *) dst_end_ptr) {
            max_count = (uint32_t) ((uint16_t *) dst_end_ptr - dst_16u_ptr);
            *count_ptr -= max_count;
            CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, max_count);
            *pp_dst = dst_end_ptr;
            return QPLC_STS_DST_IS_SHORT_ERR;
        } else {
            CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, *count_ptr);
            dst_16u_ptr += *count_ptr;
            *count_ptr = 0u;
            (*index_ptr)++;
        }
    }
    for (; *index_ptr < src_length; (*index_ptr)++) {
        count = (flag_be) ? qplc_swap_bytes_16u(src1_16u_ptr[*index_ptr]) : src1_16u_ptr[*index_ptr];
        if (count) {
            if ((dst_16u_ptr + count) > (uint16_t *) dst_end_ptr) {
                status    = QPLC_STS_DST_IS_SHORT_ERR;
                max_count = (uint32_t) ((uint16_t *) dst_end_ptr - dst_16u_ptr);
                *count_ptr = count - max_count;
                CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, max_count);
                dst_16u_ptr = (uint16_t *) dst_end_ptr;
                break;
            }
            CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, count);
            dst_16u_ptr += count;
        }
    }
    *pp_dst = (uint8_t *) dst_16u_ptr;
    return status;
}

OWN_QPLC_FUN(qplc_status_t, qplc_rle_burst_16u32u, (const uint8_t *src1_ptr,
        uint32_t flag_be,
        const uint8_t *src2_ptr,
        uint32_t src_length,
        uint8_t **pp_dst,
        uint8_t *dst_end_ptr,
        uint32_t *count_ptr,
        uint32_t *index_ptr)) {
    qplc_status_t status        = QPLC_STS_OK;
    uint32_t      count;
    uint32_t      max_count;
    uint32_t      *dst_32u_ptr  = (uint32_t *) *pp_dst;
    uint32_t      *src2_32u_ptr = (uint32_t *) src2_ptr;
    uint16_t      *src1_16u_ptr = (uint16_t *) src1_ptr;

    if (*count_ptr) {
        if ((dst_32u_ptr + *count_ptr) > (uint32_t *) dst_end_ptr) {
            max_count = (uint32_t) ((uint32_t *) dst_end_ptr - dst_32u_ptr);
            *count_ptr -= max_count;
            CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, max_count);
            *pp_dst = dst_end_ptr;
            return QPLC_STS_DST_IS_SHORT_ERR;
        } else {
            CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, *count_ptr);
            dst_32u_ptr += *count_ptr;
            *count_ptr = 0u;
            (*index_ptr)++;
        }
    }
    for (; *index_ptr < src_length; (*index_ptr)++) {
        count = (flag_be) ? qplc_swap_bytes_16u(src1_16u_ptr[*index_ptr]) : src1_16u_ptr[*index_ptr];
        if (count) {
            if ((dst_32u_ptr + count) > (uint32_t *) dst_end_ptr) {
                status    = QPLC_STS_DST_IS_SHORT_ERR;
                max_count = (uint32_t) ((uint32_t *) dst_end_ptr - dst_32u_ptr);
                *count_ptr = count - max_count;
                CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, max_count);
                dst_32u_ptr = (uint32_t *) dst_end_ptr;
                break;
            }
            CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, count);
            dst_32u_ptr += count;
        }
    }
    *pp_dst = (uint8_t *) dst_32u_ptr;
    return status;
}

OWN_QPLC_FUN(qplc_status_t, qplc_rle_burst_32u8u, (const uint8_t *src1_ptr,
        uint32_t flag_be,
        const uint8_t *src2_ptr,
        uint32_t src_length,
        uint8_t **pp_dst,
        uint8_t *dst_end_ptr,
        uint32_t *count_ptr,
        uint32_t *index_ptr)) {
    qplc_status_t status        = QPLC_STS_OK;
    uint32_t      count;
    uint32_t      count_current;
    uint32_t      count_next;
    uint32_t      max_count;
    uint8_t       *dst_ptr      = *pp_dst;
    uint32_t      *src1_32u_ptr = (uint32_t *) src1_ptr;

    if (*count_ptr) {
        if ((dst_ptr + *count_ptr) > dst_end_ptr) {
            max_count = (uint32_t) (dst_end_ptr - dst_ptr);
            *count_ptr -= max_count;
            CALL_CORE_FUN(qplc_set_8u)(src2_ptr[*index_ptr], dst_ptr, max_count);
            *pp_dst = dst_end_ptr;
            return QPLC_STS_DST_IS_SHORT_ERR;
        } else {
            CALL_CORE_FUN(qplc_set_8u)(src2_ptr[*index_ptr], dst_ptr, *count_ptr);
            dst_ptr += *count_ptr;
            *count_ptr = 0u;
            (*index_ptr)++;
        }
    }
    count_current = (flag_be) ? qplc_swap_bytes_32u(src1_32u_ptr[*index_ptr]) : src1_32u_ptr[*index_ptr];
    for (; *index_ptr < src_length; (*index_ptr)++) {
        count_next = (flag_be) ? qplc_swap_bytes_32u(src1_32u_ptr[*index_ptr + 1u]) : src1_32u_ptr[*index_ptr + 1u];
        count      = count_next - count_current;
        OWN_RETURN_ERROR((OWN_RLE_BURST_MAX_COUNT < count), QPLC_STS_INVALID_RLE_COUNT);
        count_current = count_next;
        if (count) {
            if ((dst_ptr + count) > dst_end_ptr) {
                status    = QPLC_STS_DST_IS_SHORT_ERR;
                max_count = (uint32_t) (dst_end_ptr - dst_ptr);
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

OWN_QPLC_FUN(qplc_status_t, qplc_rle_burst_32u16u, (const uint8_t *src1_ptr,
        uint32_t flag_be,
        const uint8_t *src2_ptr,
        uint32_t src_length,
        uint8_t **pp_dst,
        uint8_t *dst_end_ptr,
        uint32_t *count_ptr,
        uint32_t *index_ptr)) {
    qplc_status_t status        = QPLC_STS_OK;
    uint32_t      count;
    uint32_t      count_current;
    uint32_t      count_next;
    uint32_t      max_count;
    uint16_t      *dst_16u_ptr  = (uint16_t *) *pp_dst;
    uint16_t      *src2_16u_ptr = (uint16_t *) src2_ptr;
    uint32_t      *src1_32u_ptr = (uint32_t *) src1_ptr;

    if (*count_ptr) {
        if ((dst_16u_ptr + *count_ptr) > (uint16_t *) dst_end_ptr) {
            max_count = (uint32_t) ((uint16_t *) dst_end_ptr - dst_16u_ptr);
            *count_ptr -= max_count;
            CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, max_count);
            *pp_dst = dst_end_ptr;
            return QPLC_STS_DST_IS_SHORT_ERR;
        } else {
            CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, *count_ptr);
            dst_16u_ptr += *count_ptr;
            *count_ptr = 0u;
            (*index_ptr)++;
        }
    }
    count_current = (flag_be) ? qplc_swap_bytes_32u(src1_32u_ptr[*index_ptr]) : src1_32u_ptr[*index_ptr];
    for (; *index_ptr < src_length; (*index_ptr)++) {
        count_next = (flag_be) ? qplc_swap_bytes_32u(src1_32u_ptr[*index_ptr + 1u]) : src1_32u_ptr[*index_ptr + 1u];
        count      = count_next - count_current;
        OWN_RETURN_ERROR((OWN_RLE_BURST_MAX_COUNT < count), QPLC_STS_INVALID_RLE_COUNT);
        count_current = count_next;
        if (count) {
            if ((dst_16u_ptr + count) > (uint16_t *) dst_end_ptr) {
                status    = QPLC_STS_DST_IS_SHORT_ERR;
                max_count = (uint32_t) ((uint16_t *) dst_end_ptr - dst_16u_ptr);
                *count_ptr = count - max_count;
                CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, max_count);
                dst_16u_ptr = (uint16_t *) dst_end_ptr;
                break;
            }
            CALL_CORE_FUN(qplc_set_16u)(src2_16u_ptr[*index_ptr], dst_16u_ptr, count);
            dst_16u_ptr += count;
        }
    }
    *pp_dst = (uint8_t *) dst_16u_ptr;
    return status;
}

OWN_QPLC_FUN(qplc_status_t, qplc_rle_burst_32u, (const uint8_t *src1_ptr,
        uint32_t flag_be,
        const uint8_t *src2_ptr,
        uint32_t src_length,
        uint8_t **pp_dst,
        uint8_t *dst_end_ptr,
        uint32_t *count_ptr,
        uint32_t *index_ptr)) {
    qplc_status_t status        = QPLC_STS_OK;
    uint32_t      count;
    uint32_t      count_current;
    uint32_t      count_next;
    uint32_t      max_count;
    uint32_t      *dst_32u_ptr  = (uint32_t *) *pp_dst;
    uint32_t      *src2_32u_ptr = (uint32_t *) src2_ptr;
    uint32_t      *src1_32u_ptr = (uint32_t *) src1_ptr;

    if (*count_ptr) {
        if ((dst_32u_ptr + *count_ptr) > (uint32_t *) dst_end_ptr) {
            max_count = (uint32_t) ((uint32_t *) dst_end_ptr - dst_32u_ptr);
            *count_ptr -= max_count;
            CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, max_count);
            *pp_dst = dst_end_ptr;
            return QPLC_STS_DST_IS_SHORT_ERR;
        } else {
            CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, *count_ptr);
            dst_32u_ptr += *count_ptr;
            *count_ptr = 0u;
            (*index_ptr)++;
        }
    }
    count_current = (flag_be) ? qplc_swap_bytes_32u(src1_32u_ptr[*index_ptr]) : src1_32u_ptr[*index_ptr];
    for (; *index_ptr < src_length; (*index_ptr)++) {
        count_next = (flag_be) ? qplc_swap_bytes_32u(src1_32u_ptr[*index_ptr + 1u]) : src1_32u_ptr[*index_ptr + 1u];
        count      = count_next - count_current;
        OWN_RETURN_ERROR((OWN_RLE_BURST_MAX_COUNT < count), QPLC_STS_INVALID_RLE_COUNT);
        count_current = count_next;
        if (count) {
            if ((dst_32u_ptr + count) > (uint32_t *) dst_end_ptr) {
                status    = QPLC_STS_DST_IS_SHORT_ERR;
                max_count = (uint32_t) ((uint32_t *) dst_end_ptr - dst_32u_ptr);
                *count_ptr = count - max_count;
                CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, max_count);
                dst_32u_ptr = (uint32_t *) dst_end_ptr;
                break;
            }
            CALL_CORE_FUN(qplc_set_32u)(src2_32u_ptr[*index_ptr], dst_32u_ptr, count);
            dst_32u_ptr += count;
        }
    }
    *pp_dst = (uint8_t *) dst_32u_ptr;
    return status;
}
