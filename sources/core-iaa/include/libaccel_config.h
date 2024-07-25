/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/
#ifndef _LIBACCFG_H_
#define _LIBACCFG_H_

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <uuid/uuid.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UUID_STR_LEN
#define UUID_STR_LEN 37
#endif

#define MAX_DEV_LEN         64
#define MAX_BUF_LEN         128
#define MAX_PARAM_LEN       4096
#define TRAFFIC_CLASS_LIMIT 8
#define WQ_PRIORITY_LIMIT   15
#define UUID_ZERO           "00000000-0000-0000-0000-000000000000"

enum accfg_device_version {
    ACCFG_DEVICE_VERSION_1 = 0x100,
    ACCFG_DEVICE_VERSION_2 = 0x200,
};

/* no need to save device state */
enum accfg_device_type {
    ACCFG_DEVICE_DSA          = 0,
    ACCFG_DEVICE_IAX          = 1,
    ACCFG_DEVICE_TYPE_UNKNOWN = -1,
};

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

enum accfg_wq_type {
    ACCFG_WQT_NONE = 0,
    ACCFG_WQT_KERNEL,
    ACCFG_WQT_USER,
};

enum accfg_control_flag {
    ACCFG_DEVICE_DISABLE = 0,
    ACCFG_DEVICE_ENABLE,
    ACCFG_WQ_ENABLE,
    ACCFG_WQ_DISABLE,
};

/* no need to save device error */
struct accfg_error {
    uint32_t val[8];
};

struct accfg_op_cap {
    uint32_t bits[8];
};

struct accfg_op_config {
    uint32_t bits[8];
};

/* parameters read from sysfs of accfg driver */
struct dev_parameters {
    unsigned int token_limit __attribute((deprecated));
    unsigned int read_buffer_limit;
    int          event_log_size;
};

extern char* accfg_basenames[];

struct group_parameters {
    unsigned int tokens_reserved __attribute((deprecated));
    unsigned int tokens_allowed __attribute((deprecated));
    unsigned int use_token_limit __attribute((deprecated));
    unsigned int read_buffers_reserved;
    unsigned int read_buffers_allowed;
    unsigned int use_read_buffer_limit;
    int          traffic_class_a;
    int          traffic_class_b;
    int          desc_progress_limit;
    int          batch_progress_limit;
};

struct wq_parameters {
    int          group_id;
    unsigned int wq_size;
    unsigned int threshold;
    unsigned int priority;
    int          block_on_fault;
    unsigned int max_batch_size;
    uint64_t     max_transfer_size;
    int          ats_disable;
    int          prs_disable;
    const char*  mode;
    const char*  type;
    const char*  name;
    const char*  driver_name;
    const char*  op_config;
};

struct engine_parameters {
    int group_id;
};

struct accfg_ctx;

/* Retrieve current library loglevel */
int accfg_get_log_priority(struct accfg_ctx* ctx);

/* Set log level */
void accfg_set_log_priority(struct accfg_ctx* ctx, int priority);

/* instantiate a new library context */
struct accfg_ctx* accfg_ref(struct accfg_ctx* ctx);

/* drop a context reference count */
struct accfg_ctx* accfg_unref(struct accfg_ctx* ctx);

/* instantiate a new library context */
int accfg_new(struct accfg_ctx** ctx);

/* override default log routine */
void accfg_set_log_fn(struct accfg_ctx* ctx,
                      void (*log_fn)(struct accfg_ctx* ctx, int priority, const char* file, int line, const char* fn,
                                     const char* format, va_list args));

/* libaccfg function for device */
struct accfg_device;
/* Helper function to enable/disable the part in device */
int accfg_device_enable(struct accfg_device* device);
int accfg_device_disable(struct accfg_device* device, bool force);

/* Helper function to double check the state of the device/wq after enable/disable */
struct accfg_device* accfg_device_get_first(struct accfg_ctx* ctx);
struct accfg_device* accfg_device_get_next(struct accfg_device* device);
#define accfg_device_foreach(ctx, device) \
    for ((device) = accfg_device_get_first(ctx); (device) != NULL; (device) = accfg_device_get_next(device))
