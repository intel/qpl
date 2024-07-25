/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/
/*
 * Accel-config APIs for tests
 */
#ifndef QPL_TOOLS_UTILS_HW_DISPATCHER_QPL_TEST_LIBACCEL_CONFIG_H_
#define QPL_TOOLS_UTILS_HW_DISPATCHER_QPL_TEST_LIBACCEL_CONFIG_H_

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

enum accfg_device_state {
    ACCFG_DEVICE_DISABLED = 0,
    ACCFG_DEVICE_ENABLED  = 1,
    ACCFG_DEVICE_UNKNOWN  = -1,
};

enum accfg_wq_mode {
    ACCFG_WQ_SHARED = 0,
    ACCFG_WQ_DEDICATED,
    ACCFG_WQ_MODE_UNKNOWN,
};

enum accfg_wq_state {
    ACCFG_WQ_DISABLED,
    ACCFG_WQ_ENABLED,
    ACCFG_WQ_QUIESCING,
    ACCFG_WQ_LOCKED,
    ACCFG_WQ_UNKNOWN = -1,
};

struct accfg_op_config {
    uint32_t bits[8];
};

struct accfg_ctx;

/* drop a context reference count */
struct accfg_ctx* accfg_unref(struct accfg_ctx* ctx);

/* instantiate a new library context */
int accfg_new(struct accfg_ctx** ctx);

/* libaccfg function for device */
struct accfg_device;

/* Helper function to double check the state of the device/wq after enable/disable */
struct accfg_device*    accfg_device_get_first(struct accfg_ctx* ctx);
struct accfg_device*    accfg_device_get_next(struct accfg_device* device);
const char*             accfg_device_get_devname(struct accfg_device* device);
int                     accfg_device_get_numa_node(struct accfg_device* device);
enum accfg_device_state accfg_device_get_state(struct accfg_device* device);
unsigned int            accfg_device_get_version(struct accfg_device* device);
int                     accfg_device_get_iaa_cap(struct accfg_device* device, uint64_t* iaa_cap);

/* libaccfg function for wq */
struct accfg_wq;
struct accfg_wq*    accfg_wq_get_first(struct accfg_device* device);
struct accfg_wq*    accfg_wq_get_next(struct accfg_wq* wq);
enum accfg_wq_mode  accfg_wq_get_mode(struct accfg_wq* wq);
uint64_t            accfg_wq_get_size(struct accfg_wq* wq);
int                 accfg_wq_get_priority(struct accfg_wq* wq);
enum accfg_wq_state accfg_wq_get_state(struct accfg_wq* wq);
int                 accfg_wq_get_user_dev_path(struct accfg_wq* wq, char* buf, size_t size);
int                 accfg_wq_get_op_config(struct accfg_wq* wq, struct accfg_op_config* op_config);

/* libaccfg function for engine */
struct accfg_engine;
struct accfg_engine* accfg_engine_get_first(struct accfg_device* device);
struct accfg_engine* accfg_engine_get_next(struct accfg_engine* engine);
int                  accfg_engine_get_group_id(struct accfg_engine* engine);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif //QPL_TOOLS_UTILS_HW_DISPATCHER_QPL_TEST_LIBACCEL_CONFIG_H_
