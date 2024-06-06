/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_ISAL_IGZIP_IGZIP_ICF_BODY_C
#define QPL_SOURCES_ISAL_IGZIP_IGZIP_ICF_BODY_C

// includes extracted from sources/isal/igzip/igzip_icf_body.c
#include "igzip_lib.h"
#include "huffman.h"
#include "encode_df.h"
#include "igzip_level_buf_structs.h"


#ifdef __cplusplus
extern "C" {
#endif
    void qpl_set_long_icf_fg_base(uint8_t *next_in, uint64_t processed, uint64_t input_size, struct deflate_icf *match_lookup);
    uint64_t qpl_gen_icf_map_h1_base(struct isal_zstream *stream, struct deflate_icf *matches_icf_lookup, uint64_t input_size);
    void qpl_icf_body_hash1_fillgreedy_lazy(struct isal_zstream *stream);
    void qpl_icf_body_lazyhash1_fillgreedy_greedy(struct isal_zstream *stream);
    void qpl_isal_deflate_icf_body(struct isal_zstream *stream);
#ifdef __cplusplus
} // extern "C"
#endif


#endif
