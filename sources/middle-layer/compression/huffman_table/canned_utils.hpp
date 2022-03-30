/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Middle-level layer
 */

#ifndef QPL_MIDDLE_LAYER_COMPRESSION_CANNED_UTILS_HPP
#define QPL_MIDDLE_LAYER_COMPRESSION_CANNED_UTILS_HPP

#include "compression_table.hpp"
#include "decompression_table.hpp"
#include "common/defs.hpp"
#include "dispatcher/dispatcher.hpp"
#include "util/util.hpp"

#include <cstddef>
#include <type_traits>

/**
 * @brief Structure that represents information that is required for compression
 */
struct qpl_compression_huffman_table {
    /**
    * Buffer that contains Intel QPL representation of the software compression table
    */
    std::aligned_storage_t<sizeof(qplc_compression_huffman_table),
                           qpl::ml::util::default_alignment> sw_compression_table_data;
    
    /**
    * Buffer that contains ISA-L representation of the software compression table
    */
    std::aligned_storage_t<sizeof(isal_hufftables),
                           qpl::ml::util::default_alignment> isal_compression_table_data;

    /**
    * Buffer that contains representation of the hardware compression table
    * @note currently this is just a stab, this field is not actually used anywhere
    */
    std::aligned_storage_t<sizeof(qpl::ml::compression::hw_compression_huffman_table),
                           qpl::ml::util::default_alignment> hw_compression_table_data;

    /**
    * Buffer that contains information about deflate header
    */
    std::aligned_storage_t<sizeof(qpl::ml::compression::deflate_header),
                           qpl::ml::util::default_alignment> deflate_header_buffer;

    /**
    * Flag that indicates which representation is used. Possible values are (or their combinations):
    * QPL_HW_REPRESENTATION
    * QPL_SW_REPRESENTATION
    * QPL_DEFLATE_REPRESENTATION
    */
    uint32_t representation_mask;
};

/**
* @brief Structure that represents information that is required for decompression
*/
struct qpl_decompression_huffman_table {
    /**
    * Buffer that contains representation of the software decompression table
    */
    std::aligned_storage_t<sizeof(qpl::ml::compression::sw_decompression_huffman_table),
                           qpl::ml::util::default_alignment> sw_flattened_table;

    /**
    * Buffer that contains representation of the hardware compression table
    * @note currently this is just a stab, this field is not actually used anywhere
    */
    std::aligned_storage_t<sizeof(qpl::ml::compression::hw_decompression_state),
                           HW_PATH_STRUCTURES_REQUIRED_ALIGN> hw_decompression_state;

    /**
    * Buffer that contains information about deflate header
    */
    std::aligned_storage_t<sizeof(qpl::ml::compression::deflate_header),
                           qpl::ml::util::default_alignment> deflate_header_buffer;

    /**
    * Flag that indicates which representation is used. Possible values are (or their combinations):
    * QPL_HW_REPRESENTATION
    * QPL_SW_REPRESENTATION
    * QPL_DEFLATE_REPRESENTATION
    */
    uint32_t representation_mask;

    /**
    * This field is used for canned mode only (software path). Contains lookup table for further decompression.
    */
    std::aligned_storage_t<sizeof(qpl::ml::compression::canned_table),
                           qpl::ml::util::default_alignment> lookup_table_buffer;
};

uint32_t own_build_compression_table(const uint32_t *literal_lengths_histogram_ptr,
                                     const uint32_t *offsets_histogram_ptr,
                                     qpl_compression_huffman_table *compression_table_ptr,
                                     uint32_t representation_flags);

uint32_t own_comp_to_decompression_table(const qpl_compression_huffman_table *compression_table_ptr,
                                         qpl_decompression_huffman_table *decompression_table_ptr,
                                         uint32_t representation_flags);

extern "C" {
uint8_t *get_lookup_table_buffer_ptr(qpl_decompression_huffman_table *decompression_table_ptr);
void *get_aecs_decompress(qpl_decompression_huffman_table *decompression_table_ptr);

uint32_t *get_literals_lengths_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr);
uint32_t *get_offsets_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr);

uint8_t *get_deflate_header_ptr(qpl_compression_huffman_table *const huffman_table_ptr);
uint32_t get_deflate_header_bits_size(qpl_compression_huffman_table *const huffman_table_ptr);
void set_deflate_header_bits_size(qpl_compression_huffman_table *const huffman_table_ptr, uint32_t header_bits);

uint8_t *get_sw_compression_huffman_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr);
uint8_t *get_isal_compression_huffman_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr);
uint8_t *get_hw_compression_huffman_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr);

uint8_t *get_sw_decompression_table_buffer(qpl_decompression_huffman_table *const decompression_table_ptr);
uint8_t *get_hw_decompression_table_buffer(qpl_decompression_huffman_table *const decompression_table_ptr);
uint8_t *get_deflate_header_buffer(qpl_decompression_huffman_table *const decompression_table_ptr);

bool is_sw_representation_used(qpl_decompression_huffman_table *const decompression_table_ptr);
bool is_hw_representation_used(qpl_decompression_huffman_table *const decompression_table_ptr);
bool is_deflate_representation_used(qpl_decompression_huffman_table *const decompression_table_ptr);

uint16_t *get_number_of_codes_ptr(qpl_decompression_huffman_table *const decompression_table_ptr);
uint16_t *get_first_codes_ptr(qpl_decompression_huffman_table *const decompression_table_ptr);
uint16_t *get_first_table_indexes_ptr(qpl_decompression_huffman_table *const decompression_table_ptr);
uint8_t  *get_index_to_char_ptr(qpl_decompression_huffman_table *const decompression_table_ptr);
}

namespace qpl::ml::compression {

auto triplets_to_compression_table(const qpl_triplet *triplets_ptr,
                                   std::size_t triplets_count,
                                   compression_huffman_table &compression_table) noexcept -> qpl_ml_status;

auto triplets_to_decompression_table(const qpl_triplet *triplets_ptr,
                                     size_t triplets_count,
                                     decompression_huffman_table &compression_table) noexcept -> qpl_ml_status;

auto comp_to_decompression_table(compression_huffman_table &compression_table,
                                 decompression_huffman_table &decompression_table) noexcept -> qpl_ml_status;
}

#endif // QPL_MIDDLE_LAYER_COMPRESSION_CANNED_UTILS_HPP
