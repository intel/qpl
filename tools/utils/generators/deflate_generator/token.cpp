/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "token.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>

static char_type_c g_ctype;
#define CTYPE(c)   (g_ctype.get_type(c))
#define TOLOWER(c) (static_cast<unsigned char>(g_ctype.tolower(c)))

////////////////////////////////////////////////////////////////

char_type_c::char_type_c() {
    uint32_t i = 0U;
    for (; i < 0x100; i++)
        m_array[i] = 0U;
    m_array[' '] |= CC_WHITE;
    m_array['\t'] |= CC_WHITE;
    m_array['\n'] |= CC_EOL;
    m_array['\\'] |= CC_ESCAPE;
    for (i = 'a'; i <= 'z'; i++)
        m_array[i] |= CC_WORD;
    for (i = 'A'; i <= 'Z'; i++)
        m_array[i] |= CC_WORD;
    m_array['_'] |= CC_WORD;
    for (i = '0'; i <= '9'; i++)
        m_array[i] |= CC_NUMBER | CC_HEXNUMBER;
    for (i = 'a'; i <= 'f'; i++)
        m_array[i] |= CC_HEXNUMBER;
    for (i = 'A'; i <= 'F'; i++)
        m_array[i] |= CC_HEXNUMBER;
    m_array['#'] |= CC_COMMENT;

    for (i = 0U; i < 0x100; i++)
        m_tolower[i] = (char)i;
    for (i = 'A'; i <= 'Z'; i++)
        m_tolower[i] = (char)(i + 'a' - 'A');
}

////////////////////////////////////////////////////////////////////////

token_type_t token_parser_c::get_token_(token_c* token) {
    int      c     = 0;
    uint32_t ctype = 0U;

    if (m_on_deck_valid) {
        *token          = m_on_deck;
        m_on_deck_valid = false;
    }

    // skip white space
    do { //NOLINT(cppcoreguidelines-avoid-do-while)
        c     = getc();
        ctype = CTYPE(c);
        if (ctype & CC_ESCAPE) {
            c     = getc();
            ctype = CTYPE(c);
            if (ctype & CC_EOL) ctype = CC_WHITE;
        }
    } while (ctype & CC_WHITE);

    if (ctype & CC_COMMENT) {
        do { //NOLINT(cppcoreguidelines-avoid-do-while)
            c     = getc();
            ctype = CTYPE(c);
        } while ((ctype != CC_EOL) && (ctype != CC_EOF));
    }

    if (ctype & CC_EOL) return TT_EOL;
    if (ctype & CC_EOF) return TT_EOF;

    if (ctype & CC_WORD) return parse_word(token, c);
    if (ctype & CC_NUMBER) return parse_number(token, c);

    switch (c) {
        case '*': return TT_STAR;
        case '?': {
            token->m_value = 0xFFFFFFFFU;
            return TT_NUM;
        }
        default: break;
    }

    fprintf(stderr, "unexpected char 0x%02x at line %d\n", c, m_line_num);
    throw std::exception();
}

token_type_t token_parser_c::parse_word(token_c* token, int c) {
    c = TOLOWER(c);
    switch (c) {
        case 'a': return parse_a(token);
        case 'b': return parse_b(token);
        case 'c': return parse_c(token);
        case 'd': return parse_d(token);
        case 'e': return parse_e(token);
        case 'f': return parse_f(token);
        case 'i': return parse_i(token);
        case 'l': return parse_l(token);
        case 'm': return parse_m(token);
        case 'n': return parse_n(token);
        case 'p': return parse_p(token);
        case 'r': return parse_r(token);
        case 's': return parse_s(token);
        case 't': return parse_t(token);
        default: break;
    }
    fprintf(stderr, "Unknown keyword: %c at line %d\n", c, m_line_num);
    throw std::exception();
}

token_type_t token_parser_c::parse_one_string(token_c* token, const char* str, uint32_t strlen, token_type_t type) {
    int      c     = 0;
    uint32_t ctype = 0U;

    for (uint32_t i = 1U; i < strlen; i++) {
        c = getc();
        if (c == -1) {
            fprintf(stderr, "End of file while parsing string.");
            throw std::exception();
        }
        c = TOLOWER(c);
        if (c != str[i]) {
            fprintf(stderr, "Unknown keyword: ");
            for (uint32_t j = 0U; j < i; j++)
                putc(str[j], stderr);
            fprintf(stderr, "%c at line %d\n", c, m_line_num);
            throw std::exception();
        }
    }
    c     = getc();
    ctype = CTYPE(c);
    if (ctype & (CC_WORD | CC_NUMBER)) {
        fprintf(stderr, "Unknown keyword: %s%c at line %d\n", str, c, m_line_num);
        throw std::exception();
    }
    ungetc(c);
    return type;
}

