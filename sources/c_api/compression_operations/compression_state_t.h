/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_OWN_QPL_STRUCTURES_H_
#define QPL_OWN_QPL_STRUCTURES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "igzip_lib.h"

#define ISAL_LEVEL_BUFFER_SIZE 348160u

/**
 *@brief Intel® Query Processing Library (Intel® QPL) decompress State struct definition
 */
typedef struct {
    struct inflate_state inflate_state;
} own_decompression_state_t;

typedef struct {
    uint32_t middle_layer_compression_style;
    uint8_t  is_modifiable;
    uint8_t  is_initialized;
} own_deflate_meta_t;

typedef struct {
    own_deflate_meta_t  meta_data;          /**< Structure that contains information for valid deflation */
    uint8_t             *verification_state_buffer_ptr;;
    void                *deflate_job_ptr;    /**< Pointer to internal data in high level compression */
    struct isal_zstream isal_stream;        /**< ISA-L stream information */
} own_compression_state_t;

#ifdef __cplusplus
}
#endif

#endif //QPL_SOURCES_INCLUDE_OWN_QPL_STRUCTURES_H_
