/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TEST_COMPARE_HUFFMAN_TABLE_HPP
#define QPL_TEST_COMPARE_HUFFMAN_TABLE_HPP

// qpl_huffman_table_t definition
#include "qpl/c_api/huffman_table.h"

namespace qpl::test {
[[nodiscard]] qpl_status qpl_huffman_table_compare(const qpl_huffman_table_t table,
                                                   const qpl_huffman_table_t other_table,
                                                   bool*                     are_huffman_tables_equal);
}

#endif