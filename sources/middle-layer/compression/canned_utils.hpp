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
    std::aligned_storage_t<sizeof(qpl::ml::compression::sw_compression_huffman_table),
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
