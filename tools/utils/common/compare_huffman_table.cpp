/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <cstring>

#include "qpl/c_api/huffman_table.h" // qpl_huffman_table_t

// middle-layer
#include "compression/huffman_table/huffman_table.hpp" // huffman_table_t
#include "util/checkers.hpp"                           // bad_argument

namespace qpl::test {

/**
 * @brief Function to compare two Huffman tables.
 * Internals of huffman_table_t are used for validation purposes,
 * so we need access to internal details (namespaces, declarations, implementations).
 */
qpl_status qpl_huffman_table_compare(const qpl_huffman_table_t table, const qpl_huffman_table_t other_table,
                                     bool* are_huffman_tables_equal) {
    using namespace qpl::ml;
    using namespace qpl::ml::compression;

    *are_huffman_tables_equal = false;

    OWN_QPL_CHECK_STATUS(bad_argument::check_for_nullptr(table));
    OWN_QPL_CHECK_STATUS(bad_argument::check_for_nullptr(other_table));

    auto meta_ptr       = reinterpret_cast<huffman_table_meta_t*>(table);
    auto other_meta_ptr = reinterpret_cast<huffman_table_meta_t*>(other_table);

    bool      is_meta_equal = true;
    const int meta_diff     = std::memcmp(meta_ptr, other_meta_ptr, //NOLINT(bugprone-suspicious-memory-comparison)
                                          sizeof(huffman_table_meta_t));
    if (meta_diff != 0) is_meta_equal = false;

    bool are_int_tables_equal = true;
    if (meta_ptr->algorithm == compression_algorithm_e::deflate) {
        auto table_impl       = reinterpret_cast<huffman_table_t<compression_algorithm_e::deflate>*>(table);
        auto other_table_impl = reinterpret_cast<huffman_table_t<compression_algorithm_e::deflate>*>(other_table);

        are_int_tables_equal = table_impl->is_equal(*other_table_impl);
    } else {
        return QPL_STS_NOT_SUPPORTED_MODE_ERR;
    }

    *are_huffman_tables_equal = (is_meta_equal && are_int_tables_equal);

    return QPL_STS_OK;
}

} // namespace qpl::test
