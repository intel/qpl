/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 8/10/2020
 * @brief
 *
 */

#ifndef QPL_MIDDLE_LAYER_CONTAINERS_COMPRESSION_BUFFER_HPP
#define QPL_MIDDLE_LAYER_CONTAINERS_COMPRESSION_BUFFER_HPP

#include <memory>

#include "igzip_lib.h"
#include "../c_api/own_defs.h"

namespace qpl::ml::compression {

constexpr uint32_t bit_buffer_size        = QPL_ALIGNED_SIZE(sizeof(BitBuf2), QPL_DEFAULT_ALIGNMENT);
constexpr uint32_t static_huff_table_size = QPL_ALIGNED_SIZE(sizeof(struct isal_hufftables), QPL_DEFAULT_ALIGNMENT);

enum compression_mode : uint32_t;

class compression_buffer {
public:
    compression_buffer() = delete;

    compression_buffer(uint8_t *buffer_ptr, uint32_t buffer_size) noexcept {
        level_buffer_ptr_  = buffer_ptr;
        level_buffer_size_ = buffer_size - (bit_buffer_size + static_huff_table_size);
        bit_buffer_ptr_    = level_buffer_ptr_ + level_buffer_size_;
        static_huffman_table_buffer_ptr_ = bit_buffer_ptr_ + bit_buffer_size;
    }

    compression_buffer(compression_buffer &&other) noexcept = default;

    compression_buffer(const compression_buffer &other) = delete;

    inline auto level_buffer() noexcept -> uint8_t * {
        return level_buffer_ptr_;
    }

    inline auto level_buffer_size() noexcept -> uint32_t {
        return level_buffer_size_;
    }

    inline auto bit_buffer() noexcept -> BitBuf2 * {
        return reinterpret_cast<BitBuf2 *>(bit_buffer_ptr_);
    }

    inline auto static_huffman_table_buffer() noexcept ->isal_hufftables * {
        return reinterpret_cast<isal_hufftables *>(static_huffman_table_buffer_ptr_);
    }

private:
    uint8_t  *level_buffer_ptr_;
    uint8_t  *bit_buffer_ptr_;
    uint8_t  *static_huffman_table_buffer_ptr_;
    uint32_t level_buffer_size_;
};

} // namespace qpl::ml::compression

#endif // QPL_MIDDLE_LAYER_CONTAINERS_COMPRESSION_BUFFER_HPP
