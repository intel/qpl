/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/cpp_api/operations/operation.hpp"
#include "qpl/cpp_api/operations/compression/inflate_stateful_operation.hpp"
#include "qpl/cpp_api/util/qpl_util.hpp"

namespace qpl::internal {

void inflate_stateful_operation::set_proper_flags() noexcept {
    auto job = reinterpret_cast<qpl_job *>(buffer_);

    switch (random_access_mode) {
        case util::mini_block: {
            job->flags = QPL_FLAG_RND_ACCESS | QPL_FLAG_NO_BUFFERING;
            break;
        }
        case util::header: {
            job->flags = QPL_FLAG_FIRST | QPL_FLAG_NO_BUFFERING;
            break;
        }
        default: {
            if (is_first_chunk_ && is_last_chunk_) {
                job->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST;
            } else if (is_first_chunk_) {
                job->flags = QPL_FLAG_FIRST;
            } else if (is_last_chunk_) {
                job->flags = QPL_FLAG_LAST;
            } else {
                job->flags &= ~(QPL_FLAG_FIRST | QPL_FLAG_LAST);
            }
            break;
        }
    }

    job->op = qpl_op_decompress;

    // Configuring GZIP mode
    if (properties_.gzip_mode_) {
        job->flags |= QPL_FLAG_GZIP_MODE;
    } else {
        job->flags &= ~QPL_FLAG_GZIP_MODE;
    }
}

void inflate_stateful_operation::set_buffers() noexcept {
    auto job = reinterpret_cast<qpl_job *>(buffer_);

    job->next_in_ptr   = const_cast<uint8_t *>(operation::source_);
    job->available_in  = static_cast<uint32_t>(operation::source_size_);
    job->next_out_ptr  = const_cast<uint8_t *>(operation::destination_);
    job->available_out = static_cast<uint32_t>(operation::destination_size_);

    if (random_access_mode != util::disabled) {
        job->ignore_start_bits = static_cast<uint32_t>(start_bit_offset_);
        job->ignore_end_bits   = static_cast<uint32_t>(end_bit_offset_);
    }
}

void inflate_stateful_operation::set_job_buffer(uint8_t *buffer) noexcept {
    common_operation::reset_job_buffer(buffer);
}

auto inflate_stateful_operation::init_job(const execution_path path) noexcept -> uint32_t {
    auto job = reinterpret_cast<qpl_job *>(buffer_);

    auto status = qpl_init_job(util::execution_path_to_qpl_path(path), job);

    if (QPL_STS_OK != status) {
        return status;
    }

    this->set_proper_flags();
    this->set_buffers();

    return status;
}

void inflate_stateful_operation::first_chunk(bool value) noexcept {
    is_first_chunk_ = value;
}

void inflate_stateful_operation::last_chunk(bool value) noexcept {
    is_last_chunk_ = value;
}

auto inflate_stateful_operation::get_processed_bytes() -> uint32_t {
    auto job = reinterpret_cast<qpl_job *>(buffer_);
    return job->total_in;
}

void inflate_stateful_operation::enable_random_access(util::random_access_mode value) noexcept {
    random_access_mode = value;

    this->set_proper_flags();
}

void inflate_stateful_operation::set_start_bit_offset(const uint32_t value) noexcept {
    start_bit_offset_ = value;
}

void inflate_stateful_operation::set_end_bit_offset(const uint32_t value) noexcept {
    end_bit_offset_ = value;
}

template <execution_path path>
auto execute(inflate_stateful_operation &op,
             uint8_t *source_begin,
             uint8_t *source_end,
             uint8_t *dest_begin,
             uint8_t *dest_end) -> execution_result<uint32_t, sync> {
    auto *operation_helper = dynamic_cast<operation *>(&op);
    operation_helper->set_buffers(source_begin, source_end, dest_begin, dest_end);

    op.set_buffers();
    auto result = op.execute();

    return execution_result<uint32_t, sync>(result.first, result.second);
}

template
auto execute<execution_path::software>(inflate_stateful_operation &op,
                                       uint8_t *source_begin,
                                       uint8_t *source_end,
                                       uint8_t *dest_begin,
                                       uint8_t *dest_end) -> execution_result<uint32_t, sync>;

template
auto execute<execution_path::hardware>(inflate_stateful_operation &op,
                                       uint8_t *source_begin,
                                       uint8_t *source_end,
                                       uint8_t *dest_begin,
                                       uint8_t *dest_end) -> execution_result<uint32_t, sync>;

} // namespace qpl::internal
