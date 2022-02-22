/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Middle Layer API (private C++ API)
 */

#ifndef QPL_DEFAULT_DECORATOR_HPP_
#define QPL_DEFAULT_DECORATOR_HPP_

#include "compression/compression_defs.hpp"
#include "compression/inflate/isal_kernels_wrappers.hpp"
#include "util/checksum.hpp"

namespace qpl::ml::compression {

class default_decorator {
public:
    template <class F, class state_t, class ...arguments>
    static auto unwrap(F function, state_t &state, arguments... args) noexcept -> decompression_operation_result_t {
        uint8_t* saved_output_ptr  = state.get_output_data(); //state.get_output_buffer;

        auto result = function(state, args...);

        if (result.status_code_) {
            return result;
        }

        auto crc = state.get_crc();

        // @todo Add both crc32 support
        if constexpr (state_t::execution_path == execution_path_t::hardware) {
            crc = result.checksums_.crc32_;
        } else {
            crc = util::crc32_gzip(saved_output_ptr, saved_output_ptr + result.output_bytes_, crc);
        }

        state.crc_seed(crc);
        result.checksums_.crc32_ = crc;

        return result;
    }

    template <class F, class state_t, class ...arguments>
    static auto wrap(F function, state_t &state, arguments... args) noexcept -> compression_operation_result_t {
        return function(state, args...);
    }
};

}

#endif //QPL_DEFAULT_DECORATOR_HPP_
