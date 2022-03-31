/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Job API (private C API)
 */

#ifndef QPL_STATISTICS__HPP_
#define QPL_STATISTICS__HPP_

#include "qpl/c_api/huffman_table.h"
#include "compression/huffman_table/huffman_table_utils.hpp"

#ifdef __cplusplus
extern "C" {
#endif

// Todo Remove after refactoring
qpl_compression_huffman_table* own_huffman_table_get_compression_table(const qpl_huffman_table_t table);

qpl_decompression_huffman_table* own_huffman_table_get_decompression_table(const qpl_huffman_table_t table);

#ifdef __cplusplus
}
#endif

#endif //QPL_STATISTICS__HPP_
