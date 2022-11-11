/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/qpl.h"

#include "qpl/cpp_api/util/status_handler.hpp"
#include "qpl/cpp_api/util/constants.hpp"

namespace qpl::util {

void handle_status(uint32_t status_code) {
    switch (status_code) {
        case QPL_STS_BEING_PROCESSED: {
            throw operation_process_exception(messages::being_processed);
        }
        case QPL_STS_MISSING_INDEX_TABLE_ERR: {
            throw invalid_argument_exception(messages::index_table_missed);
        }
        case QPL_STS_MORE_OUTPUT_NEEDED: {
            throw memory_underflow_exception(messages::more_output_needed);
        }
        case QPL_STS_INVALID_DEFLATE_DATA_ERR: {
            throw invalid_data_exception(messages::invalid_deflate_data);
        }
        case QPL_STS_INVALID_PARAM_ERR: {
            throw invalid_argument_exception(messages::invalid_parameter);
        }
        case QPL_STS_BUFFER_TOO_LARGE_ERR: {
            throw memory_overflow_exception(messages::buffer_exceeds_max_size);
        }
        case QPL_STS_LIBRARY_INTERNAL_ERR: {
            throw exception(messages::library_internal_error);
        }
        case QPL_STS_INTL_VERIFY_ERR: {
            throw operation_process_exception(messages::verify_error);
        }
        case QPL_STS_INDEX_GENERATION_ERR: {
            throw operation_process_exception(messages::invalid_index_generation);
        }
        case QPL_STS_INDEX_ARRAY_TOO_SMALL: {
            throw memory_underflow_exception(messages::index_array_too_small);
        }
        case QPL_STS_ARCHIVE_HEADER_ERR: {
            throw invalid_data_exception(messages::invalid_gzip_header);
        }
        case QPL_STS_MORE_INPUT_NEEDED: {
            throw memory_underflow_exception(messages::input_too_small);
        }
        case QPL_STS_INVALID_BLOCK_SIZE_ERR: {
            throw invalid_data_exception(messages::invalid_block_size);
        }
        case QPL_STS_INVALID_HUFFMAN_TABLE_ERR: {
            throw invalid_data_exception(messages::invalid_huffman_table);
        }
        case QPL_STS_NULL_PTR_ERR: {
            throw invalid_argument_exception(messages::null_ptr_error);
        }
        case QPL_STS_DROP_BITS_OVERFLOW_ERR: {
            throw invalid_argument_exception(messages::incorrect_ignore_bits_value);
        }
        case QPL_STS_CRC64_BAD_POLYNOM: {
            throw invalid_argument_exception(messages::incorrect_crc_64_polynomial);
        }
        case QPL_STS_NO_MEM_ERR: {
            throw memory_underflow_exception(messages::memory_allocation_error);
        }
        case QPL_STS_SIZE_ERR: {
            throw memory_underflow_exception(messages::incorrect_size);
        }
        case QPL_STS_PRLE_FORMAT_ERR: {
            throw invalid_argument_exception(messages::incorrect_prle_format);
        }
        case QPL_STS_OUTPUT_OVERFLOW_ERR: {
            throw memory_overflow_exception(messages::output_overflow);
        }
        case QPL_STS_SRC2_IS_SHORT_ERR: {
            throw invalid_data_exception(messages::short_mask);
        }
        case QPL_STS_DST_IS_SHORT_ERR: {
            throw short_destination_exception(messages::short_destination);
        }
        case QPL_STS_INTL_DIST_SPANS_MINI_BLOCKS: {
            throw invalid_data_exception(messages::distance_spans_mini_blocks);
        }
        case QPL_STS_INTL_LEN_SPANS_MINI_BLOCKS: {
            throw invalid_data_exception(messages::length_spans_mini_blocks);
        }
        case QPL_STS_INTL_VERIF_INVALID_BLOCK_SIZE: {
            throw invalid_data_exception(messages::verif_invalid_block_size);
        }
        case QPL_STS_SRC_IS_SHORT_ERR: {
            throw memory_underflow_exception(messages::short_source);
        }
        default: {
            // TODO should be removed
            if (status_code) {
                throw operation_process_exception(
                        "Unexpected error code returned while processing an operation");
            }
        }
    }
}

} // namespace qpl::util
