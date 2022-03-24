/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef CANNED_UTILITY_H
#define CANNED_UTILITY_H

#include "qpl/qpl.h"
#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus

namespace internal {
// Temporary solution, because High-level-API is based on low-level API and hense use the same C structures
// TODO: remove this function after migration to middle-layer is complete
uint32_t own_gather_deflate_statistics(uint8_t *source_ptr,
                                       const uint32_t source_length,
                                       uint32_t *literal_length_histogram_ptr,
                                       uint32_t *offsets_histogram_ptr,
                                       const qpl_compression_levels level,
                                       const qpl_path_t path);
}

extern "C" {
#endif

typedef struct isal_hufftables isal_hufftables;
typedef struct qpl_compression_huffman_table qpl_compression_huffman_table;
typedef struct qpl_decompression_huffman_table qpl_decompression_huffman_table;

void isal_huffman_table_to_qpl(isal_hufftables *isal_table_ptr,
                               qpl_compression_huffman_table *qpl_table_ptr);

uint32_t own_collect_statistic(uint8_t *const source_ptr,
                               const uint32_t source_size,
                               qpl_histogram *const histogram_ptr,
                               const qpl_compression_levels level,
                               const qpl_path_t path);

uint32_t own_build_compression_table(const uint32_t *literal_lengths_histogram_ptr,
                                     const uint32_t *offsets_histogram_ptr,
                                     qpl_compression_huffman_table *compression_table_ptr,
                                     uint32_t representation_flags);

uint32_t own_triplets_to_compression_table(const qpl_huffman_triplet *triplets_ptr,
                                           size_t triplets_count,
                                           qpl_compression_huffman_table *compression_table_ptr,
                                           uint32_t representation_flags);

uint32_t own_triplets_to_decompression_table(const qpl_huffman_triplet *triplets_ptr,
                                             size_t triplets_count,
                                             qpl_decompression_huffman_table *compression_table_ptr,
                                             uint32_t representation_flags);

uint32_t own_comp_to_decompression_table(const qpl_compression_huffman_table *compression_table_ptr,
                                         qpl_decompression_huffman_table *decompression_table_ptr,
                                         uint32_t representation_flags);

uint8_t * get_lookup_table_buffer_ptr(qpl_decompression_huffman_table *decompression_table_ptr);
void * get_aecs_decompress(qpl_decompression_huffman_table *decompression_table_ptr);

uint32_t * get_literals_lengths_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr);
uint32_t * get_offsets_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr);
uint32_t get_size_of_ll_huffman_table();
uint32_t get_size_of_offsets_huffman_table();

uint8_t * get_deflate_header_ptr(qpl_compression_huffman_table *const huffman_table_ptr);
uint32_t get_deflate_header_bits_size(qpl_compression_huffman_table *const huffman_table_ptr);
void set_deflate_header_bits_size(qpl_compression_huffman_table *const huffman_table_ptr, uint32_t header_bits);

uint8_t * get_sw_compression_huffman_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr);
uint8_t * get_isal_compression_huffman_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr);
uint8_t * get_hw_compression_huffman_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr);
uint8_t * get_deflate_header_buffer_ptr(qpl_compression_huffman_table *const huffman_table_ptr);

uint8_t * get_sw_decompression_table_buffer(qpl_decompression_huffman_table *const decompression_table_ptr);
uint8_t * get_hw_decompression_table_buffer(qpl_decompression_huffman_table *const decompression_table_ptr);
uint8_t *get_deflate_header_buffer(qpl_decompression_huffman_table *const decompression_table_ptr);

bool is_sw_representation_used(qpl_decompression_huffman_table *const decompression_table_ptr);
bool is_hw_representation_used(qpl_decompression_huffman_table *const decompression_table_ptr);
bool is_deflate_representation_used(qpl_decompression_huffman_table *const decompression_table_ptr);

uint16_t * get_number_of_codes_ptr(qpl_decompression_huffman_table *const decompression_table_ptr);
uint16_t * get_first_codes_ptr(qpl_decompression_huffman_table *const decompression_table_ptr);
uint16_t * get_first_table_indexes_ptr(qpl_decompression_huffman_table *const decompression_table_ptr);
uint8_t * get_index_to_char_ptr(qpl_decompression_huffman_table *const decompression_table_ptr);

#ifdef __cplusplus
}
#endif

#endif // CANNED_UTILITY_H
