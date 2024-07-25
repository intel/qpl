/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 * HW configuration functions for tests
 */

#if defined(__linux__)
#include "test_hw_configuration_driver.h"

#ifdef DYNAMIC_LOADING_LIBACCEL_CONFIG
#include <dlfcn.h>
#include <fcntl.h>
#include <string.h> // strncmp
#include <sys/mman.h>

static const char* accelerator_configuration_driver_name = "libaccel-config.so.1";

/**
 * @brief Table with functions required from accelerator configuration library
 */
static qpl_test_desc_t qpl_test_functions_table[] = {{NULL, "accfg_unref"},
                                                     {NULL, "accfg_new"},
                                                     {NULL, "accfg_device_get_first"},
                                                     {NULL, "accfg_device_get_next"},
                                                     {NULL, "accfg_device_get_devname"},
                                                     {NULL, "accfg_device_get_numa_node"},
                                                     {NULL, "accfg_device_get_state"},
                                                     {NULL, "accfg_device_get_version"},
                                                     {NULL, "accfg_device_get_iaa_cap"},
                                                     {NULL, "accfg_wq_get_first"},
                                                     {NULL, "accfg_wq_get_next"},
                                                     {NULL, "accfg_wq_get_mode"},
                                                     {NULL, "accfg_wq_get_size"},
                                                     {NULL, "accfg_wq_get_priority"},
                                                     {NULL, "accfg_wq_get_state"},
                                                     {NULL, "accfg_wq_get_user_dev_path"},
                                                     {NULL, "accfg_wq_get_op_config"},
                                                     {NULL, "accfg_engine_get_first"},
                                                     {NULL, "accfg_engine_get_next"},
                                                     {NULL, "accfg_engine_get_group_id"},

                                                     // Terminate list/init
                                                     {NULL, NULL}};

static inline qpl_test_hw_accelerator_status
qpl_test_own_load_accelerator_configuration_driver(void** driver_instance_pptr);

static inline bool qpl_test_own_load_configuration_functions(void* driver_instance_ptr);

qpl_test_hw_accelerator_status qpl_test_hw_initialize_accelerator_driver(qpl_test_hw_driver_t* driver_ptr) {

    // Variables
    driver_ptr->driver_instance_ptr = NULL;

    // Load DLL
    qpl_test_hw_accelerator_status status =
            qpl_test_own_load_accelerator_configuration_driver(&driver_ptr->driver_instance_ptr);

    if (status || driver_ptr->driver_instance_ptr == NULL) {
        qpl_test_hw_finalize_accelerator_driver(driver_ptr);

        return QPL_TEST_HW_ACCELERATOR_LIBACCEL_NOT_FOUND;
    }

    // If DLL is loaded successfully
    if (!qpl_test_own_load_configuration_functions(driver_ptr->driver_instance_ptr)) {
        qpl_test_hw_finalize_accelerator_driver(driver_ptr);

        return QPL_TEST_HW_ACCELERATOR_LIBACCEL_NOT_FOUND;
    }

    return QPL_TEST_HW_ACCELERATOR_STATUS_OK;
}

void qpl_test_hw_finalize_accelerator_driver(qpl_test_hw_driver_t* driver_ptr) {
    if (driver_ptr->driver_instance_ptr) { dlclose(driver_ptr->driver_instance_ptr); }

    driver_ptr->driver_instance_ptr = NULL;
}

/*
 * QPL wrappers for accel-config APIs
 */
accfg_ctx* qpl_test_accfg_unref(accfg_ctx* ctx) {
    return ((accfg_unref_ptr)qpl_test_functions_table[0].function)(ctx);
}

int32_t qpl_test_accfg_new(accfg_ctx** ctx) {
    return ((accfg_new_ptr)qpl_test_functions_table[1].function)(ctx);
}

accfg_dev* qpl_test_accfg_device_get_first(accfg_ctx* ctx) {
    return ((accfg_device_get_first_ptr)qpl_test_functions_table[2].function)(ctx);
}

accfg_dev* qpl_test_accfg_device_get_next(accfg_dev* device) {
    return ((accfg_device_get_next_ptr)qpl_test_functions_table[3].function)(device);
}

const char* qpl_test_accfg_device_get_devname(accfg_dev* device) {
    return ((accfg_device_get_devname_ptr)qpl_test_functions_table[4].function)(device);
}

