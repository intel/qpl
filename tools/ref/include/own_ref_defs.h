/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Reference library
 */

/**
 * @defgroup REFERENCE Reference Code
 *
 * @defgroup REFERENCE_PUBLIC Public
 * @ingroup REFERENCE
 *
 * @defgroup REFERENCE_PRIVATE Private
 * @ingroup REFERENCE
 */

/**
 * @date 11/07/2018
 * @brief Internal Types and Macro Definitions for reference functionality
 *
 * @addtogroup REFERENCE_PUBLIC
 * @{
 */

#ifndef OWN_REF_DEFS_H__
#define OWN_REF_DEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "qpl/c_api/defs.h"
#include "qpl/c_api/status.h"

#define QPL_ONE_8U  (1U)   /**< 1 presented by unsigned 8-bit integer */
#define QPL_ONE_32U (1U)   /**< 1 presented by unsigned 16-bit integer */
#define QPL_ONE_64U (1ULL) /**< 1 presented by unsigned 64-bit integer */

#define REF_BYTE_BIT_LEN   8    /**< @todo */
#define REF_HIGH_BIT_MASK  0x80 /**< @todo */
#define REF_LOW_BIT_MASK   1    /**< @todo */
#define REF_MAX_BIT_IDX    7    /**< @todo */
#define REF_BIT_LEN_2_BYTE 3    /**< @todo */
#define REF_8U_BITS        8    /**< @todo */
#define REF_16U_BITS       16   /**< @todo */
#define REF_32U_BITS       32   /**< @todo */
#define REF_7_BIT_MASK     0x7f /**< @todo */
#define REF_5_BIT_MASK     0x1f /**< @todo */

#define REF_OCTA_GROUP_SIZE 8U  /**< @todo */
#define REF_MAX_BIT_WIDTH   32U /**< @todo */

#define REF_BIT_BUF_LEN      (sizeof(uint64_t) * REF_BYTE_BIT_LEN) /**< @todo */
#define REF_BIT_BUF_LEN_HALF (REF_BIT_BUF_LEN >> 1)                /**< @todo */

/**
 * @todo
 */
#define REF_CHECK_PTR_END(ptr, limit, upd, sts)          \
    {                                                    \
        if ((ptr) > ((limit) - (upd))) { return (sts); } \
    };

/**
 * @todo
 */
#define REF_CHECK_FUNC_STS(func)             \
    {                                        \
        qpl_status _s = func;                \
        if (QPL_STS_OK != _s) { return _s; } \
    }

#define QPL_MIN(a, b) (((a) < (b)) ? (a) : (b)) /**< Simple maximal value idiom */

/**
 * @todo
 */
#define REF_BIT_2_BYTE(x) (((x) + REF_MAX_BIT_IDX) >> REF_BIT_LEN_2_BYTE)

#if defined(__INTEL_COMPILER) || defined(_MSC_VER)
#define REF_INLINE static __inline /**< @todo */
#elif defined(__GNUC__)
#define REF_INLINE static __inline__
#else
#define REF_INLINE static /**< @todo */
#endif

/* Define NULL pointer value */
#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif
#endif

#if defined(_WIN32) || defined(_WIN64)
#define QPL_STDCALL
#else
#define REFCALL
#endif

#if !defined(REF_API)
#define REF_API(type, name, arg) type QPL_STDCALL name arg; /**< @todo */
#endif

#define REF_ERROR_RET(err_code) return (err_code) /**< @todo */

#ifdef REF_BAD_ARG_CHECK
#define REF_BAD_ARG_RET(expr, err_code)        \
    {                                          \
        if (expr) { REF_ERROR_RET(err_code); } \
    } /**< @todo */
#else
#define REF_BAD_ARG_RET(expr, err_code) /**< @todo */
#endif

#define REF_BAD_PTR_RET(ptr) REF_BAD_ARG_RET(NULL == (ptr), QPL_STS_NULL_PTR_ERR) /**< @todo */

/**
 * @todo
 */
#define REF_BAD_PTR2_RET(ptr1, ptr2) \
    {                                \
        REF_BAD_PTR_RET(ptr1);       \
        REF_BAD_PTR_RET(ptr2)        \
    }

/**
 * @todo
 */
#define REF_BAD_PTR3_RET(ptr1, ptr2, ptr3) \
    {                                      \
        REF_BAD_PTR2_RET(ptr1, ptr2);      \
        REF_BAD_PTR_RET(ptr3)              \
    }

/**
 * @todo
 */
#define REF_FREE_PTR(ptr)    \
    {                        \
        if (NULL != (ptr)) { \
            free(ptr);       \
            (ptr) = NULL;    \
        }                    \
    }

/**
 * @todo
 */
#define REF_FREE_PTR2(ptr1, ptr2) \
    {                             \
        REF_FREE_PTR(ptr1);       \
        REF_FREE_PTR(ptr2);       \
    }

/**
 * @todo
 */
#define REF_FREE_PTR3(ptr1, ptr2, ptr3) \
    {                                   \
        REF_FREE_PTR2(ptr1, ptr2);      \
        REF_FREE_PTR(ptr3);             \
    }

/**
 * @todo
 */
#define REF_BAD_SIZE_RET(n) REF_BAD_ARG_RET((n) <= 0, QPL_STS_SIZE_ERR)

/**
 * @todo
 */
#define REF_TERNARY(cond, tru, fal) ((cond) ? (tru) : (fal))

/**
 * @todo
 */
#define REF_FMT_2_BITS(format) \
    (REF_TERNARY((qpl_ow_8 == (format)), REF_8U_BITS, REF_TERNARY((qpl_ow_16 == (format)), REF_16U_BITS, REF_32U_BITS)))

/**
 * @todo
 */
#define REF_FMT_2_BYTES(format) (REF_BIT_2_BYTE(REF_FMT_2_BITS(format)))

#ifdef __cplusplus
}
#endif

#endif

/** @} */
