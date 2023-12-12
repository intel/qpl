/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "own_qplc_defs.h"
#include "qplc_compression_consts.h"
#include "immintrin.h"

#include "stdbool.h"

#include "igzip_lib.h"
#include "encode_df.h"

#include "deflate_defs.h"
#include "qplc_checksum.h"
#include "crc.h"
#include "huffman.h"

#include "deflate_slow_utils.h"
#include "deflate_slow_matcher.h"

typedef struct deflate_icf_stream deflate_icf_stream;

#define MAX_MATCH   258 // Largest deflate match length
#define MIN_MATCH4  4
#define _CMP_MATCH_LENGTH MIN_MATCH4

static inline uint32_t hash_crc(const uint8_t* p_src) {
    return compute_hash(*(uint32_t*)(p_src));
}

static inline void own_write_deflate_icf(struct deflate_icf* icf,
                                         uint32_t lit_len,
                                         uint32_t lit_dist,
                                         uint32_t extra_bits) {
    *(uint32_t*)icf = lit_len |
        (lit_dist << LIT_LEN_BIT_COUNT) |
        (extra_bits << (LIT_LEN_BIT_COUNT + DIST_LIT_BIT_COUNT));
}

inline static void own_write_deflate_icf_lit(struct deflate_icf* icf,
                                             uint32_t lit_len) {
    *(uint32_t*)icf = lit_len | (LITERAL_DISTANCE_IN_ICF << LIT_LEN_BIT_COUNT);
}

static inline uint32_t own_get_offset_table_index(const uint32_t offset) {
    uint32_t x, y, z, u;
    uint32_t res;
    x = offset - 1;
    y = x >> 2;
    z = bsr(y);
    u = z + z;
    res = u + (x >> z);
    return res;
}

/**
 * @brief Main deflate body function for high-level dynamic mode compression,
 * returns number of processed bytes.
 *
 * @note hash_table_ptr->hash_table_ptr stores head of hash chain for specific hash key,
 * hash_table_ptr->hash_story_ptr stores and links the positions of strings with the same
 * hash index. By default, first is initialized to OWN_UNINITIALIZED_INDEX_32u and second to 0.
 *
 * @note There's another similar function slow_deflate_body, without "_icf_" in its name.
 * The difference is that this function writes an "intermediate compressed format" and
 * increments histogram counters, while the non-icf function writes the actual huffman codes.
 * The icf version is used for dynamic mode, while the non-icf version
 * is used for static/fixed mode, or dynamic with mini blocks.
 **/
