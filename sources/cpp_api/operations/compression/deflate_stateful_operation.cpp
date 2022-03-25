/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/cpp_api/operations/operation.hpp"
#include "qpl/cpp_api/operations/compression/deflate_stateful_operation.hpp"
#include "qpl/cpp_api/util/qpl_util.hpp"
#include "../c_api/compression_operations/huffman_table.hpp"

namespace qpl::internal {
void deflate_stateful_operation::set_proper_flags() noexcept {
    auto job = reinterpret_cast<qpl_job *>(buffer_);

    job->op    = qpl_op_compress;
    job->flags = QPL_FLAG_OMIT_VERIFY | QPL_FLAG_NO_BUFFERING;

    if (is_first_chunk_ && is_last_chunk_) {
        job->flags |= QPL_FLAG_FIRST | QPL_FLAG_LAST;
    } else if (is_first_chunk_) {
        job->flags |= QPL_FLAG_FIRST;
    } else if (is_last_chunk_) {
        job->flags |= QPL_FLAG_LAST;
    } else {
        job->flags &= ~(QPL_FLAG_FIRST | QPL_FLAG_LAST);
    }

    if (properties_.compression_mode_ == compression_modes::static_mode) {
//        own_huffman_table_set_compression_table(job->huffman_table, properties_.huffman_table_.get_table_data()); // @todo Enable static mode
        job->flags |= QPL_FLAG_START_NEW_BLOCK;
    } else if (properties_.compression_mode_ == compression_modes::dynamic_mode) {
        job->flags |= QPL_FLAG_DYNAMIC_HUFFMAN;
    }

    // Configuring GZIP mode
    if (properties_.gzip_mode_) {
        job->flags |= QPL_FLAG_GZIP_MODE;
    } else {
        job->flags &= ~QPL_FLAG_GZIP_MODE;
    }

    // Configuring compression level
    job->level = properties_.compression_level_ == default_level ? qpl_default_level : qpl_high_level;
}

void deflate_stateful_operation::set_buffers() noexcept {
    auto job = reinterpret_cast<qpl_job *>(buffer_);

    job->next_in_ptr   = const_cast<uint8_t *>(operation::source_);
    job->available_in  = static_cast<uint32_t>(operation::source_size_);
    job->next_out_ptr  = const_cast<uint8_t *>(operation::destination_);
    job->available_out = static_cast<uint32_t>(operation::destination_size_);
}

void deflate_stateful_operation::set_job_buffer(uint8_t *buffer) noexcept {
    common_operation::reset_job_buffer(buffer);
}

void deflate_stateful_operation::first_chunk(bool value) noexcept {
    is_first_chunk_ = value;
}

void deflate_stateful_operation::last_chunk(bool value) noexcept {
    is_last_chunk_ = value;
}

auto deflate_stateful_operation::execute() -> std::pair<uint32_t, uint32_t> {
    auto const job  = reinterpret_cast<qpl_job *>(buffer_);
    auto job_status = qpl_submit_job(job);

    std::pair<uint32_t, uint32_t> result(job_status, job->total_out);
    return result;
}

auto deflate_stateful_operation::get_processed_bytes() -> uint32_t {
    auto job = reinterpret_cast<qpl_job *>(buffer_);
    return job->total_out;
}

} // namespace qpl::internal
