/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 10/25/2018
 * Contains a reference implementation of the @ref ref_zero_compress_16u function
 */

#include "qpl_api_ref.h"

qpl_status ref_zero_compress_16u(qpl_job *const qpl_job_ptr) {
    if (!qpl_job_ptr) {
        return QPL_STS_NULL_PTR_ERR;
    }
    if (!qpl_job_ptr->next_in_ptr) {
        return QPL_STS_NULL_PTR_ERR;
    }
    if (!qpl_job_ptr->next_out_ptr) {
        return QPL_STS_NULL_PTR_ERR;
    }
    if (!qpl_job_ptr->available_in) {
        return QPL_STS_SIZE_ERR;
    }
    if (!qpl_job_ptr->available_out) {
        return QPL_STS_SIZE_ERR;
    }

    qpl_status status = QPL_STS_OK;

    const uint32_t elem_size      = 2;    // size of element in bytes
    const uint32_t elems_in_block = 64;    // number of elements in block

    uint16_t *src = (uint16_t *) qpl_job_ptr->next_in_ptr;
    uint16_t *dst = (uint16_t *) qpl_job_ptr->next_out_ptr;

    const uint32_t elems_num = qpl_job_ptr->available_in / elem_size;    // number of input elements

    uint32_t elem_ind               = 0;
    uint32_t tag_ind                = 0;
    uint64_t tag64                  = (0 - 1ULL);
    uint32_t last_source_index      = 0;
    uint32_t last_destination_index = 0;

    uint32_t j = 0;
    for (uint32_t i = 0; i < elems_num; i += elems_in_block) {
        tag64 = (0 - 1ULL);
        tag_ind = elem_ind;
        elem_ind += 4;
        for (j = i; (j < i + elems_in_block) && (j < elems_num); j++) {
            if (src[j] == 0) {
                tag64 &= ~(1ULL << (j & 63));
            } else {
                last_destination_index = elem_ind;
                dst[elem_ind++] = src[j];
            }
        }
        dst[tag_ind + 0] = (uint16_t) ((tag64 >> 0) & 0xFFFF);
        dst[tag_ind + 1] = (uint16_t) ((tag64 >> 16) & 0xFFFF);
        dst[tag_ind + 2] = (uint16_t) ((tag64 >> 32) & 0xFFFF);
        dst[tag_ind + 3] = (uint16_t) ((tag64 >> 48) & 0xFFFF);

        if (tag_ind + 3u > last_destination_index) {
            last_destination_index = tag_ind + 3u;
        }
    }

    last_source_index = j - 1;

    uint32_t total_bytes_read    = (last_source_index + 1u) * sizeof(*src);
    uint32_t total_bytes_written = (last_destination_index + 1u) * sizeof(*src);

    qpl_job_ptr->available_in -= total_bytes_read;
    qpl_job_ptr->next_in_ptr  += total_bytes_read;
    qpl_job_ptr->total_in     = total_bytes_read;

    qpl_job_ptr->available_out -= total_bytes_written;
    qpl_job_ptr->next_out_ptr  += total_bytes_written;
    qpl_job_ptr->total_out     = total_bytes_written;

    return status;
}
