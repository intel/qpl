/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <ops/c_api/base.hpp>
#include <stdexcept>

namespace bench::ops::c_api
{
template <path_e path>
class deflate_t: public operation_base_t<deflate_t<path>>
{
public:
    using result_t    = deflate_results_t;
    using params_t    = deflate_params_t;
    using data_type_t = typename result_t::data_type_t;
    using base_t      = ops::operation_base_t<deflate_t<path>>;
    using base_api_t  = operation_base_t<deflate_t<path>>;

    static constexpr auto path_v = path;

private:
    using base_api_t::deinit_lib_impl;
    using base_api_t::job_;
    using base_t::bytes_read_;
    using base_t::bytes_written_;

public:
    deflate_t() noexcept {}
    ~deflate_t() noexcept(false)
    {
        deinit_lib_impl();
    }

protected:
    void init_buffers_impl(const params_t &params)
    {
        params_ = params;
        stream_.resize(params_.p_source_data_->buffer.size()*1.5);
    }

    void init_lib_params_impl()
    {
        job_->next_in_ptr   = const_cast<std::uint8_t*>(params_.p_source_data_->buffer.data());
        job_->available_in  = static_cast<std::uint32_t>(params_.p_source_data_->buffer.size());
        job_->next_out_ptr  = stream_.data();
        job_->available_out = static_cast<std::uint32_t>(stream_.size());
        job_->op            = qpl_op_compress;
        job_->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY;
        job_->huffman_table = nullptr;

        if(params_.huffman_only_)
            job_->flags |= QPL_FLAG_GEN_LITERALS;
        if(params_.no_headers_)
            job_->flags |= QPL_FLAG_NO_HDRS;

        if(params_.huffman_ == huffman_type_e::dynamic)
            job_->flags |= QPL_FLAG_DYNAMIC_HUFFMAN;
        else if(params_.huffman_ == huffman_type_e::fixed)
            job_->flags &= ~QPL_FLAG_DYNAMIC_HUFFMAN;
        else if (params_.huffman_ == huffman_type_e::canned)
        {
            job_->flags |= QPL_FLAG_CANNED_MODE;
            job_->huffman_table = params_.p_huffman_table_.get();
        }
        else
            throw std::runtime_error(format("invalid Huffman mode: %s", to_string(params_.huffman_).c_str()));

        if(params_.level_ == 1)
            job_->level = qpl_level_1;
        else if(params_.level_ == 3)
            job_->level = qpl_level_3;
        else
            throw std::runtime_error(format("invalid level: %d", params_.level_));
    }

    void sync_execute_impl()
    {
        auto status = qpl_execute_job(job_);
        if(QPL_STS_OK == status)
        {
            stream_size_   = job_->total_out;
            bytes_read_    = job_->total_in;
            bytes_written_ = job_->total_out;
        }
        else
            throw std::runtime_error(format("qpl_execute_job() failed with status %d", status));
    }

    void async_submit_impl()
    {
        auto status = qpl_submit_job(job_);
        if(QPL_STS_OK != status)
            throw std::runtime_error(format("qpl_submit_job() failed with status %d", status));
    }

    task_status_e async_wait_impl()
    {
        auto status = qpl_wait_job(job_);
        if(QPL_STS_OK == status)
        {
            stream_size_   = job_->total_out;
            bytes_read_    = job_->total_in;
            bytes_written_ = job_->total_out;
            return task_status_e::completed;
        }
        else
            throw std::runtime_error(format("qpl_wait_job() failed with status %d", status));
    }

    [[nodiscard]] task_status_e async_poll_impl()
    {
        auto status = qpl_check_job(job_);
        if(QPL_STS_BEING_PROCESSED == status)
            return task_status_e::in_progress;
        else
        {
            if(QPL_STS_OK == status)
            {
                stream_size_   = job_->total_out;
                bytes_read_    = job_->total_in;
                bytes_written_ = job_->total_out;
                return task_status_e::completed;
            }
            else
                throw std::runtime_error(format("qpl_check_job() failed with status %d", status));
        }
    }

    void light_reset_impl() noexcept
    {
        job_->next_in_ptr   -= job_->total_in;
        job_->available_in  += job_->total_in;
        job_->next_out_ptr  -= job_->total_out;
        job_->available_out += job_->total_out;
        job_->total_in  = 0;
        job_->total_out = 0;
    }

    void mem_control_impl(mem_loc_e op, mem_loc_mask_e mask) const noexcept
    {
        if(mask&mem_loc_mask_e::src1)
            details::mem_control(params_.p_source_data_->buffer.begin(), params_.p_source_data_->buffer.end(), op);
        if(mask&mem_loc_mask_e::dst1)
            details::mem_control(stream_.begin(), stream_.end(), op);
    }

    result_t& get_result_impl() noexcept
    {
        result_.stream_ = stream_;
        result_.stream_.resize(stream_size_);
        return result_;
    }

private:
    friend class ops::operation_base_t<deflate_t>;
    friend class operation_base_t<deflate_t>;

    params_t    params_;
    data_type_t stream_;
    std::size_t stream_size_{0};
    result_t    result_;
};
}

namespace bench::ops
{
template <path_e path>
struct traits<operation_base_t<c_api::deflate_t<path>>>
{
    using result_t = deflate_results_t;
};
}
