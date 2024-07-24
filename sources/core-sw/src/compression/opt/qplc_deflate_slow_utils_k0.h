/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  SW Core API (Private API)
 */

#ifndef OWN_SETUP_DICTIONARY_H
#define OWN_SETUP_DICTIONARY_H

#include "deflate_defs.h"
#include "deflate_hash_table.h"
#include "immintrin.h"
#include "own_qplc_defs.h"

#if PLATFORM >= K0

OWN_OPT_FUN(
        void, k0_setup_dictionary,
        (uint8_t * dictionary_ptr, uint32_t dictionary_size, deflate_hash_table_t* hash_table_ptr) {
            uint8_t* current_ptr = dictionary_ptr;

            for (uint32_t index = 0; index < dictionary_size; index++) {
                // Variables

                uint32_t hash_value = 0U;

                hash_value = _mm_crc32_u32(0U, *((uint32_t*)current_ptr)) & hash_table_ptr->hash_mask;

                // Updating hash table
                own_deflate_hash_table_update(hash_table_ptr, index - dictionary_size, hash_value);

                // End of iteration
                current_ptr++;
            }
        })

#endif

#endif // OWN_SETUP_DICTIONARY_H
