/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/qpl.h"

#include "qpl/cpp_api/operations/operation.hpp"
#include "qpl/cpp_api/operations/common_operation.hpp"
#include "qpl/cpp_api/util/qpl_util.hpp"

namespace qpl {

void common_operation::reset_job_buffer(uint8_t *buffer) noexcept {
    buffer_ = buffer;
}

auto common_operation::init_job(const execution_path path) noexcept -> uint32_t {
    auto job = reinterpret_cast<qpl_job *>(buffer_);

    auto status = qpl_init_job(util::execution_path_to_qpl_path(path), job);

    if (status != QPL_STS_OK) {
        return status;
    }

    this->set_proper_flags();
    this->set_buffers();

    return QPL_STS_OK;
}

auto common_operation::execute() -> std::pair<uint32_t, uint32_t> {
    auto job        = reinterpret_cast<qpl_job *const>(buffer_);
    auto job_status = qpl_execute_job(job);

    return {job_status, this->get_output_elements_count()};
}

auto common_operation::get_output_elements_count() noexcept -> uint32_t {
    auto job = reinterpret_cast<qpl_job *>(buffer_);

    return job->total_out;
}

} // namespace qpl
