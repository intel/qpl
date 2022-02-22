/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "canned_utility.h"
#include "compression/compression_table.hpp"
#include "compression/decompression_table.hpp"
#include "compression/canned_utils.hpp"
#include "compression/compression_table_utils.hpp"
#include "compression/deflate/histogram.hpp"

/**
 * Compression table size in bytes
 */
const size_t QPL_COMPRESSION_TABLE_SIZE = sizeof(qpl_compression_huffman_table);

/**
 * Decompression table size in bytes
 */
const size_t QPL_DECOMPRESSION_TABLE_SIZE = sizeof(qpl_decompression_huffman_table);

static_assert(sizeof(qpl::ml::compression::sw_compression_huffman_table) <=
              sizeof(qpl_compression_huffman_table::sw_compression_table_data));

namespace qpl::ml {
uint32_t convert_to_qpl_status(uint32_t ml_status) {
    switch (ml_status) {
        case (status_list::ok): {
            return QPL_STS_OK;
        }

        case (status_list::status_invalid_params): {
            return QPL_STS_INVALID_PARAM_ERR;
        }

        default:
        case (status_list::internal_error): {
            return QPL_STS_LIBRARY_INTERNAL_ERR;
        }
    }
}
}

namespace internal {
uint32_t own_gather_deflate_statistics(uint8_t *source_ptr,
                                       const uint32_t source_length,
                                       uint32_t *literal_length_histogram_ptr,
                                       uint32_t *offsets_histogram_ptr,
                                       const qpl_compression_levels level,
                                       const qpl_path_t path) {
    using namespace qpl::ml::dispatcher;

    auto memcopy_kernel = kernels_dispatcher::get_instance().get_memory_copy_table()[0];

    qpl_histogram histogram;

    // Copy given histogram into internal one
    memcopy_kernel(reinterpret_cast<uint8_t *>(literal_length_histogram_ptr),
                   reinterpret_cast<uint8_t *>(histogram.literal_lengths),
                   sizeof(histogram.literal_lengths));

    memcopy_kernel(reinterpret_cast<uint8_t *>(offsets_histogram_ptr),
                   reinterpret_cast<uint8_t *>(histogram.distances),
                   sizeof(histogram.distances));

    auto status = qpl_gather_deflate_statistics(source_ptr, source_length, &histogram, level, path);

    // Copy back from internal histogram to given once
    memcopy_kernel(reinterpret_cast<uint8_t *>(histogram.literal_lengths),
                   reinterpret_cast<uint8_t *>(literal_length_histogram_ptr),
                   sizeof(histogram.literal_lengths));

    memcopy_kernel(reinterpret_cast<uint8_t *>(histogram.distances),
                   reinterpret_cast<uint8_t *>(offsets_histogram_ptr),
                   sizeof(histogram.distances));

    return status;
}
}

uint32_t own_triplets_to_decompression_table(const qpl_huffman_triplet *triplets_ptr,
                                             size_t triplets_count,
                                             qpl_decompression_huffman_table *decompression_table_ptr,
                                             uint32_t representation_flags) {
    using namespace qpl::ml::compression;

    auto sw_flattened_table_ptr     = reinterpret_cast<uint8_t *>(&decompression_table_ptr->sw_flattened_table);
    auto hw_decompression_state_ptr = reinterpret_cast<uint8_t *>(&decompression_table_ptr->hw_decompression_state);
    auto deflate_header_buffer_ptr  = reinterpret_cast<uint8_t *>(&decompression_table_ptr->deflate_header_buffer);
    auto lookup_table_buffer_ptr    = reinterpret_cast<uint8_t *>(&decompression_table_ptr->lookup_table_buffer);

    decompression_huffman_table decompression_table(sw_flattened_table_ptr,
                                                    hw_decompression_state_ptr,
                                                    deflate_header_buffer_ptr,
                                                    lookup_table_buffer_ptr);

    if (representation_flags & QPL_DEFLATE_REPRESENTATION) {
        return QPL_STS_INVALID_PARAM_ERR;
    }

    if (representation_flags & QPL_SW_REPRESENTATION) {
        decompression_table.enable_sw_decompression_table();
        decompression_table_ptr->representation_mask |= QPL_SW_REPRESENTATION;
    }

    if (representation_flags & QPL_HW_REPRESENTATION) {
        decompression_table.enable_hw_decompression_table();
        decompression_table_ptr->representation_mask |= QPL_HW_REPRESENTATION;
    }

    auto status = triplets_to_decompression_table(reinterpret_cast<const qpl_triplet *>(triplets_ptr),
                                                  triplets_count,
                                                  decompression_table);

    return status;
}

