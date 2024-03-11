/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 10/25/2018
 * Contains a reference implementation of the @ref ref_zero_decompress_32u function
 */

#include "qpl_api_ref.h"

#define BLOCK_SIZE 128    /**< @todo */

qpl_status ref_zero_decompress_32u(qpl_job *const qpl_job_ptr) {
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

    uint32_t *src = (uint32_t *) qpl_job_ptr->next_in_ptr;
    uint32_t *dst = (uint32_t *) qpl_job_ptr->next_out_ptr;

    const uint32_t elem_size      = sizeof(*src);
    const uint32_t elems_num      = qpl_job_ptr->available_in / elem_size;
    const uint32_t elems_in_block = BLOCK_SIZE / elem_size;

    uint32_t elem_ind               = 0;
    uint32_t tag_ind                = 0;
    uint32_t tag                    = 0;
    uint32_t last_destination_index = 0;
    uint32_t last_source_index      = 0;

    for (uint32_t i = 0; i < elems_num;) {
        last_source_index = i;

        tag = src[i++];

        for (uint32_t j = 0; j < elems_in_block; j++) {
            if (tag & (1 << j)) {
                if (i >= elems_num) {
                    break;
                }

                last_source_index = i;

                dst[elem_ind++] = src[i++];
            } else {
                dst[elem_ind++] = 0;
            }
        }

        last_destination_index = elem_ind - 1U;
    }

    uint32_t total_bytes_read    = (last_source_index + 1U) * sizeof(*src);
    uint32_t total_bytes_written = (last_destination_index + 1U) * sizeof(*src);

    qpl_job_ptr->available_in -= total_bytes_read;
    qpl_job_ptr->next_in_ptr  += total_bytes_read;
    qpl_job_ptr->total_in     = total_bytes_read;

    qpl_job_ptr->available_out -= total_bytes_written;
    qpl_job_ptr->next_out_ptr  += total_bytes_written;
    qpl_job_ptr->total_out     = total_bytes_written;

    return status;
}