token_type_t token_parser_c::parse_two_strings(token_c* token, const char* str1, uint32_t strlen1, token_type_t type1,
                                               const char* str2, uint32_t strlen2, token_type_t type2) {
    int          c     = 0;
    uint32_t     ctype = 0U;
    const char*  str   = nullptr;
    token_type_t type  = type1;

    c = getc();
    if (c == -1) {
        fprintf(stderr, "End of file while parsing string.");
        throw std::exception();
    }
    c     = TOLOWER(c);
    ctype = CTYPE(c);
    if (('\0' == str1[1]) && !(ctype & (CC_WORD | CC_NUMBER))) {
        str  = str1;
        type = type1;
        ungetc(c);
    } else if (('\0' == str2[1]) && !(ctype & (CC_WORD | CC_NUMBER))) {
        str  = str2;
        type = type2;
        ungetc(c);
    } else if (c == str1[1]) {
        for (uint32_t i = 2U; i < strlen1; i++) {
            c = getc();
            if (c == -1) {
                fprintf(stderr, "End of file while parsing string.");
                throw std::exception();
            }
            c = TOLOWER(c);
            if (c != str1[i]) {
                fprintf(stderr, "Unknown keyword: ");
                for (uint32_t j = 0U; j < i; j++)
                    putc(str1[j], stderr);
                fprintf(stderr, "%c at line %d\n", c, m_line_num);
                throw std::exception();
            }
        }
        str  = str1;
        type = type1;
    } else if (c == str2[1]) {
        for (uint32_t i = 2U; i < strlen2; i++) {
            c = getc();
            if (c == -1) {
                fprintf(stderr, "End of file while parsing string.");
                throw std::exception();
            }
            c = TOLOWER(c);
            if (c != str2[i]) {
                fprintf(stderr, "Unknown keyword: ");
                for (uint32_t j = 0U; j < i; j++)
                    putc(str2[j], stderr);
                fprintf(stderr, "%c at line %d\n", c, m_line_num);
                throw std::exception();
            }
        }
        str  = str2;
        type = type2;
    } else {
        fprintf(stderr, "Unknown keyword: %c%c at line %d\n", str1[0], c, m_line_num);
        throw std::exception();
    }
    c     = getc();
    ctype = CTYPE(c);
    if (ctype & (CC_WORD | CC_NUMBER)) {
        fprintf(stderr, "Unknown keyword: %s%c at line %d\n", str, c, m_line_num);
        throw std::exception();
    }
    ungetc(c);
    return type;
}

token_type_t token_parser_c::parse_three_strings(token_c* token, const char* str1, uint32_t strlen1, token_type_t type1,
                                                 const char* str2, uint32_t strlen2, token_type_t type2,
                                                 const char* str3, uint32_t strlen3, token_type_t type3) {
    int          c     = 0;
    uint32_t     ctype = 0U;
    const char*  str   = nullptr;
    token_type_t type  = type1;

    c = getc();
    if (c == -1) {
        fprintf(stderr, "End of file while parsing string.");
        throw std::exception();
    }
    c     = TOLOWER(c);
    ctype = CTYPE(c);
    if (('\0' == str1[1]) && !(ctype & (CC_WORD | CC_NUMBER))) {
        str  = str1;
        type = type1;
        ungetc(c);
    } else if (('\0' == str2[1]) && !(ctype & (CC_WORD | CC_NUMBER))) {
        str  = str2;
        type = type2;
        ungetc(c);
    } else if (('\0' == str3[1]) && !(ctype & (CC_WORD | CC_NUMBER))) {
        str  = str3;
        type = type3;
        ungetc(c);
    } else if (c == str1[1]) {
        for (uint32_t i = 2U; i < strlen1; i++) {
            c = getc();
            if (c == -1) {
                fprintf(stderr, "End of file while parsing string.");
                throw std::exception();
            }
            c = TOLOWER(c);
            if (c != str1[i]) {
                fprintf(stderr, "Unknown keyword: ");
                for (uint32_t j = 0U; j < i; j++)
                    putc(str1[j], stderr);
                fprintf(stderr, "%c at line %d\n", c, m_line_num);
                throw std::exception();
            }
        }
        str  = str1;
        type = type1;
    } else if (c == str2[1]) {
        for (uint32_t i = 2U; i < strlen2; i++) {
            c = getc();
            if (c == -1) {
                fprintf(stderr, "End of file while parsing string.");
                throw std::exception();
            }
            c = TOLOWER(c);
            if (c != str2[i]) {
                fprintf(stderr, "Unknown keyword: ");
                for (uint32_t j = 0; j < i; j++)
                    putc(str2[j], stderr);
                fprintf(stderr, "%c at line %d\n", c, m_line_num);
                throw std::exception();
            }
        }
        str  = str2;
        type = type2;
    } else if (c == str3[1]) {
        for (uint32_t i = 2U; i < strlen3; i++) {
            c = getc();
            if (c == -1) {
                fprintf(stderr, "End of file while parsing string.");
                throw std::exception();
            }
            c = TOLOWER(c);
            if (c != str3[i]) {
                fprintf(stderr, "Unknown keyword: ");
                for (uint32_t j = 0U; j < i; j++)
                    putc(str3[j], stderr);
                fprintf(stderr, "%c at line %d\n", c, m_line_num);
                throw std::exception();
            }
        }
        str  = str3;
        type = type3;
    } else {
        fprintf(stderr, "Unknown keyword: %c%c at line %d\n", str1[0], c, m_line_num);
        throw std::exception();
    }
    c     = getc();
    ctype = CTYPE(c);
    if (ctype & (CC_WORD | CC_NUMBER)) {
        fprintf(stderr, "Unknown keyword: %s%c at line %d\n", str, c, m_line_num);
        throw std::exception();
    }
    ungetc(c);
    return type;
}

