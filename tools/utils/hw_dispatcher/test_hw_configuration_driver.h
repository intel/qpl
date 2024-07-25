/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TOOLS_UTILS_HW_DISPATCHER_TEST_HW_CONFIGURATION_DRIVER_H_
#define QPL_TOOLS_UTILS_HW_DISPATCHER_TEST_HW_CONFIGURATION_DRIVER_H_

#if defined(__linux__)
#include "qpl_test_libaccel_config.h"

#ifdef DYNAMIC_LOADING_LIBACCEL_CONFIG

#include "stdbool.h"
#include "test_hw_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Type of function that should be loaded from accelerator configuration driver
 */
typedef void* qpl_test_library_function;

/**
 * @brief Structure that maps function implementation to its name
 */
typedef struct {
    qpl_test_library_function function;      /**< Function address */
    const char*               function_name; /**< Function name */
} qpl_test_desc_t;

/**
 * @brief Structure represents configuration driver used for access to accelerator instances and their properties
 */
typedef struct {
    void* driver_instance_ptr; /**< Pointer to a loaded driver */
} qpl_test_hw_driver_t;

typedef struct accfg_ctx    accfg_ctx;
typedef struct accfg_device accfg_dev;
typedef struct accfg_wq     accfg_wq;
typedef struct accfg_engine accfg_eng;

typedef struct accfg_op_config accfg_opcfg;

/**
 * @brief Initializes driver functions
 *
 * @note Should be called only once
 *
 * @return status of initialization
 */
qpl_test_hw_accelerator_status qpl_test_hw_initialize_accelerator_driver(qpl_test_hw_driver_t* driver_ptr);

void qpl_test_hw_finalize_accelerator_driver(qpl_test_hw_driver_t* driver_ptr);

/**
 * accel-config APIs
 */
typedef accfg_ctx* (*accfg_unref_ptr)(accfg_ctx* ctx);

typedef int (*accfg_new_ptr)(accfg_ctx** ctx);

typedef accfg_dev* (*accfg_device_get_first_ptr)(accfg_ctx* ctx);

typedef accfg_dev* (*accfg_device_get_next_ptr)(accfg_dev* device);

typedef const char* (*accfg_device_get_devname_ptr)(accfg_dev* device);

typedef int (*accfg_device_get_numa_node_ptr)(accfg_dev* device);

typedef enum accfg_device_state (*accfg_device_get_state_ptr)(accfg_dev* device);

typedef unsigned int (*accfg_device_get_version_ptr)(accfg_dev* device);

typedef int (*accfg_device_get_iaa_cap_ptr)(accfg_dev* device, uint64_t* iaa_cap);

typedef accfg_wq* (*accfg_wq_get_first_ptr)(accfg_dev* device);

typedef accfg_wq* (*accfg_wq_get_next_ptr)(accfg_wq* wq);

typedef enum accfg_wq_mode (*accfg_wq_get_mode_ptr)(accfg_wq* wq);

typedef uint64_t (*accfg_wq_get_size_ptr)(accfg_wq* wq);

typedef int (*accfg_wq_get_priority_ptr)(accfg_wq* wq);

typedef enum accfg_wq_state (*accfg_wq_get_state_ptr)(accfg_wq* wq);

typedef int (*accfg_wq_get_user_dev_path_ptr)(accfg_wq* wq, char* buf, size_t size);

typedef int (*accfg_wq_get_op_config_ptr)(accfg_wq* wq, accfg_opcfg* op_config);

typedef accfg_eng* (*accfg_engine_get_first_ptr)(accfg_dev* device);

typedef accfg_eng* (*accfg_engine_get_next_ptr)(accfg_eng* engine);

typedef int (*accfg_engine_get_group_id_ptr)(accfg_eng* engine);

#else //DYNAMIC_LOADING_LIBACCEL_CONFIG=OFF
#ifdef __cplusplus
extern "C" {
#endif

typedef struct accfg_ctx    accfg_ctx;
typedef struct accfg_device accfg_dev;
typedef struct accfg_wq     accfg_wq;
typedef struct accfg_engine accfg_eng;

typedef struct accfg_op_config accfg_opcfg;

#endif //DYNAMIC_LOADING_LIBACCEL_CONFIG=ON

/**
 * accel-config APIs with Intel® Query Processing Library (Intel® QPL) wrappers
 */
accfg_ctx* qpl_test_accfg_unref(accfg_ctx* ctx);

int32_t qpl_test_accfg_new(accfg_ctx** ctx);

accfg_dev* qpl_test_accfg_device_get_first(accfg_ctx* ctx);

accfg_dev* qpl_test_accfg_device_get_next(accfg_dev* device);

const char* qpl_test_accfg_device_get_devname(accfg_dev* device);

int qpl_test_accfg_device_get_numa_node(accfg_dev* device);

enum accfg_device_state qpl_test_accfg_device_get_state(accfg_dev* device);

unsigned int qpl_test_accfg_device_get_version(accfg_dev* device);

int qpl_test_accfg_device_get_iaa_cap(accfg_dev* device, uint64_t* iaa_cap);

accfg_wq* qpl_test_accfg_wq_get_first(accfg_dev* device);

accfg_wq* qpl_test_accfg_wq_get_next(accfg_wq* wq);

enum accfg_wq_mode qpl_test_accfg_wq_get_mode(accfg_wq* wq);

uint64_t qpl_test_accfg_wq_get_size(accfg_wq* wq);

int qpl_test_accfg_wq_get_priority(accfg_wq* wq);

enum accfg_wq_state qpl_test_accfg_wq_get_state(accfg_wq* wq);

int qpl_test_accfg_wq_get_user_dev_path(accfg_wq* wq, char* buf, size_t size);

int qpl_test_accfg_wq_get_op_config(accfg_wq* wq, accfg_opcfg* op_config);

accfg_eng* qpl_test_accfg_engine_get_first(accfg_dev* device);

accfg_eng* qpl_test_accfg_engine_get_next(accfg_eng* engine);

int qpl_test_accfg_engine_get_group_id(accfg_eng* engine);

#ifdef __cplusplus
}
#endif

#endif //__linux__
#endif //QPL_TOOLS_UTILS_HW_DISPATCHER_TEST_HW_CONFIGURATION_DRIVER_H_
