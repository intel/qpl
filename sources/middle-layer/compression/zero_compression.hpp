/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef ZERO_COMPRESS_OPERATION_HPP
#define ZERO_COMPRESS_OPERATION_HPP

#include "zero_defs.hpp"
#include "common/defs.hpp"
#include "common/buffer.hpp"

namespace qpl::ml::compression {

template <execution_path_t path,
        zero_operation_type operation_type>
auto call_zero_operation(zero_input_format_t input_format,
                         const buffer_t &source_buffer,
                         const buffer_t &destination_buffer,
                         crc_type_t crc_type,
                         int32_t numa_id = -1) noexcept -> zero_operation_result_t;

} // namespace qpl::ml::compression

#endif // ZERO_COMPRESS_OPERATION_HPP