token_type_t token_parser_c::parse_a(token_c* token) {
    const char     str[]    = "alt";
    const uint32_t str_size = sizeof(str) - 1;
    return parse_one_string(token, str, str_size, TT_ALT);
}

token_type_t token_parser_c::parse_b(token_c* token) {
    const char     str1[]    = "block";
    const uint32_t str1_size = sizeof(str1) - 1;
    const char     str2[]    = "bfinal";
    const uint32_t str2_size = sizeof(str2) - 1;
    const char     str3[]    = "bout";
    const uint32_t str3_size = sizeof(str3) - 1;
    return parse_three_strings(token, str1, str1_size, TT_BLOCK, str2, str2_size, TT_BFINAL, str3, str3_size, TT_BOUT);
}

token_type_t token_parser_c::parse_f(token_c* token) {
    const char     str[]    = "fixed";
    const uint32_t str_size = sizeof(str) - 1;
    return parse_one_string(token, str, str_size, TT_FIXED);
}

token_type_t token_parser_c::parse_i(token_c* token) {
    const char     str[]    = "invalid";
    const uint32_t str_size = sizeof(str) - 1;
    return parse_one_string(token, str, str_size, TT_INVALID);
}

token_type_t token_parser_c::parse_c(token_c* token) {
    const char     str[]    = "cl_lens";
    const uint32_t str_size = sizeof(str) - 1;
    return parse_one_string(token, str, str_size, TT_CL_LENS);
}

token_type_t token_parser_c::parse_d(token_c* token) {
    const char     str[]    = "d_lens";
    const uint32_t str_size = sizeof(str) - 1;
    return parse_one_string(token, str, str_size, TT_D_LENS);
}

token_type_t token_parser_c::parse_e(token_c* token) {
    const char     str1[]    = "encoded";
    const uint32_t str1_size = sizeof(str1) - 1;
    const char     str2[]    = "extra_lens";
    const uint32_t str2_size = sizeof(str2) - 1;
    return parse_two_strings(token, str1, str1_size, TT_ENCODED, str2, str2_size, TT_EXTRALEN);
}

token_type_t token_parser_c::parse_l(token_c* token) {
    const char     str1[]    = "l";
    const uint32_t str1_size = sizeof(str1) - 1;
    const char     str2[]    = "ll_lens";
    const uint32_t str2_size = sizeof(str2) - 1;
    const char     str3[]    = "log";
    const uint32_t str3_size = sizeof(str3) - 1;
    return parse_three_strings(token, str1, str1_size, TT_L, str2, str2_size, TT_LL_LENS, str3, str3_size, TT_LOG);
}

token_type_t token_parser_c::parse_m(token_c* token) {
    int      c     = 0;
    uint32_t ctype = 0U;

    c     = getc();
    ctype = CTYPE(c);
    if (ctype & (CC_WORD | CC_NUMBER)) {
        fprintf(stderr, "Unknown keyword: l%c at line %d\n", c, m_line_num);
        throw std::exception();
    }
    ungetc(c);
    return TT_M;
}

token_type_t token_parser_c::parse_n(token_c* token) {
    const char     str[]    = "no_eob";
    const uint32_t str_size = sizeof(str) - 1;
    return parse_one_string(token, str, str_size, TT_NOEOB);
}

