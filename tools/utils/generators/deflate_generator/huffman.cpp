/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <assert.h>

#include "huffman.h"
#include "huff_codes.h"
#include "myintrin.h"
#include "gen.h"

#define DEBUG_BB

namespace gz_generator
{
    uint32_t bit_reverse(uint32_t code, uint32_t length);

    void huffman_c::warn_no_huffman(const char *str, uint32_t val)
    {
        if (m_warn_no_huff_seen)
            return;
        m_warn_no_huff_seen = true;

        fprintf(stderr, "Huffman code for %s %d has zero length\n", str, val);
    }

    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////

    uint32_t
    huffman_c::get_lit_code(uint32_t lit, uint32_t *p_code)
    {
        *p_code = m_ll_codes[lit] & 0xFFFF;
        return m_ll_codes[lit] >> 24;
    }

    uint32_t
    huffman_c::get_len_code(uint32_t len, uint32_t *p_code)
    {
        const uint32_t N = 2;
        uint32_t       msb;
        uint32_t       num_extra_bits;
        uint32_t       extra_bits;
        uint32_t       sym;
        uint32_t       clen;

        len -= 3;
        if (len == 0xFF)
        {
            //if (p_code)
            *p_code = m_ll_codes[285] & 0xFFFF;
            clen = m_ll_codes[285] >> 24;
        }
        else
        {
            if (len == 0x100)
                len = 0xFF;
            msb = bsr(len);
            num_extra_bits = (msb < N) ? 0 : msb - N;
            extra_bits= len & ((1 << num_extra_bits) - 1);
            len >>= num_extra_bits;
            sym = len + (num_extra_bits << N) + 256 + 1;
            *p_code = (m_ll_codes[sym] & 0xFFFF) |
                      (extra_bits << (m_ll_codes[sym] >> 24));
            clen = (m_ll_codes[sym] >> 24) + num_extra_bits;
        }
        return clen;
    }

    uint32_t
    huffman_c::get_dist_code(uint32_t dist, uint32_t *p_code)
    {
        dist -= 1;
        const uint32_t N = 1;
        uint32_t       msb;
        uint32_t       num_extra_bits;
        uint32_t       extra_bits;
        uint32_t       sym;
        uint32_t       len;

        if (dist > 32768 - 1)
        {
            sym = dist - (32768 - 1);
            *p_code = m_d_codes[sym] & 0xFFFF;
            return m_d_codes[sym] >> 24;
        }

        msb            = bsr(dist);
        num_extra_bits = (msb < N) ? 0 : msb - N;
        extra_bits     = dist & ((1 << num_extra_bits) - 1);
        dist >>= num_extra_bits;
        sym            = dist + (num_extra_bits << N);
        //        assert(sym < NUM_D_CODES);
        //    if (p_code)
        *p_code = (m_d_codes[sym] & 0xFFFF) |
                  (extra_bits << (m_d_codes[sym] >> 24));
        len = (m_d_codes[sym] >> 24) + num_extra_bits;
        return len;
    }

    void
    huffman_c::wr_dyn_hdr(BitBuffer *buffer, bool b_final, gen_c *gen)
    {
        uint32_t *ll_hist, *d_hist;
        if ((gen->m_num_ll_lens == 0) || (gen->m_num_d_lens == 0))
            m_hist.consolidate();

        if (gen->m_num_ll_lens == 0)
            ll_hist = m_hist.m_llcodes;
        else
            ll_hist = gen->m_ll_lens;
        if (gen->m_num_d_lens == 0)
            d_hist  = m_hist.m_distcodes;
        else
            d_hist = gen->m_d_lens;

        create_hufftables(buffer, m_ll_codes, m_d_codes,
                          b_final ? 1 : 0,
                          ll_hist, d_hist,
                          gen);
    }

    void
    huffman_c::wr_fixed_hdr(BitBuffer *buffer, bool b_final)
    {
        uint32_t i;

        for (i = 0; i < 144; i++)
        {
            m_ll_codes[i] = bit_reverse(0x30 + i, 8) | (8 << 24);
        }
        for (; i < 256; i++)
        {
            m_ll_codes[i] = bit_reverse(0x190 + i - 144, 9) | (9 << 24);
        }
        for (; i < 280; i++)
        {
            m_ll_codes[i] = bit_reverse(i - 256, 7) | (7 << 24);
        }
        for (; i < 286 + 2; i++)
        {
            m_ll_codes[i] = bit_reverse(0xC0 + i - 280, 8) | (8 << 24);
        }
        for (i = 0; i < 30 + 2; i++)
        {
            m_d_codes[i] = bit_reverse(i, 5) | (5 << 24);
        }

        if (b_final)
            buffer->write(3U, 3U);
        else
            buffer->write(2U, 3U);
    }

