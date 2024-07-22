/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Middle Layer API (private C++ API)
 */

#ifndef QPL_ZLIB_DECORATOR_HPP_
#define QPL_ZLIB_DECORATOR_HPP_

#include <array>
#include <cstdint>

#include "common/defs.hpp"
#include "compression/compression_defs.hpp"
#include "compression/deflate/streams/compression_stream.hpp"
#include "compression/inflate/isal_kernels_wrappers.hpp"
#include "compression/verification/verification_defs.hpp"
#include "util/checksum.hpp"

namespace qpl::ml::compression {

namespace zlib_sizes {
constexpr size_t zlib_header_size  = 2;
constexpr size_t zlib_trailer_size = 4;
} // namespace zlib_sizes

/**
 * @brief Default header for zlib wrappers.
 *
 * Lower bits of default_zlib_header[0] is a CM, (compression method, 8 is deflate):
 *   default_zlib_header[0] |= (8 & 0x0F).
 * Upper bits of default_zlib_header[0] is a CINFO (window size, 4 corresponds to 4 kb):
 *   default_zlib_header[0] |= ((4 << 4) & 0xF0).
 * default_zlib_header[1] is the checksum for FLEVEL 0 (fastest compression, possibly lower compression ratio)
 * and CINFO 4.
*/
constexpr std::array<uint8_t, zlib_sizes::zlib_header_size> default_zlib_header = {0x48, 0x0D};

class zlib_decorator {
public:
    template <class F, class state_t, class... arguments>
    static auto unwrap(F function, state_t& state, arguments... args) noexcept -> decompression_operation_result_t;

    template <class F, class state_t>
    static auto wrap(F function, state_t& state, uint8_t* begin, const uint32_t current_in_size,
                     const uint32_t prev_adler32) noexcept -> compression_operation_result_t;

    struct zlib_header {
        uint8_t  compression_info;
        uint8_t  flags;
        bool     dictionary_flag;
        uint32_t dictionary_id;
        uint8_t  compression_level;
        uint32_t byte_size;
    };

    static auto read_header(const uint8_t* stream_ptr, uint32_t stream_size, zlib_header& header) noexcept
            -> qpl_ml_status;

    static inline void write_header_unsafe(const uint8_t* destination_ptr) noexcept {
        *(uint8_t*)(destination_ptr)      = default_zlib_header[0];
        *(uint8_t*)(destination_ptr + 1U) = default_zlib_header[1];
    }

    static inline void write_trailer_unsafe(const uint8_t* destination_ptr, const uint32_t trailer) noexcept {

        *(uint32_t*)(destination_ptr) = trailer;
    }
};

} // namespace qpl::ml::compression

#endif //QPL_ZLIB_DECORATOR_HPP_
