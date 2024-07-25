/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef __HUFFMAN_H__
#define __HUFFMAN_H__

#include "bitbuffer.h"
#include "histogram.h"
#include "symbol.h"

// LL 286 and 287 are treated as strange literals
// D 30 and 31 are treated as strange distances (32768+30, +31)

namespace gz_generator {
class gen_c;

class huffman_c {
protected:
    symbol_list m_syms;
    histogram   m_hist;

    // bits 31:24 = code len
    // bits 15:0  = bit reversed code len in low order bits
    uint32_t m_ll_codes[286 + 2];
    uint32_t m_d_codes[30 + 2];

    bool m_warn_no_huff_seen;
    bool m_noeob;
    bool m_bout;

    uint32_t m_byte_count;

    std::vector<uint32_t> m_indices;

    uint32_t get_lit_code(uint32_t lit, uint32_t* p_code);

    uint32_t get_len_code(uint32_t len, uint32_t* p_code);

    uint32_t get_dist_code(uint32_t dist, uint32_t* p_code);

    void warn_no_huffman(const char* str, uint32_t val);

public:
    huffman_c() {
        m_warn_no_huff_seen = false;
        m_byte_count        = 0;
        m_noeob             = false;
        m_bout              = false;
        //push zero-index
        m_indices.push_back(0);
    }

    void add_len_dist(uint32_t len, uint32_t dist) {
        m_syms.push(len, dist);
        if (len == 259) len = 257;
        if (dist <= 32768) m_hist.add_len_dist(len, dist);
    }

    void add_lit(uint32_t lit) {
        m_syms.push(lit);
        if (lit < 286) m_hist.add_lit(lit);
    }

    void reset() {
        m_syms.reset();
        m_hist.reset();
        m_noeob = false;
    }

    void log(uint32_t type) { m_syms.push(type, DIST_LOG); }

    void noeob() { m_noeob = true; }

    bool get_noeob() { return m_noeob; }

    void set_bout(bool bout) { m_bout = bout; }

    void wr_dyn_hdr(BitBuffer* buffer, bool b_final, gen_c* gen);

    void wr_fixed_hdr(BitBuffer* buffer, bool b_final);

    void wr_block(BitBuffer& buffer);

    void end_block(BitBuffer& buffer);

    void wr_stored_blocks(BitBuffer& buffer, bool b_final, gen_c* gen, uint32_t extra = 0);

    static void wr_invalid_block(BitBuffer& buffer, bool b_final);

    void writeIndex(BitBuffer* bit_buffer);

    std::vector<uint32_t> getIndexes();
};
} // namespace gz_generator
#endif // ifndef __HUFFMAN_H__
