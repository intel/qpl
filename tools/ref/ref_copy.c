/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/29/2018
 * Contains a reference implementation of the qpl_copy_8u
 *
 */

#include "ref_copy.h"
#include "qpl/c_api/job.h"

qpl_status ref_copy_8u(qpl_job *const qpl_job_ptr) {
    REF_BAD_PTR_RET (qpl_job_ptr);
    REF_BAD_PTR2_RET(qpl_job_ptr->next_in_ptr, qpl_job_ptr->next_out_ptr);
    REF_BAD_SIZE_RET(qpl_job_ptr->available_in);
    REF_BAD_SIZE_RET(qpl_job_ptr->available_out);
    REF_BAD_ARG_RET((qpl_op_memcpy != qpl_job_ptr->op), QPL_STS_OPERATION_ERR);

    uint8_t  *src_ptr      = qpl_job_ptr->next_in_ptr;                       // source
    uint8_t  *dst_ptr      = qpl_job_ptr->next_out_ptr;                      // destination
    uint32_t i_bytes       = qpl_job_ptr->available_in;                      // number of bytes available in src_ptr
    uint32_t o_bytes        = qpl_job_ptr->available_out;                     // number of bytes available in src_ptr

    REF_BAD_ARG_RET((o_bytes < i_bytes), QPL_STS_DST_IS_SHORT_ERR);// not enough space in destination buffer
    ref_own_copy_8u(src_ptr, dst_ptr, i_bytes);                        // operation itself
    qpl_job_ptr->next_in_ptr += i_bytes;                               // update required fields in Job structure
    qpl_job_ptr->next_out_ptr += i_bytes;
    qpl_job_ptr->available_in -= i_bytes;
    qpl_job_ptr->available_out -= i_bytes;
    qpl_job_ptr->total_in  = i_bytes;
    qpl_job_ptr->total_out = i_bytes;

    return QPL_STS_OK;
}

void ref_fill_by_zero_8u(uint8_t *vector_ptr, uint32_t vector_size) {
    for (uint32_t i = 0; i < vector_size; i++)           // fill output vector by zero
    {
        vector_ptr[i] = 0;
    }
}
