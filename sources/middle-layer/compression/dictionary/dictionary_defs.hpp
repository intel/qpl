/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Middle Layer API (private C++ API)
 */

#ifndef QPL_COMPRESSION_DICTIONARY_DICTIONARY_DEFS_HPP_
#define QPL_COMPRESSION_DICTIONARY_DICTIONARY_DEFS_HPP_

#include <cstddef>
#include <cstdint>

constexpr int dict_none = -1;

enum class software_compression_level {
    SW_NONE = dict_none,
    LEVEL_0 = 0,
    LEVEL_1 = 1,
    LEVEL_2 = 2,
    LEVEL_3 = 3,
    LEVEL_4 = 4,
    LEVEL_9 = 9
};

enum class hardware_dictionary_level {
    HW_NONE = dict_none,
    LEVEL_1 = 0, /* 2K dictionary w/ hash table built w/ 2 pointers per entry */
    LEVEL_2 = 1, /* 4K dictionary w/ hash table built w/ 2 pointers per entry */
    LEVEL_3 = 2  /* 4K dictionary w/ hash table built w/ 4 pointers per entry */
};

struct qpl_dictionary {
    software_compression_level sw_level;            /* Software compression level */
    hardware_dictionary_level  hw_dict_level;       /* Hardware compression level */
    size_t                     raw_dictionary_size; /* Raw dictionary size */
    uint32_t aecs_raw_dictionary_offset; /* Offset of raw dictionary in @ref hw_iaa_aecs_compress dictionary section */
    uint32_t dictionary_id;              /* Dictionary ID */
    uint32_t raw_dictionary_offset;      /* Raw dictionary offset in @ref qpl_dictionary buffer */
    uint32_t hw_hash_table_offset;       /* HW hash table offset in @ref qpl_dictionary buffer */
    uint32_t sw_hash_table_offset;       /* SW hash table offset in @ref qpl_dictionary buffer */
};

// namespace qpl::ml::compression {
// }

#endif // QPL_COMPRESSION_DICTIONARY_DICTIONARY_DEFS_HPP_
