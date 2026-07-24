/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_MIDDLE_LAYER_COMPRESSION_VERIFICATION_VERIFICATION_STATE_HPP
#define QPL_SOURCES_MIDDLE_LAYER_COMPRESSION_VERIFICATION_VERIFICATION_STATE_HPP

#include <cstring>

#include "common/defs.hpp"
#include "common/linear_allocator.hpp"
#include "compression/huffman_table/inflate_huffman_table.hpp"
#include "compression/inflate/deflate_header_decompression.hpp"
#include "compression/utils.hpp"
#include "simple_memory_ops.hpp"
#include "util/util.hpp"
#include "verification_defs.hpp"

namespace qpl::ml::compression {
template <execution_path_t path>
class verify_state;

template <execution_path_t path>
class verification_state_builder;

template <>
class verify_state<execution_path_t::software> {
private:
    friend class verification_state_builder<execution_path_t::software>;

public:
    template <class iterator_t>
    inline auto input(iterator_t begin, iterator_t end) noexcept -> verify_state&;

    inline auto first(bool value) noexcept -> verify_state&;

    inline auto decompress_table(uint8_t* deflate_header_ptr, uint32_t deflate_header_bits) noexcept -> verify_state&;

    inline auto required_crc(uint32_t crc) noexcept -> verify_state&;

    inline auto crc_seed(uint32_t seed) noexcept -> verify_state&;

    inline auto set_parser_position(parser_position_t value) noexcept -> verify_state&;

    inline auto get_parser_position() noexcept -> parser_position_t;

    inline auto reset_miniblock_state() noexcept -> verify_state&;

    inline auto slide_output_buffer() noexcept -> verify_state&;

    inline auto reset_state() noexcept -> verify_state&;

    [[nodiscard]] inline auto is_first() const noexcept -> bool;

    [[nodiscard]] inline auto get_input_data() const noexcept -> uint8_t*;

    [[nodiscard]] inline auto get_input_size() const noexcept -> uint32_t;

    [[nodiscard]] inline auto get_required_crc() const noexcept -> uint32_t;

    [[nodiscard]] inline auto get_output_data() const noexcept -> uint8_t*;

    [[nodiscard]] inline auto get_crc() const noexcept -> uint32_t;

    [[nodiscard]] inline auto get_state() -> isal_inflate_state*;

    // The verify decompression buffer must hold a full 32 KB history window (so that
    // matches spanning an output-buffer recycle can still look back the maximal deflate
    // distance) plus room to decode the next window and the kernel look-ahead slack.
    // This mirrors the sizing of isal_inflate_state::tmp_out_buffer.
    static constexpr uint32_t decompression_buffer_bytes = 2U * ISAL_DEF_HIST_SIZE + ISAL_LOOK_AHEAD;

    [[nodiscard]] constexpr static inline auto get_buffer_size() noexcept -> uint32_t {
        size_t size = 0;
        size += sizeof(state_buffer);
        size += sizeof(uint8_t) * decompression_buffer_bytes;

        return static_cast<uint32_t>(util::align_size(size, 1_kb));
    }

private:
    inline void reset() noexcept { verify_state_ptr->state_ptr.disable_multisymbol_lookup_table = 1U; }

    explicit verify_state(const qpl::ml::util::linear_allocator& allocator)
        : verify_state_ptr(allocator.allocate<state_buffer, qpl::ml::util::memory_block_t::not_aligned>(1U)) {
        verify_state_ptr->decompression_buffer_ptr =
                allocator.allocate<uint8_t, util::memory_block_t::not_aligned>(decompression_buffer_bytes);
        verify_state_ptr->decompression_buffer_size = decompression_buffer_bytes;
    };

    struct state_buffer {
        parser_position_t  parser_position = parser_position_t::verify_header;
        uint8_t*           decompression_buffer_ptr;
        uint32_t           decompression_buffer_size;
        isal_inflate_state state_ptr;
        uint32_t           crc;
    }* verify_state_ptr;

