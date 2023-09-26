/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TOOLS_UTILS_COMMON_HW_CONFIGURATION_DRIVER_H_
#define QPL_TOOLS_UTILS_COMMON_HW_CONFIGURATION_DRIVER_H_

#if defined( __linux__ )
#include "hw_dispatcher/qpl_test_libaccel_config.h"

#ifdef DYNAMIC_LOADING_LIBACCEL_CONFIG

#include "stdbool.h"
#include "hw_dispatcher/hw_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Type of function that should be loaded from accelerator configuration driver
 */
typedef void *qpl_test_library_function;

/**
 * @brief Structure that maps function implementation to its name
 */
typedef struct {
    qpl_test_library_function function;          /**< Function address */
    const char                *function_name;    /**< Function name */
} qpl_test_desc_t;

/**
 * @brief Structure represents configuration driver used for access to accelerator instances and their properties
 */
typedef struct {
    void *driver_instance_ptr; /**< Pointer to a loaded driver */
} qpl_test_hw_driver_t;

typedef struct accfg_ctx    accfg_ctx;
typedef struct accfg_device accfg_dev;

/**
 * @brief Initializes driver functions
 *
 * @note Should be called only once
 *
 * @return status of initialization
 */
qpl_test_hw_accelerator_status qpl_test_hw_initialize_accelerator_driver(qpl_test_hw_driver_t *driver_ptr);

void qpl_test_hw_finalize_accelerator_driver(qpl_test_hw_driver_t *driver_ptr);

/**
 * accel-config APIs
 */
typedef int                     (*accfg_new_ptr)(accfg_ctx **ctx);

typedef accfg_dev *             (*accfg_device_get_first_ptr)(accfg_ctx *ctx);

typedef const char *            (*accfg_device_get_devname_ptr)(accfg_dev *device);

typedef accfg_dev *             (*accfg_device_get_next_ptr)(accfg_dev *device);

typedef enum accfg_device_state (*accfg_device_get_state_ptr)(accfg_dev *device);

typedef accfg_ctx *             (*accfg_unref_ptr)(accfg_ctx *ctx);

typedef unsigned int            (*accfg_device_get_version_ptr)(accfg_dev *device);

typedef int                     (*accfg_device_get_iaa_cap_ptr)(accfg_dev *device, uint64_t *iaa_cap);

#else  //DYNAMIC_LOADING_LIBACCEL_CONFIG=OFF
#ifdef __cplusplus
extern "C" {
#endif

typedef struct accfg_ctx    accfg_ctx;
typedef struct accfg_device accfg_dev;

#endif //DYNAMIC_LOADING_LIBACCEL_CONFIG=ON

/**
 * accel-config APIs with QPL wrappers
 */
int32_t qpl_test_accfg_new(accfg_ctx **ctx);

accfg_dev *qpl_test_accfg_device_get_first(accfg_ctx *ctx);

const char *qpl_test_accfg_device_get_devname(accfg_dev *device);

accfg_dev *qpl_test_accfg_device_get_next(accfg_dev *device);

enum accfg_device_state qpl_test_accfg_device_get_state(accfg_dev *device);

accfg_ctx *qpl_test_accfg_unref(accfg_ctx *ctx);

unsigned int qpl_test_accfg_device_get_version(accfg_dev *device);

int qpl_test_accfg_device_get_iaa_cap(struct accfg_device *device, uint64_t *iaa_cap);

#ifdef __cplusplus
}
#endif


#endif //__linux__
#endif //QPL_TOOLS_UTILS_COMMON_HW_CONFIGURATION_DRIVER_H_
