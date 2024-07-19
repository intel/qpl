/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Middle Layer API (private C++ API)
 */

#include "dictionary_utils.hpp"
#include "simple_memory_ops.hpp"
#include <algorithm>

namespace qpl::ml::compression {

constexpr uint32_t software_hash_table_size[] = {
        16 * qpl_1k,
        16 * qpl_1k,
        64 * qpl_1k,
        64 * qpl_1k,
        256 * qpl_1k,
        32 * qpl_1k,
        32 * qpl_1k,
        32 * qpl_1k,
        32 * qpl_1k,
        32 * qpl_1k
};

constexpr uint32_t hardware_hash_table_size[] = {
        4 * qpl_1k, /* Dictionary hash table size for HW dictionary style 1 */
        4 * qpl_1k, /* Dictionary hash table size for HW dictionary style 2 */
        8 * qpl_1k  /* Dictionary hash table size for HW dictionary style 3 */
};

constexpr uint32_t hardware_dictionary_text_size[] = {
        2 * qpl_1k, /* Dictionary text size for HW dictionary style 1 */
        4 * qpl_1k, /* Dictionary text size for HW dictionary style 2 */
        4 * qpl_1k  /* Dictionary text size for HW dictionary style 3 */
};

static const uint32_t hash_vals[24] = {
    0x001, 0x002, 0x004, 0x008, 0x010, 0x020, 0x040, 0x080,
    0x100, 0x200, 0x009, 0x012, 0x024, 0x048, 0x090, 0x120,
    0x240, 0x089, 0x112, 0x224, 0x041, 0x082, 0x104, 0x208
};

static inline uint32_t calc_hash(uint32_t data) {
    uint32_t hash = 0U;

    for (uint32_t i = 0; i < 24; i++) {
        if (data & (1U << i)) {
            hash ^= hash_vals[i];
        }
    }
    return hash;
}

static inline uint8_t calc_meta(uint16_t data) {
    data ^= (data >> 4);
    data ^= (data >> 2);
    data = (data & 3) | ((data & 0x300U) >> 6);
    return (uint8_t) data;
}

/** @brief Initializes hash table for HW dictionary */
static inline void init_hw_dict_hash_table(qpl_dictionary &dictionary) {
    uint32_t raw_dict_size = 0U, ptrs_per_entry = 0U, raw_dict_offset = 0U;
    uint32_t data32 = 0U;
    uint16_t data16 = 0U;
    uint8_t  meta = 0U, *ddata = nullptr, *hw_hash_table_ptr = nullptr;
    uint32_t hash_val = 0U;
    uint64_t hash_tbl[1024];
    uint64_t entry = 0U;
    uint32_t *entries32 = nullptr;
    uint64_t *entries64 = nullptr;

    qpl::core_sw::util::set_zeros((uint8_t *) hash_tbl, sizeof(hash_tbl));

    raw_dict_size = static_cast<uint32_t>(dictionary.raw_dictionary_size);
    ptrs_per_entry = (dictionary.hw_dict_level < hardware_dictionary_level::LEVEL_3) ? 2U : 4U;
    ddata = get_dictionary_data(dictionary);
    hw_hash_table_ptr = get_dictionary_hw_hash_table(dictionary);
    raw_dict_offset = dictionary.aecs_raw_dictionary_offset;

    // 3 bytes in raw dictionary data generate one hash update.
    // So if raw dictionary is less than 3 bytes, we don't generate hash entries
    if (raw_dict_size < 3U) {
        return;
    }

    for (uint32_t i = 0; i < raw_dict_size - 2; i++) {
        data32 = *(uint32_t*)(ddata + i);
        if (i < raw_dict_size - 4) {
            data16 = *(uint16_t*)(ddata + i + 3);
            meta = calc_meta(data16);
        } else
            meta = 0U;
        hash_val = calc_hash(data32);
        entry = hash_tbl[hash_val];
        entry = (entry << 16) | ((i + raw_dict_offset) << 4) | meta;
        hash_tbl[hash_val] = entry;
    }

    if (ptrs_per_entry == 2U) {
        entries32 = (uint32_t*) (hw_hash_table_ptr);
        std::transform(std::begin(hash_tbl), std::end(hash_tbl), entries32, [&](uint32_t entry) {
            return ((entry >>  0) & 0x0000000F) |
                   ((entry >> 12) & 0x000000F0) |
                   ((entry <<  4) & 0x000FFF00) |
                   ((entry <<  0) & 0xFFF00000);
        });
    } else {
        // ptrs_per_entry == 4
        entries64 = (uint64_t*) (hw_hash_table_ptr);
        for (uint32_t i = 0; i < 1024; i++) {
            entry = hash_tbl[i];
            entries64[i] =
                ((entry >>  0) & 0x000000000000000F) |
                ((entry >> 12) & 0x00000000000000F0) |
                ((entry >> 24) & 0x0000000000000F00) |
                ((entry >> 36) & 0x000000000000F000) |
                ((entry << 12) & 0x000000000FFF0000) |
                ((entry <<  8) & 0x000000FFF0000000) |
                ((entry <<  4) & 0x000FFF0000000000) |
                ((entry <<  0) & 0xFFF0000000000000);
        }
    }
}

/** @brief Convert the public enum @ref hw_compression_level to internal enum @ref hardware_dictionary_level */
auto convert_public_hw_dict_level_to_internal(hw_compression_level hw_dict_level) noexcept -> hardware_dictionary_level {
    hardware_dictionary_level hw_dict_level_internal = hardware_dictionary_level::LEVEL_1;

    switch(hw_dict_level) {
        case hw_compression_level::SMALL: {
            hw_dict_level_internal = hardware_dictionary_level::LEVEL_1;
            break;
        }
        case hw_compression_level::LARGE: {
            hw_dict_level_internal = hardware_dictionary_level::LEVEL_3;
            break;
        }
        case hw_compression_level::HW_LEVEL_1: {
            hw_dict_level_internal = hardware_dictionary_level::LEVEL_1;
            break;
        }
        case hw_compression_level::HW_LEVEL_2: {
            hw_dict_level_internal = hardware_dictionary_level::LEVEL_2;
            break;
        }
        case hw_compression_level::HW_LEVEL_3: {
            hw_dict_level_internal = hardware_dictionary_level::LEVEL_3;
            break;
        }
        default: {
            hw_dict_level_internal = hardware_dictionary_level::HW_NONE;
        }
    }

    return hw_dict_level_internal;
}

/** @brief Gets the history size (max size for dictionary text) of dictionary */
auto get_history_size_for_dictionary(hardware_dictionary_level hw_dict_level) noexcept -> size_t {
    size_t hist_size = max_history_size;

    if (hardware_dictionary_level::HW_NONE != hw_dict_level) {
        // For HW dictionary, check dictionary level for history size
        hist_size = hardware_dictionary_text_size[static_cast<uint32_t>(hw_dict_level)];
    }

    return hist_size;
}

/**
 * @brief Gets the size of HW dictionary in aecs, which equals to the sum of dictionary text size
 *        and hash table size
*/
auto get_dictionary_size_in_aecs(qpl_dictionary &dictionary) noexcept -> uint32_t {
    if (hardware_dictionary_level::HW_NONE == dictionary.hw_dict_level) {
        return 0U;
    }
    const uint32_t hash_table_size   = hardware_hash_table_size[static_cast<uint32_t>(dictionary.hw_dict_level)];
    const uint32_t dict_text_size    = hardware_dictionary_text_size[static_cast<uint32_t>(dictionary.hw_dict_level)];
    const uint32_t dict_size_in_aecs = hash_table_size + dict_text_size;

    return dict_size_in_aecs;
}

/**
 * @brief Gets the corresponding load dictionary value for the dictionary.
 *        The load dictionary is a 2-bit compression flag in Intel® In-Memory Analytics Accelerator (Intel® IAA).
 *        Its value can range from 0 to 3: 0 means do not load dictionary,
 *        while 1-3 correspond to dictionary with different levels.
*/
auto get_load_dictionary_flag(qpl_dictionary &dictionary) noexcept -> uint8_t {
    uint8_t load_dictionary_val = 0U;
    const hardware_dictionary_level hw_dict_level = dictionary.hw_dict_level;

    switch (hw_dict_level) {
        case hardware_dictionary_level::LEVEL_1: {
            load_dictionary_val = 1U;
            break;
        }
        case hardware_dictionary_level::LEVEL_2: {
            load_dictionary_val = 2U;
            break;
        }
        case hardware_dictionary_level::LEVEL_3: {
            load_dictionary_val = 3U;
            break;
        }
        default: {
            load_dictionary_val = 0U;
        }
    }

    return load_dictionary_val;
}

/**
 * @brief Gets the size of the dictionary buffer, including the qpl_dictionary structure, raw dictionary,
 *        SW and/or HW hash tables
*/
auto get_dictionary_size(software_compression_level sw_level,
                         hardware_dictionary_level hw_dict_level,
                         size_t raw_dictionary_size) noexcept -> size_t {

    const size_t hist_size = get_history_size_for_dictionary(hw_dict_level);
    raw_dictionary_size = std::min(raw_dictionary_size, static_cast<size_t>(hist_size));
    size_t result_size = raw_dictionary_size + sizeof(qpl_dictionary);

    if (software_compression_level::SW_NONE != sw_level) {
        result_size += software_hash_table_size[static_cast<uint32_t>(sw_level)];
    }

    if (hardware_dictionary_level::HW_NONE != hw_dict_level) {
        result_size += hardware_hash_table_size[static_cast<uint32_t>(hw_dict_level)];
    }

    return result_size;
}

/**
 * @brief Fills in the dictionary structure and buffer,
 *        copies the raw dictionary into the dictionary buffer,
 *        and calculates the hash table for HW dictionary
 *        (hash table for SW dictionary is calculated during job execution)
 *        The dictionary object consists of the following parts in order:
 *        dictionary structure fields, raw dictionary,
 *        hw hash table (if applicable), sw hash table (if applicable)
*/
auto build_dictionary(qpl_dictionary &dictionary,
                      software_compression_level sw_level,
                      hardware_dictionary_level hw_dict_level,
                      const uint8_t *raw_dict_ptr,
                      size_t raw_dict_size) noexcept -> qpl_ml_status {
    dictionary.raw_dictionary_offset      = 0U;
    dictionary.aecs_raw_dictionary_offset = 0U;
    dictionary.hw_hash_table_offset       = 0U;
    dictionary.sw_hash_table_offset       = 0U;
    dictionary.sw_level                   = sw_level;
    dictionary.hw_dict_level              = hw_dict_level;

    uint32_t current_offset = static_cast<uint32_t>(sizeof(qpl_dictionary));
    dictionary.raw_dictionary_offset = current_offset;

    const size_t hist_size = get_history_size_for_dictionary(hw_dict_level);
    if (raw_dict_size < hist_size) {
        if (hardware_dictionary_level::HW_NONE != hw_dict_level) {
            // If raw dictionary size is smaller than history size,
            // set aecs_raw_dictionary_offset equal to the difference,
            // so that raw dictionary will copied at this offset in AECS
            dictionary.aecs_raw_dictionary_offset = static_cast<uint32_t>(hist_size - raw_dict_size);
        }
    } else {
        // In case when passed dictionary is larger than 4k
        // Build dictionary from last 4k bytes
        raw_dict_ptr += (raw_dict_size - hist_size);
        raw_dict_size = hist_size;
    }
    dictionary.raw_dictionary_size = raw_dict_size;

    core_sw::util::copy(raw_dict_ptr,
                        raw_dict_ptr + raw_dict_size,
                        get_dictionary_data(dictionary));

    current_offset += static_cast<uint32_t>(raw_dict_size);

    if (hardware_dictionary_level::HW_NONE != hw_dict_level) {
        dictionary.hw_hash_table_offset = current_offset;
        current_offset += hardware_hash_table_size[static_cast<uint32_t>(hw_dict_level)];

        // Build hash table for HW dictionary compression
        init_hw_dict_hash_table(dictionary);
    }

    if (software_compression_level::SW_NONE != sw_level) {
        dictionary.sw_hash_table_offset = current_offset;
        current_offset += software_hash_table_size[static_cast<uint32_t>(sw_level)];

        // TODO: investigate if moving SW hash table generation from within job execution
        // to here brings performance benefits
    }

    return status_list::ok;
}

auto get_dictionary_data(qpl_dictionary &dictionary) noexcept -> uint8_t * {
    return (reinterpret_cast<uint8_t *>(&dictionary) + dictionary.raw_dictionary_offset);
}

auto get_dictionary_hw_hash_table(qpl_dictionary &dictionary) noexcept -> uint8_t * {
    return (reinterpret_cast<uint8_t *>(&dictionary) + dictionary.hw_hash_table_offset);
}

} // namespace qpl::ml::compression