uint32_t own_triplets_to_compression_table(const qpl_huffman_triplet *triplets_ptr,
                                           size_t triplets_count,
                                           qpl_compression_huffman_table *compression_table_ptr,
                                           uint32_t representation_flags) {
    using namespace qpl::ml::compression;

    auto sw_compression_table_data_ptr   = reinterpret_cast<uint8_t *>(&compression_table_ptr->sw_compression_table_data);
    auto isal_compression_table_data_ptr = reinterpret_cast<uint8_t *>(&compression_table_ptr->sw_compression_table_data);
    auto hw_compression_table_data_ptr   = reinterpret_cast<uint8_t *>(&compression_table_ptr->hw_compression_table_data);
    auto deflate_header_buffer_ptr       = reinterpret_cast<uint8_t *>(&compression_table_ptr->deflate_header_buffer);

    compression_huffman_table compression_table(sw_compression_table_data_ptr,
                                                isal_compression_table_data_ptr,
                                                hw_compression_table_data_ptr,
                                                deflate_header_buffer_ptr);

    if (representation_flags & QPL_DEFLATE_REPRESENTATION) {
        return QPL_STS_INVALID_PARAM_ERR;
    }

    if (representation_flags & QPL_SW_REPRESENTATION) {
        compression_table.enable_sw_compression_table();
        compression_table_ptr->representation_mask |= QPL_SW_REPRESENTATION;
    }

    if (representation_flags & QPL_HW_REPRESENTATION) {
        compression_table.enable_hw_compression_table();
        compression_table_ptr->representation_mask |= QPL_HW_REPRESENTATION;
    }

    auto status = triplets_to_compression_table(reinterpret_cast<const qpl_triplet *>(triplets_ptr),
                                                triplets_count,
                                                compression_table);

    return status;
}

