/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef COPY_OPERATION_HPP
#define COPY_OPERATION_HPP

#include "other/other_defs.hpp"
#include "common/defs.hpp"

namespace qpl::ml::other {

template <execution_path_t path>
auto call_copy(const uint8_t *src_ptr,
               uint8_t *dst_ptr,
               uint32_t length,
               int32_t numa_id = -1) noexcept -> copy_operation_result_t;

} // namespace qpl::ml::other

#endif // COPY_OPERATION_HPP
