/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <iosfwd>
#include <sstream>
#include <stdint.h>
#include <stdio.h>

enum token_type_t {
    TT_EOF,
    TT_NUM, // numeric
    TT_EOL,

    // words
    TT_L,        // "L"
    TT_R,        // "R"
    TT_M,        // "M"
    TT_BLOCK,    // "block"
    TT_BFINAL,   // "bfinal"
    TT_STORED,   // "stored"
    TT_FIXED,    // "fixed"
    TT_LL_LENS,  // "ll_lens"
    TT_D_LENS,   // "d_lens"
    TT_CL_LENS,  // "cl_lens"
    TT_LOG,      // "log"
    TT_ALT,      // "alt"
    TT_ENCODED,  // "encoded"
    TT_TESTMODE, // "testmode"
    TT_INVALID,  // "invalid"
    TT_NOEOB,    // "no_eob"
    TT_BOUT,     // "bout"
    TT_RAW,      // "raw"
    TT_PAD,      // "pad"
    TT_SET,      // "set"
    TT_EXTRALEN, // "extra_lens"

    // symbols
    TT_STAR, // '*'
    //    TT_QUEST,   // '?'
};

enum char_type_t {
    CC_WHITE     = 0x0001,
    CC_EOL       = 0x0002,
    CC_ESCAPE    = 0x0004,
    CC_WORD      = 0x0008,
    CC_NUMBER    = 0x0010,
    CC_HEXNUMBER = 0x0020,
    CC_EOF       = 0x0040,
    CC_COMMENT   = 0x0080,
};

class char_type_c {
protected:
    uint32_t m_array[0x100];
    char     m_tolower[0x100];

public:
    uint32_t get_type(int c) {
        if (c == EOF)
            return CC_EOF;
        else
            return m_array[c & 0xFF];
    }

    char tolower(int c) { return m_tolower[c]; }

    char_type_c();
};

class token_c {
protected:
public:
    token_type_t m_type  = TT_EOF;
    uint32_t     m_value = 0;

    token_c(token_type_t type, uint32_t value = 0) {
        m_type  = type;
        m_value = value;
    }

    token_c() {};
};

class token_parser_c {
protected:
    std::stringstream* m_config;
    token_c            m_on_deck;
    bool               m_on_deck_valid;
    uint32_t           m_line_num;

    int getc() {
        char c = 0;

        m_config->get(c);
        if (m_config->eof()) {
            m_line_num++;
            return -1;
        }

        if (c == '\n') m_line_num++;

        return c;
    }

    void ungetc(int c) {
        if (c == '\n') m_line_num--;

        if (m_config) {
            m_config->unget();
        } else {
            throw std::exception();
        }
    }

    token_type_t parse_word(token_c* token, int c);

    token_type_t parse_a(token_c* token);

    token_type_t parse_b(token_c* token);

    token_type_t parse_c(token_c* token);

    token_type_t parse_d(token_c* token);

    token_type_t parse_e(token_c* token);

    token_type_t parse_f(token_c* token);

    token_type_t parse_i(token_c* token);

    token_type_t parse_l(token_c* token);

    token_type_t parse_m(token_c* token);

    token_type_t parse_n(token_c* token);

    token_type_t parse_p(token_c* token);

    token_type_t parse_r(token_c* token);

    token_type_t parse_s(token_c* token);

    token_type_t parse_t(token_c* token);

    token_type_t parse_number(token_c* token, int c);

    token_type_t parse_decimal(token_c* token, int c);

    token_type_t parse_hex(token_c* token);

    token_type_t parse_octal(token_c* token, int c);

    token_type_t parse_one_string(token_c* token, const char* str, uint32_t strlen, token_type_t type);

    token_type_t parse_two_strings(token_c* token, const char* str1, uint32_t strlen1, token_type_t type1,
                                   const char* str2, uint32_t strlen2, token_type_t type2);

    token_type_t parse_three_strings(token_c* token, const char* str1, uint32_t strlen1, token_type_t type1,
                                     const char* str2, uint32_t strlen2, token_type_t type2, const char* str3,
                                     uint32_t strlen3, token_type_t type3);

public:
    token_parser_c(std::stringstream* config) {
        if (config) {
            m_config        = config;
            m_on_deck_valid = false;
            m_line_num      = 1;
        } else {
            throw std::exception();
        }
    }

    ~token_parser_c() = default;

    token_type_t get_token_(token_c* token);

    token_type_t get_token(token_c* token) {
        const token_type_t type = get_token_(token);
        token->m_type           = type;
        return type;
    }

    uint32_t line_num() { return m_line_num; }
};

/*
class TokenParser
{
    public:
        explicit TokenParser(std::stringstream *config);

        TokenParser() = delete;

        ~TokenParser() = default;

        token_type_t nextToken(token_c *token);

        uint32_t getLine();

    private:
        std::stringstream *m_config;
        std::string        m_currentConfigLine;
        struct
        {
            uint32_t          m_currentConfigLineNumber ;
            token_c           m_trackedToken;
            bool              m_tokenOnTrack;
        }m_state;



};*/

#endif // ifndef __TOKEN_H__