uint32_t own_comp_to_decompression_table(const qpl_compression_huffman_table *compression_table_ptr,
                                         qpl_decompression_huffman_table *decompression_table_ptr,
                                         uint32_t representation_flags) {
    using namespace qpl::ml::compression;

    // This is a workaround, because currently compression_huffman_table cannot accept pointer to const data
    // as argument in it's constructor, TODO: remove const cast
    auto casted_compression_table = const_cast<qpl_compression_huffman_table *>(compression_table_ptr);

    // Setup compression table
    auto sw_compression_table_data_ptr   = reinterpret_cast<uint8_t *>(&casted_compression_table->sw_compression_table_data);
    auto isal_compression_table_data_ptr = reinterpret_cast<uint8_t *>(&casted_compression_table->isal_compression_table_data);
    auto hw_compression_table_data_ptr   = reinterpret_cast<uint8_t *>(&casted_compression_table->hw_compression_table_data);
    auto comp_deflate_header_buffer_ptr  = reinterpret_cast<uint8_t *>(&casted_compression_table->deflate_header_buffer);

    compression_huffman_table compression_table(sw_compression_table_data_ptr,
                                                isal_compression_table_data_ptr,
                                                hw_compression_table_data_ptr,
                                                comp_deflate_header_buffer_ptr);

    if (compression_table_ptr->representation_mask & QPL_DEFLATE_REPRESENTATION) {
        compression_table.enable_deflate_header();
    }

    if (compression_table_ptr->representation_mask & QPL_SW_REPRESENTATION) {
        compression_table.enable_sw_compression_table();
    }

    if (compression_table_ptr->representation_mask & QPL_HW_REPRESENTATION) {
        compression_table.enable_hw_compression_table();
    }

    auto sw_flattened_table_ptr           = reinterpret_cast<uint8_t *>(&decompression_table_ptr->sw_flattened_table);
    auto hw_decompression_state_ptr       = reinterpret_cast<uint8_t *>(&decompression_table_ptr->hw_decompression_state);
    auto decomp_deflate_header_buffer_ptr = reinterpret_cast<uint8_t *>(&decompression_table_ptr->deflate_header_buffer);
    auto lookup_table_buffer_ptr          = reinterpret_cast<uint8_t *>(&decompression_table_ptr->lookup_table_buffer);

    // Setup decompression table
    decompression_huffman_table decompression_table(sw_flattened_table_ptr,
                                                    hw_decompression_state_ptr,
                                                    decomp_deflate_header_buffer_ptr,
                                                    lookup_table_buffer_ptr);

    if (representation_flags & QPL_DEFLATE_REPRESENTATION) {
        decompression_table.enable_deflate_header();
        decompression_table_ptr->representation_mask |= QPL_DEFLATE_REPRESENTATION;
    }

    if (representation_flags & QPL_SW_REPRESENTATION) {
        decompression_table.enable_sw_decompression_table();
        decompression_table_ptr->representation_mask |= QPL_SW_REPRESENTATION;
    }

    if (representation_flags & QPL_HW_REPRESENTATION) {
        decompression_table.enable_hw_decompression_table();
        decompression_table_ptr->representation_mask |= QPL_HW_REPRESENTATION;
    }

    auto status = comp_to_decompression_table(compression_table, decompression_table);

    return qpl::ml::convert_to_qpl_status(status);
}

uint8_t *get_sw_decompression_table_buffer(qpl_decompression_huffman_table *const decompression_table_ptr) {
    return reinterpret_cast<uint8_t *>(&decompression_table_ptr->sw_flattened_table);
}

uint8_t *get_hw_decompression_table_buffer(qpl_decompression_huffman_table *const decompression_table_ptr) {
    return reinterpret_cast<uint8_t *>(&decompression_table_ptr->hw_decompression_state);
}

uint8_t *get_deflate_header_buffer(qpl_decompression_huffman_table *const decompression_table_ptr) {
    return reinterpret_cast<uint8_t *>(&decompression_table_ptr->deflate_header_buffer);
}

bool is_sw_representation_used(qpl_decompression_huffman_table *const decompression_table_ptr) {
    if (decompression_table_ptr->representation_mask & QPL_SW_REPRESENTATION) {
        return 1u;
    } else {
        return 0u;
    }
}

bool is_hw_representation_used(qpl_decompression_huffman_table *const decompression_table_ptr) {
    if (decompression_table_ptr->representation_mask & QPL_HW_REPRESENTATION) {
        return 1u;
    } else {
        return 0u;
    }
}

bool is_deflate_representation_used(qpl_decompression_huffman_table *const decompression_table_ptr) {
    if (decompression_table_ptr->representation_mask & QPL_DEFLATE_REPRESENTATION) {
        return true;
    } else {
        return false;
    }
}

uint8_t *get_lookup_table_buffer_ptr(qpl_decompression_huffman_table *decompression_table_ptr) {
    return reinterpret_cast<uint8_t *>(&decompression_table_ptr->lookup_table_buffer);
}

void *get_aecs_decompress(qpl_decompression_huffman_table *decompression_table_ptr) {
    using namespace qpl::ml::compression;

    auto sw_flattened_table_ptr     = reinterpret_cast<uint8_t *>(&decompression_table_ptr->sw_flattened_table);
    auto hw_decompression_state_ptr = reinterpret_cast<uint8_t *>(&decompression_table_ptr->hw_decompression_state);
    auto deflate_header_buffer_ptr  = reinterpret_cast<uint8_t *>(&decompression_table_ptr->deflate_header_buffer);
    auto lookup_table_buffer_ptr    = reinterpret_cast<uint8_t *>(&decompression_table_ptr->lookup_table_buffer);

    decompression_huffman_table decompression_table(sw_flattened_table_ptr,
                                                    hw_decompression_state_ptr,
                                                    deflate_header_buffer_ptr,
                                                    lookup_table_buffer_ptr);

    return decompression_table.get_hw_decompression_state();
}

