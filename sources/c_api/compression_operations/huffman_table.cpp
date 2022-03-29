/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Job API (public C API)
 */

#include "array"

#include "util/memory.hpp"
#include "huffman_table.hpp"
#include "util/checkers.hpp"
#include "own_checkers.h"

extern "C" {

typedef struct {
    qpl_huffman_table_type_e        type;
    qpl_path_t                      path;
    allocator_t                     allocator;
    bool                            is_initialized;
    uint32_t                        representation_flags;
    qpl_compression_huffman_table   *compression_huffman_table;
    qpl_decompression_huffman_table *decompression_huffman_table;
}                                    this_huffman_table_t;

static inline auto own_get_path_flags(qpl_path_t path) {
    switch (path) {
        case qpl_path_hardware:
            return QPL_HW_REPRESENTATION;
        case qpl_path_software:
            return QPL_SW_REPRESENTATION;
        default:
            return QPL_COMPLETE_COMPRESSION_TABLE;
    }
}

static inline auto own_get_allocator(const allocator_t allocator) {
    constexpr allocator_t default_allocator = DEFAULT_ALLOCATOR_C;

    return (allocator.allocator && allocator.deallocator) ? allocator : default_allocator;
}

qpl_status qpl_deflate_huffman_table_create(const qpl_huffman_table_type_e type,
                                            const qpl_path_t path,
                                            const allocator_t allocator,
                                            qpl_huffman_table_t *huffman_table) {
    using namespace qpl::ml;
    OWN_QPL_CHECK_STATUS(bad_argument::check_for_nullptr(huffman_table))
    QPL_BADARG_RET(path > qpl_path_software, QPL_STS_PATH_ERR)
    QPL_BADARG_RET(type > decompression_table_type, QPL_STS_HUFFMAN_TABLE_TYPE_ERROR)

    *huffman_table = nullptr;

    allocator_t table_allocator = own_get_allocator(allocator);

    auto allocated_size               = sizeof(this_huffman_table_t);
    size_t compression_table_offset   = 0u;
    size_t decompression_table_offset = 0u;

    switch (type) {
        case compression_table_type:
            allocated_size += QPL_COMPRESSION_TABLE_SIZE;
            compression_table_offset   = sizeof(this_huffman_table_t);
            break;
        case decompression_table_type:
            allocated_size += QPL_DECOMPRESSION_TABLE_SIZE;
            decompression_table_offset = sizeof(this_huffman_table_t);
            break;
        default:
            allocated_size += QPL_COMPRESSION_TABLE_SIZE + QPL_DECOMPRESSION_TABLE_SIZE;
            compression_table_offset   = sizeof(this_huffman_table_t);
            decompression_table_offset = compression_table_offset + QPL_COMPRESSION_TABLE_SIZE;
    }

    auto huffman_table_ptr = reinterpret_cast<this_huffman_table_t *>(table_allocator.allocator(allocated_size));

    qpl::ml::util::set_zeros(huffman_table_ptr, allocated_size);

    huffman_table_ptr->type                 = type;
    huffman_table_ptr->path                 = path;
    huffman_table_ptr->allocator            = table_allocator;
    huffman_table_ptr->is_initialized       = false;
    huffman_table_ptr->representation_flags = own_get_path_flags(path) | QPL_DEFLATE_REPRESENTATION;
    huffman_table_ptr->compression_huffman_table =
            reinterpret_cast<qpl_compression_huffman_table *>((compression_table_offset) ?
                                                              reinterpret_cast<uint8_t *>(huffman_table_ptr) +
                                                              compression_table_offset :
                                                              nullptr);
    huffman_table_ptr->decompression_huffman_table =
            reinterpret_cast<qpl_decompression_huffman_table *>((decompression_table_offset) ?
                                                                reinterpret_cast<uint8_t *>(huffman_table_ptr) +
                                                                decompression_table_offset :
                                                                nullptr);

    *huffman_table = reinterpret_cast<qpl_huffman_table_t>(huffman_table_ptr);

    return QPL_STS_OK;
}

qpl_status qpl_huffman_only_table_create(const qpl_huffman_table_type_e type,
                                         const qpl_path_t path,
                                         const allocator_t allocator,
                                         qpl_huffman_table_t *huffman_table) {
    using namespace qpl::ml;
    OWN_QPL_CHECK_STATUS(bad_argument::check_for_nullptr(huffman_table))
    QPL_BADARG_RET(path > qpl_path_software, QPL_STS_PATH_ERR)
    QPL_BADARG_RET(type > decompression_table_type, QPL_STS_HUFFMAN_TABLE_TYPE_ERROR)

    *huffman_table = nullptr;

    allocator_t table_allocator = own_get_allocator(allocator);

    auto allocated_size             = sizeof(this_huffman_table_t);
    size_t compression_table_offset   = 0u;
    size_t decompression_table_offset = 0u;

    switch (type) {
        case compression_table_type:
            allocated_size += QPL_COMPRESSION_TABLE_SIZE;
            compression_table_offset   = sizeof(this_huffman_table_t);
            break;
        case decompression_table_type:
            allocated_size += QPL_DECOMPRESSION_TABLE_SIZE;
            decompression_table_offset = sizeof(this_huffman_table_t);
            break;
        default:
            allocated_size += QPL_COMPRESSION_TABLE_SIZE + QPL_DECOMPRESSION_TABLE_SIZE;
            compression_table_offset   = sizeof(this_huffman_table_t);
            decompression_table_offset = compression_table_offset + QPL_COMPRESSION_TABLE_SIZE;
    }

    auto huffman_table_ptr = reinterpret_cast<this_huffman_table_t *>(table_allocator.allocator(allocated_size));

    qpl::ml::util::set_zeros(huffman_table_ptr, allocated_size);

    huffman_table_ptr->type                 = type;
    huffman_table_ptr->path                 = path;
    huffman_table_ptr->allocator            = table_allocator;
    huffman_table_ptr->is_initialized       = false;
    huffman_table_ptr->representation_flags = own_get_path_flags(path) | QPL_HUFFMAN_ONLY_REPRESENTATION;
    huffman_table_ptr->compression_huffman_table   =
            reinterpret_cast<qpl_compression_huffman_table *>((compression_table_offset) ?
                                                              reinterpret_cast<uint8_t *>(huffman_table_ptr) +
                                                              compression_table_offset :
                                                              nullptr);
    huffman_table_ptr->decompression_huffman_table =
            reinterpret_cast<qpl_decompression_huffman_table *>((decompression_table_offset) ?
                                                                reinterpret_cast<uint8_t *>(huffman_table_ptr) +
                                                                decompression_table_offset :
                                                                nullptr);

    *huffman_table = reinterpret_cast<qpl_huffman_table_t>(huffman_table_ptr);

    return QPL_STS_OK;
}

void qpl_huffman_table_destroy(qpl_huffman_table_t table) {
    using namespace qpl::ml;
    if (bad_argument::check_for_nullptr(table)) {
        return;
    }

    auto this_ptr = reinterpret_cast<this_huffman_table_t *>(table);
    this_ptr->allocator.deallocator(this_ptr);
}

qpl_status qpl_huffman_table_init(qpl_huffman_table_t table,
                                  const qpl_histogram *const histogram_ptr) {
    using namespace qpl::ml;
    OWN_QPL_CHECK_STATUS(bad_argument::check_for_nullptr(table, histogram_ptr));

    QPL_BADARG_RET(histogram_ptr->reserved_literal_lengths[0] ||
                   histogram_ptr->reserved_literal_lengths[1] ||
                   histogram_ptr->reserved_distances[0] ||
                   histogram_ptr->reserved_distances[1],
                   QPL_STS_INVALID_PARAM_ERR);

    auto this_ptr = reinterpret_cast<this_huffman_table_t *>(table);

    if (this_ptr->compression_huffman_table) {
        auto status = own_build_compression_table(histogram_ptr->literal_lengths,
                                                  histogram_ptr->distances,
                                                  this_ptr->compression_huffman_table,
                                                  this_ptr->representation_flags);
        if (status) {
            return static_cast<qpl_status>(status);
        }
    }

    if (this_ptr->decompression_huffman_table) {
        if (this_ptr->compression_huffman_table &&
            this_ptr->representation_flags & QPL_DEFLATE_REPRESENTATION) {
            auto status = own_comp_to_decompression_table(this_ptr->compression_huffman_table,
                                                          this_ptr->decompression_huffman_table,
                                                          this_ptr->representation_flags);
            if (status) {
                return static_cast<qpl_status>(status);
            }
        } else {
            return QPL_STS_NOT_SUPPORTED_MODE_ERR;
        }
    }

    this_ptr->is_initialized = true;

    return QPL_STS_OK;
}

qpl_status qpl_huffman_table_init_with_triplet(qpl_huffman_table_t table,
                                               const qpl_huffman_triplet *const triplet_ptr,
                                               const uint32_t triplet_count) {
    using namespace qpl::ml;
    OWN_QPL_CHECK_STATUS(bad_argument::check_for_nullptr(table, triplet_ptr));

    auto this_ptr = reinterpret_cast<this_huffman_table_t *>(table);

    QPL_BADARG_RET(this_ptr->representation_flags & QPL_DEFLATE_REPRESENTATION, QPL_STS_INVALID_HUFFMAN_TABLE_ERR);
    QPL_BADARG_RET(this_ptr->representation_flags & QPL_HUFFMAN_ONLY_REPRESENTATION &&
                   triplet_count != 256,
                   QPL_STS_SIZE_ERR);

    if (this_ptr->compression_huffman_table) {
        auto status = own_triplets_to_compression_table(triplet_ptr,
                                                        triplet_count,
                                                        this_ptr->compression_huffman_table,
                                                        this_ptr->representation_flags);
        if (status) {
            return static_cast<qpl_status>(status);
        }
    }

    if (this_ptr->decompression_huffman_table) {
        auto status = own_triplets_to_decompression_table(triplet_ptr,
                                                          triplet_count,
                                                          this_ptr->decompression_huffman_table,
                                                          this_ptr->representation_flags);
        if (status) {
            return static_cast<qpl_status>(status);
        }
    }

    this_ptr->is_initialized = true;

    return QPL_STS_OK;
}

qpl_status qpl_huffman_table_init_with_other(qpl_huffman_table_t table,
                                             const qpl_huffman_table_t other) {
    using namespace qpl::ml;
    constexpr auto TABLE_TYPE_FLAG_MASK = QPL_HUFFMAN_ONLY_REPRESENTATION | QPL_DEFLATE_REPRESENTATION;
    OWN_QPL_CHECK_STATUS(bad_argument::check_for_nullptr(table, other));

    auto this_ptr  = reinterpret_cast<this_huffman_table_t *>(table);
    auto other_ptr = reinterpret_cast<this_huffman_table_t *>(other);

    QPL_BADARG_RET((this_ptr->representation_flags & TABLE_TYPE_FLAG_MASK) !=
                   (other_ptr->representation_flags & TABLE_TYPE_FLAG_MASK),
                   QPL_STS_INVALID_HUFFMAN_TABLE_ERR)

    if (this_ptr->type == decompression_table_type) {
        if (this_ptr->representation_flags & QPL_DEFLATE_REPRESENTATION) {
            auto status = own_comp_to_decompression_table(other_ptr->compression_huffman_table,
                                                          this_ptr->decompression_huffman_table,
                                                          this_ptr->representation_flags);
            if (status) {
                return static_cast<qpl_status>(status);
            }
        }

        if (this_ptr->representation_flags & QPL_HUFFMAN_ONLY_REPRESENTATION) {
            constexpr auto     QPL_HUFFMAN_CODE_BIT_LENGTH = 15u;
            constexpr uint16_t code_mask                   = (1u << QPL_HUFFMAN_CODE_BIT_LENGTH) - 1u;

            std::array<qpl_huffman_triplet, 256> triplets_tmp{};

            auto literals_matches_table_ptr = reinterpret_cast<uint32_t *>(other_ptr->compression_huffman_table);

            // Prepare triplets
            for (uint16_t i = 0u; i < 256u; i++) {
                triplets_tmp[i].code        = literals_matches_table_ptr[i] & code_mask;
                triplets_tmp[i].code_length = literals_matches_table_ptr[i] >> QPL_HUFFMAN_CODE_BIT_LENGTH;
                triplets_tmp[i].value       = static_cast<uint8_t>(i);
            }

            auto status = own_triplets_to_decompression_table(triplets_tmp.data(),
                                                              triplets_tmp.size(),
                                                              this_ptr->decompression_huffman_table,
                                                              QPL_SW_REPRESENTATION | QPL_HW_REPRESENTATION);
            return static_cast<qpl_status>(status);
        }
    } else {
        return QPL_STS_NOT_SUPPORTED_MODE_ERR;
    }

    this_ptr->is_initialized = true;

    return QPL_STS_OK;
}

qpl_status qpl_huffman_table_get_type(qpl_huffman_table_t table,
                                      qpl_huffman_table_type_e *const type_ptr) {
    using namespace qpl::ml;

    OWN_QPL_CHECK_STATUS(bad_argument::check_for_nullptr(table, type_ptr))

    const auto *const this_ptr = reinterpret_cast<this_huffman_table_t *>(table);
    *type_ptr = this_ptr->type;

    return QPL_STS_OK;
}

qpl_compression_huffman_table *own_huffman_table_get_compression_table(const qpl_huffman_table_t table) {
    auto const this_ptr = reinterpret_cast<this_huffman_table_t *>(table);
    return this_ptr->compression_huffman_table;
}

qpl_decompression_huffman_table *own_huffman_table_get_decompression_table(const qpl_huffman_table_t table) {
    auto const this_ptr = reinterpret_cast<this_huffman_table_t *>(table);
    return this_ptr->decompression_huffman_table;
}

}