struct accfg_ctx*       accfg_device_get_ctx(struct accfg_device*);
const char*             accfg_device_get_devname(struct accfg_device* device);
int                     accfg_device_type_validate(const char* dev_name);
enum accfg_device_type  accfg_device_get_type(struct accfg_device* device);
char*                   accfg_device_get_type_str(struct accfg_device* device);
int                     accfg_device_get_id(struct accfg_device* device);
struct accfg_device*    accfg_ctx_device_get_by_id(struct accfg_ctx* ctx, int id);
struct accfg_device*    accfg_ctx_device_get_by_name(struct accfg_ctx* ctx, const char* dev_name);
unsigned int            accfg_device_get_max_groups(struct accfg_device* device);
unsigned int            accfg_device_get_max_work_queues(struct accfg_device* device);
unsigned int            accfg_device_get_max_engines(struct accfg_device* device);
unsigned int            accfg_device_get_max_work_queues_size(struct accfg_device* device);
int                     accfg_device_get_numa_node(struct accfg_device* device);
unsigned int            accfg_device_get_ims_size(struct accfg_device* device);
unsigned int            accfg_device_get_max_batch_size(struct accfg_device* device);
uint64_t                accfg_device_get_max_transfer_size(struct accfg_device* device);
unsigned int            accfg_device_get_compl_size(struct accfg_device* device);
int                     accfg_device_get_op_cap(struct accfg_device* device, struct accfg_op_cap* op_cap);
uint64_t                accfg_device_get_gen_cap(struct accfg_device* device);
int                     accfg_device_get_iaa_cap(struct accfg_device* device, uint64_t* iaa_cap);
unsigned int            accfg_device_get_configurable(struct accfg_device* device);
bool                    accfg_device_get_pasid_enabled(struct accfg_device* device);
int                     accfg_device_get_errors(struct accfg_device* device, struct accfg_error* error);
enum accfg_device_state accfg_device_get_state(struct accfg_device* device);
unsigned int            accfg_device_get_max_tokens(struct accfg_device* device) __attribute((deprecated));
unsigned int            accfg_device_get_max_read_buffers(struct accfg_device* device);
unsigned int            accfg_device_get_max_batch_size(struct accfg_device* device);
unsigned int            accfg_device_get_token_limit(struct accfg_device* device) __attribute((deprecated));
unsigned int            accfg_device_get_read_buffer_limit(struct accfg_device* device);
unsigned int            accfg_device_get_cdev_major(struct accfg_device* device);
unsigned int            accfg_device_get_version(struct accfg_device* device);
int                     accfg_device_get_clients(struct accfg_device* device);
int                     accfg_device_set_token_limit(struct accfg_device* dev, int val) __attribute((deprecated));
int                     accfg_device_set_read_buffer_limit(struct accfg_device* dev, int val);
int                     accfg_device_get_event_log_size(struct accfg_device* device);
int                     accfg_device_set_event_log_size(struct accfg_device* dev, int val);
int                     accfg_device_is_active(struct accfg_device* device);
unsigned int            accfg_device_get_cmd_status(struct accfg_device* device);
const char*             accfg_device_get_cmd_status_str(struct accfg_device* device);
unsigned int            accfg_ctx_get_last_error(struct accfg_ctx* ctx);
const char*             accfg_ctx_get_last_error_str(struct accfg_ctx* ctx);
struct accfg_device*    accfg_ctx_get_last_error_device(struct accfg_ctx* ctx);
struct accfg_wq*        accfg_ctx_get_last_error_wq(struct accfg_ctx* ctx);
struct accfg_group*     accfg_ctx_get_last_error_group(struct accfg_ctx* ctx);
struct accfg_engine*    accfg_ctx_get_last_error_engine(struct accfg_ctx* ctx);

