/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_MIDDLE_LAYER_COMPRESSION_COMPRESSION_TABLE_UTILS_HPP
#define QPL_MIDDLE_LAYER_COMPRESSION_COMPRESSION_TABLE_UTILS_HPP

#include "common/defs.hpp"
#include "compression_table.hpp"
#include "igzip_lib.h"

namespace qpl::ml::compression {

void qpl_compression_table_to_isal(const qplc_huffman_table_default_format *qpl_compression_table,
                                   isal_hufftables *isal_compression_table) noexcept;

void isal_compression_table_to_qpl(const isal_hufftables *isal_compression_table,
                                   qplc_huffman_table_default_format *qpl_compression_table) noexcept;

auto build_compression_table(const uint32_t *literals_lengths_histogram_ptr,
                             const uint32_t *distances_histogram_ptr,
                             compression_huffman_table &compression_table) noexcept -> qpl_ml_status;
}

/**
 * Flag which indicates whenever hardware representation of compression/decompression table should be used
 */
#define QPL_HW_REPRESENTATION            0x01u

/**
 * Flag which indicates whenever deflate header should be used
 */
#define QPL_DEFLATE_REPRESENTATION       0x04u

/**
 * Flag which indicates whenever software representation of compression/decompression table should be used
 */
#define QPL_SW_REPRESENTATION            0x08u

/**
 * Flag which indicates whenever huffman only representation of compression/decompression table should be used
 */
#define QPL_HUFFMAN_ONLY_REPRESENTATION  0x10u

/**
 * Combine all (software, hardware, deflate) representation flags to build the complete compression table
 */
#define QPL_COMPLETE_COMPRESSION_TABLE (QPL_HW_REPRESENTATION | QPL_DEFLATE_REPRESENTATION | QPL_SW_REPRESENTATION)
/** @} */

#endif // QPL_MIDDLE_LAYER_COMPRESSION_COMPRESSION_TABLE_UTILS_HPP
