/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/cpp_api/results/inflate_stream.hpp"
#include "qpl/cpp_api/operations/compression/inflate_stateful_operation.hpp"
#include "qpl/cpp_api/util/status_handler.hpp"

template <qpl::execution_path path>
auto qpl::inflate_stream<path>::extract_impl(uint8_t *destination_begin,
                                             size_t destination_size) -> inflate_stream & {
    if (this->state_ == compression_stream_state::initial) {
        operation_->first_chunk(true);
        operation_->last_chunk(false);
        this->state_ = compression_stream_state::basic;
    } else {
        operation_->first_chunk(false);
        operation_->last_chunk(false);
    }

    if (this->buffer_current_ >= this->buffer_end_) {
        throw operation_process_exception("Source buffer has no more bytes");
    }

    operation_->set_proper_flags();
    auto result = internal::execute<path>(*operation_,
                                          this->buffer_current_,
                                          this->buffer_end_,
                                          destination_begin,
                                          destination_begin + destination_size);

    result.if_absent([](uint32_t status) -> void {
        util::handle_status(status);
    });

    auto shift = operation_->get_processed_bytes();
    this->buffer_current_ = this->destination_buffer_.get() + shift;

    return *this;
}

template <qpl::execution_path path>
void qpl::inflate_stream<path>::constructor_impl(uint8_t *source_begin,
                                                 size_t source_size) {
    std::copy(source_begin, source_begin + source_size, this->destination_buffer_.get());

    operation_->set_job_buffer(this->job_buffer_.get());
    operation_->init_job(path);
}

template
void qpl::inflate_stream<qpl::execution_path::software>::constructor_impl(uint8_t *source_begin,
                                                                          size_t source_size);

template
void qpl::inflate_stream<qpl::execution_path::hardware>::constructor_impl(uint8_t *source_begin,
                                                                          size_t source_size);

template
auto qpl::inflate_stream<qpl::execution_path::software>::extract_impl(uint8_t *destination_begin,
                                                                      size_t destination_size) -> inflate_stream &;

template
auto qpl::inflate_stream<qpl::execution_path::hardware>::extract_impl(uint8_t *destination_begin,
                                                                      size_t destination_size) -> inflate_stream &;
