/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @brief Contains API to work with Intel® In-Memory Analytics Accelerator (Intel® IAA) Devices.
 *
 * @details @todo
 *
 * @defgroup HW_DEVICES_API Devices API
 * @ingroup HW_PUBLIC_API
 * @{
 */

#ifndef HW_PATH_HW_DEVICES_H_
#define HW_PATH_HW_DEVICES_H_

#include "stdbool.h"
#include <inttypes.h>

#ifdef LOG_HW_INIT

#include <stdio.h>
#define DIAGA(...) printf(__VA_ARGS__); fflush(stdout)                  /**< Diagnostic printer for appending to line */
#define DIAG(...) printf("qpl-diag: " __VA_ARGS__); fflush(stdout)      /**< Diagnostic printer */
#else
#define DIAGA(...)                                                      /**< Diagnostic printer for appending to line */
#define DIAG(...)                                                       /**< Diagnostic printer */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ====== Definitions ====== */

/**
 * @todo
 */
#define IAA_DEVICE ((uint32_t)(((uint32_t)0xFF << 24U) | \
                   ((uint32_t)('x') << 16U) | ((uint32_t)('a') << 8U) | (uint32_t)('i')))
#define MAX_NUM_DEV 100U        /**< @todo */
#define MAX_NUM_WQ  100U        /**< @todo */
#define CHAR_MSK    0xFF202020  /**< @todo */

#define OWN_PAGE_MASK             0x0FFFLLU     /**< Defines page mask for portal incrementing */

/* ====== Macros ====== */
/**
 * @name Gencap Configuration Macros
 * @anchor HW_GENCAP_MACROS
 * @todo
 * @{
 */
#define GC_BLOCK_ON_FAULT(GENCAP)           (((GENCAP))    &0x01)       /**< GENCAP bit 0      - block on fault support                    */
#define GC_OVERLAPPING(GENCAP)              (((GENCAP)>>1) &0x01)       /**< GENCAP bit 1      - overlapping copy support                  */
#define GC_CACHE_WRITE(GENCAP)              (((GENCAP)>>2) &0x01)       /**< GENCAP bit 2      - cache control support (memory)            */
#define GC_CACHE_FLUSH(GENCAP)              (((GENCAP)>>3) &0x01)       /**< GENCAP bit 3      - cache control support (cache flush)       */
#define GC_COM_CAP(GENCAP)                  (((GENCAP)>>4) &0x01)       /**< GENCAP bit 4      - command capabilities support              */
#define GC_DST_READBACK(GENCAP)             (((GENCAP)>>8) &0x01)       /**< GENCAP bit 8      - destination readback support              */
#define GC_DRAIN_READBACK(GENCAP)           (((GENCAP)>>9) &0x01)       /**< GENCAP bit 9      - drain descriptor readback address support */
#define GC_MAX_TRANSFER_SIZE(GENCAP)  (1 << (((GENCAP)>>16)&0x1F))      /**< GENCAP 20-16 bits - maximum supported transfer size           */
#define GC_INTERRUPT_STORAGE(GENCAP)       ((((GENCAP)>>25)&0x3F)*256U) /**< GENCAP 30-25 bits - interrupt message storage size            */
#define GC_CONF_SUPPORT(GENCAP)             (((GENCAP)>>31)&0x01)       /**< GENCAP bit 31     - configuration support                     */
#define GC_DECOMP_SUPPORT(GENCAP)           (((GENCAP)>>40)&0x01)       /**< GENCAP bit 40     - decompression support                     */
#define GC_IDX_SUPPORT(GENCAP)              (((GENCAP)>>41)&0x01)       /**< GENCAP bit 41     - indexing support                          */
#define GC_MAX_DECOMP_SET_SIZE(GENCAP)     ((((GENCAP)>>42)&0x1F) + 1U) /**< GENCAP 46-42 bits - maximum decompression set size            */
#define GC_MAX_SET_SIZE(GENCAP)            ((((GENCAP)>>47)&0x1F) + 1U) /**< GENCAP 51-47 bits - maximum set size                          */

/** @} */

/**
 * @name Iaacap Configuration Macros
 * @anchor HW_IAACAP_MACROS
 * @{
 */
#define IC_GEN_2_MIN_CAP(IAACAP)            (((IAACAP))    &0x01)       /**< IAACAP bit 0      - generation 2 Minimum Capabilities support */
#define IC_FORCE_ARRAY(IAACAP)              (((IAACAP)>>3) &0x01)       /**< IAACAP bit 3      - force array output modification support   */
#define IC_LOAD_PARTIAL_AECS(IAACAP)        (((IAACAP)>>4) &0x01)       /**< IAACAP bit 4      - load partial AECS support                 */
#define IC_DICT_COMP(IAACAP)                (((IAACAP)>>7) &0x01)       /**< IAACAP bit 7      - dictionary compression support            */
#define IC_HEADER_GEN(IAACAP)               (((IAACAP)>>8) &0x01)       /**< IAACAP bit 8      - header Generation support                 */

/** @} */

/**
 * @name Opconfig Configuration Macros
 * @anchor HW_OPCFG_MACROS
 * @{
 */
#define OC_GET_OP_SUPPORTED(OPCFG, OP)      (((OPCFG[7 - (OP/32)])>>OP%32) &0x01) /**< OPCFG bit {OP}  - generic operation support          */

/** @} */

/* ====== Structures ====== */

/**
 * @todo
 */
typedef struct {
    uint32_t max_set_size;                                   /**< @todo */
    uint32_t max_decompressed_set_size;                      /**< @todo */
    uint32_t max_transfer_size;                              /**< @todo */
    bool     cache_flush_available;                          /**< @todo */
    bool     cache_write_available;                          /**< @todo */
    bool     overlapping_available;                          /**< @todo */
    bool     indexing_support_enabled;                       /**< @todo */
    bool     decompression_support_enabled;                  /**< @todo */
    bool     block_on_fault_enabled;                         /**< @todo */
    bool     gen_2_min_capabilities_available;               /**< @todo */
    bool     header_gen_supported;                           /**< @todo */
    bool     dict_compression_supported;                     /**< @todo */
    bool     load_partial_aecs_supported;                    /**< @todo */
    bool     force_array_output_mod_available;               /**< @todo */
} hw_device_properties;


/* ====== Functions ====== */

#if defined( __linux__ )
typedef struct accfg_ctx    accfg_ctx;   /**< @todo */
typedef struct accfg_device accfg_dev;   /**< @todo */
typedef struct accfg_group  accfgGrp;    /**< @todo */
typedef struct accfg_wq     accfg_wq;    /**< @todo */
typedef struct accfg_engine accfgEngn;   /**< @todo */
#endif

#if defined( __linux__ )
typedef accfg_ctx  hw_context; /**< Linux defined context type */
#else
typedef void       hw_context; /**< Windows defined context type */
#endif

/**
 * @todo
 */
typedef struct {
    hw_device_properties  device_properties;   /**< Accelerator properties */
    hw_context           *ctx_ptr;             /**< Hardware context instance */
} hw_accelerator_context;

/**
 * @brief Structure for HW descriptor submit options
 */
typedef struct {
    uint64_t submit_flags;  /**< @todo */
    int32_t numa_id;        /**< ID of the NUMA. Set it to -1 for auto detecting */
} hw_accelerator_submit_options;

#ifdef __cplusplus
}
#endif

#endif //HW_PATH_HW_DEVICES_H_

/** @} */
