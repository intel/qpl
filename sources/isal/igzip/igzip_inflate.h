/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_ISAL_IGZIP_IGZIP_INFLATE_C
#define QPL_SOURCES_ISAL_IGZIP_IGZIP_INFLATE_C

// includes extracted from sources/isal/igzip/igzip_inflate.c
#include <stdint.h>
#include "igzip_lib.h"
#include "crc.h"
#include "huff_codes.h"
#include "igzip_checksums.h"
#include "igzip_wrapper.h"
#include "unaligned.h"
#ifndef NO_STATIC_INFLATE_H
#include "static_inflate.h"
#endif
#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/endian.h>
# define bswap_32(x) bswap32(x)
#elif defined (__APPLE__)
#include <libkern/OSByteOrder.h>
# define bswap_32(x) OSSwapInt32(x)
#elif defined (__GNUC__) && !defined (__MINGW32__)
# include <byteswap.h>
#elif defined _WIN64
# define bswap_32(x) _byteswap_ulong(x)
#endif


#ifdef __cplusplus
extern "C" {
#endif
    int qpl_decode_huffman_code_block_stateless_base(struct inflate_state *state, uint8_t *start_out);
#ifdef __cplusplus
} // extern "C"
#endif


#endif
