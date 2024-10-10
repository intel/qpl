/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <assert.h>

#include "crc.h"
#include "deflate_defs.h"
#include "deflate_hash_table.h"
#include "deflate_slow_matcher.h"
#include "deflate_slow_utils.h"
#include "huffman.h"
#include "igzip_lib.h"
#include "immintrin.h"
#include "own_qplc_defs.h"
#include "qplc_checksum.h"
#include "qplc_compression_consts.h"
#include "stdbool.h"

#define MAX_MATCH        258 // Largest deflate match length
#define MIN_MATCH4       4
#define CMP_MATCH_LENGTH MIN_MATCH4

static inline uint32_t hash_crc(const uint8_t* p_src) {
    return compute_hash(*(uint32_t*)(p_src));
} /* hash_src */

static inline void own_flush_bits(struct BitBuf2* me) {
    uint32_t bits            = 0U;
    *(int64_t*)me->m_out_buf = me->m_bits;
    bits                     = me->m_bit_count & ~7;
    me->m_bit_count -= bits;
    me->m_out_buf += bits / 8;
    me->m_bits >>= bits;
}

static inline void own_flush_bits_safe(struct BitBuf2* me) {
    uint32_t bits = 0U;
    if ((me->m_out_buf + 8) <= me->m_out_end) {
        *(int64_t*)me->m_out_buf = me->m_bits;
        bits                     = me->m_bit_count & ~7;
        me->m_bit_count -= bits;
        me->m_out_buf += bits / 8;
        me->m_bits >>= bits;
    } else {
        for (; me->m_bit_count >= 8; me->m_bit_count -= 8) {
            if (me->m_out_buf >= me->m_out_end) { break; }
            *me->m_out_buf++ = (uint8_t)me->m_bits;
            me->m_bits >>= 8;
        }
    }
}

static inline void own_write_bits(struct BitBuf2* me, uint64_t code,
                                  uint32_t count) { /* Assumes there is space to fit code into m_bits. */
    me->m_bits |= code << me->m_bit_count;
    me->m_bit_count += count;
    own_flush_bits(me);
}

static inline void own_write_bits_safe(struct BitBuf2* me, uint64_t code,
                                       uint32_t count) { /* Assumes there is space to fit code into m_bits. */
    me->m_bits |= code << me->m_bit_count;
    me->m_bit_count += count;
    own_flush_bits_safe(me);
}

static inline uint32_t own_count_significant_bits(uint32_t value) {
    return bsr(value);
}

static inline void compute_offset_code(const struct isal_hufftables* huffman_table_ptr, uint16_t offset,
                                       uint64_t* const code_ptr, uint32_t* const code_length_ptr) {

    // Variables
    uint32_t significant_bits     = 0U;
    uint32_t number_of_extra_bits = 0U;
    uint32_t extra_bits           = 0U;
    uint32_t symbol               = 0U;
    uint32_t length               = 0U;
    uint32_t code                 = 0U;

    offset -= 1U;
    significant_bits = own_count_significant_bits(offset);
    assert(significant_bits >= 2);

    number_of_extra_bits = significant_bits - 2U;
    extra_bits           = offset & ((1U << number_of_extra_bits) - 1U);
    //extra_bits = _bzhi_u32(offset, number_of_extra_bits);
    offset >>= number_of_extra_bits;
    symbol = offset + 2 * number_of_extra_bits;

    // Extracting information from table
    code   = huffman_table_ptr->dcodes[symbol];
    length = huffman_table_ptr->dcodes_sizes[symbol];

    // Return of the calculated results
    *code_ptr        = code | (extra_bits << length);
    *code_length_ptr = length + number_of_extra_bits;
}

static void
_get_offset_code(const struct isal_hufftables* const huffman_table_ptr, //NOLINT(bugprone-reserved-identifier)
                 uint32_t offset, uint64_t* const code_ptr, uint32_t* const code_length_ptr) {

    if (offset <= IGZIP_DIST_TABLE_SIZE && offset > 0U) {
        const uint64_t offset_info = huffman_table_ptr->dist_table[offset - 1];

        *code_ptr        = offset_info >> 5U;
        *code_length_ptr = offset_info & 0x1FU;
    } else {
        compute_offset_code(huffman_table_ptr, offset, code_ptr, code_length_ptr);
    }
}

