/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include <stdint.h>

class histogram {
protected:
    enum { NUM_DIST = 32768 };
    enum { NUM_LIT_LEN = 257 + 258 - 2 };

public:
    uint32_t m_dist[NUM_DIST];
    uint32_t m_lit_len[NUM_LIT_LEN];

    uint32_t m_distcodes[32];
    uint32_t m_llcodes[286];

    void reset() {
        for (uint32_t i = 0U; i < NUM_DIST; i++)
            m_dist[i] = 0U;
        for (uint32_t i = 0U; i < NUM_LIT_LEN; i++)
            m_lit_len[i] = 0U;
    }

    histogram() { reset(); }

    void consolidate(); // consolidate m_dist -> m_distcodes, etc.

    void normalize();

    //    uint64_t estimate_out_length();

    // 0 <= lit <= 256
    void add_lit(uint32_t lit) { m_lit_len[lit] += 1U; }

    // 3 <= len <= 258 ; 1 <= dist <= 32768
    void add_len_dist(uint32_t len, uint32_t dist) {
        m_lit_len[len + (257 - 3)] += 1U;
        m_dist[dist - 1] += 1U;
    }
};

#endif // ifndef __HISTOGRAM_H__
