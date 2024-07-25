/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "histogram.h"

static const int dist_start[31]        = {1,    2,    3,    4,    5,    7,     9,     13,    17,   25,   33,
                                          49,   65,   97,   129,  193,  257,   385,   513,   769,  1025, 1537,
                                          2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577, 32769};
static const int extra_dbits_plus5[30] = {5,  5,  5,  5,  6,  6,  7,  7,  8,  8,  9,  9,  10, 10, 11,
                                          11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18};

void histogram::consolidate() {
    static const int ll_start[30] = {3,  4,  5,  6,  7,  8,  9,  10, 11,  13,  15,  17,  19,  23,  27,
                                     31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 259};

    for (int i = 0; i < 30; i++) {
        m_distcodes[i] = 0;
        for (int j = dist_start[i] - 1; j < dist_start[i + 1] - 1; j++)
            m_distcodes[i] += m_dist[j];
    }
    m_distcodes[30U] = 0U;
    m_distcodes[31U] = 0U;
    for (int i = 0; i < 257; i++)
        m_llcodes[i] = m_lit_len[i];
    for (int i = 257; i < 286; i++) {
        m_llcodes[i] = 0U;
        for (int j = ll_start[i - 257] + 257 - 3; j < ll_start[i - 257 + 1] + 257 - 3; j++)
            m_llcodes[i] += m_lit_len[j];
    }
    normalize();
}

void histogram::normalize() {
    uint64_t sum = 0, sum2 = 0;
    uint64_t x = 0;

    for (int i = 0; i < 30; i++)
        sum += m_distcodes[i];
    if (sum >= (1 << 16)) {
        sum2 = sum / 2;
        for (int i = 0; i < 30; i++) {
            if (m_distcodes[i] == 0U) continue;
            x = m_distcodes[i];
            x <<= 15;
            x = (x + sum2) / sum;
            if (x == 0U) x = 1U;
            m_distcodes[i] = (uint32_t)x;
        }
    }

    sum = 0U;
    for (int i = 0; i < 286; i++)
        sum += m_llcodes[i];
    if (sum >= (1 << 16)) {
        sum2 = sum / 2;
        for (int i = 0; i < 286; i++) {
            if (m_llcodes[i] == 0U) continue;
            x = m_llcodes[i];
            x <<= 15;
            x = (x + sum2) / sum;
            if (x == 0U) x = 1U;
            m_llcodes[i] = (uint32_t)x;
        }
    }
}
