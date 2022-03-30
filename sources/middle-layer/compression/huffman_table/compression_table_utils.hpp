/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_MIDDLE_LAYER_COMPRESSION_COMPRESSION_TABLE_UTILS_HPP
#define QPL_MIDDLE_LAYER_COMPRESSION_COMPRESSION_TABLE_UTILS_HPP

#include <cstdint>
#include "compression_table.hpp"
#include "igzip_lib.h"
#include "dispatcher/dispatcher.hpp"
#include "common/defs.hpp"
#include "common/bit_reverse.hpp"

namespace qpl::ml::compression {
constexpr uint32_t literals_count = 256u;
constexpr uint32_t literal_length_count = 286u;
constexpr uint32_t distances_count = 30u;

void qpl_compression_table_to_isal(const qplc_compression_huffman_table *qpl_compression_table,
                                   isal_hufftables *isal_compression_table) noexcept;

void isal_compression_table_to_qpl(const isal_hufftables *isal_compression_table,
                                   qplc_compression_huffman_table *qpl_compression_table) noexcept;

auto build_compression_table(const uint32_t *literals_lengths_histogram_ptr,
                             const uint32_t *distances_histogram_ptr,
                             compression_huffman_table &compression_table) noexcept -> qpl_ml_status;

static void inline fill_histogram(const uint32_t *literals_lengths_histogram_ptr,
                                  const uint32_t *distances_histogram_ptr,
                                  isal_huff_histogram *histogram) {
    for (uint32_t i = 0u; i < literal_length_count; i++) {
        histogram->lit_len_histogram[i] = static_cast<uint32_t>(literals_lengths_histogram_ptr[i]);
    }

    for (uint32_t i = 0u; i < distances_count; i++) {
        histogram->dist_histogram[i] = static_cast<uint32_t>(distances_histogram_ptr[i]);
    }
}

static void inline fill_histogram_literals_only(const uint32_t *literals_lengths_histogram_ptr,
                                                isal_huff_histogram *histogram) {
    for (uint32_t i = 0u; i < literals_count; i++) {
        histogram->lit_len_histogram[i] = static_cast<uint32_t>(literals_lengths_histogram_ptr[i]);
    }
}


void inline store_isal_deflate_header(isal_hufftables *isal_huffman_table,
                                      compression_huffman_table &compression_table) noexcept {
    auto header_complete_byte_size = isal_huffman_table->deflate_hdr_count;
    header_complete_byte_size += (0u == isal_huffman_table->deflate_hdr_extra_bits) ? 0u : 1u;

    // Use copy kernel to copy deflate header from isal huffman tables
    auto copy_kernel = dispatcher::kernels_dispatcher::get_instance().get_memory_copy_table();
    copy_kernel[0]((uint8_t *) isal_huffman_table->deflate_hdr,
                compression_table.get_deflate_header_data(),
                header_complete_byte_size);

    // Calculate and store deflate header bits size
    const auto header_bit_size = isal_huffman_table->deflate_hdr_count * byte_bits_size + (isal_huffman_table->deflate_hdr_extra_bits);
    compression_table.set_deflate_header_bit_size(header_bit_size);
}
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
