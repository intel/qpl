/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "huffman.h"

#include <cassert>

#include "gen.h"
#include "huff_codes.h"
#include "myintrin.h"

#define DEBUG_BB

namespace gz_generator {
uint32_t bit_reverse(uint32_t code, uint32_t length);

void huffman_c::warn_no_huffman(const char* str, uint32_t val) {
    if (m_warn_no_huff_seen) return;
    m_warn_no_huff_seen = true;

    fprintf(stderr, "Huffman code for %s %d has zero length\n", str, val);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

uint32_t huffman_c::get_lit_code(uint32_t lit, uint32_t* p_code) {
    *p_code = m_ll_codes[lit] & 0xFFFFU;
    return m_ll_codes[lit] >> 24;
}

uint32_t huffman_c::get_len_code(uint32_t len, uint32_t* p_code) {
    const uint32_t N              = 2U;
    uint32_t       msb            = 0U;
    uint32_t       num_extra_bits = 0U;
    uint32_t       extra_bits     = 0U;
    uint32_t       sym            = 0U;
    uint32_t       code_len       = 0U;

    len -= 3U;
    if (len == 0xFFU) {
        //if (p_code)
        *p_code  = m_ll_codes[285U] & 0xFFFF;
        code_len = m_ll_codes[285U] >> 24;
    } else {
        if (len == 0x100) len = 0xFF;
        msb            = bsr(len);
        num_extra_bits = (msb < N) ? 0U : msb - N;
        extra_bits     = len & ((1 << num_extra_bits) - 1U);
        len >>= num_extra_bits;
        sym      = len + (num_extra_bits << N) + 256U + 1U;
        *p_code  = (m_ll_codes[sym] & 0xFFFF) | (extra_bits << (m_ll_codes[sym] >> 24));
        code_len = (m_ll_codes[sym] >> 24) + num_extra_bits;
    }
    return code_len;
}

uint32_t huffman_c::get_dist_code(uint32_t dist, uint32_t* p_code) {
    dist -= 1U;
    const uint32_t N   = 1U;
    uint32_t       sym = 0U;

    if (dist > 32768U - 1U) {
        sym     = dist - (32768U - 1U);
        *p_code = m_d_codes[sym] & 0xFFFFU;
        return m_d_codes[sym] >> 24;
    }

    const uint32_t msb            = bsr(dist);
    const uint32_t num_extra_bits = (msb < N) ? 0U : msb - N;
    const uint32_t extra_bits     = dist & ((1 << num_extra_bits) - 1U);
    dist >>= num_extra_bits;
    sym = dist + (num_extra_bits << N);
    //        assert(sym < NUM_D_CODES);
    //    if (p_code)
    *p_code            = (m_d_codes[sym] & 0xFFFF) | (extra_bits << (m_d_codes[sym] >> 24));
    const uint32_t len = (m_d_codes[sym] >> 24) + num_extra_bits;
    return len;
}

void huffman_c::wr_dyn_hdr(BitBuffer* buffer, bool b_final, gen_c* gen) {
    uint32_t *ll_hist = nullptr, *d_hist = nullptr;
    if ((gen->m_num_ll_lens == 0U) || (gen->m_num_d_lens == 0U)) m_hist.consolidate();

    if (gen->m_num_ll_lens == 0U)
        ll_hist = m_hist.m_llcodes;
    else
        ll_hist = gen->m_ll_lens;
    if (gen->m_num_d_lens == 0U)
        d_hist = m_hist.m_distcodes;
    else
        d_hist = gen->m_d_lens;

    create_hufftables(buffer, m_ll_codes, m_d_codes, b_final ? 1 : 0, ll_hist, d_hist, gen);
}

void huffman_c::wr_fixed_hdr(BitBuffer* buffer, bool b_final) {
    uint32_t i = 0U;

    for (; i < 144U; i++) {
        m_ll_codes[i] = bit_reverse(0x30U + i, 8U) | (8 << 24);
    }
    for (; i < 256U; i++) {
        m_ll_codes[i] = bit_reverse(0x190U + i - 144U, 9U) | (9 << 24);
    }
    for (; i < 280U; i++) {
        m_ll_codes[i] = bit_reverse(i - 256U, 7U) | (7 << 24);
    }
    for (; i < 286U + 2U; i++) {
        m_ll_codes[i] = bit_reverse(0xC0U + i - 280U, 8U) | (8 << 24);
    }
    for (i = 0U; i < 30U + 2U; i++) {
        m_d_codes[i] = bit_reverse(i, 5U) | (5 << 24);
    }

    if (b_final)
        buffer->write(3U, 3U);
    else
        buffer->write(2U, 3U);
}

void huffman_c::wr_invalid_block(BitBuffer& bb, bool b_final) {
    if (b_final)
        bb.write(7U, 3U);
    else
        bb.write(6U, 3U);
}

void huffman_c::wr_block(BitBuffer& bit_buffer) {
    symbol_page* sp   = nullptr;
    symbol*      s    = nullptr;
    uint32_t     code = 0U, count = 0U, count2 = 0U;

    // push index for start of block
    writeIndex(&bit_buffer);

    for (sp = m_syms.m_head; sp; sp = sp->m_next) {
        for (uint32_t i = 0U; i < sp->m_used; i++) {
            s = &sp->m_symbols[i];
            if (s->dist == 0U) {
                count = get_lit_code(s->lit_len, &code);
#if defined(DEBUG) || (_DEBUG)
                if (count == 0U) warn_no_huffman("Lit", s->lit_len);
#endif
                bit_buffer.write(code, count);
                if (m_bout) m_byte_count++;
            } else if (s->dist == DIST_LOG) {
                writeIndex(&bit_buffer);
            } else {
                count = get_len_code(s->lit_len, &code);
#if defined(DEBUG) || (_DEBUG)
                if (count == 0U) warn_no_huffman("Len", s->lit_len);
#endif
                bit_buffer.write(code, count);
                count2 = get_dist_code(s->dist, &code);
#if defined(DEBUG) || (_DEBUG)
                if (count2 == 0U) warn_no_huffman("Dist", s->dist);
#endif
                bit_buffer.write(code, count2);
                if (m_bout) m_byte_count += s->lit_len;
            }
        }
    }
}

void huffman_c::end_block(BitBuffer& bb) {
    uint32_t code = 0U, count = 0U;

    if (!m_noeob) {
        count = get_lit_code(256U, &code);
        bb.write(code, count);
    }

    writeIndex(&bb);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void huffman_c::wr_stored_blocks(BitBuffer& bb, bool b_final, gen_c* gen, uint32_t extra) {
    uint32_t     lcount = 0U, lcountx = 0U, page_idx = 0U;
    symbol_page* sp = nullptr;
    symbol*      s  = nullptr;

    sp             = m_syms.m_head;
    uint32_t count = m_syms.m_num_lit;

    do { //NOLINT(cppcoreguidelines-avoid-do-while)
        lcount = count;
        if (lcount > 0xFFFFU) lcount = 0xFFFFU;

        // write SB header
        if (b_final && (lcount == count)) {
            bb.write(1U, 3U);
        } else {
            bb.write(0U, 3U);
        }
        bb.padToByte();
        lcountx = lcount;
        if (lcount == count) {
            lcountx += extra;
            if (lcountx > 0xFFFFU) {
                fprintf(stderr, "Internal error: stored block size %d > FFFF\n", lcountx);
                throw std::exception();
            }
        }
        bb.write(lcountx, 16U);
        if (gen->m_testmode & 8U)
            bb.write(gen->m_testparam ^ 0xFFFFU ^ lcountx, 16U);
        else
            bb.write(0xFFFFU ^ lcountx, 16U);

        writeIndex(&bb);

        for (uint32_t i = 0U; i < lcount; i++) {
            assert(sp != NULL);
            s = &sp->m_symbols[page_idx];
            if (s->dist == DIST_LOG) {
                writeIndex(&bb);
                i--;
            } else {
                assert(s->dist == 0U);
                assert(s->lit_len < 0x100U);
                bb.write(s->lit_len, 8U);
                if (m_bout) m_byte_count++;
            }

            // adv symbol
            page_idx++;
            if (page_idx >= sp->m_used) {
                page_idx = 0U;
                sp       = sp->m_next;
            }
        }
        count -= lcount;
    } while (count != 0U);

    writeIndex(&bb);
}

inline void huffman_c::writeIndex(BitBuffer* bit_buffer) {
    m_indices.push_back(bit_buffer->getBitsWritten());
}

std::vector<uint32_t> huffman_c::getIndexes() {
    return m_indices;
}
} // namespace gz_generator