int qpl_test_accfg_device_get_numa_node(accfg_dev* device) {
    return ((accfg_device_get_numa_node_ptr)qpl_test_functions_table[5].function)(device);
}

enum accfg_device_state qpl_test_accfg_device_get_state(accfg_dev* device) {
    return ((accfg_device_get_state_ptr)qpl_test_functions_table[6].function)(device);
}

unsigned int qpl_test_accfg_device_get_version(accfg_dev* device) {
    return ((accfg_device_get_version_ptr)qpl_test_functions_table[7].function)(device);
}

// @todo this is a workaround to optionally load accfg_device_get_iaa_cap
int qpl_test_accfg_device_get_iaa_cap(accfg_dev* device, uint64_t* iaa_cap) {
    if (qpl_test_functions_table[8].function == NULL) return 1;
    return ((accfg_device_get_iaa_cap_ptr)qpl_test_functions_table[8].function)(device, iaa_cap);
}

accfg_wq* qpl_test_accfg_wq_get_first(accfg_dev* device) {
    return ((accfg_wq_get_first_ptr)qpl_test_functions_table[9].function)(device);
}

accfg_wq* qpl_test_accfg_wq_get_next(accfg_wq* wq) {
    return ((accfg_wq_get_next_ptr)qpl_test_functions_table[10].function)(wq);
}

enum accfg_wq_mode qpl_test_accfg_wq_get_mode(accfg_wq* wq) {
    return ((accfg_wq_get_mode_ptr)qpl_test_functions_table[11].function)(wq);
}

uint64_t qpl_test_accfg_wq_get_size(accfg_wq* wq) {
    return ((accfg_wq_get_size_ptr)qpl_test_functions_table[12].function)(wq);
}

int qpl_test_accfg_wq_get_priority(accfg_wq* wq) {
    return ((accfg_wq_get_priority_ptr)qpl_test_functions_table[13].function)(wq);
}

enum accfg_wq_state qpl_test_accfg_wq_get_state(accfg_wq* wq) {
    return ((accfg_wq_get_state_ptr)qpl_test_functions_table[14].function)(wq);
}

int qpl_test_accfg_wq_get_user_dev_path(accfg_wq* wq, char* buf, size_t size) {
    return ((accfg_wq_get_user_dev_path_ptr)qpl_test_functions_table[15].function)(wq, buf, size);
}

int qpl_test_accfg_wq_get_op_config(accfg_wq* wq, accfg_opcfg* op_config) {
    if (qpl_test_functions_table[16].function == NULL) return 1;
    return ((accfg_wq_get_op_config_ptr)qpl_test_functions_table[16].function)(wq, op_config);
}

accfg_eng* qpl_test_accfg_engine_get_first(accfg_dev* device) {
    return ((accfg_engine_get_first_ptr)qpl_test_functions_table[17].function)(device);
}

accfg_eng* qpl_test_accfg_engine_get_next(accfg_eng* engine) {
    return ((accfg_engine_get_next_ptr)qpl_test_functions_table[18].function)(engine);
}

int qpl_test_accfg_engine_get_group_id(accfg_eng* engine) {
    return ((accfg_engine_get_group_id_ptr)qpl_test_functions_table[19].function)(engine);
}

/* ------ Internal functions implementation ------ */

bool qpl_test_own_load_configuration_functions(void* driver_instance_ptr) {
    uint32_t i = 0U;

    while (qpl_test_functions_table[i].function_name) {
        qpl_test_functions_table[i].function =
                (qpl_test_library_function)dlsym(driver_instance_ptr, qpl_test_functions_table[i].function_name);

        char* err_message = dlerror();

        if (err_message) {
            // @todo this is a workaround to optionally load accfg_device_get_iaa_cap
            char*  iaa_cap_func_name     = "accfg_device_get_iaa_cap";
            size_t iaa_cap_func_name_len = strlen(iaa_cap_func_name);
            // @todo this is a workaround to optionally load accfg_wq_get_op_config
            char*  get_op_config_func_name     = "accfg_wq_get_op_config";
            size_t get_op_config_func_name_len = strlen(get_op_config_func_name);
            if (strlen(qpl_test_functions_table[i].function_name) == iaa_cap_func_name_len &&
                strncmp(qpl_test_functions_table[i].function_name, iaa_cap_func_name, iaa_cap_func_name_len) == 0) {
            } else if (strlen(qpl_test_functions_table[i].function_name) == get_op_config_func_name_len &&
                       strncmp(qpl_test_functions_table[i].function_name, get_op_config_func_name,
                               get_op_config_func_name_len) == 0) {
            } else {
                return false;
            }
        }

        i++;
    }

    return true;
}

