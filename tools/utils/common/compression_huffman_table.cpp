/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

// tool_common
#include "compression_huffman_table.hpp"

// qpl_c_api
#include "compression_operations/huffman_table.hpp"

extern "C" qpl_compression_huffman_table* own_huffman_table_get_compression_table(const qpl_huffman_table_t table);

namespace qpl::test {
qpl_status fill_compression_table(qpl_huffman_table_t table) {
    auto table_ptr = own_huffman_table_get_compression_table(table);
    if (!table_ptr) return QPL_STS_NULL_PTR_ERR;

    uint32_t* literals_lengths_ptr = get_literals_lengths_table_ptr(table_ptr);
    uint32_t* offsets_ptr          = get_offsets_table_ptr(table_ptr);

    for (uint32_t i = 0; i < 286; i++) {
        literals_lengths_ptr[i] = literals_match_table[i];
    }

    for (uint32_t i = 0; i < 30; i++) {
        offsets_ptr[i] = offsets_table[i];
    }

    for (uint32_t i = 0; i < 206; i++) {
        get_deflate_header_ptr(table_ptr)[i] = deflate_header[i];
    }

    set_deflate_header_bits_size(table_ptr, 1002U);

    auto isal_table = reinterpret_cast<isal_hufftables*>(get_isal_compression_huffman_table_ptr(table_ptr));

    ml::compression::huffman_table_convert(*table_ptr, *isal_table);
    return QPL_STS_OK;
}
} // namespace qpl::test