uint32_t own_collect_statistic(uint8_t *const source_ptr,
                               const uint32_t source_size,
                               qpl_histogram *const histogram_ptr,
                               const qpl_compression_levels level,
                               const qpl_path_t path) {
    return qpl::ml::compression::update_histogram<qpl::ml::execution_path_t::hardware>(source_ptr,
                                                                                       source_ptr + source_size,
                                                                                       *histogram_ptr);
}

uint32_t own_build_compression_table(const uint32_t *literal_lengths_histogram_ptr,
                                     const uint32_t *offsets_histogram_ptr,
                                     qpl_compression_huffman_table *compression_table_ptr,
                                     uint32_t representation_flags) {
    using namespace qpl::ml;
    using namespace qpl::ml::compression;

    auto sw_compression_table_data_ptr   = reinterpret_cast<uint8_t *>(&compression_table_ptr->sw_compression_table_data);
    auto isal_compression_table_data_ptr = reinterpret_cast<uint8_t *>(&compression_table_ptr->isal_compression_table_data);
    auto hw_compression_table_data_ptr   = reinterpret_cast<uint8_t *>(&compression_table_ptr->hw_compression_table_data);
    auto deflate_header_buffer_ptr       = reinterpret_cast<uint8_t *>(&compression_table_ptr->deflate_header_buffer);

    compression_huffman_table compression_table(sw_compression_table_data_ptr,
                                                isal_compression_table_data_ptr,
                                                hw_compression_table_data_ptr,
                                                deflate_header_buffer_ptr);

    if (representation_flags & QPL_DEFLATE_REPRESENTATION) {
        compression_table.enable_deflate_header();
        compression_table_ptr->representation_mask |= QPL_DEFLATE_REPRESENTATION;
    }

    if (representation_flags & QPL_SW_REPRESENTATION) {
        compression_table.enable_sw_compression_table();
        compression_table_ptr->representation_mask |= QPL_SW_REPRESENTATION;
    }

    if (representation_flags & QPL_HW_REPRESENTATION) {
        compression_table.enable_hw_compression_table();
        compression_table_ptr->representation_mask |= QPL_HW_REPRESENTATION;
    }

    if (representation_flags & QPL_HUFFMAN_ONLY_REPRESENTATION) {
        compression_table.make_huffman_only();
        compression_table_ptr->representation_mask |= QPL_HUFFMAN_ONLY_REPRESENTATION;
    }

    auto status = build_compression_table(literal_lengths_histogram_ptr, offsets_histogram_ptr, compression_table);

    return qpl::ml::convert_to_qpl_status(status);
}

void isal_huffman_table_to_qpl(isal_hufftables *isal_table_ptr,
                               qpl_compression_huffman_table *qpl_table_ptr) {
    using namespace qpl::ml::compression;
    auto sw_compression_table_data_ptr   = reinterpret_cast<uint8_t *>(&qpl_table_ptr->sw_compression_table_data);
    auto isal_compression_table_data_ptr = reinterpret_cast<uint8_t *>(&qpl_table_ptr->isal_compression_table_data);
    auto hw_compression_table_data_ptr   = reinterpret_cast<uint8_t *>(&qpl_table_ptr->hw_compression_table_data);
    auto deflate_header_buffer_ptr       = reinterpret_cast<uint8_t *>(&qpl_table_ptr->deflate_header_buffer);

    compression_huffman_table compression_table(sw_compression_table_data_ptr,
                                                isal_compression_table_data_ptr,
                                                hw_compression_table_data_ptr,
                                                deflate_header_buffer_ptr);

    isal_compression_table_to_qpl(isal_table_ptr, compression_table.get_sw_compression_table());

    store_isal_deflate_header(isal_table_ptr, compression_table);
}

