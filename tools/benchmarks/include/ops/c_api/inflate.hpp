/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <ops/c_api/base.hpp>
#include <stdexcept>

namespace bench::ops::c_api {
template <path_e path>
class inflate_t : public operation_base_t<inflate_t<path>> {
public:
    using result_t    = inflate_results_t;
    using params_t    = inflate_params_t;
    using data_type_t = typename result_t::data_type_t;
    using base_t      = ops::operation_base_t<inflate_t<path>>;
    using base_api_t  = operation_base_t<inflate_t<path>>;

    static constexpr auto path_v = path;

private:
    using base_api_t::deinit_lib_impl;
    using base_api_t::job_;
    using base_t::bytes_read_;
    using base_t::bytes_written_;

public:
    inflate_t() noexcept {}
    ~inflate_t() noexcept(false) { deinit_lib_impl(); }

protected:
    void init_buffers_impl(const params_t& params) {
        params_ = params;
        if (params.original_size_)
            data_.resize(params.original_size_);
        else
            data_.resize(params_.p_stream_->buffer.size() * 10);
    }

    void init_lib_params_impl() noexcept {
        job_->next_in_ptr   = const_cast<std::uint8_t*>(params_.p_stream_->buffer.data());
        job_->available_in  = static_cast<std::uint32_t>(params_.p_stream_->buffer.size());
        job_->next_out_ptr  = data_.data();
        job_->available_out = static_cast<std::uint32_t>(data_.size());
        job_->op            = qpl_op_decompress;
        job_->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;
        job_->huffman_table = nullptr;

        if (params_.huffman_ == huffman_type_e::canned) {
            job_->flags |= QPL_FLAG_CANNED_MODE;
            job_->huffman_table = params_.p_huffman_table_.get();
        }

        if (params_.no_headers_) job_->flags |= QPL_FLAG_NO_HDRS;
    }

    void sync_execute_impl() {
        auto status = qpl_execute_job(job_);
        if (QPL_STS_OK == status) {
            data_size_     = job_->total_out;
            bytes_read_    = job_->total_in;
            bytes_written_ = job_->total_out;
        } else
            throw std::runtime_error(format("qpl_execute_job() failed with status %d", status));
    }

    void async_submit_impl() {
        auto status = qpl_submit_job(job_);
        if (QPL_STS_OK != status) throw std::runtime_error(format("qpl_submit_job() failed with status %d", status));
    }

    task_status_e async_wait_impl() {
        auto status = qpl_wait_job(job_);
        if (QPL_STS_OK == status) {
            data_size_     = job_->total_out;
            bytes_read_    = job_->total_in;
            bytes_written_ = job_->total_out;
            return task_status_e::completed;
        } else
            throw std::runtime_error(format("qpl_wait_job() failed with status %d", status));
    }

    [[nodiscard]] task_status_e async_poll_impl() {
        auto status = qpl_check_job(job_);
        if (QPL_STS_BEING_PROCESSED == status)
            return task_status_e::in_progress;
        else {
            if (QPL_STS_OK == status) {
                data_size_     = job_->total_out;
                bytes_read_    = job_->total_in;
                bytes_written_ = job_->total_out;
                return task_status_e::completed;
            } else
                throw std::runtime_error(format("qpl_check_job() failed with status %d", status));
        }
    }

    void light_reset_impl() noexcept {
        job_->next_in_ptr -= job_->total_in;
        job_->available_in += job_->total_in;
        job_->next_out_ptr -= job_->total_out;
        job_->available_out += job_->total_out;
        job_->total_in  = 0;
        job_->total_out = 0;
    }

    void mem_control_impl(mem_loc_e op, mem_loc_mask_e mask) const noexcept {
        if (mask & mem_loc_mask_e::src1)
            details::mem_control(params_.p_stream_->buffer.begin(), params_.p_stream_->buffer.end(), op);
        if (mask & mem_loc_mask_e::dst1) details::mem_control(data_.begin(), data_.end(), op);
    }

    result_t& get_result_impl() noexcept {
        result_.data_ = data_;
        result_.data_.resize(data_size_);
        return result_;
    }

private:
    friend class ops::operation_base_t<inflate_t>;
    friend class operation_base_t<inflate_t>;

    params_t    params_;
    data_type_t data_;
    std::size_t data_size_ {0};
    result_t    result_;
};
} // namespace bench::ops::c_api

namespace bench::ops {
template <path_e path>
struct traits<operation_base_t<c_api::inflate_t<path>>> {
    using result_t = inflate_results_t;
};
} // namespace bench::ops
