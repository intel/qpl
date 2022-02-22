/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Job API (public C API)
 */

#ifndef QPL_SOURCES_C_API_OTHER_OPERATIONS_ARGUMENTS_CHECK_HPP_
#define QPL_SOURCES_C_API_OTHER_OPERATIONS_ARGUMENTS_CHECK_HPP_

#include "job.hpp"
#include "util/checkers.hpp"
#include "own_checkers.h"

namespace qpl::job {
template<>
inline auto validate_operation<qpl_op_memcpy>(const qpl_job *const job_ptr) noexcept {
    if (ml::bad_argument::check_for_nullptr(job_ptr)) {
        return QPL_STS_NULL_PTR_ERR;
    }

    QPL_BADARG_RET((qpl_op_memcpy != job_ptr->op), QPL_STS_OPERATION_ERR);

    if (ml::bad_argument::check_for_nullptr(job_ptr->next_in_ptr, job_ptr->next_out_ptr)) {
        return QPL_STS_NULL_PTR_ERR;
    }

    if (ml::bad_argument::buffers_overlap(job_ptr->next_in_ptr, job_ptr->available_in,
                                          job_ptr->next_out_ptr, job_ptr->available_out)) {
        return QPL_STS_BUFFER_OVERLAP_ERR;
    }

    QPL_BAD_SIZE_RET(job_ptr->available_in);
    QPL_BAD_SIZE_RET(job_ptr->available_out);
    QPL_BADARG_RET((job_ptr->available_out < job_ptr->available_in), QPL_STS_DST_IS_SHORT_ERR);

    return QPL_STS_OK;
}

template<>
inline auto validate_operation<qpl_op_crc64>(const qpl_job *const job_ptr) noexcept {
    QPL_BAD_PTR2_RET(job_ptr, job_ptr->next_in_ptr);
    QPL_BAD_SIZE_RET(job_ptr->available_in);

    if (job_ptr->crc64_poly == 0) {
        return QPL_STS_CRC64_BAD_POLYNOM;
    }

    return QPL_STS_OK;
}

}

#endif //QPL_SOURCES_C_API_OTHER_OPERATIONS_ARGUMENTS_CHECK_HPP_