uint32_t *get_literals_lengths_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr) {
    using namespace qpl::ml::compression;
    auto sw_compression_table = reinterpret_cast<sw_compression_huffman_table *>(&huffman_table_ptr->sw_compression_table_data);

    return sw_compression_table->literals_matches;
}

uint32_t *get_offsets_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr) {
    using namespace qpl::ml::compression;
    auto sw_compression_table = reinterpret_cast<sw_compression_huffman_table *>(&huffman_table_ptr->sw_compression_table_data);

    return sw_compression_table->offsets;
}

uint32_t get_size_of_ll_huffman_table() {
    return qpl::ml::compression::literals_matches_table_size;
}

uint32_t get_size_of_offsets_huffman_table() {
    return qpl::ml::compression::offsets_table_size;
}

uint8_t *get_deflate_header_ptr(qpl_compression_huffman_table *const huffman_table_ptr) {
    using namespace qpl::ml::compression;
    return reinterpret_cast<deflate_header *>(&huffman_table_ptr->deflate_header_buffer)->data;
}

uint32_t get_deflate_header_bits_size(qpl_compression_huffman_table *const huffman_table_ptr) {
    using namespace qpl::ml::compression;
    return reinterpret_cast<deflate_header *>(&huffman_table_ptr->deflate_header_buffer)->header_bit_size;
}

void set_deflate_header_bits_size(qpl_compression_huffman_table *const huffman_table_ptr, uint32_t header_bits) {
    using namespace qpl::ml::compression;
    reinterpret_cast<deflate_header *>(&huffman_table_ptr->deflate_header_buffer)->header_bit_size = header_bits;
}

uint8_t *get_sw_compression_huffman_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr) {
    return reinterpret_cast<uint8_t *>(&huffman_table_ptr->sw_compression_table_data);
}

uint8_t * get_isal_compression_huffman_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr) {
    return reinterpret_cast<uint8_t *>(&huffman_table_ptr->isal_compression_table_data);
}

uint8_t * get_hw_compression_huffman_table_ptr(qpl_compression_huffman_table *const huffman_table_ptr) {
    return reinterpret_cast<uint8_t *>(&huffman_table_ptr->hw_compression_table_data);
}

uint8_t *get_deflate_header_buffer_ptr(qpl_compression_huffman_table *const huffman_table_ptr) {
    return reinterpret_cast<uint8_t *>(&huffman_table_ptr->deflate_header_buffer);
}

uint16_t *get_number_of_codes_ptr(qpl_decompression_huffman_table *const decompression_table_ptr) {
    using namespace qpl::ml::compression;
    auto sw_decompr_table = reinterpret_cast<sw_decompression_huffman_table *>(&decompression_table_ptr->sw_flattened_table);

    return sw_decompr_table->number_of_codes;
}

uint16_t *get_first_codes_ptr(qpl_decompression_huffman_table *const decompression_table_ptr) {
    using namespace qpl::ml::compression;
    auto sw_decompr_table = reinterpret_cast<sw_decompression_huffman_table *>(&decompression_table_ptr->sw_flattened_table);

    return sw_decompr_table->first_codes;
}

uint16_t *get_first_table_indexes_ptr(qpl_decompression_huffman_table *const decompression_table_ptr) {
    using namespace qpl::ml::compression;
    auto sw_decompr_table = reinterpret_cast<sw_decompression_huffman_table *>(&decompression_table_ptr->sw_flattened_table);

    return sw_decompr_table->first_table_indexes;
}

uint8_t *get_index_to_char_ptr(qpl_decompression_huffman_table *const decompression_table_ptr) {
    using namespace qpl::ml::compression;
    auto sw_decompr_table = reinterpret_cast<sw_decompression_huffman_table *>(&decompression_table_ptr->sw_flattened_table);

    return sw_decompr_table->index_to_char;
}
