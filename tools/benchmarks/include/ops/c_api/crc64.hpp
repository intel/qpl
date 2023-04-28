/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <ops/c_api/base.hpp>
#include <stdexcept>

namespace bench::ops::c_api
{

template <path_e path>
class crc64_t: public operation_base_t<crc64_t<path>>
{
public:
    using result_t    = crc64_results_t;
    using params_t    = crc64_params_t;
    using data_type_t = typename result_t::data_type_t;
    using base_t      = ops::operation_base_t<crc64_t<path>>;
    using base_api_t  = operation_base_t<crc64_t<path>>;

    static constexpr auto path_v = path;

private:
    using base_api_t::deinit_lib_impl;
    using base_api_t::job_;
    using base_t::bytes_read_;
    using base_t::bytes_written_;
    static constexpr const uint64_t poly_crc32_gzip_wimax  = 0x04C11DB700000000;
    static constexpr const uint64_t poly_crc32_iscsi       = 0x1EDC6F4100000000;
    static constexpr const uint64_t poly_T10DIF            = 0x8BB7000000000000;
    static constexpr const uint64_t poly_crc16_ccitt       = 0x1021000000000000;

public:
    crc64_t() noexcept {}
    ~crc64_t() noexcept
    {
        deinit_lib_impl();
    }

protected:
    void init_buffers_impl(const params_t &params)
    {
        params_ = params;
        if(params.original_size_)
            data_.resize(params.original_size_);
        else
            data_.resize(params_.p_stream_->buffer.size()*10);
    }

    void init_lib_params_impl() noexcept
    {
        job_->next_in_ptr   = const_cast<std::uint8_t*>(params_.p_stream_->buffer.data());
        job_->available_in  = static_cast<std::uint32_t>(params_.p_stream_->buffer.size());
        job_->next_out_ptr  = data_.data();
        job_->available_out = static_cast<std::uint32_t>(data_.size());
        job_->op            = qpl_op_crc64;
        job_->flags        |= QPL_FLAG_FIRST | QPL_FLAG_LAST;
        switch (params_.crc_type)
        {
        case crc_type_e::crc32_gzip:
            job_->crc64_poly    = poly_crc32_gzip_wimax;
            job_->flags |= QPL_FLAG_CRC64_INV;
            job_->flags |= QPL_FLAG_CRC64_BE;
            break;
        case crc_type_e::crc32_wimax:
            job_->crc64_poly    = poly_crc32_gzip_wimax;
            job_->flags |= QPL_FLAG_CRC64_INV;
            break;
        case crc_type_e::crc32_iscsi:
            job_->crc64_poly = poly_crc32_iscsi;
            job_->flags |= QPL_FLAG_CRC64_INV;
            break;
        case crc_type_e::T10DIF:
            job_->crc64_poly = poly_T10DIF;
            break;
        case crc_type_e::crc16_ccitt:
            job_->crc64_poly = poly_crc16_ccitt;
            job_->flags |= QPL_FLAG_CRC64_INV;
            job_->flags |= QPL_FLAG_CRC64_BE;
            break;
        default:
            // crc64;
            auto poly = get_random_crc64_poly(4, 0);
            job_->crc64_poly = poly;
            break;
        }
    }

    void sync_execute_impl()
    {
        auto status = qpl_execute_job(job_);
        if(QPL_STS_OK == status)
        {
            data_size_     = job_->total_out;
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
            data_size_     = job_->total_out;
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
                data_size_     = job_->total_out;
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
            details::mem_control(params_.p_stream_->buffer.begin(), params_.p_stream_->buffer.end(), op);
        if(mask&mem_loc_mask_e::dst1)
            details::mem_control(data_.begin(), data_.end(), op);
    }

    result_t& get_result_impl() noexcept
    {
        result_.data_ = data_;
        result_.data_.resize(data_size_);
        return result_;
    }

    auto get_random_crc64_poly(uint32_t poly_shift, uint32_t seed) -> uint64_t
    {
        // auto number = rand() % UINT16_MAX + 1;
        uint16_t number = 65523;
        uint64_t      result_poly;
        result_poly = (uint16_t) number;
        result_poly = (result_poly << 16u) ^ (uint16_t) number;
        result_poly = (result_poly << 16u) ^ (uint16_t) number;
        result_poly = (result_poly << 16u) ^ (uint16_t) number;
        result_poly |= 1u;
        result_poly <<= poly_shift;

        return result_poly;
    }    

private:
    friend class ops::operation_base_t<crc64_t>;
    friend class operation_base_t<crc64_t>;

    params_t    params_;
    data_type_t data_;
    std::size_t data_size_{0};
    result_t    result_;
};
}

namespace bench::ops
{
template <path_e path>
struct traits<operation_base_t<c_api::crc64_t<path>>>
{
    using result_t = crc64_results_t;
};
}
