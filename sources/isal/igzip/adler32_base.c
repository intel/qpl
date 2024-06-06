/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "adler32_base.h"
#include <stddef.h>
#include <stdint.h>
#include "igzip_checksums.h"

uint32_t qpl_adler32_base(uint32_t adler32, uint8_t * start, uint32_t length)
{
    uint8_t *end = NULL, *next = start;
    uint64_t A = adler32 & 0xffff;
    uint64_t B = adler32 >> 16;

    while (length > MAX_ADLER_BUF) {
        end = next + MAX_ADLER_BUF;
        for (; next < end; next++) {
            A += *next;
            B += A;
        }

        A = A % ADLER_MOD;
        B = B % ADLER_MOD;
        length -= MAX_ADLER_BUF;
    }

    end = next + length;
    for (; next < end; next++) {
        A += *next;
        B += A;
    }

    A = A % ADLER_MOD;
    B = B % ADLER_MOD;

    return B << 16 | A;
}
