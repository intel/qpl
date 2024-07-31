/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <memory.h>
#include <ops/base.hpp>
#include <ops/params.hpp>
#include <ops/results.hpp>
#include <qpl/qpl.h>
#include <stdexcept>
#include <types.hpp>

namespace bench::ops::c_api {
template <path_e path>
static inline qpl_path_t to_qpl_path() {
    if constexpr (path == path_e::cpu)
        return qpl_path_software;
    else if (path == path_e::iaa)
        return qpl_path_hardware;
    else if (path == path_e::auto_)
        return qpl_path_auto;
    else
        throw std::runtime_error("Invalid path conversion!");
}

template <typename DerivedT>
class operation_base_t : public ops::operation_base_t<DerivedT> {
public:
    using base_t = ops::operation_base_t<DerivedT>;

private:
    using base_t::cache_control_;
    using base_t::numa_id_;

public:
    operation_base_t() noexcept {}
    ~operation_base_t() noexcept {}

    void init_lib_impl() {
        if (!cache_control_) throw std::runtime_error("manual cache control option is not supported in C API");

        std::uint32_t size   = 0U;
        auto          status = qpl_get_job_size(to_qpl_path<DerivedT::path_v>(), &size);
        if (QPL_STS_OK != status) throw std::runtime_error(format("qpl_get_job_size() failed with status %d", status));

        job_ = (qpl_job*)malloc(size);
        if (!job_) throw std::runtime_error("malloc() failed");

        status = qpl_init_job(to_qpl_path<DerivedT::path_v>(), job_);
        if (QPL_STS_OK != status) {
            free(job_);
            job_ = nullptr;
            throw std::runtime_error(format("qpl_init_job() failed with status %d", status));
        }
        job_->numa_id = numa_id_;
    }

    void deinit_lib_impl() {
        if (job_) {
            auto status = qpl_fini_job(job_);
            free(job_);
            job_ = nullptr;
            if (QPL_STS_OK != status) throw std::runtime_error(format("qpl_fini_job() failed with status %d", status));
        }
    }

protected:
    friend class ops::operation_base_t<DerivedT>;

    qpl_job* job_ {nullptr};
};
} // namespace bench::ops::c_api
