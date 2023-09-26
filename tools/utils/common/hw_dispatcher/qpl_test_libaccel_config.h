/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/
/*
 * Accel-config APIs for tests
 */
#ifndef QPL_TOOLS_UTILS_COMMON_LIBACCFG_H_
#define QPL_TOOLS_UTILS_COMMON_LIBACCFG_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum accfg_device_state {
	ACCFG_DEVICE_DISABLED = 0,
	ACCFG_DEVICE_ENABLED = 1,
	ACCFG_DEVICE_UNKNOWN = -1,
};

struct accfg_ctx;

/* drop a context reference count */
struct accfg_ctx *accfg_unref(struct accfg_ctx *ctx);

/* instantiate a new library context */
int accfg_new(struct accfg_ctx **ctx);

/* libaccfg function for device */
struct accfg_device;

/* Helper function to double check the state of the device/wq after enable/disable */
struct accfg_device *accfg_device_get_first(struct accfg_ctx *ctx);
struct accfg_device *accfg_device_get_next(struct accfg_device *device);
const char *accfg_device_get_devname(struct accfg_device *device);
enum accfg_device_state accfg_device_get_state(struct accfg_device *device);
unsigned int accfg_device_get_version(struct accfg_device *device);
int accfg_device_get_iaa_cap(struct accfg_device *device, uint64_t *iaa_cap);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif //QPL_TOOLS_UTILS_COMMON_LIBACCFG_H_
