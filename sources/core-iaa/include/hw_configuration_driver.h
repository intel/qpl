/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_HW_PATH_INCLUDE_HW_CONFIGURATION_DRIVER_H_
#define QPL_SOURCES_HW_PATH_INCLUDE_HW_CONFIGURATION_DRIVER_H_

#include "hw_definitions.h"
#include "hw_devices.h"

#if defined( linux )

#include "libaccel_config.h"

#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Type of function that should be loaded from accelerator configuration driver
 */
typedef void *library_function;

/**
 * @brief Structure that maps function implementation to its name
 */
typedef struct {
    library_function function;          /**< Function address */
    const char       *function_name;    /**< Function name */
} qpl_desc_t;

/**
 * @brief Structure represents configuration driver used for access to accelerator instances and their properties
 */
typedef struct {
    void *driver_instance_ptr; /**< Pointer to a loaded driver */
} hw_driver_t;

/**
 * @brief Initializes driver functions
 *
 * @note Should be called only once
 *
 * @return status of initialization
 */
HW_PATH_GENERAL_API(hw_accelerator_status, initialize_accelerator_driver, (hw_driver_t *driver_ptr));

HW_PATH_GENERAL_API(void, finalize_accelerator_driver, (hw_driver_t *driver_ptr));

HW_PATH_GENERAL_API(int32_t, driver_new_context, (accfg_ctx **ctx));

HW_PATH_GENERAL_API(accfg_dev *, context_get_first_device,(accfg_ctx *ctx));

HW_PATH_GENERAL_API(accfg_ctx *, context_close, (accfg_ctx *ctx));

HW_PATH_GENERAL_API( const char *,  device_get_name, (accfg_dev *device));

HW_PATH_GENERAL_API(accfg_dev *, device_get_next, (accfg_dev *device));

HW_PATH_GENERAL_API(enum accfg_device_state , device_get_state, (accfg_dev *device));

HW_PATH_GENERAL_API(uint64_t, device_get_gen_cap_register, (accfg_dev *device));

HW_PATH_GENERAL_API(uint64_t, device_get_numa_node, (accfg_dev *device));

HW_PATH_GENERAL_API(accfg_wq *, get_first_work_queue, (accfg_dev *device));

HW_PATH_GENERAL_API(accfg_wq *, work_queue_get_next,(accfg_wq *wq));

HW_PATH_GENERAL_API(int32_t, work_queue_get_id, (accfg_wq *wq));

HW_PATH_GENERAL_API(int32_t, work_queue_get_priority, (accfg_wq *wq));

HW_PATH_GENERAL_API(enum accfg_wq_state, work_queue_get_state, (accfg_wq *wq));

HW_PATH_GENERAL_API(enum accfg_wq_mode, work_queue_get_mode,(accfg_wq *wq));

HW_PATH_GENERAL_API (int, work_queue_get_device_path, (accfg_wq *wq, char *buf, size_t size));

HW_PATH_GENERAL_API (const char *, work_queue_get_device_name, (accfg_wq *wq));

HW_PATH_GENERAL_API (unsigned int,  device_get_version, (accfg_dev *device));

HW_PATH_GENERAL_API (int,  work_queue_get_block_on_fault, (accfg_wq *wq));

#ifdef __cplusplus
}
#endif

#endif //QPL_SOURCES_HW_PATH_INCLUDE_HW_CONFIGURATION_DRIVER_H_
