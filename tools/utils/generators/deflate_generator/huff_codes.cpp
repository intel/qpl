/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "huff_codes.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory.h>

#include "bitbuffer.h"
#include "gen.h"
#include "heap_alt.h"

// Heap value:
// 63:32 Frequency  / code len
// 31    0
// 30:24 Tree Depth
// 15:0  Left child / histogram idx
namespace gz_generator {
constexpr uint32_t BYTE            = 8U;
constexpr uint32_t WORD            = 16U;
constexpr uint32_t DWORD           = 32U;
constexpr uint32_t QWORD           = 64U;
constexpr uint32_t MAX_HEAP        = 286U;
constexpr uint32_t DEPTH_MASK      = 0x7FU;
constexpr uint32_t FREQ_SHIFT      = 32U;
constexpr uint32_t DEPTH_SHIFT     = 24U;
constexpr uint32_t MAX_CODE_LEN    = 15U;
constexpr uint32_t MAX_BL_CODE_LEN = 7U;

constexpr uint64_t FREQ_MASK_HI = 0xFFFFFFFF80000000U;

constexpr uint32_t HEAP_SIZE  = 3U * MAX_HEAP + 1U;
constexpr uint32_t NODE_START = HEAP_SIZE - 1U;

constexpr uint32_t DEPTH_MASK_HI = DEPTH_MASK << DEPTH_SHIFT;
constexpr uint32_t DEPTH_1       = 1U << DEPTH_SHIFT;

static bool     testmode_4  = false;
static uint32_t testmode_16 = 0U;
static bool     testmode_32 = false;

// clang-format off

