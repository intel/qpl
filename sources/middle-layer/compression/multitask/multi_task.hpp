/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Middle Layer API (private C++ API)
 */

#ifndef QPL_SOURCES_MIDDLE_LAYER_COMPRESSION_MULTITASK_MULTITASK_HPP_
#define QPL_SOURCES_MIDDLE_LAYER_COMPRESSION_MULTITASK_MULTITASK_HPP_

namespace qpl::ml::util {
enum multitask_status : uint32_t {
    ready                   = 0U,
    multi_chunk_in_progress = 0U,
    multi_chunk_last_chunk  = 1U,
    multi_chunk_first_chunk = 2U,
    single_chunk_processing = 3U,
};

extern std::array<uint16_t, 4U> aecs_decompress_access_lookup_table;
extern std::array<uint16_t, 4U> aecs_verify_access_lookup_table;
extern std::array<uint16_t, 4U> aecs_compress_access_lookup_table;
} // namespace qpl::ml::util

#endif //QPL_SOURCES_MIDDLE_LAYER_COMPRESSION_MULTITASK_MULTITASK_HPP_
