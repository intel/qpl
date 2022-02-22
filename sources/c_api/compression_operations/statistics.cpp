/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/qpl.h"
#include "isal_statistics.h"
#include "deflate_hash_table.h"
#include "dispatcher/dispatcher.hpp"

static inline void remove_empty_places_in_histogram(qpl_histogram *histogram) noexcept {
    for (size_t i = 0; i < 286; i++) {
        if (histogram->literal_lengths[i] == 0) {
            histogram->literal_lengths[i] = 1;
        }
    }

    for (size_t i = 0; i < 30; i++) {
        if (histogram->distances[i] == 0) {
            histogram->distances[i] = 1;
        }
    }
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage=4096"
#endif

QPL_FUN("C" qpl_status, qpl_gather_deflate_statistics, (uint8_t * source_ptr,
        const uint32_t               source_length,
        qpl_histogram                *histogram_ptr,
        const qpl_compression_levels level,
        const qpl_path_t             path)) {
    using namespace qpl::ml::dispatcher;

    const auto &histogram_reset = ((qplc_deflate_histogram_reset_ptr)(kernels_dispatcher::get_instance().get_deflate_table()[1]));

    QPL_BAD_PTR_RET(source_ptr)
    QPL_BAD_PTR_RET(histogram_ptr)

    if (qpl_path_hardware == path) {
        auto status = own_collect_statistic(source_ptr, source_length, histogram_ptr, level, path);

        remove_empty_places_in_histogram(histogram_ptr);

        return static_cast<qpl_status>(status);
    }

    if (qpl_default_level == level) {
        isal_histogram histogram = {0u};

        isal_histogram_set_statistics(&histogram,
                                      histogram_ptr->literal_lengths,
                                      histogram_ptr->distances);

        // Update ISA-L histogram and create huffman table from it
        isal_update_histogram(source_ptr, source_length, &histogram);

        // Store result
        isal_histogram_get_statistics(&histogram,
                                      histogram_ptr->literal_lengths,
                                      histogram_ptr->distances);
    } else {
        own_deflate_job deflateJob = {0u};

        deflate_histogram_t deflate_histogram_ptr = {0u};

        uint8_t temporary_buffer[1u];

        uint32_t hash_table[OWN_HIGH_HASH_TABLE_SIZE];
        uint32_t hash_history_table[OWN_HIGH_HASH_TABLE_SIZE];
        deflateJob.histogram_ptr = &deflate_histogram_ptr;

        deflateJob.histogram_ptr->table.hash_table_ptr = hash_table;
        deflateJob.histogram_ptr->table.hash_story_ptr = hash_history_table;

        histogram_reset(&deflate_histogram_ptr);

        deflate_histogram_set_statistics(&deflate_histogram_ptr,
                                         histogram_ptr->literal_lengths,
                                         histogram_ptr->distances);

        own_initialize_deflate_job(&deflateJob,
                                   source_ptr,
                                   source_length,
                                   temporary_buffer,
                                   1u,
                                   initial_status,
                                   qpl_gathering_mode);

        own_update_deflate_histogram_high_level(&deflateJob);

        deflate_histogram_get_statistics(&deflate_histogram_ptr,
                                         histogram_ptr->literal_lengths,
                                         histogram_ptr->distances);
    }

    remove_empty_places_in_histogram(histogram_ptr);

    return QPL_STS_OK;
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

QPL_FUN("C" qpl_status, qpl_build_compression_table, (const qpl_histogram *histogram_ptr,
        qpl_compression_huffman_table *table_ptr,
        uint32_t representation_flags)) {
    QPL_BAD_PTR_RET(histogram_ptr)
    QPL_BAD_PTR_RET(table_ptr)

    auto status = own_build_compression_table(histogram_ptr->literal_lengths,
                                              histogram_ptr->distances,
                                              table_ptr,
                                              representation_flags);

    return static_cast<qpl_status>(status);
}

QPL_FUN("C" qpl_status, qpl_triplets_to_compression_table, (const qpl_huffman_triplet *triplets_ptr,
        size_t triplets_count,
        qpl_compression_huffman_table *table_ptr,
        uint32_t representation_flags)) {
    auto status = own_triplets_to_compression_table(triplets_ptr, triplets_count, table_ptr, representation_flags);

    return static_cast<qpl_status>(status);
}

QPL_FUN("C" qpl_status, qpl_triplets_to_decompression_table, (const qpl_huffman_triplet *triplets_ptr,
        size_t triplets_count,
        qpl_decompression_huffman_table *table_ptr,
        uint32_t representation_flags)) {
    auto status = own_triplets_to_decompression_table(triplets_ptr, triplets_count, table_ptr, representation_flags);

    return static_cast<qpl_status>(status);
}

QPL_FUN("C" qpl_status, qpl_comp_to_decompression_table, (const qpl_compression_huffman_table *compression_table_ptr,
        qpl_decompression_huffman_table *decompression_table_ptr,
        uint32_t representation_flags)) {
    QPL_BAD_PTR_RET(compression_table_ptr);
    QPL_BAD_PTR_RET(decompression_table_ptr);

    auto status = own_comp_to_decompression_table(compression_table_ptr, decompression_table_ptr, representation_flags);

    return static_cast<qpl_status>(status);
}
