/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef __GRAMMAR_H__
#define __GRAMMAR_H__

#include <iosfwd>
#include <stdint.h>
#include <stdio.h>

#include "token.h"

namespace gz_generator {
enum len_type_t { LT_LL, LT_D, LT_CL, LT_CL_ALT, LT_LLE, LT_DE, LT_CLE }; // encoded versions

class gen_c;

class grammar_c {
public:
    grammar_c(std::stringstream* config) : m_tp(config), m_gen(nullptr) {}

    void set_gen(gen_c* gen) { m_gen = gen; }

    void parse_lines();

    uint32_t line_num() { return m_tp.line_num(); }

protected:
    token_parser_c m_tp;
    gen_c*         m_gen;

    void syntax_error();

    void parse_l();

    void parse_r();

    void parse_m();

    void parse_block();

    void parse_testmode();

    void parse_lens(len_type_t type);

    void parse_bfinal();

    void parse_log();

    void parse_noeob();

    void parse_bout();

    void parse_pad();

    void parse_set();
};
} // namespace gz_generator
#endif // ifndef __GRAMMAR_H__
