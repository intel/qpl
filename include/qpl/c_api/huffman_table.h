/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Job API (public C API)
 */

#ifndef QPL_HUFFMAN_TABLE_H_
#define QPL_HUFFMAN_TABLE_H_

#include <stdint.h>

#include "qpl/c_api/status.h"
#include "qpl/c_api/statistics.h"
#include "qpl/c_api/serialization.h"
#include "qpl/c_api/triplet.h"

/**
 * @defgroup HUFFMAN_TABLE_API Huffman Table API
 * @ingroup JOB_API
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Huffman table::defs
 * @{
 */

typedef struct qpl_huffman_table *qpl_huffman_table_t; /**< Unified compression/decompression table*/

/**
 * Describes allocator
 */
typedef struct {
    void *(*allocator)(size_t);  /**< Allocation function */
    void (*deallocator)(void *); /**< Deallocation function */
} allocator_t;

/**
 * @brief An specific type of unified compression/decompression table for data space control
 */
typedef enum {
    combined_table_type,      /**< @ref qpl_huffman_table_t contain both tables */
    compression_table_type,   /**< @ref qpl_huffman_table_t contain compression table only */
    decompression_table_type, /**< @ref qpl_huffman_table_t contain decompression table only */
} qpl_huffman_table_type_e;

#define DEFAULT_ALLOCATOR_C {malloc, free} /**< Allocator used in Intel QPL C API by default */

/** @} */

/* --------------------------------------------------------------------------------*/

/**
 * @name Huffman table::Lifetime API
 * @{
 */

/**
 * @brief Creates a @ref qpl_huffman_table_t object for deflate. Allocate and markup of internal structures
 *
 * @param[in] type       @ref qpl_huffman_table_type_e
 * @param[in] path       @ref qpl_path_t
 * @param[in] allocator  allocator that must be used
 * @param[out] table_ptr output parameter for created object
 *
 * @return status from @ref qpl_status
 */
qpl_status qpl_deflate_huffman_table_create(const qpl_huffman_table_type_e type,
                                            const qpl_path_t path,
                                            const allocator_t allocator,
                                            qpl_huffman_table_t *table_ptr);

/**
 * @brief Creates a @ref qpl_huffman_table_t object for Huffman Only. Allocate and markup of internal structures
 *
 * @param[in]  type      @ref qpl_huffman_table_type_e
 * @param[in]  path      @ref qpl_path_t
 * @param[in]  allocator allocator that must be used
 * @param[out] table_ptr output parameter for created object
 *
 * @return status from @ref qpl_status
 */
qpl_status qpl_huffman_only_table_create(const qpl_huffman_table_type_e type,
                                         const qpl_path_t path,
                                         const allocator_t allocator,
                                         qpl_huffman_table_t *table_ptr);

/**
 * @brief Destroy an @ref qpl_huffman_table_t object. Deallocates internal structures
 *
 * @param[in,out] table  @ref qpl_huffman_table_t object to destroy
 */
void qpl_huffman_table_destroy(qpl_huffman_table_t table);

/** @} */

/* --------------------------------------------------------------------------------*/

/**
 * @name Huffman table::Initialization API
 * @{
 */

/**
 * @brief Initializes huffman table with provided histogram
 *
 * @param[in,out] table      @ref qpl_huffman_table_t object to init
 * @param[in] histogram_ptr  source statistics
 *
 * @return status from @ref qpl_status
 */
qpl_status qpl_huffman_table_init(qpl_huffman_table_t table,
                                  const qpl_histogram *const histogram_ptr);

/**
 * @brief Initializes huffman table with provided triplets
 *
 * @param[in,out] table @ref qpl_huffman_table_t object to init
 * @param[in] triplet_ptr   user defined triplet huffman codes
 * @param[in] triplet_count huffman codes count
 *
 * @return status from @ref qpl_status
 */
qpl_status qpl_huffman_table_init_with_triplet(qpl_huffman_table_t table,
                                               const qpl_huffman_triplet *const triplet_ptr,
                                               const uint32_t triplet_count);

/**
 * @brief Initializes huffman table with information from another table
 *
 * @param[in,out] table @ref qpl_huffman_table_t object to init
 * @param[in] other base @ref qpl_huffman_table_t object
 *
 * @return status from @ref qpl_status
 */
qpl_status qpl_huffman_table_init_with_other(qpl_huffman_table_t table,
                                             const qpl_huffman_table_t other);

/** @} */

/* --------------------------------------------------------------------------------*/

/**
 * @name Huffman table::Service API
 * @{
 */

/**
 * @brief Gets type of @ref qpl_huffman_table_t
 *
 * @param[in]  table    source @ref qpl_huffman_table_t object
 * @param[out] type_ptr output parameter for type
 *
 * @return status from @ref qpl_status
 */
qpl_status qpl_huffman_table_get_type(const qpl_huffman_table_t table,
                                      qpl_huffman_table_type_e *const type_ptr);

/** @} */

/* --------------------------------------------------------------------------------*/

/**
 * @name Huffman table::Serialization API
 * @{
 * @warning API is introduced only. Implementation is in progress.
 */

/**
 * @brief To keep user space on filesystem we can serialize and pack qpl_huffman_table_t into more compact form
 */

/**
 * @brief Gets size to serialize table
 * @warning API is introduced only. Implementation is in progress.
 *
 * @param[in]  table    @ref qpl_huffman_table_t object to serialize
 * @param[in]  options  serialization_options
 * @param[out] size_ptr output parameter for size
 *
 * @return status from @ref qpl_status
 */
qpl_status qpl_huffman_table_get_serialize_size(const qpl_huffman_table_t table,
                                                const serialization_options_t options,
                                                size_t *const size_ptr);

/**
 * @brief Serializes qpl_huffman_table_t into unusable but compact format
 * @warning API is introduced only. Implementation is in progress.
 *
 * @param[in] table @ref qpl_huffman_table_t object to serialize
 * @param[out] dump_buffer_ptr serialized object buffer
 * @param[in] dump_buffer_size serialized object buffer size
 * @param[in] options serialization_options
 *
 * @return status from @ref qpl_status
 */
qpl_status qpl_huffman_table_serialize(const qpl_huffman_table_t table,
                                       uint8_t *const dump_buffer_ptr,
                                       const size_t dump_buffer_size,
                                       const serialization_options_t options);

/**
 * @brief Deserializes previously serialized huffman table
 * @warning API is introduced only. Implementation is in progress.
 *
 * @param[in] dump_buffer_ptr serialized object buffer
 * @param[in] dump_buffer_size serialized object buffer size
 * @param[in] allocator allocator that must be used
 * @param[out] table_ptr output parameter for created object
 *
 * @return status from @ref qpl_status
 */
qpl_status qpl_huffman_table_deserialize(const uint8_t *const dump_buffer_ptr,
                                         const size_t dump_buffer_size,
                                         allocator_t allocator,
                                         qpl_huffman_table_t *table_ptr);

/** @} */

#ifdef __cplusplus
}
#endif

/** @} */

#endif //QPL_HUFFMAN_TABLE_H_