    void
    huffman_c::wr_invalid_block(BitBuffer &bb, bool b_final)
    {
        if (b_final)
            bb.write(7U, 3U);
        else
            bb.write(6U, 3U);
    }

    void
    huffman_c::wr_block(BitBuffer &bit_buffer)
    {
        symbol_page *sp;
        symbol      *s;
        uint32_t    i;
        uint32_t    code, count, count2;

        // push index for start of block
        writeIndex(&bit_buffer);

        for (sp = m_syms.m_head; sp; sp = sp->m_next)
        {
            for (i = 0; i < sp->m_used; i++)
            {
                s = &sp->m_symbols[i];
                if (s->dist == 0)
                {
                    count = get_lit_code(s->lit_len, &code);
#if defined (DEBUG) || (_DEBUG)
                    if (count == 0)
                        warn_no_huffman("Lit", s->lit_len);
#endif
                    bit_buffer.write(code, count);
                    if (m_bout)
                        m_byte_count++;
                }
                else if (s->dist == DIST_LOG)
                {
                    writeIndex(&bit_buffer);
                }
                else
                {
                    count = get_len_code(s->lit_len, &code);
#if defined (DEBUG) || (_DEBUG)
                    if (count == 0)
                        warn_no_huffman("Len", s->lit_len);
#endif
                    bit_buffer.write(code, count);
                    count2 = get_dist_code(s->dist, &code);
#if defined (DEBUG) || (_DEBUG)
                    if (count2 == 0)
                        warn_no_huffman("Dist", s->dist);
#endif
                    bit_buffer.write(code, count2);
                    if (m_bout)
                        m_byte_count += s->lit_len;
                }
            }
        }
    }

    void
    huffman_c::end_block(BitBuffer &bb)
    {
        uint32_t code, count;

        if (!m_noeob)
        {
            count = get_lit_code(256, &code);
            bb.write(code, count);
        }

        writeIndex(&bb);
    }

    ////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////

    void
    huffman_c::wr_stored_blocks(BitBuffer &bb, bool b_final, gen_c *gen, uint32_t extra)
    {
        uint32_t    i, count, lcount, lcountx, page_idx;
        symbol_page *sp;
        symbol      *s;

        sp       = m_syms.m_head;
        page_idx = 0;

        count = m_syms.m_num_lit;
        do
        {
            lcount     = count;
            if (lcount > 0xFFFF)
                lcount = 0xFFFF;

            // write SB header
            if (b_final && (lcount == count))
            {
                bb.write(1U, 3U);
            }
            else
            {
                bb.write(0, 3);
            }
            bb.padToByte();
            lcountx = lcount;
            if (lcount == count)
            {
                lcountx += extra;
                if (lcountx > 0xffff)
                {
                    fprintf(stderr, "Internal error: stored block size %d > FFFF\n", lcountx);
                    throw std::exception();
                }
            }
            bb.write(lcountx, 16);
            if (gen->m_testmode & 8)
                bb.write(gen->m_testparam ^ 0xFFFF ^ lcountx, 16);
            else
                bb.write(0xFFFF ^ lcountx, 16);

            writeIndex(&bb);

            for (i = 0; i < lcount; i++)
            {
                assert(sp != NULL);
                s = &sp->m_symbols[page_idx];
                if (s->dist == DIST_LOG)
                {
                    writeIndex(&bb);
                    i--;
                }
                else
                {
                    assert(s->dist == 0);
                    assert(s->lit_len < 0x100);
                    bb.write(s->lit_len, 8);
                    if (m_bout)
                        m_byte_count++;
                }

                // adv symbol
                page_idx++;
                if (page_idx >= sp->m_used)
                {
                    page_idx = 0;
                    sp       = sp->m_next;
                }
            }
            count -= lcount;
        } while (count != 0);

        writeIndex(&bb);
    }

    inline void huffman_c::writeIndex(BitBuffer *bit_buffer)
    {
        m_indices.push_back(bit_buffer->getBitsWritten());
    }

    std::vector<uint32_t> huffman_c::getIndexes()
    {
        return m_indices;
    }
}