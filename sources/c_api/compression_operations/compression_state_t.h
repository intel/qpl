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

/**
 * @brief Compression state struct definition.
 *
 * middle_layer_compression_style is used mainly in middle-layer.
 *
 * adler32 is required for writing zlib trailer and is used to accumulate checksums
 * of input streams when compression in multiple chunks is used.
 */
typedef struct {
    uint32_t middle_layer_compression_style;
    uint32_t adler32;
} own_compression_state_t;

#ifdef __cplusplus
}
#endif

#endif //QPL_SOURCES_INCLUDE_OWN_QPL_STRUCTURES_H_
