/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_ISAL_IGZIP_IGZIP_BASE_C
#define QPL_SOURCES_ISAL_IGZIP_IGZIP_BASE_C

// includes extracted from sources/isal/igzip/igzip_base.c
#include <stdint.h>
#include "igzip_lib.h"
#include "huffman.h"
#include "huff_codes.h"
#include "bitbuf2.h"


#ifdef __cplusplus
extern "C" {
#endif
    void qpl_isal_deflate_body_base(struct isal_zstream *stream);
    void qpl_isal_deflate_finish_base(struct isal_zstream *stream);
    void qpl_isal_deflate_hash_base(uint16_t *hash_table, uint32_t hash_mask, uint32_t current_index, uint8_t *dict, uint32_t dict_len);
#ifdef __cplusplus
} // extern "C"
#endif


#endif