    bool     is_first_          = false;
    uint32_t required_crc_value = 0U;
};

template <>
class verify_state<execution_path_t::hardware> {
public:
    // for symmetry, no allocations are required for this state
    [[nodiscard]] constexpr static inline auto get_buffer_size() noexcept -> uint32_t { return 0; }
};

// ------ SOFTWARE PATH ------ //
template <class iterator_t>
inline auto verify_state<execution_path_t::software>::input(iterator_t begin, iterator_t end) noexcept
        -> verify_state& {
    verify_state_ptr->state_ptr.next_in  = begin;
    verify_state_ptr->state_ptr.avail_in = static_cast<uint32_t>(std::distance(begin, end));
    return *this;
}

inline auto verify_state<execution_path_t::software>::decompress_table(uint8_t* deflate_header_ptr,
                                                                       uint32_t deflate_header_bits) noexcept
        -> verify_state& {
    auto saved_next_in_ptr    = verify_state_ptr->state_ptr.next_in;
    auto saved_avail_in       = verify_state_ptr->state_ptr.avail_in;
    auto saved_read_in        = verify_state_ptr->state_ptr.read_in;
    auto saved_read_in_length = verify_state_ptr->state_ptr.read_in_length;

    verify_state_ptr->state_ptr.read_in        = 0;
    verify_state_ptr->state_ptr.read_in_length = 0;
    verify_state_ptr->state_ptr.next_in        = deflate_header_ptr;
    verify_state_ptr->state_ptr.avail_in       = (deflate_header_bits + 7U) >> 3;

    auto status = read_header_stateful(verify_state_ptr->state_ptr);
    MAYBE_UNUSED(status);

    verify_state_ptr->state_ptr.next_in        = saved_next_in_ptr;
    verify_state_ptr->state_ptr.avail_in       = saved_avail_in;
    verify_state_ptr->state_ptr.read_in        = saved_read_in;
    verify_state_ptr->state_ptr.read_in_length = saved_read_in_length;

    verify_state_ptr->state_ptr.block_state = ISAL_BLOCK_CODED;

    return *this;
}

inline auto verify_state<execution_path_t::software>::required_crc(uint32_t crc) noexcept -> verify_state& {
    required_crc_value = crc;
    return *this;
}

inline auto verify_state<execution_path_t::software>::set_parser_position(parser_position_t value) noexcept
        -> verify_state& {
    verify_state_ptr->parser_position = value;
    return *this;
}

inline auto verify_state<execution_path_t::software>::first(bool value) noexcept -> verify_state& {
    is_first_ = value;
    return *this;
}

inline auto verify_state<execution_path_t::software>::get_parser_position() noexcept -> parser_position_t {
    return verify_state_ptr->parser_position;
}

inline auto verify_state<execution_path_t::software>::crc_seed(uint32_t seed) noexcept -> verify_state& {
    verify_state_ptr->crc = seed;

    return *this;
}

inline auto verify_state<execution_path_t::software>::reset_miniblock_state() noexcept -> verify_state& {
    verify_state_ptr->state_ptr.next_out  = verify_state_ptr->decompression_buffer_ptr;
    verify_state_ptr->state_ptr.avail_out = verify_state_ptr->decompression_buffer_size;
    verify_state_ptr->state_ptr.total_out = 0;

    return *this;
}

inline auto verify_state<execution_path_t::software>::slide_output_buffer() noexcept -> verify_state& {
    auto&    state     = verify_state_ptr->state_ptr;
    uint8_t* buf_start = verify_state_ptr->decompression_buffer_ptr;

    const auto decoded_bytes = static_cast<uint32_t>(state.next_out - buf_start);

    // Recycle the decompression buffer while keeping the last 32 KB of output as history.
    // A match may span the recycle boundary (deflate distances reach back up to 32 KB), so
    // that history must remain physically present for the copy to resolve correctly. This
    // mirrors the sliding-window logic of qpl_isal_inflate.
    if (decoded_bytes > ISAL_DEF_HIST_SIZE) {
        std::memmove(buf_start, state.next_out - ISAL_DEF_HIST_SIZE, ISAL_DEF_HIST_SIZE);
        state.next_out = buf_start + ISAL_DEF_HIST_SIZE;
    }

    state.avail_out = verify_state_ptr->decompression_buffer_size - static_cast<uint32_t>(state.next_out - buf_start);

    return *this;
}

inline auto verify_state<execution_path_t::software>::reset_state() noexcept -> verify_state& {
    reset_inflate_state(&verify_state_ptr->state_ptr);

    return *this;
}

[[nodiscard]] inline auto verify_state<execution_path_t::software>::is_first() const noexcept -> bool {
    return is_first_;
}

[[nodiscard]] inline auto verify_state<execution_path_t::software>::get_input_data() const noexcept -> uint8_t* {
    return verify_state_ptr->state_ptr.next_in;
}

[[nodiscard]] inline auto verify_state<execution_path_t::software>::get_input_size() const noexcept -> uint32_t {
    return verify_state_ptr->state_ptr.avail_in +
           util::bit_to_byte(static_cast<uint32_t>(verify_state_ptr->state_ptr.read_in_length));
}

[[nodiscard]] inline auto verify_state<execution_path_t::software>::get_required_crc() const noexcept -> uint32_t {
    return required_crc_value;
}

[[nodiscard]] inline auto verify_state<execution_path_t::software>::get_output_data() const noexcept -> uint8_t* {
    return verify_state_ptr->decompression_buffer_ptr;
}

[[nodiscard]] inline auto verify_state<execution_path_t::software>::get_crc() const noexcept -> uint32_t {
    return verify_state_ptr->crc;
}

[[nodiscard]] inline auto verify_state<execution_path_t::software>::get_state() -> isal_inflate_state* {
    return &verify_state_ptr->state_ptr;
}

// ------ HARDWARE PATH ------ //
// implementation goes there

} // namespace qpl::ml::compression
#endif //QPL_SOURCES_MIDDLE_LAYER_COMPRESSION_VERIFICATION_VERIFICATION_STATE_HPP
