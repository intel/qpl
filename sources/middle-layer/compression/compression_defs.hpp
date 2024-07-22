/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Middle Layer API (private C++ API)
 */

#ifndef QPL_COMPRESSION_DEFS_HPP_
#define QPL_COMPRESSION_DEFS_HPP_

#include "common/defs.hpp"
#include "hw_definitions.h"

namespace qpl::ml::compression {

enum class compression_algorithm_e : uint8_t { deflate, canned, huffman_only };

struct decompression_operation_result_t {
    uint32_t    status_code_     = 0U;
    uint32_t    output_bytes_    = 0U;
    uint32_t    completed_bytes_ = 0U;
    checksums_t checksums_       = {};
};

struct compression_operation_result_t {
    uint32_t                  status_code_      = 0U;
    uint32_t                  output_bytes_     = 0U;
    uint32_t                  completed_bytes_  = 0U;
    uint32_t                  indexes_written_  = 0U;
    uint32_t                  last_bit_offset   = 0U;
    checksums_t               checksums_        = {};
    hw_multidescriptor_status multi_desc_status = qpl_none_completed;
};

struct verification_pass_result_t {
    uint32_t    status_code_     = 0U;
    uint32_t    indexes_written_ = 0U;
    uint32_t    completed_bytes_ = 0U;
    checksums_t checksums_       = {};
};

} // namespace qpl::ml::compression

#endif //QPL_COMPRESSION_DEFS_HPP_
