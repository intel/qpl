/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 10/25/2018
 * Contains a reference implementation of the @ref ref_zero_compress_32u function
 */

#include "qpl_api_ref.h"

qpl_status ref_zero_compress_32u(qpl_job *const qpl_job_ptr) {
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

    uint32_t status = QPL_STS_OK;

    const uint32_t elem_size      = 4; // size of element in bytes
    const uint32_t elems_in_block = 32; // number of elements in block
    const uint32_t max_32u        = 0xFFFFFFFF;

    uint32_t *src = (uint32_t *) qpl_job_ptr->next_in_ptr;
    uint32_t *dst = (uint32_t *) qpl_job_ptr->next_out_ptr;

    const uint32_t elems_num = qpl_job_ptr->available_in / elem_size; // number of input elements

    uint32_t elem_ind               = 0;
    uint32_t tag_ind                = 0;
    uint32_t tag                    = max_32u;
    uint32_t last_source_index      = 0;
    uint32_t last_destination_index = 0;

    for (uint32_t i = 0; i < elems_num; i += elems_in_block) {
        tag     = max_32u;
        tag_ind = elem_ind++;
        for (uint32_t j = i; (j < i + elems_in_block) && (j < elems_num); j++) {
            if (0 == src[j]) {
                tag &= ~(1 << (j & (elems_in_block - 1)));
            } else {
                last_destination_index = elem_ind;
                dst[elem_ind++] = src[j];
            }

            last_source_index = j;
        }
        dst[tag_ind] = tag;

        if (tag_ind > last_destination_index) {
            last_destination_index = tag_ind;
        }
    }

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
