/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Job API (public C API)
 */

#ifndef QPL_SERIALIZATION_H_
#define QPL_SERIALIZATION_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup SERIALIZATION_API Serialization API
 * @ingroup JOB_API
 * @{
 */

/**
 * Describes how to perform serialization
 */
typedef enum {
    serialization_compact,   /**< Use format to keep save on the disk */
    serialization_raw,       /**< Use format to keep speed of serialisation/deserialization in case high load */
} qpl_serialization_format_e;

typedef uint64_t serialization_flags_t; /**< Type of serialization flags */

/**
 * @brief Describes serialization options
 */
typedef struct {
    qpl_serialization_format_e format;  /**< Needed format of serialized object */
    serialization_flags_t flags;        /**< Advanced serialization options */
} serialization_options_t;

#define DEFAULT_SERIALIZATION_OPTIONS {serialization_raw, 0} /**< Default serialization options */

/** @} */

#ifdef __cplusplus
}
#endif

#endif //QPL_SERIALIZATION_H_
