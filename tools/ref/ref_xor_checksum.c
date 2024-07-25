/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 10/30/2018
 * Contains a reference implementation of the XOR checksum function
 */

#include "qpl_api_ref.h"

uint32_t ref_xor_checksum(const uint8_t* buf, uint32_t len, uint32_t init_xor) {
    uint32_t checksum = init_xor;

    for (uint32_t i = 0; i < (len & ~1); i += 2) {
        checksum ^= (uint32_t)(*(uint16_t*)(buf + i));
    }

    if (len & 1) { checksum ^= (uint32_t)buf[len - 1]; }

    return checksum;
}
