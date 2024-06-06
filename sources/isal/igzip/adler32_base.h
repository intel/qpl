/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_ISAL_IGZIP_ADLER32_BASE_C
#define QPL_SOURCES_ISAL_IGZIP_ADLER32_BASE_C

// includes extracted from sources/isal/igzip/adler32_base.c
#include <stddef.h>
#include <stdint.h>
#include "igzip_checksums.h"


#ifdef __cplusplus
extern "C" {
#endif
    uint32_t qpl_adler32_base(uint32_t adler32, uint8_t *start, uint32_t length);
#ifdef __cplusplus
} // extern "C"
#endif


#endif
