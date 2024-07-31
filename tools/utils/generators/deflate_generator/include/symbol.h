/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include <stdint.h>
#include <stdio.h>

struct symbol {
    unsigned int lit_len : 10U;
    unsigned int dist : 22U;
};

const uint32_t DIST_LOG = (1U << 22U) - 1U;

////////////////////////////////////////////////////////////////////////

class symbol_page {
protected:
    enum { PAGE_SIZE = 1022U };

public:
    symbol_page* m_next;
    uint32_t     m_used;
    symbol       m_symbols[PAGE_SIZE];

    symbol_page() {
        m_next = NULL;
        m_used = 0u;
    }

    bool is_full() { return (m_used == PAGE_SIZE); }

    void push(uint32_t lit_len, uint32_t dist) {
        m_symbols[m_used].lit_len = lit_len;
        m_symbols[m_used].dist    = dist;
        m_used++;
    }
};

////////////////////////////////////////////////////////////////////////

class symbol_list {
public:
    symbol_page* m_head;
    symbol_page* m_tail;
    uint32_t     m_num_lit;
    uint32_t     m_num_sym;

    symbol_list() {
        m_head = m_tail = NULL;
        m_num_lit = m_num_sym = 0U;
    }

    symbol_list(const symbol_list& other) = delete;

    symbol_list& operator=(const symbol_list& list) = delete;

    ~symbol_list() {
        symbol_page *curr = nullptr, *next = nullptr;
        for (curr = m_head; curr; curr = next) {
            next = curr->m_next;
            delete curr;
        }
    }

    void reset() {
        symbol_page *curr = nullptr, *next = nullptr;
        for (curr = m_head; curr; curr = next) {
            next = curr->m_next;
            delete curr;
        }
        m_head = m_tail = NULL;
        m_num_lit = m_num_sym = 0U;
    }

    void push(uint32_t lit_len, uint32_t dist = 0U) {
        if (m_tail == NULL) {
            m_head = m_tail = new symbol_page;
        } else if (m_tail->is_full()) {
            m_tail->m_next = new symbol_page;
            m_tail         = m_tail->m_next;
        }
        m_tail->push(lit_len, dist);
        if (dist == 0)
            m_num_lit++;
        else
            m_num_sym++;
    }
};

#endif // ifndef __SYMBOL_H__
