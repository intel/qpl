/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef MIDDLE_LAYER_DEFS_HPP
#define MIDDLE_LAYER_DEFS_HPP

/**
 * @defgroup MIDDLE_LAYER_API Private API: Middle Layer API
 */

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <limits>

#include "qpl/c_api/defs.h"
#include "qpl/c_api/status.h"

namespace qpl::ml {

typedef uint32_t qpl_ml_status;

namespace status_list {

constexpr qpl_ml_status ok                                 = 0;
constexpr qpl_ml_status more_output_needed                 = QPL_STS_MORE_OUTPUT_NEEDED;
constexpr qpl_ml_status internal_error                     = QPL_STS_LIBRARY_INTERNAL_ERR;
constexpr qpl_ml_status nullptr_error                      = QPL_STS_NULL_PTR_ERR;
constexpr qpl_ml_status verify_error                       = QPL_STS_INTL_VERIFY_ERR;
constexpr qpl_ml_status index_generation_error             = QPL_STS_INDEX_GENERATION_ERR;
constexpr qpl_ml_status gzip_header_error                  = QPL_STS_ARCHIVE_HEADER_ERR;
constexpr qpl_ml_status need_dictionary_error              = QPL_STS_INFLATE_NEED_DICT_ERR;
constexpr qpl_ml_status input_too_small                    = QPL_STS_MORE_INPUT_NEEDED;
constexpr qpl_ml_status invalid_compression_style_error    = QPL_STS_INVALID_COMPRESS_STYLE_ERR;
constexpr qpl_ml_status not_supported_err                  = QPL_STS_NOT_SUPPORTED_MODE_ERR;
constexpr qpl_ml_status not_supported_level_err            = QPL_STS_UNSUPPORTED_COMPRESSION_LEVEL;
constexpr qpl_ml_status status_invalid_params              = QPL_STS_INVALID_PARAM_ERR;
constexpr qpl_ml_status hardware_error_base                = QPL_OPERATION_ERROR_BASE;
constexpr qpl_ml_status hardware_status_base               = QPL_OPERATION_STATUS_BASE;
constexpr qpl_ml_status destination_is_short_error         = QPL_STS_DST_IS_SHORT_ERR;
constexpr qpl_ml_status source_is_short_error              = QPL_STS_SRC_IS_SHORT_ERR;
constexpr qpl_ml_status bit_width_error                    = QPL_STS_BIT_WIDTH_ERR;
constexpr qpl_ml_status source_2_is_short_error            = QPL_STS_SRC2_IS_SHORT_ERR;
constexpr qpl_ml_status drop_bits_overflow_error           = QPL_STS_DROP_BITS_OVERFLOW_ERR;
constexpr qpl_ml_status size_error                         = QPL_STS_SIZE_ERR;
constexpr qpl_ml_status output_overflow_error              = QPL_STS_OUTPUT_OVERFLOW_ERR;
constexpr qpl_ml_status buffers_overlap                    = QPL_STS_BUFFER_OVERLAP_ERR;
constexpr qpl_ml_status compression_reference_before_start = QPL_STS_REF_BEFORE_START_ERR;
constexpr qpl_ml_status output_format_error                = QPL_STS_OUT_FORMAT_ERR;

} // namespace status_list

struct checksums_t {
    uint32_t crc32_   = 0U;
    uint32_t xor_     = 0U;
    uint32_t adler32_ = 0U;
};

struct aggregates_t {
    uint32_t min_value_ = std::numeric_limits<uint32_t>::max();
    uint32_t max_value_ = 0U;
    uint32_t sum_       = 0U;
    uint32_t index_     = 0U;
};

enum class execution_path_t { auto_detect, hardware, software };

constexpr uint32_t bit_bits_size                = 1U;
constexpr uint32_t byte_bits_size               = 8U;
constexpr uint32_t short_bits_size              = 16U;
constexpr uint32_t int_bits_size                = 32U;
constexpr uint32_t bit_len_to_byte_shift_offset = 3U;
constexpr uint32_t max_bit_index                = 7U;
constexpr uint32_t qpl_1k                       = 1024U;
constexpr uint32_t max_history_size             = 4U * qpl_1k;

namespace limits {
constexpr uint32_t max_bit_width    = int_bits_size;
constexpr uint32_t min_bit_width    = bit_bits_size;
constexpr uint32_t max_set_size     = 15U;
constexpr uint32_t set_buf_bit_size = (1U << max_set_size);
constexpr uint32_t set_buf_size     = set_buf_bit_size / byte_bits_size;

static_assert(set_buf_size == 4096U, "Intermediate buffer for size is too small");
} // namespace limits

} // namespace qpl::ml

#endif // MIDDLE_LAYER_DEFS_HPP