qpl_test_hw_accelerator_status qpl_test_own_load_accelerator_configuration_driver(void** driver_instance_pptr) {

    // Try to load the accelerator configuration library
    void* driver_instance_ptr = dlopen(accelerator_configuration_driver_name, RTLD_LAZY);

    if (!driver_instance_ptr) {
        // This is needed for error handle. We need to call dlerror
        // for emptying error message. Otherwise we will receive error
        // message during loading symbols from another library
        dlerror();

        return QPL_TEST_HW_ACCELERATOR_LIBACCEL_NOT_FOUND;
    }

    *driver_instance_pptr = driver_instance_ptr;

    return QPL_TEST_HW_ACCELERATOR_STATUS_OK;
}

#else //DYNAMIC_LOADING_LIBACCEL_CONFIG=OFF
/*
 * QPL wrappers for accel-config APIs
 */
accfg_ctx* qpl_test_accfg_unref(accfg_ctx* ctx) {
    return accfg_unref(ctx);
}

int32_t qpl_test_accfg_new(accfg_ctx** ctx) {
    return accfg_new(ctx);
}

accfg_dev* qpl_test_accfg_device_get_first(accfg_ctx* ctx) {
    return accfg_device_get_first(ctx);
}

accfg_dev* qpl_test_accfg_device_get_next(accfg_dev* device) {
    return accfg_device_get_next(device);
}

const char* qpl_test_accfg_device_get_devname(accfg_dev* device) {
    return accfg_device_get_devname(device);
}

int qpl_test_accfg_device_get_numa_node(accfg_dev* device) {
    return accfg_device_get_numa_node(device);
}

enum accfg_device_state qpl_test_accfg_device_get_state(accfg_dev* device) {
    return accfg_device_get_state(device);
}

unsigned int qpl_test_accfg_device_get_version(accfg_dev* device) {
    return accfg_device_get_version(device);
}

int qpl_test_accfg_device_get_iaa_cap(accfg_dev* device, uint64_t* iaa_cap) {
    return accfg_device_get_iaa_cap(device, iaa_cap);
}

accfg_wq* qpl_test_accfg_wq_get_first(accfg_dev* device) {
    return accfg_wq_get_first(device);
}

accfg_wq* qpl_test_accfg_wq_get_next(accfg_wq* wq) {
    return accfg_wq_get_next(wq);
}

enum accfg_wq_mode qpl_test_accfg_wq_get_mode(accfg_wq* wq) {
    return accfg_wq_get_mode(wq);
}

uint64_t qpl_test_accfg_wq_get_size(accfg_wq* wq) {
    return accfg_wq_get_size(wq);
}

int qpl_test_accfg_wq_get_priority(accfg_wq* wq) {
    return accfg_wq_get_priority(wq);
}

enum accfg_wq_state qpl_test_accfg_wq_get_state(accfg_wq* wq) {
    return accfg_wq_get_state(wq);
}

int qpl_test_accfg_wq_get_user_dev_path(accfg_wq* wq, char* buf, size_t size) {
    return accfg_wq_get_user_dev_path(wq, buf, size);
}

int qpl_test_accfg_wq_get_op_config(accfg_wq* wq, accfg_opcfg* op_config) {
    return accfg_wq_get_op_config(wq, op_config);
}

accfg_eng* qpl_test_accfg_engine_get_first(accfg_dev* device) {
    return accfg_engine_get_first(device);
}

accfg_eng* qpl_test_accfg_engine_get_next(accfg_eng* engine) {
    return accfg_engine_get_next(engine);
}

int qpl_test_accfg_engine_get_group_id(accfg_eng* engine) {
    return accfg_engine_get_group_id(engine);
}

#endif //DYNAMIC_LOADING_LIBACCEL_CONFIG
#endif //__linux__
