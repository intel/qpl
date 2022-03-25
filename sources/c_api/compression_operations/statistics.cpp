/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "deflate_hash_table.h"
#include "dispatcher/dispatcher.hpp"
#include "own_deflate.h"

extern "C" {

static inline void isal_histogram_set_statistics(isal_histogram *isal_histogram_ptr,
                                                 const uint32_t *literal_length_histogram_ptr,
                                                 const uint32_t *offsets_histogram_ptr) {
    for (uint32_t i = 0u; i < OWN_LITERALS_MATCHES_TABLE_SIZE; i++) {
        isal_histogram_ptr->lit_len_histogram[i] = literal_length_histogram_ptr[i];
    }

    for (uint32_t i = 0; i < OWN_OFFSETS_TABLE_SIZE; i++) {
        isal_histogram_ptr->dist_histogram[i] = offsets_histogram_ptr[i];
    }
}

static inline void isal_histogram_get_statistics(const isal_histogram *isal_histogram_ptr,
                                                 uint32_t *literal_length_histogram_ptr,
                                                 uint32_t *offsets_histogram_ptr) {
    for (uint32_t i = 0u; i < OWN_LITERALS_MATCHES_TABLE_SIZE; i++) {
        literal_length_histogram_ptr[i] = (uint32_t) isal_histogram_ptr->lit_len_histogram[i];
    }

    for (uint32_t i = 0u; i < OWN_OFFSETS_TABLE_SIZE; i++) {
        offsets_histogram_ptr[i] = (uint32_t) isal_histogram_ptr->dist_histogram[i];
    }
}

static inline void isal_histogram_make_complete(isal_histogram *isal_histogram_ptr) {
    for ( uint32_t i = 0u; i < OWN_LITERALS_MATCHES_TABLE_SIZE; i++) {
        if (0u == isal_histogram_ptr->lit_len_histogram[i]) {
            isal_histogram_ptr->lit_len_histogram[i] = 1u;
        }
    }

    for ( uint32_t i = 0u; i < OWN_OFFSETS_TABLE_SIZE; i++) {
        if (0u == isal_histogram_ptr->dist_histogram[i]) {
            isal_histogram_ptr->dist_histogram[i] = 1u;
        }
    }
}

static inline void remove_empty_places_in_histogram(qpl_histogram *histogram) {
    for (unsigned int &literal_length: histogram->literal_lengths) {
        if (literal_length == 0) {
            literal_length = 1;
        }
    }

    for (unsigned int &distance: histogram->distances) {
        if (distance == 0) {
            distance = 1;
        }
    }
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage=4096"
#endif

QPL_FUN(qpl_status, qpl_gather_deflate_statistics, (uint8_t * source_ptr,
        const uint32_t               source_length,
        qpl_histogram                *histogram_ptr,
        const qpl_compression_levels level,
        const qpl_path_t             path)) {
    using
    namespace qpl::ml::dispatcher;

    const auto
    &histogram_reset =
            ((qplc_deflate_histogram_reset_ptr) (kernels_dispatcher::get_instance().get_deflate_table()[1]));

    QPL_BAD_PTR_RET(source_ptr)
    QPL_BAD_PTR_RET(histogram_ptr)

    if (qpl_path_hardware == path) {
        auto status = own_collect_statistic(source_ptr, source_length, histogram_ptr, level, path);

        remove_empty_places_in_histogram(histogram_ptr);

        return static_cast<qpl_status>(status);
    }

    if (qpl_default_level == level) {
        isal_histogram histogram = {{0u}, {0u}, {0u}};

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
        own_deflate_job deflateJob = {nullptr, nullptr, nullptr, nullptr, nullptr, 
                                      {0u, 0u, 0u, 0u, 0u, (writer_status_e)0u}, 
                                      0u, 0u, initial_status, initial_status, 
                                      (qpl_statistics_mode)0};

        deflate_histogram_t deflate_histogram_ptr = {{0u}, {0u}, {nullptr, nullptr, 0u, 0u, 0u, 0u, 0u}};

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

}
