/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_ISAL_IGZIP_IGZIP_C
#define QPL_SOURCES_ISAL_IGZIP_IGZIP_C

// includes extracted from sources/isal/igzip/igzip.c
#include <assert.h>
#include <string.h>
#ifdef _WIN32
# include <intrin.h>
#endif
#include "huffman.h"
#include "bitbuf2.h"
#include "igzip_lib.h"
#include "crc.h"
#include "repeated_char_result.h"
#include "huff_codes.h"
#include "encode_df.h"
#include "igzip_level_buf_structs.h"
#include "igzip_checksums.h"
#include "igzip_wrapper.h"
#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/endian.h>
# define to_be32(x) bswap32(x)
#elif defined (__APPLE__)
#include <libkern/OSByteOrder.h>
# define to_be32(x) OSSwapInt32(x)
#elif defined (__GNUC__) && !defined (__MINGW32__)
# include <byteswap.h>
# define to_be32(x) bswap_32(x)
#elif defined _WIN64
# define to_be32(x) _byteswap_ulong(x)
#endif


#ifdef __cplusplus
extern "C" {
#endif
    void qpl_isal_deflate_body_huffman_only(struct isal_zstream *stream);
    void qpl_isal_deflate_finish_huffman_only(struct isal_zstream *stream);
    void qpl_isal_deflate_hash(struct isal_zstream *stream, uint8_t *dict, uint32_t dict_len);
#ifdef __cplusplus
} // extern "C"
#endif


#endif
