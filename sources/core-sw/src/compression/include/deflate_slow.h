/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_CORE_SW_SRC_COMPRESSION_INCLUDE_DEFLATE_SLOW_H
#define QPL_SOURCES_CORE_SW_SRC_COMPRESSION_INCLUDE_DEFLATE_SLOW_H

#include "bitbuf2.h"
#include "deflate_defs.h"
#include "deflate_hash_table.h"
#include "igzip_lib.h"
#include "own_qplc_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

OWN_QPLC_FUN(uint32_t, slow_deflate_body,
             (uint8_t* current_ptr, const uint8_t* const lower_bound_ptr, const uint8_t* const upper_bound_ptr,
              deflate_hash_table_t* hash_table_ptr, struct isal_hufftables* huffman_tables_ptr,
              struct BitBuf2* bit_writer_ptr));

#ifdef __cplusplus
}
#endif

#endif //QPL_SOURCES_CORE_SW_SRC_COMPRESSION_INCLUDE_DEFLATE_SLOW_H