/* libaccfg function for group */
struct accfg_group;
struct accfg_group* accfg_group_get_first(struct accfg_device* device);
struct accfg_group* accfg_group_get_next(struct accfg_group* group);
#define accfg_group_foreach(device, group) \
    for ((group) = accfg_group_get_first(device); (group) != NULL; (group) = accfg_group_get_next(group))
int                  accfg_group_get_id(struct accfg_group* group);
struct accfg_group*  accfg_device_group_get_by_id(struct accfg_device* device, int id);
int                  accfg_group_get_device_id(struct accfg_group* group);
const char*          accfg_group_get_devname(struct accfg_group* group);
uint64_t             accfg_group_get_size(struct accfg_group* group);
uint64_t             accfg_group_get_available_size(struct accfg_group* group);
struct accfg_device* accfg_group_get_device(struct accfg_group* group);
struct accfg_ctx*    accfg_group_get_ctx(struct accfg_group* group);
int                  accfg_group_get_tokens_reserved(struct accfg_group* group) __attribute((deprecated));
int                  accfg_group_get_read_buffers_reserved(struct accfg_group* group);
int                  accfg_group_get_tokens_allowed(struct accfg_group* group) __attribute((deprecated));
int                  accfg_group_get_read_buffers_allowed(struct accfg_group* group);
int                  accfg_group_get_use_token_limit(struct accfg_group* group) __attribute((deprecated));
int                  accfg_group_get_use_read_buffer_limit(struct accfg_group* group);
int                  accfg_group_get_traffic_class_a(struct accfg_group* group);
int                  accfg_group_get_traffic_class_b(struct accfg_group* group);
int                  accfg_group_set_tokens_reserved(struct accfg_group* group, int val) __attribute((deprecated));
int                  accfg_group_set_read_buffers_reserved(struct accfg_group* group, int val);
int                  accfg_group_set_tokens_allowed(struct accfg_group* group, int val) __attribute((deprecated));
int                  accfg_group_set_read_buffers_allowed(struct accfg_group* group, int val);
int                  accfg_group_set_use_token_limit(struct accfg_group* group, int val) __attribute((deprecated));
int                  accfg_group_set_use_read_buffer_limit(struct accfg_group* group, int val);
int                  accfg_group_set_traffic_class_a(struct accfg_group* group, int val);
int                  accfg_group_set_traffic_class_b(struct accfg_group* group, int val);
int                  accfg_group_get_desc_progress_limit(struct accfg_group* group);
int                  accfg_group_set_desc_progress_limit(struct accfg_group* group, int val);
int                  accfg_group_get_batch_progress_limit(struct accfg_group* group);
int                  accfg_group_set_batch_progress_limit(struct accfg_group* group, int val);

/* libaccfg function for wq */
struct accfg_wq;
struct accfg_wq* accfg_wq_get_first(struct accfg_device* device);
struct accfg_wq* accfg_wq_get_next(struct accfg_wq* wq);

#define accfg_wq_foreach(device, wq) for ((wq) = accfg_wq_get_first(device); (wq) != NULL; (wq) = accfg_wq_get_next(wq))

