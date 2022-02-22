/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "isal_statistics.h"

void isal_histogram_set_statistics(isal_histogram *isal_histogram_ptr,
                                   const uint32_t *literal_length_histogram_ptr,
                                   const uint32_t *offsets_histogram_ptr) {
    for (uint32_t i = 0u; i < OWN_LITERALS_MATCHES_TABLE_SIZE; i++) {
        isal_histogram_ptr->lit_len_histogram[i] = literal_length_histogram_ptr[i];
    }

    for (uint32_t i = 0; i < OWN_OFFSETS_TABLE_SIZE; i++) {
        isal_histogram_ptr->dist_histogram[i] = offsets_histogram_ptr[i];
    }
}

void isal_histogram_get_statistics(const isal_histogram *isal_histogram_ptr,
                                   uint32_t *literal_length_histogram_ptr,
                                   uint32_t *offsets_histogram_ptr) {
    for (uint32_t i = 0u; i < OWN_LITERALS_MATCHES_TABLE_SIZE; i++) {
        literal_length_histogram_ptr[i] = (uint32_t) isal_histogram_ptr->lit_len_histogram[i];
    }

    for (uint32_t i = 0u; i < OWN_OFFSETS_TABLE_SIZE; i++) {
        offsets_histogram_ptr[i] = (uint32_t) isal_histogram_ptr->dist_histogram[i];
    }
}

void isal_histogram_make_complete(isal_histogram *isal_histogram_ptr) {
    for (uint32_t i = 0u; i < OWN_LITERALS_MATCHES_TABLE_SIZE; i++) {
        if (0u == isal_histogram_ptr->lit_len_histogram[i]) {
            isal_histogram_ptr->lit_len_histogram[i] = 1u;
        }
    }

    for (uint32_t i = 0u; i < OWN_OFFSETS_TABLE_SIZE; i++) {
        if (0u == isal_histogram_ptr->dist_histogram[i]) {
            isal_histogram_ptr->dist_histogram[i] = 1u;
        }
    }
}