/**
 * @brief Main deflate body function for high-level static/fixed mode compression,
 * returns number of processed bytes.
 *
 * @note There's another similar function named slow_deflate_icf_body, with "_icf_" in its name.
 * The difference is that this function writes the actual huffman codes,
 * while the icf version writes an "intermediate compressed format" and
 * increments histogram counters.
 * The icf version is used for dynamic mode, while the non-icf version
 * is used for static/fixed mode, or dynamic with mini blocks.
 *
 **/
OWN_QPLC_FUN(uint32_t, slow_deflate_body,
             (uint8_t * current_ptr, const uint8_t* const lower_bound_ptr, const uint8_t* const upper_bound_ptr,
              deflate_hash_table_t* hash_table_ptr, struct isal_hufftables* huffman_tables_ptr,
              struct BitBuf2* bit_writer_ptr)) {

    const uint8_t*       p_src_tmp    = NULL;
    const uint8_t*       p_str        = NULL;
    const uint8_t* const p_src        = lower_bound_ptr;
    int32_t*             p_hash_table = (int32_t*)hash_table_ptr->hash_table_ptr;
    int32_t*             p_hash_story = (int32_t*)hash_table_ptr->hash_story_ptr;
    int                  src_start    = (int)(current_ptr - lower_bound_ptr);
    int                  src_len      = (int)(upper_bound_ptr - lower_bound_ptr) - (MAX_MATCH + MIN_MATCH4 - 1);
    int                  indx_src     = (int)(current_ptr - lower_bound_ptr);
    int                  hash_mask    = hash_table_ptr->hash_mask;
    int                  win_mask     = QPLC_DEFLATE_MAXIMAL_OFFSET - 1;
    int                  hash_key     = 0;
    int                  bound = 0, win_bound = 0, tmp = 0, candidate = 0, index = 0;
    uint32_t             win_size = QPLC_DEFLATE_MAXIMAL_OFFSET;

    {
        int chain_length_current = 256; /* temporary */
        int good_match           = hash_table_ptr->good_match;
        int nice_match           = hash_table_ptr->nice_match;
        int lazy_match           = hash_table_ptr->lazy_match;
        int chain_length         = 0;

        {
#if PLATFORM >= K0
            __m256i aUnit, bUnit;
#else
            uint64_t aUnit = 0U, bUnit = 0U;
#endif

            int flag_cmp   = 0;
            int prev_bound = 0;

            uint16_t prev_dist = 0;
            uint8_t  prev_ch   = 0;
            //dst_len -= 1;

            for (; (indx_src < src_len) && ((bit_writer_ptr->m_out_buf + 8) <= bit_writer_ptr->m_out_end); indx_src++) {
                p_str     = p_src + indx_src;
                win_bound = indx_src - (int)win_size;
                hash_key  = hash_crc(p_str) & hash_mask;
                index = tmp                       = p_hash_table[hash_key];
                p_hash_story[indx_src & win_mask] = index;
                p_hash_table[hash_key]            = indx_src;
                bound                             = prev_bound;
                chain_length                      = chain_length_current;

                if (prev_bound < lazy_match) {
                    if (prev_bound >= good_match) { chain_length >>= 2; }
                    if (bound < (CMP_MATCH_LENGTH - 1)) { bound = (CMP_MATCH_LENGTH - 1); }

#if PLATFORM >= K0
                    aUnit = _mm256_loadu_si256((__m256i const*)p_str);
#else
                    aUnit = *(const uint64_t*)p_str;
#endif

                    for (int k = 0; k < chain_length; k++) {
                        if (!(win_bound < tmp)) { break; }
                        p_src_tmp = p_src + tmp;
                        candidate = tmp;
                        tmp       = p_hash_story[tmp & win_mask];
                        if (*(uint32_t*)(p_str + bound - 3) != *(uint32_t*)(p_src_tmp + bound - 3)) { continue; }

#if PLATFORM >= K0

                        if (bound < 32) {
                            bUnit    = _mm256_loadu_si256((__m256i const*)p_src_tmp);
                            bUnit    = _mm256_cmpeq_epi8(bUnit, aUnit);
                            flag_cmp = (uint32_t)_mm256_movemask_epi8(bUnit);
                            flag_cmp = ~flag_cmp;
                            flag_cmp = _tzcnt_u32(flag_cmp);
#else

                        if (bound < 8) {
                            bUnit = *(const uint64_t*)p_src_tmp;
                            bUnit ^= aUnit;              // 1-bits = difference
                            flag_cmp = tzbytecnt(bUnit); // count low-order 0-bits

#endif

                            if (flag_cmp < bound) { continue; }
                            if (flag_cmp >= good_match) {
                                if (chain_length == chain_length_current) { chain_length >>= 2; }
                            }
                            bound = flag_cmp;
                            index = candidate;

#if PLATFORM >= K0

                            if (flag_cmp != 32) { continue; }
                            for (; bound < 256; bound += 32) {
                                bUnit    = _mm256_cmpeq_epi8(_mm256_loadu_si256((__m256i const*)(p_str + bound)),
                                                             _mm256_loadu_si256((__m256i const*)(p_src_tmp + bound)));
                                flag_cmp = (uint32_t)_mm256_movemask_epi8(bUnit);
                                if ((uint32_t)flag_cmp != 0xffffffff) { break; }
                            }
                            if (bound != 256) {
                                flag_cmp = ~flag_cmp;
                                flag_cmp = _tzcnt_u32(flag_cmp);
                                bound += flag_cmp;

#else

                            if (flag_cmp != 8) { continue; }
                            for (; bound < 256; bound += 8) {
                                bUnit = (*(const uint64_t*)(p_str + bound)) ^ (*(const uint64_t*)(p_src_tmp + bound));
                                if (bUnit != 0) break;
                            }
                            if (bound != 256) {
                                flag_cmp = tzbytecnt(bUnit); // count low-order 0-bits
                                bound += flag_cmp;

#endif

                            } else {
                                if (p_str[256] == p_src_tmp[256]) {
                                    bound = 257;
                                    if (p_str[257] == p_src_tmp[257]) { bound = 258; }
                                }
                            }
                            if (bound >= nice_match) { break; }
                        } else {
                            int l = 0;

#if PLATFORM >= K0

                            for (l = 0; l < 256; l += 32) {
                                bUnit    = _mm256_cmpeq_epi8(_mm256_loadu_si256((__m256i const*)(p_str + l)),
                                                             _mm256_loadu_si256((__m256i const*)(p_src_tmp + l)));
                                flag_cmp = (uint32_t)_mm256_movemask_epi8(bUnit);
                                if ((uint32_t)flag_cmp != 0xffffffff) { break; }
                            }
                            if (l != 256) {
                                flag_cmp = ~flag_cmp;
                                flag_cmp = _tzcnt_u32(flag_cmp);
                                l += flag_cmp;

#else

                            for (l = 0; l < 256; l += 8) {
                                bUnit = (*(const uint64_t*)(p_str + l)) ^ (*(const uint64_t*)(p_src_tmp + l));
                                if (bUnit != 0) break;
                            }
                            if (l != 256) {
                                flag_cmp = tzbytecnt(bUnit); // count low-order 0-bits
                                l += flag_cmp;

#endif

                            } else {
                                if (p_str[256] == p_src_tmp[256]) {
                                    l = 257;
                                    if (p_str[257] == p_src_tmp[257]) { l = 258; }
                                }
                            }
                            if (l > bound) {
                                bound = l;
                                index = candidate;
                            }
                            if (bound >= nice_match) { break; }
                        }
                    }
                }

                if (prev_bound > 1) {
                    uint64_t code        = 0U;
                    uint32_t code_length = 0U;
                    if ((prev_bound >= bound) && (prev_bound > (CMP_MATCH_LENGTH - 1))) {
                        const uint64_t match_length_info  = huffman_tables_ptr->len_table[prev_bound - 3U];
                        uint64_t       code_match         = 0U;
                        uint32_t       code_match_length  = 0U;
                        uint64_t       code_offset        = 0U;
                        uint32_t       code_offset_length = 0U;
                        int            k                  = 0;

                        if (prev_dist < prev_bound) {
                            int m = prev_bound - prev_dist - 3;
                            if (m < 1) { m = 1; }
                            for (k = indx_src + m, indx_src += prev_bound - 2; k <= indx_src; k++) {
                                hash_key                   = hash_crc(p_src + k) & hash_mask;
                                p_hash_story[k & win_mask] = p_hash_table[hash_key];
                                p_hash_table[hash_key]     = k;
                            }
                        } else {
                            for (k = indx_src + 1, indx_src += prev_bound - 2; k <= indx_src; k++) {
                                hash_key                   = hash_crc(p_src + k) & hash_mask;
                                p_hash_story[k & win_mask] = p_hash_table[hash_key];
                                p_hash_table[hash_key]     = k;
                            }
                        }

                        code_match        = match_length_info >> 5U;
                        code_match_length = match_length_info & 0x1FU;
                        _get_offset_code(huffman_tables_ptr, prev_dist, &code_offset,
                                         &code_offset_length); //NOLINT(bugprone-reserved-identifier)
                        // Combining two codes
                        code_match |= code_offset << code_match_length;
                        // Writing to the output
                        own_write_bits(bit_writer_ptr, code_match, code_match_length + code_offset_length);
                        bound = 0;
                    } else {
                        code        = huffman_tables_ptr->lit_table[prev_ch];
                        code_length = huffman_tables_ptr->lit_table_sizes[prev_ch];
                        own_write_bits(bit_writer_ptr, code, code_length);
                    }
                }
                prev_dist  = (uint16_t)(indx_src - index);
                prev_bound = bound;
                prev_ch    = p_src[indx_src];
            }

            {
                int bound_lim = 0;

                src_len += MAX_MATCH;
                for (; (indx_src < src_len) && (bit_writer_ptr->m_out_buf <= bit_writer_ptr->m_out_end); indx_src++) {
                    p_str     = p_src + indx_src;
                    win_bound = indx_src - (int)win_size;
                    hash_key  = hash_crc(p_str) & hash_mask;
                    index = tmp                       = p_hash_table[hash_key];
                    p_hash_story[indx_src & win_mask] = index;
                    p_hash_table[hash_key]            = indx_src;
                    bound_lim                         = QPL_MIN((src_len - indx_src), MAX_MATCH);
                    bound                             = prev_bound;
                    chain_length                      = chain_length_current;

                    if (prev_bound < lazy_match) {
                        if (prev_bound >= good_match) { chain_length >>= 2; }
                        if (bound < (CMP_MATCH_LENGTH - 1)) { bound = (CMP_MATCH_LENGTH - 1); }
                        for (int k = 0; k < chain_length; k++) {
                            if (!(win_bound < tmp)) { break; }
                            p_src_tmp = p_src + tmp;
                            candidate = tmp;
                            tmp       = p_hash_story[tmp & win_mask];
                            if (*(uint32_t*)(p_str + bound - 3) != *(uint32_t*)(p_src_tmp + bound - 3)) { continue; }
                            {
                                int l = 0;
                                for (l = 0; l < bound_lim; l++) {
                                    if (p_str[l] != p_src_tmp[l]) { break; }
                                }
                                if (bound < l) {
                                    bound = l;
                                    index = candidate;
                                    if (bound >= nice_match) { break; }
                                }
                            }
                        }
                    }
                    if (prev_bound > 1) {
                        uint64_t code        = 0U;
                        uint32_t code_length = 0U;
                        if ((prev_bound >= bound) && (prev_bound > (CMP_MATCH_LENGTH - 1))) {
                            const uint64_t match_length_info  = huffman_tables_ptr->len_table[prev_bound - 3U];
                            uint64_t       code_match         = 0U;
                            uint32_t       code_match_length  = 0U;
                            uint64_t       code_offset        = 0U;
                            uint32_t       code_offset_length = 0U;
                            int            k                  = 0;

                            if (prev_dist < prev_bound) {
                                int m = prev_bound - prev_dist - 3;
                                if (m < 1) { m = 1; }
                                for (k = indx_src + m, indx_src += prev_bound - 2; k <= indx_src; k++) {
                                    hash_key                   = hash_crc(p_src + k) & hash_mask;
                                    p_hash_story[k & win_mask] = p_hash_table[hash_key];
                                    p_hash_table[hash_key]     = k;
                                }
                            } else {
                                for (k = indx_src + 1, indx_src += prev_bound - 2; k <= indx_src; k++) {
                                    hash_key                   = hash_crc(p_src + k) & hash_mask;
                                    p_hash_story[k & win_mask] = p_hash_table[hash_key];
                                    p_hash_table[hash_key]     = k;
                                }
                            }
                            code_match        = match_length_info >> 5U;
                            code_match_length = match_length_info & 0x1FU;
                            _get_offset_code(huffman_tables_ptr, prev_dist, &code_offset,
                                             &code_offset_length); //NOLINT(bugprone-reserved-identifier)
                            // Combining two codes
                            code_match |= code_offset << code_match_length;
                            // Writing to the output
                            own_write_bits_safe(bit_writer_ptr, code_match, code_match_length + code_offset_length);
                            bound = 0;
                        } else {
                            code        = huffman_tables_ptr->lit_table[prev_ch];
                            code_length = huffman_tables_ptr->lit_table_sizes[prev_ch];
                            own_write_bits_safe(bit_writer_ptr, code, code_length);
                        }
                    }
                    prev_dist  = (uint16_t)(indx_src - index);
                    prev_bound = bound;
                    prev_ch    = p_src[indx_src];
                }
            }
            if (prev_bound > 1) {
                uint64_t code        = 0U;
                uint32_t code_length = 0U;
                if (prev_bound > (CMP_MATCH_LENGTH - 1)) {
                    const uint64_t match_length_info  = huffman_tables_ptr->len_table[prev_bound - 3U];
                    uint64_t       code_match         = 0U;
                    uint32_t       code_match_length  = 0U;
                    uint64_t       code_offset        = 0U;
                    uint32_t       code_offset_length = 0U;
                    int            k                  = 0;

                    for (k = indx_src + 1, indx_src += prev_bound - 1; k < indx_src; k++) {
                        hash_key                   = hash_crc(p_src + k) & hash_mask;
                        p_hash_story[k & win_mask] = p_hash_table[hash_key];
                        p_hash_table[hash_key]     = k;
                    }
                    code_match        = match_length_info >> 5U;
                    code_match_length = match_length_info & 0x1FU;
                    _get_offset_code(huffman_tables_ptr, prev_dist, &code_offset,
                                     &code_offset_length); //NOLINT(bugprone-reserved-identifier)
                    // Combining two codes
                    code_match |= code_offset << code_match_length;
                    // Writing to the output
                    own_write_bits_safe(bit_writer_ptr, code_match, code_match_length + code_offset_length);
                    bound = 0;
                } else {
                    code        = huffman_tables_ptr->lit_table[prev_ch];
                    code_length = huffman_tables_ptr->lit_table_sizes[prev_ch];
                    own_write_bits_safe(bit_writer_ptr, code, code_length);
                }
            }
            {
                uint64_t code        = 0U;
                uint32_t code_length = 0U;

                // Process the last few bytes of input where no match can be made and emit literals
                for (src_len += (MIN_MATCH4 - 1);
                     (indx_src < src_len) && (bit_writer_ptr->m_out_buf <= bit_writer_ptr->m_out_end); indx_src++) {
                    prev_ch     = p_src[indx_src];
                    code        = huffman_tables_ptr->lit_table[prev_ch];
                    code_length = huffman_tables_ptr->lit_table_sizes[prev_ch];
                    own_write_bits_safe(bit_writer_ptr, code, code_length);
                    if ((src_len - (indx_src + (CMP_MATCH_LENGTH - 1))) > 0) {
                        hash_key                          = hash_crc(p_src + indx_src) & hash_mask;
                        p_hash_story[indx_src & win_mask] = p_hash_table[hash_key];
                        p_hash_table[hash_key]            = indx_src;
                    }
                }
            }
        }
    }
    return (uint32_t)(indx_src - src_start);
}