token_type_t token_parser_c::parse_p(token_c* token) {
    const char     str[]    = "pad";
    const uint32_t str_size = sizeof(str) - 1;
    return parse_one_string(token, str, str_size, TT_PAD);
}

token_type_t token_parser_c::parse_r(token_c* token) {
    const char     str1[]    = "r";
    const uint32_t str1_size = sizeof(str1) - 1;
    const char     str2[]    = "raw";
    const uint32_t str2_size = sizeof(str2) - 1;
    return parse_two_strings(token, str1, str1_size, TT_R, str2, str2_size, TT_RAW);
}

token_type_t token_parser_c::parse_s(token_c* token) {
    const char     str1[]    = "stored";
    const uint32_t str1_size = sizeof(str1) - 1;
    const char     str2[]    = "set";
    const uint32_t str2_size = sizeof(str2) - 1;
    return parse_two_strings(token, str1, str1_size, TT_STORED, str2, str2_size, TT_SET);
}

token_type_t token_parser_c::parse_t(token_c* token) {
    const char     str[]    = "testmode";
    const uint32_t str_size = sizeof(str) - 1;
    return parse_one_string(token, str, str_size, TT_TESTMODE);
}

token_type_t token_parser_c::parse_number(token_c* token, int c) {
    uint32_t ctype = 0U;
    if (c == '0') {
        c = getc();
        if (c == -1) {
            fprintf(stderr, "End of file while parsing number.");
            throw std::exception();
        }
        c = TOLOWER(c);
        if (c == 'x') return parse_hex(token);
        ctype = CTYPE(c);
        if (ctype & CC_NUMBER) return parse_octal(token, c);
        ungetc(c);
        token->m_value = 0U;
        return TT_NUM;
    }
    return parse_decimal(token, c);
}

token_type_t token_parser_c::parse_decimal(token_c* token, int c) {
    uint32_t ctype = 0U;
    uint32_t value = (c - '0');

    while (1U) {
        c     = getc();
        ctype = CTYPE(c);
        if (ctype & CC_NUMBER) {
            value = value * 10U + c - '0';
            continue;
        }
        ungetc(c);
        token->m_value = value;
        return TT_NUM;
    }
}

token_type_t token_parser_c::parse_hex(token_c* token) {
    uint32_t             value       = 0U;
    int                  c           = 0;
    uint32_t             ctype       = 0U;
    static const uint8_t hval[0x100] = {
            0U, 0U,  0U,  0U,  0U,  0U,  0U,  0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, // 00 - 0F
            0U, 0U,  0U,  0U,  0U,  0U,  0U,  0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, // 10 - 1F
            0U, 0U,  0U,  0U,  0U,  0U,  0U,  0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, // 20 - 2F
            0U, 1U,  2U,  3U,  4U,  5U,  6U,  7U, 8U, 9U, 0U, 0U, 0U, 0U, 0U, 0U, // 30 - 3F
            0U, 10U, 11U, 12U, 13U, 14U, 15U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, // 40 - 4F
            0U, 0U,  0U,  0U,  0U,  0U,  0U,  0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, // 50 - 5F
            0U, 10U, 11U, 12U, 13U, 14U, 15U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, // 60 - 6F
            0U, 0U,  0U,  0U,  0U,  0U,  0U,  0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, // 70 - 7F
            0U, 0U,  0U,  0U,  0U,  0U,  0U,  0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
            0U, 0U,  0U,  0U,  0U,  0U,  0U,  0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
            0U, 0U,  0U,  0U,  0U,  0U,  0U,  0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
            0U, 0U,  0U,  0U,  0U,  0U,  0U,  0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
            0U, 0U,  0U,  0U,  0U,  0U,  0U,  0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
            0U, 0U,  0U,  0U,  0U,  0U,  0U,  0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};

    while (1U) {
        c     = getc();
        ctype = CTYPE(c);
        if (ctype & CC_HEXNUMBER) {
            value = value * 16U + hval[c];
            continue;
        }
        ungetc(c);
        token->m_value = value;
        return TT_NUM;
    }
}

token_type_t token_parser_c::parse_octal(token_c* token, int c) {
    uint32_t ctype = 0U;

    if (c > '7') {
        fprintf(stderr, "Invalid octal number at line %d\n", m_line_num);
        throw std::exception();
    }
    uint32_t value = (c - '0');
    while (1) {
        c     = getc();
        ctype = CTYPE(c);
        if (ctype & CC_NUMBER) {
            if (c > '7') {
                fprintf(stderr, "Invalid octal number at line %d\n", m_line_num);
                throw std::exception();
            }
            value = value * 8U + c - '0';
            continue;
        }
        ungetc(c);
        token->m_value = value;
        return TT_NUM;
    }
}
