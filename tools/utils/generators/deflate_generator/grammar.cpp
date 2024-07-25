/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "grammar.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "gen.h"

namespace gz_generator {
void grammar_c::syntax_error() {
    fprintf(stderr, "Syntax error at line %d\n", m_tp.line_num());
    throw std::exception();
}

void grammar_c::parse_lines() {
    token_c token;

    while (1U) {
        const token_type_t type = m_tp.get_token(&token);
        switch (type) {
            case TT_EOF: m_gen->fini(); return;
            case TT_EOL: break;
            case TT_BLOCK: parse_block(); break;
            case TT_L: parse_l(); break;
            case TT_M: parse_m(); break;
            case TT_R: parse_r(); break;
            case TT_LL_LENS: parse_lens(LT_LL); break;
            case TT_D_LENS: parse_lens(LT_D); break;
            case TT_CL_LENS: parse_lens(LT_CL); break;
            case TT_BFINAL: parse_bfinal(); break;
            case TT_LOG: parse_log(); break;
            case TT_TESTMODE: parse_testmode(); break;
            case TT_NOEOB: parse_noeob(); break;
            case TT_BOUT: parse_bout(); break;
            case TT_PAD: parse_pad(); break;
            case TT_SET: parse_set(); break;
            default:
                fprintf(stderr, "Unknown line type at line %d : %d\n", m_tp.line_num(), type);
                throw std::exception();
        }
    }
}

void grammar_c::parse_m() {
    token_c      token;
    token_type_t type = m_tp.get_token(&token);

    if ((type == TT_EOL) || (type == TT_EOF)) {
        m_gen->proc_rand();
        return;
    }
    if (type != TT_STAR) syntax_error();

    type = m_tp.get_token(&token);
    if (type != TT_NUM) syntax_error();
    for (uint32_t i = 0U; i < token.m_value; i++) {
        m_gen->proc_rand();
    }

    type = m_tp.get_token(&token);
    if ((type == TT_EOL) || (type == TT_EOF)) return;
    syntax_error();
}

void grammar_c::parse_l() {
    token_c  token;
    uint32_t lit = 0U;

    token_type_t type = m_tp.get_token(&token);
    while (1U) {
        if (type != TT_NUM) syntax_error();
        lit = token.m_value;

        type = m_tp.get_token(&token);
        if ((type == TT_EOL) || (type == TT_EOF)) {
            m_gen->proc_lit(lit);
            return;
        }
        if (type == TT_NUM) {
            m_gen->proc_lit(lit);
            continue;
        }
        if (type != TT_STAR) syntax_error();

        type = m_tp.get_token(&token);
        if (type != TT_NUM) syntax_error();
        for (uint32_t i = 0U; i < token.m_value; i++) {
            m_gen->proc_lit(lit);
        }

        type = m_tp.get_token(&token);
        if ((type == TT_EOL) || (type == TT_EOF)) return;
    } // end while
}

void grammar_c::parse_r() {
    token_c  token;
    uint32_t len = 0U, dist = 0U;

    token_type_t type = m_tp.get_token(&token);
    while (1U) {
        if (type != TT_NUM) syntax_error();
        len = token.m_value;

        type = m_tp.get_token(&token);
        if (type != TT_NUM) syntax_error();
        dist = token.m_value;

        type = m_tp.get_token(&token);
        if ((type == TT_EOL) || (type == TT_EOF)) {
            m_gen->proc_len_dist(len, dist);
            return;
        }
        if (type == TT_NUM) {
            m_gen->proc_len_dist(len, dist);
            continue;
        }

        if (type != TT_STAR) syntax_error();

        type = m_tp.get_token(&token);
        if (type != TT_NUM) syntax_error();
        for (uint32_t i = 0U; i < token.m_value; i++) {
            m_gen->proc_len_dist(len, dist);
        }

        type = m_tp.get_token(&token);
        if ((type == TT_EOL) || (type == TT_EOF)) return;
    } // end while
}

void grammar_c::parse_lens(len_type_t ltype) {
    token_c  token;
    uint32_t lit = 0U;

    token_type_t type = m_tp.get_token(&token);

    if (type == TT_ALT) {
        if (ltype != LT_CL) syntax_error();
        ltype = LT_CL_ALT;
        type  = m_tp.get_token(&token);
    }

    if (type == TT_ENCODED) {
        switch (ltype) {
            case LT_LL: ltype = LT_LLE; break;
            case LT_D: ltype = LT_DE; break;
            case LT_CL: ltype = LT_CLE; break;
            default: syntax_error();
        }
        type = m_tp.get_token(&token);
    }

    while (true) {
        if (type != TT_NUM) syntax_error();
        lit = token.m_value;

        type = m_tp.get_token(&token);
        if ((type == TT_EOL) || (type == TT_EOF)) {
            m_gen->proc_lens(ltype, lit);
            return;
        }
        if (type == TT_NUM) {
            m_gen->proc_lens(ltype, lit);
            continue;
        }
        if (type != TT_STAR) syntax_error();

        type = m_tp.get_token(&token);
        if (type != TT_NUM) syntax_error();
        for (uint32_t i = 0U; i < token.m_value; i++) {
            m_gen->proc_lens(ltype, lit);
        }

        type = m_tp.get_token(&token);
        if ((type == TT_EOL) || (type == TT_EOF)) return;
    } // end while
}

void grammar_c::parse_bfinal() {
    token_c      token;
    token_type_t type = m_tp.get_token(&token);
    if (type != TT_NUM) syntax_error();
    if (token.m_value == 0U)
        m_gen->set_bfinal(false);
    else
        m_gen->set_bfinal(true);

    type = m_tp.get_token(&token);
    if ((type != TT_EOL) && (type != TT_EOF)) syntax_error();
}

void grammar_c::parse_bout() {
    token_c      token;
    token_type_t type = m_tp.get_token(&token);
    if (type != TT_NUM) syntax_error();
    if (token.m_value == 0U)
        m_gen->set_bout(false);
    else
        m_gen->set_bout(true);

    type = m_tp.get_token(&token);
    if ((type != TT_EOL) && (type != TT_EOF)) syntax_error();
}

void grammar_c::parse_pad() {
    token_c      token;
    token_type_t type = m_tp.get_token(&token);
    if (type != TT_NUM) syntax_error();
    m_gen->set_pad(token.m_value);

    type = m_tp.get_token(&token);
    if ((type != TT_EOL) && (type != TT_EOF)) syntax_error();
}

void grammar_c::parse_set() {
    token_c      token;
    token_type_t type = m_tp.get_token(&token);
    if (type != TT_EXTRALEN) syntax_error();

    m_gen->set_extra_len();

    type = m_tp.get_token(&token);
    if ((type != TT_EOL) && (type != TT_EOF)) syntax_error();
}

void grammar_c::parse_log() {
    token_c token;

    token_type_t type = m_tp.get_token(&token);
    if (type != TT_NUM) syntax_error();
    const uint32_t val = token.m_value;
    type               = m_tp.get_token(&token);
    if ((type != TT_EOL) && (type != TT_EOF)) syntax_error();
    m_gen->log(val);
}

void grammar_c::parse_noeob() {
    token_c            token;
    const token_type_t type = m_tp.get_token(&token);
    if ((type != TT_EOL) && (type != TT_EOF)) syntax_error();
    m_gen->noeob();
}

void grammar_c::parse_testmode() {
    token_c  token;
    uint32_t param = 0U;

    token_type_t type = m_tp.get_token(&token);
    if (type != TT_NUM) syntax_error();
    const uint32_t val = token.m_value;
    type               = m_tp.get_token(&token);
    if (type == TT_NUM) {
        param = token.m_value;
        type  = m_tp.get_token(&token);
    }
    if ((type != TT_EOL) && (type != TT_EOF)) syntax_error();
    m_gen->testmode(val, param);
}

void grammar_c::parse_block() {
    token_c token;
    bool    bfinal    = false;
    bool    stored    = false;
    bool    fixed     = false;
    bool    invalid   = false;
    bool    raw       = false;
    bool    type_seen = false;

    while (true) {
        const token_type_t type = m_tp.get_token(&token);
        switch (type) {
            case TT_EOL:
            case TT_EOF: break;
            case TT_BFINAL: bfinal = true; continue;
            case TT_STORED:
                stored = true;
                if (type_seen) goto error; //NOLINT(cppcoreguidelines-avoid-goto)
                type_seen = true;
                continue;
            case TT_FIXED:
                fixed = true;
                if (type_seen) goto error; //NOLINT(cppcoreguidelines-avoid-goto)
                type_seen = true;
                continue;
            case TT_INVALID:
                invalid = true;
                if (type_seen) goto error; //NOLINT(cppcoreguidelines-avoid-goto)
                type_seen = true;
                continue;
            case TT_RAW: raw = true; continue;
            default: syntax_error();
        }
        break;
    }

    m_gen->start_block(bfinal, fixed, stored, invalid, raw);

    return;
error:
    fprintf(stderr, "Too many block types at line %d\n", m_tp.line_num());
    throw std::exception();
}
} // namespace gz_generator
