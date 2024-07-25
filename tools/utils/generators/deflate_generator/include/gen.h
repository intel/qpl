/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef _GZ_GENERATOR_H_
#define _GZ_GENERATOR_H_

#include <memory>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "grammar.h"
#include "huffman.h"

namespace gz_generator {
constexpr uint32_t HIST_SIZE    = 32768U;
constexpr uint32_t NUM_LL_LENS  = 286U;
constexpr uint32_t NUM_D_LENS   = 32U;
constexpr uint32_t NUM_CL_LENS  = 19U;
constexpr uint32_t NUM_ENC_LENS = 1024;

class gen_c {
protected:
    enum warn_t { WARN_D_BEFORE_START, WARN_D_GREATER_HIST, WARN_NO_HUFF_CODE, NUM_WARN };

    struct {
        bool in_block;
        bool bfinal;
        bool bout;
        bool raw;
    } m_state;

    enum { BT_DYN, BT_FIXED, BT_STORED, BT_INVALID } m_blktype;

    huffman_c               m_huff;
    grammar_c               m_grammar;
    gz_generator::BitBuffer m_binaryBitBuffer;
    std::vector<uint8_t>*   m_pReferenceBitBuffer;
    uint32_t                m_cum_bytes;

public:
    uint32_t m_ll_lens[NUM_LL_LENS];
    uint32_t m_num_ll_lens;
    uint32_t m_d_lens[NUM_D_LENS];
    uint32_t m_num_d_lens;
    uint32_t m_cl_lens[NUM_CL_LENS];
    uint32_t m_num_cl_lens;
    uint32_t m_cl_lens_alt[NUM_CL_LENS];
    uint32_t m_num_cl_lens_alt;
    uint32_t m_ll_enc_lens[NUM_ENC_LENS];
    uint32_t m_num_ll_enc_lens;
    uint32_t m_d_enc_lens[NUM_ENC_LENS];
    uint32_t m_num_d_enc_lens;
    uint32_t m_cl_enc_lens[NUM_CL_LENS];
    uint32_t m_num_cl_enc_lens;
    uint32_t m_testmode;
    uint32_t m_testparam;
    bool     m_extra_len;

protected:
    uint8_t  m_hist[HIST_SIZE];
    uint32_t m_hist_ptr;
    uint32_t m_byte_count;
    bool     m_warn_printed[NUM_WARN];
    uint32_t m_pad;

    uint32_t line_num();

    void end_block();

public:
    // fp_in is text file with description
    // fp_bout is the binary output file to get the deflate stream
    // fp_out is the binary output file to get the uncomp output

    gen_c(std::stringstream* config, std::vector<uint8_t>* pBinaryVector, std::vector<uint8_t>* pReferenceVector);

    void proc_lit(int lit);

    void proc_len_dist(uint32_t len, uint32_t dist);

    void proc_rand();

    void proc_lens(len_type_t ltype, uint32_t len);

    void start_block(bool bfinal, bool fixed, bool stored, bool invalid, bool raw);

    void fini();

    void set_bfinal(bool value) { m_state.bfinal = value; }

    void set_bout(bool bout);

    void log(uint32_t type);

    void noeob();

    void set_pad(uint32_t pad);

    void set_BE16() { m_binaryBitBuffer.setBigEndian16(true); }

    void testmode(uint32_t tm, uint32_t p) {
        m_testmode  = tm;
        m_testparam = p;
    }

    void set_extra_len() { m_extra_len = true; }

    std::vector<uint32_t> getIndexes();

    void parse_lines();

    void die(const char* format, ...);

    void warn(warn_t warn_id);
};
} // namespace gz_generator
#endif //_GZ_GENERATOR_H_