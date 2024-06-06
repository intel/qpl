/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_ISAL_IGZIP_IGZIP_ICF_BASE_C
#define QPL_SOURCES_ISAL_IGZIP_IGZIP_ICF_BASE_C

// includes extracted from sources/isal/igzip/igzip_icf_base.c
#include <stdint.h>
#include "igzip_lib.h"
#include "huffman.h"
#include "huff_codes.h"
#include "encode_df.h"
#include "igzip_level_buf_structs.h"
#include "unaligned.h"


#ifdef __cplusplus
extern "C" {
#endif
    void qpl_isal_deflate_icf_body_hash_hist_base(struct isal_zstream *stream);
    void qpl_isal_deflate_icf_finish_hash_hist_base(struct isal_zstream *stream);
    void qpl_isal_deflate_icf_finish_hash_map_base(struct isal_zstream *stream);
    void qpl_isal_deflate_hash_mad_base(uint16_t *hash_table, uint32_t hash_mask, uint32_t current_index, uint8_t *dict, uint32_t dict_len);
#ifdef __cplusplus
} // extern "C"
#endif


#endif