    static const uint8_t bit_rev_8[0x100] = {
            0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
            0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
            0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
            0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
            0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
            0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
            0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
            0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
            0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
            0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
            0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
            0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
            0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
            0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
            0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
            0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF,
    };

// clang-format on

// bit reverse low order LENGTH bits in code, and return result in low order bits
uint32_t bit_reverse(uint32_t code, uint32_t length) {
    code = (bit_rev_8[code & 0x00FFU] << BYTE) | (bit_rev_8[code >> BYTE]);
    return (code >> (WORD - length));
}
// heap array has elements from 1...n
// Make a heap starting with i
//
static void heapify64(uint64_t* heap, uint32_t n, uint32_t i) {
    uint32_t child = 0U;
    uint64_t tmp   = 0U;

    while ((child = 2U * i) <= n) {
        if (heap[child] > heap[child + 1U]) { child++; }
        if (heap[i] <= heap[child]) { break; }
        // swap i and child
        tmp         = heap[i];
        heap[i]     = heap[child];
        heap[child] = tmp;
        i           = child;
    }
}

// Upon return, codes[] contains the code lengths,
// and bl_count is the count of the lengths
void create_huff_tree(uint32_t const* histogram, uint32_t hist_size, uint32_t* bl_count, uint32_t* codes,
                      uint32_t max_code_len) {
    uint64_t  heap[HEAP_SIZE];
    uint64_t* heap_freq = (uint64_t*)(4U + (uint8_t*)&heap[0U]);
    uint32_t  heap_size = 0U;
    uint32_t  i = 0U, k = 0U, child = 0U, d1 = 0U, d2 = 0U, code_len = 0U, h_new = 0U;
    uint64_t  h1 = 0U, h2 = 0U;
    uint32_t  node_ptr = NODE_START;

    for (; i < hist_size; i++) {
        if (histogram[i] != 0U) heap[++heap_size] = (((uint64_t)histogram[i]) << FREQ_SHIFT) | i;
    }

    // handle heaps of 0 or 1 elements
    if (heap_size < 2U) {
        for (i = 1U; i <= max_code_len; i++) {
            bl_count[i] = 0U;
        }
        memset(codes, 0U, hist_size * sizeof(*codes));
        if (heap_size == 1U) {
            codes[(uint32_t)heap[1U]] = 1U;
            bl_count[1U]              = 1U;
        }
        return;
    }

    build_heap64(heap, heap_size);
    //    build_heap_asm(heap, heap_size);

    //   node_ptr = proc_heap(heap, heap_size, node_ptr);
    do { //NOLINT(cppcoreguidelines-avoid-do-while)
        h1                = heap[1U];
        heap[1U]          = heap[heap_size];
        heap[heap_size--] = 0xFFFFFFFFFFFFFFFFULL;
        heapify64(heap, heap_size, 1U);

        h2                  = heap[1U];
        heap[node_ptr]      = (uint32_t)h1;
        heap[node_ptr - 1U] = (uint32_t)h2;

        h_new = (h1 + h2) & FREQ_MASK_HI;
        d1    = (uint32_t)h1;
        d2    = (uint32_t)h2;
        if (d1 > d2) { h2 = h1; }
        h_new |= node_ptr | ((h2 + DEPTH_1) & DEPTH_MASK_HI);

        node_ptr -= 2U;
        heap[1U] = h_new;
        heapify64(heap, heap_size, 1U);
    } while (heap_size > 1U);
    heap[node_ptr] = (uint32_t)heap[1U];

    //@    for (i = node_ptr; i <= NODE_START; i++) printf("0x%x\t%016x\n",i,heap[i]);

    // compute code lengths and code length counts
    uint32_t j = node_ptr;
    for (i = node_ptr; i <= NODE_START; i++) {
        child = (uint16_t)heap[i];
        if (child > MAX_HEAP) {
            d1                                   = 1U + *(uint32_t*)&(heap_freq[i]);
            *(uint32_t*)&(heap_freq[child])      = d1;
            *(uint32_t*)&(heap_freq[child - 1U]) = d1;
        } else {
            heap[j++] = heap[i];
            d1        = *(uint32_t*)&(heap_freq[i]);
            while (code_len < d1) {
                code_len++;
                heap[code_len] = 0U;
            }
            heap[d1]++;
        }
    }

    ///    for (i=1; i <= code_len; i++)
    ///        printf("%d\t%d\n", i, heap[i]);

    if (code_len > max_code_len) {
        while (code_len > max_code_len) {
            assert(heap[code_len] > 1U);
            for (i = max_code_len - 1U; i != 0U; i--) {
                if (heap[i] != 0U) break;
            }
            assert(i != 0U);
            heap[i]--;
            heap[i + 1U] += 2U;
            heap[code_len - 1U]++;
            heap[code_len] -= 2U;
            if (heap[code_len] == 0U) code_len--;
        }

        ///        for (i=1; i <= code_len; i++)
        ///            printf("%d\t%d\n", i, heap[i]);
        for (i = 1U; i <= code_len; i++) {
            bl_count[i] = (uint32_t)(heap[i]);
        }
        for (; i <= max_code_len; i++) {
            bl_count[i] = 0U;
        }

        for (k = 1U; heap[k] == 0U; k++) {}
        for (i = node_ptr; i < j; i++) {
            *(uint32_t*)&(heap_freq[i]) = k;
            heap[k]--;
            for (; heap[k] == 0U; k++) {}
        }
    } else {
        for (i = 1U; i <= code_len; i++) {
            bl_count[i] = (uint32_t)(heap[i]);
        }
        for (; i <= max_code_len; i++) {
            bl_count[i] = 0U;
        }
    }

    ///    for (i = node_ptr; i<j; i++)
    ///        printf("%016I64x\n", heap[i]);

    memset(codes, 0U, hist_size * sizeof(*codes));
    for (i = node_ptr; i < j; i++) {
        h1                  = heap[i];
        codes[(uint32_t)h1] = (uint32_t)(h1 >> 32U);
    }

    //    for (i=0; i<hist_size; i++)
    //        printf("%d\t%d\n", i, codes[i]);
}

// Upon return, codes[] contains the code lengths,
// and bl_count is the count of the lengths
void copy_lens(uint32_t const* code_lens, uint32_t num_lens, uint32_t* bl_count, uint32_t* codes, uint32_t num_codes) {
    uint32_t i = 0U, code_len = 0U;

    for (; i <= 15U; i++) {
        bl_count[i] = 0U;
    }
    for (i = 0U; i < num_lens; i++) {
        codes[i] = code_len = code_lens[i];
        bl_count[code_len]++;
    }
    for (; i < num_codes; i++) {
        codes[i] = 0U;
    }
}

static void copy_cl_lens(uint32_t* cl_lens, uint32_t num_cl_lens, uint32_t cl_counts[19U], uint32_t cl_codes[19U],
                         uint32_t* bl_count) {
    uint32_t j = 0U, code_len = 0U;

    for (uint32_t i = 0U; i < 19U; i++) {
        cl_codes[i] = 0U;
    }
    for (uint32_t i = 0U; i <= MAX_BL_CODE_LEN; i++) {
        bl_count[i] = 0U;
    }
    for (uint32_t i = 0U; i < 19U; i++) {
        if (cl_counts[i] == 0U) continue;
        if (j == num_cl_lens) {
            fprintf(stderr, "Not enough cl codes\n");
            throw std::exception();
        }
        cl_codes[i] = code_len = cl_lens[j++];
        bl_count[code_len]++;
    }
    if (j < num_cl_lens) {
        for (uint32_t i = 0U; i < 19U; i++) {
            if (cl_counts[i] != 0U) continue;
            cl_codes[i] = code_len = cl_lens[j++];
            bl_count[code_len]++;
            if (j == num_cl_lens) break;
        }
    }
}

static void copy_cl_lens_alt(uint32_t* cl_lens, uint32_t cl_counts[19], uint32_t cl_codes[19], uint32_t* bl_count) {
    uint32_t code_len = 0U;

    for (uint32_t i = 0U; i < 19U; i++) {
        cl_codes[i] = 0U;
    }
    for (uint32_t i = 0U; i <= MAX_BL_CODE_LEN; i++) {
        bl_count[i] = 0U;
    }

    for (uint32_t i = 0U; i < 19U; i++) {
        cl_codes[i] = code_len = cl_lens[i];
        bl_count[code_len]++;
    }

    for (uint32_t i = 0U; i < 19U; i++) {
        if (cl_counts[i] == 0U) continue;
        if (cl_lens[i] == 0U) {
            fprintf(stderr, "Missing CL ALT code: %d\n", i);
            throw std::exception();
        }
    }
}

// on input, codes contain the code lengths
// on output, code contains:
// 31:24 code length
// 15:0  reverse code value in low order bits
//// 18:15 code length
//// 14:0  unreversed code value in low order bits
// returns max code value
uint32_t compute_codes(uint32_t* codes, uint32_t num_codes, uint32_t* bl_count, uint32_t max_code_len) {
    uint32_t next_code[MAX_CODE_LEN + 1U];
    uint32_t max_code = 0xFFFFFFFFU;

    uint32_t code = bl_count[0U] = 0U;
    for (uint32_t bits = 1U; bits <= max_code_len; bits++) {
        code            = (code + bl_count[bits - 1U]) << 1;
        next_code[bits] = code;
    }
    for (uint32_t i = 0U; i < num_codes; i++) {
        code = codes[i];
        if (code != 0U) {
            codes[i] = (code << 24) | bit_reverse(next_code[code], code);
            //            codes[i] = (code << 15) | next_code[code];
            next_code[code] += 1U;
            max_code = i;
        }
    }
    return max_code;
}

// on input, codes contain the code lengths
// on output, code contains:
// 31:24 code length
// 15:0  code value in low order bits
// returns max code value
uint32_t compute_codes_cl(uint32_t* codes, uint32_t num_codes, uint32_t* bl_count, uint32_t max_code_len) {
    uint32_t next_code[MAX_CODE_LEN + 1U];
    uint32_t max_code = 0U;

    uint32_t code = bl_count[0U] = 0U;
    for (uint32_t bits = 1U; bits <= max_code_len; bits++) {
        code            = (code + bl_count[bits - 1U]) << 1;
        next_code[bits] = code;
    }
    for (uint32_t i = 0U; i < num_codes; i++) {
        code = codes[i];
        if (code != 0U) {
            codes[i] = (code << 24) | bit_reverse(next_code[code], code);
            next_code[code] += 1U;
            max_code = i;
        }
    }
    return max_code;
}

static uint16_t* write_rl(uint16_t* pout, uint16_t last_len, uint32_t run_len, uint32_t* counts) {
    uint32_t last_len_tmp = last_len;
    if ((testmode_16 != 0U) && (last_len == 0U)) {
        if (testmode_16 & 1U) last_len_tmp = 1U;
        testmode_16 = (testmode_16 >> 1) | (testmode_16 << 31);
    }

    if (last_len_tmp == 0U) {
        while (run_len > 138U) {
            *pout++ = 18U | ((138U - 11U) << 8);
            run_len -= 138U;
            counts[18U]++;
        }
        // 1 <= run_len <= 138
        if (run_len > 10U) {
            *pout++ = 18U | ((run_len - 11U) << 8);
            counts[18U]++;
        } else if (run_len > 2U) {
            *pout++ = 17U | ((run_len - 3U) << 8);
            counts[17U]++;
        } else if (run_len == 1U) {
            *pout++ = 0U;
            counts[0U]++;
        } else {
            assert(run_len == 2U);
            pout[0U] = pout[1U] = 0U;
            pout += 2U;
            counts[0U] += 2U;
        }
    } else {
        // last_len != 0
        if (testmode_4)
            testmode_4 = false;
        else
            *pout++ = last_len;
        counts[last_len]++;
        run_len--;
        if (run_len != 0U) {
            while (run_len > 6U) {
                *pout++ = 16U | ((6U - 3U) << 8);
                run_len -= 6U;
                counts[16U]++;
            }
            // 1 <= run_len <= 6
            switch (run_len) {
                case 1:
                    *pout++ = last_len;
                    counts[last_len]++;
                    break;
                case 2:
                    pout[0U] = pout[1U] = last_len;
                    pout += 2U;
                    counts[last_len] += 2U;
                    break;
                default: // 3...6
                    *pout++ = 16U | ((run_len - 3U) << 8);
                    counts[16U]++;
            }
        }
    }
    return pout;
}

// convert codes into run-length symbols, write symbols into OUT
// generate histogram into COUNTS (assumed to be initialized to 0)
// Format of OUT:
// 4:0  code (0...18)
// 15:8 Extra bits (0...127)
// returns number of symbols in out
uint32_t rl_encode(uint32_t* codes, uint32_t num_codes, uint32_t* counts, uint16_t* out) {
    uint32_t  len  = 0U;
    uint16_t* pout = out;

    if (testmode_32) {
        testmode_32 = false;
        for (uint32_t i = 0U; i < num_codes; i++) {
            len     = (uint16_t)(codes[i] >> 24);
            *pout++ = len;
            counts[len]++;
        }
        return (uint32_t)(pout - out);
    }

    uint16_t last_len = (uint16_t)(codes[0U] >> 24);
    uint32_t run_len  = 1U;
    for (uint32_t i = 1U; i < num_codes; i++) {
        len = (uint16_t)(codes[i] >> 24);
        if (len == last_len) {
            run_len++;
            continue;
        }
        pout     = write_rl(pout, last_len, run_len, counts);
        last_len = len;
        run_len  = 1U;
    }
    pout = write_rl(pout, last_len, run_len, counts);

    return (uint32_t)(pout - out);
}

uint32_t rl_encode_lens(uint32_t* lens, uint32_t num_lens, uint32_t* counts, uint16_t* out) {
    uint16_t  len  = 0U;
    uint16_t* pout = out;

    if (testmode_32) {
        testmode_32 = false;
        for (uint32_t i = 0U; i < num_lens; i++) {
            len     = (uint16_t)(lens[i]);
            *pout++ = len;
            counts[len]++;
        }
        return (uint32_t)(pout - out);
    }

    uint16_t last_len = (uint16_t)(lens[0U]);
    uint32_t run_len  = 1U;
    for (uint32_t i = 1U; i < num_lens; i++) {
        len = (uint16_t)(lens[i]);
        if (len == last_len) {
            run_len++;
            continue;
        }
        pout     = write_rl(pout, last_len, run_len, counts);
        last_len = len;
        run_len  = 1U;
    }
    pout = write_rl(pout, last_len, run_len, counts);

    return (uint32_t)(pout - out);
}

static const uint32_t cl_perm[19U] = {16U, 17U, 18U, 0U, 8U,  7U, 9U,  6U, 10U, 5U,
                                      11U, 4U,  12U, 3U, 13U, 2U, 14U, 1U, 15U};

uint32_t create_header(BitBuffer* bb, uint32_t* ll_codes, uint32_t num_ll_codes, bool ll_is_lens, uint32_t* d_codes,
                       uint32_t num_d_codes, bool d_is_lens, uint32_t* cl_lens, uint32_t num_cl_lens, bool cl_alt,
                       uint32_t* cl_enc_lens, uint32_t num_cl_enc_lens, uint32_t end_of_block, gen_c* gen) {
    uint32_t       cl_counts[19U];
    uint32_t       num_cl_tokens = 0U, max_cl_code = 0U, code = 0U, bit_count = 0U, limit = 0U;
    uint16_t       cl_tokens[286U + 32U], token = 0U, len = 0U;
    uint32_t       bl_count[MAX_CODE_LEN + 1U];
    uint32_t       cl_codes[19U];
    uint64_t       data           = 0U;
    int            ii             = 0;
    const uint32_t extra_bits[3U] = {2U, 3U, 7U};

    assert(num_ll_codes >= 257U); // must be EOB code
    //    assert(num_d_codes != 0);
    if (num_d_codes == 0U) num_d_codes = 1U;

    if (gen->m_testmode & 16U) { testmode_16 = gen->m_testparam; }

    memset(cl_counts, 0U, sizeof(cl_counts));
    if ((gen->m_testmode & 0x0001U) && (ll_is_lens) && (d_is_lens)) {
        // combine LL and D when making CL tokens
        uint32_t lld_lens[286U + 32U];
        for (uint32_t i = 0U; i < num_ll_codes; i++) {
            lld_lens[i] = ll_codes[i];
        }
        for (uint32_t i = 0U; i < num_d_codes; i++) {
            lld_lens[i + num_ll_codes] = d_codes[i];
        }
        num_cl_tokens = rl_encode_lens(lld_lens, num_ll_codes + num_d_codes, cl_counts, cl_tokens);
    } else {
        if ((gen->m_testmode & 4U) && (gen->m_testparam == 0U)) testmode_4 = true;
        if ((gen->m_testmode & 32U) && (gen->m_testparam & 1U)) testmode_32 = true;
        if (ll_is_lens)
            num_cl_tokens = rl_encode_lens(ll_codes, num_ll_codes, cl_counts, cl_tokens);
        else
            num_cl_tokens = rl_encode(ll_codes, num_ll_codes, cl_counts, cl_tokens);

        if ((gen->m_testmode & 4U) && (gen->m_testparam != 0U)) testmode_4 = true;
        if ((gen->m_testmode & 32U) && (gen->m_testparam & 2U)) testmode_32 = true;
        if (d_is_lens)
            num_cl_tokens += rl_encode_lens(d_codes, num_d_codes, cl_counts, cl_tokens + num_cl_tokens);
        else
            num_cl_tokens += rl_encode(d_codes, num_d_codes, cl_counts, cl_tokens + num_cl_tokens);
    }
    testmode_32 = false;

    if (num_cl_lens == 0U) {
        create_huff_tree(cl_counts, 19U, bl_count, cl_codes, MAX_BL_CODE_LEN);
    } else if (cl_alt) {
        copy_cl_lens_alt(cl_lens, cl_counts, cl_codes, bl_count);
    } else {
        copy_cl_lens(cl_lens, num_cl_lens, cl_counts, cl_codes, bl_count);
    }
    compute_codes_cl(cl_codes, 19U, bl_count, MAX_BL_CODE_LEN);

#ifdef DBG_PRINT
    for (int i = 0; i < 19; i++)
        fprintf(stderr, "cl[%d]\t%d\n", i, (cl_codes[i] >> 24));
#endif

    for (max_cl_code = 18U; max_cl_code >= 4U; max_cl_code--) {
        if (0U != cl_codes[cl_perm[max_cl_code]]) break;
    }
    // 3 <= max_cl_code <= 18  (4 <= num_cl_code <= 19)
    if (num_cl_enc_lens) max_cl_code = num_cl_enc_lens - 1U;

    if (gen->m_testmode & 2U) num_d_codes -= gen->m_testparam;

    limit = gen->m_extra_len ? 288U : 286U;
    if (num_ll_codes > limit) {
#if defined(DEBUG) || (_DEBUG)
        fprintf(stderr, "num ll codes (%d) being truncated to %d\n", num_ll_codes, limit);
#endif
        num_ll_codes = limit;
    }
    limit = gen->m_extra_len ? 32U : 30U;
    if (num_d_codes > limit) {
#if defined(DEBUG) || (_DEBUG)
        fprintf(stderr, "num d codes (%d) being truncated to %d\n", num_d_codes, limit);
#endif
        num_d_codes = limit;
    }

    bit_count = bb->getBitsWritten();
    // actually write header
    data = (end_of_block ? 5U : 4U) | ((num_ll_codes - 257U) << 3U) | ((num_d_codes - 1U) << (3U + 5U)) |
           ((max_cl_code - 3U) << (3U + 5U + 5U));
    // write the first CL code here, because bitbuf2_write can only safely
    // write up to 56 bits
    if (num_cl_enc_lens)
        data |= static_cast<uint64_t>(cl_enc_lens[cl_perm[0U]]) << (3U + 5U + 5U + 4U);
    else
        data |= (cl_codes[cl_perm[0U]] >> 24) << (3U + 5U + 5U + 4U);
    bb->write(data, 3U + 5U + 5U + 4U + 3U);
    data = 0U;
    if (num_cl_enc_lens)
        for (ii = max_cl_code; ii >= 1; ii--) {
            data = (data << 3) | (cl_enc_lens[cl_perm[ii]]);
        }
    else
        for (ii = max_cl_code; ii >= 1; ii--) {
            data = (data << 3) | (cl_codes[cl_perm[ii]] >> 24);
        }
    bb->write(data, 3U * (max_cl_code));
    for (uint32_t i = 0U; i < num_cl_tokens; i++) {
        token = cl_tokens[i];
        len   = token & 0x1FU;
        code  = cl_codes[len];
#ifdef DBG_PRINT
        fprintf(stderr, "cl_token[%d] = 0x%x\t%08x\n", i, token, code);
#endif
        bb->write(code & 0xFFFF, code >> 24);
        if (len > 15U) bb->write(token >> 8, extra_bits[len - 16U]);
    }
    testmode_16 = 0U;
    bit_count   = bb->getBitsWritten() - bit_count;
    return bit_count;
}

void expand_len_tree(uint32_t* codes) {
    uint32_t i = 0U, len        = 0U;
    uint32_t orig[21U], *p_code = nullptr, code = 0U;

    for (; i < 21U; i++) {
        orig[i] = codes[i + 265U];
    }

    p_code = &codes[265];

    i = 0U;
    for (uint32_t eb = 1U; eb < 6U; eb++) {
        for (uint32_t k = 0U; k < 4U; k++) {
            code = orig[i++];
            len  = code >> 24;
            code += eb << 24;
            for (uint32_t j = 0U; j < (1U << eb); j++) {
                *p_code++ = code | (j << len);
            }
        } // end for k
    }     // end for eb
    // fix up last record
    code       = orig[i];
    p_code[-1] = orig[i];
}

void create_hufftables(BitBuffer* bb, uint32_t ll_codes[286], uint32_t d_codes[32], uint32_t end_of_block,
                       uint32_t* ll_hist, uint32_t* d_hist, gen_c* gen) {
    uint32_t  bl_count[MAX_CODE_LEN + 1U];
    uint32_t  max_ll_code = 0U, max_d_code = 0U;
    uint32_t* cl_lens     = nullptr;
    uint32_t  num_cl_lens = 0U;
    bool      cl_alt      = false;

    const uint32_t num_ll_lens = gen->m_num_ll_lens;
    if (num_ll_lens == 0U) {
        // make sure EOB is present
        if (ll_hist[256U] == 0U) ll_hist[256U] = 1U;

        create_huff_tree(ll_hist, 286U, bl_count, ll_codes, MAX_CODE_LEN);
    } else {
        copy_lens(ll_hist, num_ll_lens, bl_count, ll_codes, 286U);
    }
    max_ll_code = compute_codes(ll_codes, 286U, bl_count, MAX_CODE_LEN);

    if ((num_ll_lens != 0U) && (num_ll_lens - 1U > max_ll_code)) max_ll_code = num_ll_lens - 1U;

    const uint32_t num_d_lens = gen->m_num_d_lens;
    if (num_d_lens == 0U) {
        create_huff_tree(d_hist, 32U, bl_count, d_codes, MAX_CODE_LEN);
    } else {
        copy_lens(d_hist, num_d_lens, bl_count, d_codes, 32U);
    }
    max_d_code = compute_codes(d_codes, 32U, bl_count, MAX_CODE_LEN);

    if ((num_d_lens != 0U) && (num_d_lens - 1U > max_d_code)) max_d_code = num_d_lens - 1U;

#ifdef DBG_PRINT
    for (int i = 0; i < 286; i++)
        if (ll_codes[i] >> 24) fprintf(stderr, "ll[%d]\t%d\n", i, (ll_codes[i] >> 24));
    for (int i = 0; i < 32; i++)
        if (d_codes[i] >> 16) fprintf(stderr, "d[%d]\t%d\n", i, (d_codes[i] >> 16));
#endif
    uint32_t num_ll_codes = max_ll_code + 1U;
    uint32_t num_d_codes  = max_d_code + 1U;
    bool     ll_is_lens = false, d_is_lens = false;
    if (gen->m_num_ll_enc_lens) {
        ll_codes     = gen->m_ll_enc_lens;
        num_ll_codes = gen->m_num_ll_enc_lens;
        ll_is_lens   = true;
    }
    if (gen->m_num_d_enc_lens) {
        d_codes     = gen->m_d_enc_lens;
        num_d_codes = gen->m_num_d_enc_lens;
        d_is_lens   = true;
    }
    if (gen->m_num_cl_lens_alt) {
        cl_lens     = gen->m_cl_lens_alt;
        num_cl_lens = gen->m_num_cl_lens_alt;
        cl_alt      = true;
    } else {
        cl_lens     = gen->m_cl_lens;
        num_cl_lens = gen->m_num_cl_lens;
        cl_alt      = false;
    }
    if (bb)
        create_header(bb, ll_codes, num_ll_codes, ll_is_lens, d_codes, num_d_codes, d_is_lens, cl_lens, num_cl_lens,
                      cl_alt, gen->m_cl_enc_lens, gen->m_num_cl_enc_lens, end_of_block, gen);
}
} // namespace gz_generator