OWN_QPLC_FUN(uint32_t, slow_deflate_icf_body, (uint8_t *current_ptr,
                                               const uint8_t *const lower_bound_ptr,
                                               const uint8_t *const upper_bound_ptr,
                                               deflate_hash_table_t *hash_table_ptr,
                                               isal_mod_hist        *histogram_ptr,
                                               deflate_icf_stream   *icf_stream_ptr)) {

    const uint8_t*       p_src_tmp;
    const uint8_t*       p_str;
    const uint8_t* const p_src        = lower_bound_ptr;
    int32_t*             p_hash_table = (int32_t*)hash_table_ptr->hash_table_ptr;
    int32_t*             p_hash_story = (int32_t*)hash_table_ptr->hash_story_ptr;

    int      src_start = (int)(current_ptr - lower_bound_ptr);
    int      src_len   = (int)(upper_bound_ptr - lower_bound_ptr) - (MAX_MATCH + MIN_MATCH4 - 1);
    int      dst_len   = (int)((int*)icf_stream_ptr->end_ptr - (int*)(icf_stream_ptr->next_ptr)) - 1;
    int      indx_src  = (int)(current_ptr - lower_bound_ptr);
    int      indx_dst  = 0;
    int      hash_mask = hash_table_ptr->hash_mask;
    int      win_mask  = QPLC_DEFLATE_MAXIMAL_OFFSET - 1;
    int      hash_key  = 0;
    int      bound, win_bound, tmp, candidat, index;
    uint32_t win_size = QPLC_DEFLATE_MAXIMAL_OFFSET;
    uint16_t dist;
    uint8_t  length;

    {
        int chain_length_current = 256;

        int good_match = hash_table_ptr->good_match;
        int nice_match = hash_table_ptr->nice_match;
        int lazy_match = hash_table_ptr->lazy_match;
        int chain_length;

        if ((dst_len > 1) && (indx_src < src_len)) {
            #if PLATFORM >= K0
            __m256i     aUnit, bUnit;
            #else
            uint64_t    aUnit, bUnit;
            #endif

            int         flag_cmp;
            int         prev_bound = 0;

            uint16_t prev_dist = 0;
            uint8_t prev_ch = 0;
            dst_len -= 1;

            for (; (indx_src < src_len) && (indx_dst < dst_len); indx_src++) {
                p_str = p_src + indx_src;
                win_bound = indx_src - (int)win_size;
                hash_key = hash_crc(p_str) & hash_mask;
                index = tmp = p_hash_table[hash_key];
                p_hash_story[indx_src & win_mask] = index;
                p_hash_table[hash_key] = indx_src;
                bound = prev_bound;
                chain_length = chain_length_current;

                if (prev_bound < lazy_match) {
                    if (prev_bound >= good_match) {
                        chain_length >>= 2;
                    }
                    if (bound < (_CMP_MATCH_LENGTH - 1)) {
                        bound = (_CMP_MATCH_LENGTH - 1);
                    }

                    #if PLATFORM >= K0
                    aUnit = _mm256_loadu_si256((__m256i const*)p_str);
                    #else
                    aUnit = *(const uint64_t*) p_str;
                    #endif

                    for (int k = 0; k < chain_length; k++) {
                        if (!(win_bound < tmp)) {
                            break;
                        }
                        p_src_tmp = p_src + tmp;
                        candidat = tmp;
                        tmp = p_hash_story[tmp & win_mask];
                        if (*(uint32_t*)(p_str + bound - 3) != *(uint32_t*)(p_src_tmp + bound - 3)) {
                            continue;
                        }

                        #if PLATFORM >= K0

                        if (bound < 32) {
                            bUnit = _mm256_loadu_si256((__m256i const*)p_src_tmp);
                            bUnit = _mm256_cmpeq_epi8(bUnit, aUnit);
                            flag_cmp = (uint32_t)_mm256_movemask_epi8(bUnit);
                            flag_cmp = ~flag_cmp;
                            flag_cmp = _tzcnt_u32(flag_cmp);
                        #else

                        if (bound < 8) {
                            bUnit = *(const uint64_t*) p_src_tmp;
                            bUnit ^= aUnit;	// 1-bits = difference
                            flag_cmp = tzbytecnt(bUnit);   // count low-order 0-bits

                        #endif

                            if (flag_cmp < bound) {
                                continue;
                            }
                            if (flag_cmp >= good_match) {
                                if (chain_length == chain_length_current) {
                                    chain_length >>= 2;
                                }
                            }
                            bound = flag_cmp;
                            index = candidat;

                            #if PLATFORM >= K0

                            if (flag_cmp != 32) {
                                continue;
                            }
                            for (; bound < 256; bound += 32) {
                                bUnit = _mm256_cmpeq_epi8(_mm256_loadu_si256((__m256i const*)(p_str + bound)), _mm256_loadu_si256((__m256i const*)(p_src_tmp + bound)));
                                flag_cmp = (uint32_t)_mm256_movemask_epi8(bUnit);
                                if ((uint32_t)flag_cmp != 0xffffffff) {
                                    break;
                                }
                            }
                            if (bound != 256) {
                                flag_cmp = ~flag_cmp;
                                flag_cmp = _tzcnt_u32(flag_cmp);
                                bound += flag_cmp;

                            #else

                            if (flag_cmp != 8) {
                                continue;
                            }
                            for (; bound < 256; bound += 8) {
                                bUnit = (*(const uint64_t*)(p_str + bound)) ^ (*(const uint64_t*)(p_src_tmp + bound));
                                if (bUnit != 0)
                                    break;
                            }
                            if (bound != 256) {
                                flag_cmp = tzbytecnt(bUnit);   // count low-order 0-bits
                                bound += flag_cmp;

                            #endif

                            } else {
                                if (p_str[256] == p_src_tmp[256]) {
                                    bound = 257;
                                    if (p_str[257] == p_src_tmp[257]) {
                                        bound = 258;
                                    }
                                }
                            }
                            if (bound >= nice_match) {
                                break;
                            }
                        } else {
                            int l;

                            #if PLATFORM >= K0

                            for (l = 0; l < 256; l += 32) {
                                bUnit = _mm256_cmpeq_epi8(_mm256_loadu_si256((__m256i const*)(p_str + l)), _mm256_loadu_si256((__m256i const*)(p_src_tmp + l)));
                                flag_cmp = (uint32_t)_mm256_movemask_epi8(bUnit);
                                if ((uint32_t)flag_cmp != 0xffffffff) {
                                    break;
                                }
                            }
                            if (l != 256) {
                                flag_cmp = ~flag_cmp;
                                flag_cmp = _tzcnt_u32(flag_cmp);
                                l += flag_cmp;

                            #else

                            for (l = 0; l < 256; l += 8) {
                                bUnit = (*(const uint64_t*)(p_str + l)) ^ (*(const uint64_t*)(p_src_tmp + l));
                                if (bUnit != 0)
                                    break;
                            }
                            if (l != 256) {
                                flag_cmp = tzbytecnt(bUnit);   // count low-order 0-bits
                                l += flag_cmp;

                            #endif

                            } else {
                                if (p_str[256] == p_src_tmp[256]) {
                                    l = 257;
                                    if (p_str[257] == p_src_tmp[257]) {
                                        l = 258;
                                    }
                                }
                            }
                            if (l > bound) {
                                bound = l;
                                index = candidat;
                            }
                            if (bound >= nice_match) {
                                break;
                            }
                        }
                    }
                }

                if (prev_bound > 1) {
                    if ((prev_bound >= bound) && (prev_bound > (_CMP_MATCH_LENGTH - 1))) {
                        uint32_t distance = 0;
                        uint32_t extra_bits = 0;
                        int      k;

                        if (prev_dist < prev_bound) {
                            int m = prev_bound - prev_dist - 3;
                            if (m < 1) {
                                m = 1;
                            }
                            for (k = indx_src + m, indx_src += prev_bound - 2; k <= indx_src; k++) {
                                hash_key = hash_crc(p_src + k) & hash_mask;
                                p_hash_story[k & win_mask] = p_hash_table[hash_key];
                                p_hash_table[hash_key] = k;
                            }
                        } else {
                            for (k = indx_src + 1, indx_src += prev_bound - 2; k <= indx_src; k++) {
                                hash_key = hash_crc(p_src + k) & hash_mask;
                                p_hash_story[k & win_mask] = p_hash_table[hash_key];
                                p_hash_table[hash_key] = k;
                            }
                        }
                        get_distance_icf_code(prev_dist, &distance, &extra_bits);
                        own_write_deflate_icf(icf_stream_ptr->next_ptr, prev_bound + LEN_OFFSET, distance, extra_bits);
                        icf_stream_ptr->next_ptr++;
                        indx_dst++;
                        bound = 0;
                        histogram_ptr->ll_hist[prev_bound + LEN_OFFSET]++;
                        histogram_ptr->d_hist[own_get_offset_table_index(prev_dist)]++;
                    } else {
                        own_write_deflate_icf_lit(icf_stream_ptr->next_ptr, prev_ch);
                        icf_stream_ptr->next_ptr++;
                        indx_dst++;
                        histogram_ptr->ll_hist[prev_ch]++;
                    }
                }
                prev_dist = (uint16_t)(indx_src - index);
                prev_bound = bound;
                prev_ch = p_src[indx_src];
            }

            {
                int bound_lim;

                src_len += MAX_MATCH;
                for (; ((src_len - indx_src) > 0) && (indx_dst < dst_len); indx_src++) {
                    p_str = p_src + indx_src;
                    win_bound = indx_src - (int)win_size;
                    hash_key = hash_crc(p_str) & hash_mask;
                    index = tmp = p_hash_table[hash_key];
                    p_hash_story[indx_src & win_mask] = index;
                    p_hash_table[hash_key] = indx_src;
                    bound_lim = QPL_MIN((src_len - indx_src), MAX_MATCH);
                    bound = prev_bound;
                    chain_length = chain_length_current;

                    if (prev_bound < lazy_match) {
                        if (prev_bound >= good_match) {
                            chain_length >>= 2;
                        }
                        if (bound < (_CMP_MATCH_LENGTH - 1)) {
                            bound = (_CMP_MATCH_LENGTH - 1);
                        }
                        for (int k = 0; k < chain_length; k++) {
                            if (!(win_bound < tmp)) {
                                break;
                            }
                            p_src_tmp = p_src + tmp;
                            candidat = tmp;
                            tmp = p_hash_story[tmp & win_mask];
                            if (*(uint32_t*)(p_str + bound - 3) != *(uint32_t*)(p_src_tmp + bound - 3)) {
                                continue;
                            }
                            {
                                int l;
                                for (l = 0; l < bound_lim; l++) {
                                    if (p_str[l] != p_src_tmp[l]) {
                                        break;
                                    }
                                }
                                if (bound < l) {
                                    bound = l;
                                    index = candidat;
                                    if (bound >= nice_match) {
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    if (prev_bound > 1) {
                        if ((prev_bound >= bound) && (prev_bound > (_CMP_MATCH_LENGTH - 1))) {
                            uint32_t distance = 0;
                            uint32_t extra_bits = 0;
                            int k;

                            if (prev_dist < prev_bound) {
                                int m = prev_bound - prev_dist - 3;
                                if (m < 1) {
                                    m = 1;
                                }
                                for (k = indx_src + m, indx_src += prev_bound - 2; k <= indx_src; k++) {
                                    hash_key = hash_crc(p_src + k) & hash_mask;
                                    p_hash_story[k & win_mask] = p_hash_table[hash_key];
                                    p_hash_table[hash_key] = k;
                                }
                            } else {
                                for (k = indx_src + 1, indx_src += prev_bound - 2; k <= indx_src; k++) {
                                    hash_key = hash_crc(p_src + k) & hash_mask;
                                    p_hash_story[k & win_mask] = p_hash_table[hash_key];
                                    p_hash_table[hash_key] = k;
                                }
                            }

                            get_distance_icf_code(prev_dist, &distance, &extra_bits);
                            own_write_deflate_icf(icf_stream_ptr->next_ptr, prev_bound + LEN_OFFSET, distance, extra_bits);
                            icf_stream_ptr->next_ptr++;
                            indx_dst++;
                            bound = 0;
                            histogram_ptr->ll_hist[prev_bound + LEN_OFFSET]++;
                            histogram_ptr->d_hist[own_get_offset_table_index(prev_dist)]++;
                        } else {
                            own_write_deflate_icf_lit(icf_stream_ptr->next_ptr, prev_ch);
                            icf_stream_ptr->next_ptr++;
                            indx_dst++;
                            histogram_ptr->ll_hist[prev_ch]++;
                        }
                    }
                    prev_dist = (uint16_t)(indx_src - index);
                    prev_bound = bound;
                    prev_ch = p_src[indx_src];
                }
            }

            dst_len += 1;
            if (prev_bound > 1) {
                if (prev_bound > (_CMP_MATCH_LENGTH - 1)) {
                    uint32_t distance = 0;
                    uint32_t extra_bits = 0;
                    int k;

                    for (k = indx_src + 1, indx_src += prev_bound - 1; k < indx_src; k++) {
                        hash_key = hash_crc(p_src + k) & hash_mask;
                        p_hash_story[k & win_mask] = p_hash_table[hash_key];
                        p_hash_table[hash_key] = k;
                    }
                    get_distance_icf_code(prev_dist, &distance, &extra_bits);
                    own_write_deflate_icf(icf_stream_ptr->next_ptr, prev_bound + LEN_OFFSET, distance, extra_bits);
                    icf_stream_ptr->next_ptr++;
                    indx_dst++;
                    bound = 0;
                    histogram_ptr->ll_hist[prev_bound + LEN_OFFSET]++;
                    histogram_ptr->d_hist[own_get_offset_table_index(prev_dist)]++;
                } else  {
                    own_write_deflate_icf_lit(icf_stream_ptr->next_ptr, prev_ch);
                    icf_stream_ptr->next_ptr++;
                    indx_dst++;
                    histogram_ptr->ll_hist[prev_ch]++;
                }
            }

            {
                for (src_len += (MIN_MATCH4 - 1); ((src_len - indx_src) > 0) && (indx_dst < dst_len); indx_src++, indx_dst++) {
                    prev_ch = p_src[indx_src];
                    own_write_deflate_icf_lit(icf_stream_ptr->next_ptr, prev_ch);
                    icf_stream_ptr->next_ptr++;
                    histogram_ptr->ll_hist[prev_ch]++;
                    if ((src_len - (indx_src + (_CMP_MATCH_LENGTH - 1))) > 0) {
                        hash_key = hash_crc(p_src + indx_src) & hash_mask;
                        p_hash_story[indx_src & win_mask] = p_hash_table[hash_key];
                        p_hash_table[hash_key] = indx_src;
                    }
                }
            }
        } else {
            int mx_match = MAX_MATCH;
            for (; ((src_len - indx_src) > 0) && (indx_dst < dst_len); indx_src++, indx_dst++) {
                p_str = p_src + indx_src;
                win_bound = indx_src - (int)win_size;
                hash_key = hash_crc(p_str) & hash_mask;
                index = tmp = p_hash_table[hash_key];
                p_hash_story[indx_src & win_mask] = index;
                p_hash_table[hash_key] = indx_src;
                bound = (_CMP_MATCH_LENGTH - 1);
                chain_length = chain_length_current;
                for (int k = 0; k < chain_length; k++) {
                    if (!(win_bound < tmp)) {
                        break;
                    }
                    p_src_tmp = p_src + tmp;
                    candidat = tmp;
                    tmp = p_hash_story[tmp & win_mask];
                    if (*(uint32_t*)(p_str + bound - 3) == *(uint32_t*)(p_src_tmp + bound - 3)) {
                        int l;
                        for (l = 0; l < mx_match; l++) {
                            if ((p_str[l] - p_src_tmp[l])) {
                                break;
                            }
                        }
                        if (bound < l) {
                            bound = l;
                            index = candidat;
                            if (bound >= mx_match) {
                                break;
                            }
                        }
                    }
                }
                dist = 0;
                length = p_str[0];
                if (bound >= _CMP_MATCH_LENGTH)  {
                    uint32_t distance = 0;
                    uint32_t extra_bits = 0;
                    int k;

                    dist = (uint16_t)(indx_src - index);
                    get_distance_icf_code(dist, &distance, &extra_bits);
                    own_write_deflate_icf(icf_stream_ptr->next_ptr, bound + LEN_OFFSET, distance, extra_bits);
                    icf_stream_ptr->next_ptr++;
                    indx_dst++;
                    histogram_ptr->ll_hist[bound + LEN_OFFSET]++;
                    histogram_ptr->d_hist[own_get_offset_table_index(dist)]++;
                    if (dist < bound) {
                        int m = bound - dist - 3;
                        if (m < 1) {
                            m = 1;
                        }
                        for (k = indx_src + m, indx_src += bound - 1; k <= indx_src; k++) {
                            hash_key = hash_crc(p_src + k) & hash_mask;
                            p_hash_story[k & win_mask] = p_hash_table[hash_key];
                            p_hash_table[hash_key] = k;
                        }
                    } else {
                        for (k = indx_src + 1, indx_src += bound - 1; k <= indx_src; k++) {
                            hash_key = hash_crc(p_src + k) & hash_mask;
                            p_hash_story[k & win_mask] = p_hash_table[hash_key];
                            p_hash_table[hash_key] = k;
                        }
                    }
                } else {
                    own_write_deflate_icf_lit(icf_stream_ptr->next_ptr, length);
                    icf_stream_ptr->next_ptr++;
                    indx_dst++;
                    histogram_ptr->ll_hist[length]++;
                }
            }

            {
                int bound_lim;

                src_len += MAX_MATCH;
                for (; ((src_len - indx_src) > 0) && (indx_dst < dst_len); indx_src++) {
                    p_str = p_src + indx_src;
                    win_bound = indx_src - (int)win_size;
                    hash_key = hash_crc(p_str) & hash_mask;
                    index = tmp = p_hash_table[hash_key];
                    p_hash_story[indx_src & win_mask] = index;
                    p_hash_table[hash_key] = indx_src;
                    bound_lim = QPL_MIN((src_len - indx_src), MAX_MATCH);
                    bound = 3;
                    chain_length = chain_length_current;
                    for (int k = 0; k < chain_length; k++) {
                        if (!(win_bound < tmp)) {
                            break;
                        }
                        p_src_tmp = p_src + tmp;
                        candidat = tmp;
                        tmp = p_hash_story[tmp & win_mask];
                        if (*(uint32_t*)(p_str + bound - 3) == *(uint32_t*)(p_src_tmp + bound - 3)) {
                            int l;
                            for (l = 0; l < bound_lim; l++) {
                                if ((p_str[l] - p_src_tmp[l])) {
                                    break;
                                }
                            }
                            if (bound < l)
                            {
                                bound = l;
                                index = candidat;
                                if (bound >= bound_lim) {
                                    break;
                                }
                            }
                        }
                    }
                    dist = 0;
                    length = p_str[0];
                    if (bound >= _CMP_MATCH_LENGTH) {
                        uint32_t distance = 0;
                        uint32_t extra_bits = 0;
                        int k;

                        dist = (uint16_t)(indx_src - index);
                        length = (uint8_t)(bound - 3);
                        get_distance_icf_code(dist, &distance, &extra_bits);
                        own_write_deflate_icf(icf_stream_ptr->next_ptr, bound + LEN_OFFSET, distance, extra_bits);
                        icf_stream_ptr->next_ptr++;
                        indx_dst++;
                        histogram_ptr->ll_hist[bound + LEN_OFFSET]++;
                        histogram_ptr->d_hist[own_get_offset_table_index(dist)]++;
                        if (dist < bound) {
                            int m = bound - dist - 3;
                            if (m < 1) {
                                m = 1;
                            }
                            for (k = indx_src + m, indx_src += bound - 1; k <= indx_src; k++) {
                                hash_key = hash_crc(p_src + k) & hash_mask;
                                p_hash_story[k & win_mask] = p_hash_table[hash_key];
                                p_hash_table[hash_key] = k;
                            }
                        } else {
                            for (k = indx_src + 1, indx_src += bound - 1; k <= indx_src; k++) {
                                hash_key = hash_crc(p_src + k) & hash_mask;
                                p_hash_story[k & win_mask] = p_hash_table[hash_key];
                                p_hash_table[hash_key] = k;
                            }
                        }
                    } else {
                        own_write_deflate_icf_lit(icf_stream_ptr->next_ptr, length);
                        icf_stream_ptr->next_ptr++;
                        indx_dst++;
                        histogram_ptr->ll_hist[length]++;
                    }
                }
                for (src_len += (MIN_MATCH4 - 1); ((src_len - indx_src) > 0) && (indx_dst < dst_len); indx_src++) {
                    own_write_deflate_icf_lit(icf_stream_ptr->next_ptr, p_src[indx_src]);
                    icf_stream_ptr->next_ptr++;
                    indx_dst++;
                    histogram_ptr->ll_hist[p_src[indx_src]]++;
                    if ((src_len - (indx_src + (_CMP_MATCH_LENGTH - 1))) > 0) {
                        hash_key = hash_crc(p_src + indx_src) & hash_mask;
                        p_hash_story[indx_src & win_mask] = p_hash_table[hash_key];
                        p_hash_table[hash_key] = indx_src;
                    }
                }
            }
        }
    }
    return (uint32_t)(indx_src - src_start);
}
