/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace bench {
struct data_t {
    std::string               name;
    std::vector<std::uint8_t> buffer;
};

using dataset_t = std::vector<data_t>;
using blocks_t  = std::vector<data_t>;

struct dictionary_t {
    std::vector<std::uint8_t> dict;
};

// Note: completed_operations, data_read, data_written are total stats from all iterations
struct statistics_t {
    std::uint32_t queue_size {0};
    std::uint32_t operations {0};
    std::uint32_t operations_per_thread {0};
    std::uint64_t completed_operations {0};
    std::uint64_t data_read {0};
    std::uint64_t data_written {0};
};

enum class api_e { c, ml };

enum class path_e { cpu, iaa, auto_ };

enum class execution_e { sync, async };

enum class operation_e { deflate, inflate, crc64 };

enum class stat_type_e { compression, decompression, crc64, filter };

enum class huffman_type_e { fixed, static_, dynamic, canned };

enum class crc_type_e { crc32_gzip, crc32_wimax, crc32_iscsi, T10DIF, crc16_ccitt, crc64 };

enum class task_status_e { retired, in_progress, completed };

enum class mem_loc_e { cache, llc, ram, pmem, cc_ram, cc_pmem };

enum class mem_loc_mask_e : std::uint32_t {
    src1 = 0x01,
    src2 = 0x02,
    src  = 0x03,
    dst1 = 0x04,
    dst2 = 0x08,
    dst  = 0x0C,
    all  = 0x0F
};

static inline std::uint32_t operator&(mem_loc_mask_e lha, mem_loc_mask_e rha) {
    return static_cast<std::uint32_t>(lha) & static_cast<std::uint32_t>(rha);
}

static inline std::uint32_t operator&(mem_loc_mask_e lha, std::uint32_t rha) {
    return static_cast<std::uint32_t>(lha) & rha;
}

static inline std::uint32_t operator|(mem_loc_mask_e lha, mem_loc_mask_e rha) {
    return static_cast<std::uint32_t>(lha) | static_cast<std::uint32_t>(rha);
}

static inline std::uint32_t operator|(mem_loc_mask_e lha, std::uint32_t rha) {
    return static_cast<std::uint32_t>(lha) | rha;
}
} // namespace bench
