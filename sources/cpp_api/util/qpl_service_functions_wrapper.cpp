/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/qpl.h"

#include "qpl/cpp_api/util/qpl_service_functions_wrapper.hpp"
#include "../../c_api/compression_operations/canned_utility.h"

namespace qpl::util {

template <execution_path path>
constexpr auto get_representation_flags() -> uint32_t {
    switch (path) {
        case software:
            return QPL_SW_REPRESENTATION | QPL_DEFLATE_REPRESENTATION;
        case hardware:
            return QPL_HW_REPRESENTATION | QPL_DEFLATE_REPRESENTATION;
        default:
            return QPL_COMPLETE_COMPRESSION_TABLE;
    }
}

auto get_job_size() -> uint32_t {
    uint32_t job_size;

    qpl_get_job_size(qpl_path_software, &job_size);

    return job_size;
}

void gather_deflate_statistics(const uint8_t *source_ptr,
                               const uint32_t source_length,
                               uint32_t *literal_length_histogram_ptr,
                               uint32_t *offsets_histogram_ptr,
                               const uint32_t level,
                               const execution_path path) {
    internal::own_gather_deflate_statistics(const_cast<uint8_t *>(source_ptr),
                                            source_length,
                                            literal_length_histogram_ptr,
                                            offsets_histogram_ptr,
                                            static_cast<qpl_compression_levels>(level),
                                            static_cast<qpl_path_t>(path));
}

template <execution_path path>
void build_huffman_table_from_statistics(qpl_compression_huffman_table *huffman_table_buffer_ptr,
                                         const uint32_t *literal_length_histogram_ptr,
                                         const uint32_t *offsets_histogram_ptr) {
    own_build_compression_table(literal_length_histogram_ptr,
                                offsets_histogram_ptr,
                                huffman_table_buffer_ptr,
                                get_representation_flags<path>());
}

template
void build_huffman_table_from_statistics<execution_path::software>(qpl_compression_huffman_table *huffman_table_buffer_ptr,
                                                                   const uint32_t *literal_length_histogram_ptr,
                                                                   const uint32_t *offsets_histogram_ptr);

template
void build_huffman_table_from_statistics<execution_path::hardware>(qpl_compression_huffman_table *huffman_table_buffer_ptr,
                                                                   const uint32_t *literal_length_histogram_ptr,
                                                                   const uint32_t *offsets_histogram_ptr);

template
void build_huffman_table_from_statistics<execution_path::auto_detect>(qpl_compression_huffman_table *huffman_table_buffer_ptr,
                                                                      const uint32_t *literal_length_histogram_ptr,
                                                                      const uint32_t *offsets_histogram_ptr);

template <execution_path path>
void build_decompression_huffman_table(qpl_compression_huffman_table *compression_table_buffer_ptr,
                                       qpl_decompression_huffman_table *decompression_table_buffer_ptr) {
    own_comp_to_decompression_table(compression_table_buffer_ptr,
                                    decompression_table_buffer_ptr,
                                    get_representation_flags<path>());
}

template
void build_decompression_huffman_table<execution_path::software>(qpl_compression_huffman_table *compression_table_buffer_ptr,
                                                                 qpl_decompression_huffman_table *decompression_table_buffer_ptr);

template
void build_decompression_huffman_table<execution_path::hardware>(qpl_compression_huffman_table *compression_table_buffer_ptr,
                                                                 qpl_decompression_huffman_table *decompression_table_buffer_ptr);

template
void build_decompression_huffman_table<execution_path::auto_detect>(qpl_compression_huffman_table *compression_table_buffer_ptr,
                                                                    qpl_decompression_huffman_table *decompression_table_buffer_ptr);

} // namespace qpl::util
