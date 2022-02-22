/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Job API (public C API)
 */

#include "arguments_check.hpp"
#include "copy.hpp"
#include "other/copy.hpp"

uint32_t perform_copy(qpl_job *const job_ptr) noexcept {
    using namespace qpl::ml;

    OWN_QPL_CHECK_STATUS(qpl::job::validate_operation<qpl_op_memcpy>(job_ptr))

    qpl::job::reset<qpl_op_memcpy>(job_ptr);

    other::copy_operation_result_t result;

    switch (qpl::job::get_execution_path(job_ptr)) {

        case execution_path_t::auto_detect:
            result = other::call_copy<execution_path_t::auto_detect>(job_ptr->next_in_ptr,
                                                                     job_ptr->next_out_ptr,
                                                                     job_ptr->available_in,
                                                                     job_ptr->numa_id);
            break;
        case execution_path_t::hardware:
            result = other::call_copy<execution_path_t::hardware>(job_ptr->next_in_ptr,
                                                                  job_ptr->next_out_ptr,
                                                                  job_ptr->available_in,
                                                                  job_ptr->numa_id);
            break;
        case execution_path_t::software:
            result = other::call_copy<execution_path_t::software>(job_ptr->next_in_ptr,
                                                                  job_ptr->next_out_ptr,
                                                                  job_ptr->available_in,
                                                                  job_ptr->numa_id);
            break;
    }

    qpl::job::update_input_stream(job_ptr, result.copied_bytes_);
    qpl::job::update_output_stream(job_ptr, result.copied_bytes_, 0);

    return QPL_STS_OK;
}