struct accfg_ctx*    accfg_wq_get_ctx(struct accfg_wq* wq);
struct accfg_device* accfg_wq_get_device(struct accfg_wq* wq);
struct accfg_group*  accfg_wq_get_group(struct accfg_wq* wq);
int                  accfg_wq_get_id(struct accfg_wq* wq);
struct accfg_wq*     accfg_device_wq_get_by_id(struct accfg_device* device, int id);
const char*          accfg_wq_get_devname(struct accfg_wq* wq);
enum accfg_wq_mode   accfg_wq_get_mode(struct accfg_wq* wq);
uint64_t             accfg_wq_get_size(struct accfg_wq* wq);
int                  accfg_wq_get_group_id(struct accfg_wq* wq);
int                  accfg_wq_get_priority(struct accfg_wq* wq);
unsigned int         accfg_wq_get_priv(struct accfg_wq* wq);
int                  accfg_wq_get_block_on_fault(struct accfg_wq* wq);
enum accfg_wq_state  accfg_wq_get_state(struct accfg_wq* wq);
int                  accfg_wq_get_cdev_minor(struct accfg_wq* wq);
const char*          accfg_wq_get_type_name(struct accfg_wq* wq);
const char*          accfg_wq_get_driver_name(struct accfg_wq* wq);
int                  accfg_wq_driver_name_validate(struct accfg_wq* wq, const char* drv_name);
enum accfg_wq_type   accfg_wq_get_type(struct accfg_wq* wq);
unsigned int         accfg_wq_get_max_batch_size(struct accfg_wq* wq);
uint64_t             accfg_wq_get_max_transfer_size(struct accfg_wq* wq);
int                  accfg_wq_get_threshold(struct accfg_wq* wq);
int                  accfg_wq_get_clients(struct accfg_wq* wq);
int                  accfg_wq_get_ats_disable(struct accfg_wq* wq);
int                  accfg_wq_get_occupancy(struct accfg_wq* wq);
int                  accfg_wq_is_enabled(struct accfg_wq* wq);
int                  accfg_wq_set_size(struct accfg_wq* wq, int val);
int                  accfg_wq_set_priority(struct accfg_wq* wq, int val);
int                  accfg_wq_set_group_id(struct accfg_wq* wq, int val);
int                  accfg_wq_set_threshold(struct accfg_wq* wq, int val);
int                  accfg_wq_set_block_on_fault(struct accfg_wq* wq, int val);
int                  accfg_wq_set_max_batch_size(struct accfg_wq* wq, int val);
int                  accfg_wq_set_ats_disable(struct accfg_wq* wq, int val);
int                  accfg_wq_set_max_transfer_size(struct accfg_wq* wq, uint64_t val);
int                  accfg_wq_set_str_mode(struct accfg_wq* wq, const char* val);
int                  accfg_wq_set_mode(struct accfg_wq* wq, enum accfg_wq_mode mode);
int                  accfg_wq_set_str_type(struct accfg_wq* wq, const char* val);
int                  accfg_wq_set_str_name(struct accfg_wq* wq, const char* val);
int                  accfg_wq_set_str_driver_name(struct accfg_wq* wq, const char* val);
int                  accfg_wq_enable(struct accfg_wq* wq);
int                  accfg_wq_disable(struct accfg_wq* wq, bool force);
int                  accfg_wq_priority_boundary(struct accfg_wq* wq);
int                  accfg_wq_size_boundary(struct accfg_device* device, int wq_num);
int                  accfg_wq_get_user_dev_path(struct accfg_wq* wq, char* buf, size_t size);
int                  accfg_wq_get_op_config(struct accfg_wq* wq, struct accfg_op_config* op_config);
int                  accfg_wq_set_op_config(struct accfg_wq* wq, struct accfg_op_config* op_config);
int                  accfg_wq_set_op_config_str(struct accfg_wq* wq, const char* op_config);
int                  accfg_wq_get_prs_disable(struct accfg_wq* wq);
int                  accfg_wq_set_prs_disable(struct accfg_wq* wq, int val);

/* libaccfg function for engine */
struct accfg_engine;
struct accfg_engine* accfg_engine_get_first(struct accfg_device* device);
struct accfg_engine* accfg_engine_get_next(struct accfg_engine* engine);
#define accfg_engine_foreach(device, engine) \
    for ((engine) = accfg_engine_get_first(device); (engine) != NULL; (engine) = accfg_engine_get_next(engine))
struct accfg_ctx*    accfg_engine_get_ctx(struct accfg_engine* engine);
struct accfg_device* accfg_engine_get_device(struct accfg_engine* engine);
struct accfg_group*  accfg_engine_get_group(struct accfg_engine* engine);
int                  accfg_engine_get_group_id(struct accfg_engine* engine);
int                  accfg_engine_get_id(struct accfg_engine* engine);
struct accfg_engine* accfg_device_engine_get_by_id(struct accfg_device* device, int id);
const char*          accfg_engine_get_devname(struct accfg_engine* engine);
int                  accfg_engine_set_group_id(struct accfg_engine* engine, int val);
#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
