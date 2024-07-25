/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "gen.h"

#include <cstdlib>

#include "token.h"

constexpr uint32_t USE_RAND = 0xFFFFFFFFU;

namespace gz_generator {
void gen_c::proc_lit(int lit) {
    if (!m_state.in_block) die("Literal found when not in block");
    if (lit == USE_RAND)
        lit = 0xFF & rand();
    else if ((lit == 286) || (lit == 287)) {
        m_huff.add_lit(lit);
        return;
    }

    if (lit & ~0xFF) die("Invalid literal value: %x", lit);

    m_huff.add_lit(lit);

    m_cum_bytes++;

    if (m_state.bout) { m_pReferenceBitBuffer->push_back(lit); }
    m_hist[m_hist_ptr] = lit;
    m_hist_ptr         = (m_hist_ptr + 1U) & (HIST_SIZE - 1U);
    m_byte_count++;
}

void gen_c::proc_len_dist(uint32_t len, uint32_t dist) {
    uint32_t cum_bytes = 0U;
    uint8_t  c         = 0U;

    if (!m_state.in_block) die("Len/dist found when not in block\n");
    if (m_blktype == BT_STORED) die("Reference found in stored block\n");
    if (dist == USE_RAND) {
        cum_bytes = m_cum_bytes;
        if (cum_bytes > 4096U) cum_bytes = 4096U;
        dist = 1U + (rand() % m_cum_bytes);
    }
    if (len == USE_RAND) len = 3U + (0xFFU & rand());

    if ((dist == 32768U + 30U) || (dist == 32768U + 31U)) {

        m_huff.add_len_dist(len, dist);

        return;
    }

    if ((len < 3U) || (len > 259U)) die("Invalid length: %d\n", len);
    if ((dist == 0U) || (dist > 32768U)) die("Invalid distance: %d\n", dist);

#if defined(DEBUG) || (_DEBUG)
    if (dist > m_cum_bytes) warn(WARN_D_BEFORE_START);
    if (dist > 8192) warn(WARN_D_GREATER_HIST);
#endif
    m_huff.add_len_dist(len, dist);

    if (len == 259U) len = 258U;
    m_cum_bytes += len;

    for (uint32_t i = 0U; i < len; i++) {
        c = m_hist[(m_hist_ptr - dist) & (HIST_SIZE - 1U)];
        if (m_state.bout) { m_pReferenceBitBuffer->push_back(c); }
        m_hist[m_hist_ptr] = c;
        m_hist_ptr         = (m_hist_ptr + 1U) & (HIST_SIZE - 1U);
    }
    m_byte_count += len;
}

void gen_c::proc_rand() {
    if (rand() & 1U)
        proc_lit(USE_RAND);
    else
        proc_len_dist(USE_RAND, USE_RAND);
}

void gen_c::proc_lens(len_type_t ltype, uint32_t len) {
    switch (ltype) {
        case LT_LL:
            if (m_num_ll_lens >= NUM_LL_LENS) die("too many LL_LENS\n");
            m_ll_lens[m_num_ll_lens++] = len;
            break;
        case LT_D:
            if (m_num_d_lens >= NUM_D_LENS) die("too many D_LENS\n");
            m_d_lens[m_num_d_lens++] = len;
            break;
        case LT_CL:
            if (m_num_cl_lens_alt) die("can't mix ALT and normal CL_LENS\n");
            if (m_num_cl_lens >= NUM_CL_LENS) die("too many CL_LENS\n");
            m_cl_lens[m_num_cl_lens++] = len;
            break;
        case LT_CL_ALT:
            if (m_num_cl_lens) die("can't mix normal and ALT CL_LENS\n");
            if (m_num_cl_lens_alt >= NUM_CL_LENS) die("too many CL_LENS ALT\n");
            m_cl_lens_alt[m_num_cl_lens_alt++] = len;
            break;
        case LT_LLE:
            if (m_num_ll_enc_lens >= NUM_ENC_LENS) die("too many LL_LENS Enc\n");
            m_ll_enc_lens[m_num_ll_enc_lens++] = len;
            break;
        case LT_DE:
            if (m_num_d_enc_lens >= NUM_ENC_LENS) die("too many D_LENS Enc\n");
            m_d_enc_lens[m_num_d_enc_lens++] = len;
            break;
        case LT_CLE:
            if (m_num_cl_enc_lens >= NUM_CL_LENS) die("too many CL_LENS Enc\n");
            m_cl_enc_lens[m_num_cl_enc_lens++] = len;
            break;
    }
}

void gen_c::start_block(bool bfinal, bool fixed, bool stored, bool invalid, bool raw) {
    if (m_state.in_block) { end_block(); }
    m_huff.reset();
    m_num_ll_lens = m_num_d_lens = m_num_cl_lens = m_num_cl_lens_alt = 0U;
    m_num_ll_enc_lens = m_num_d_enc_lens = m_num_cl_enc_lens = 0U;
    m_testmode = m_testparam = 0U;
    m_state.in_block         = true;
    m_state.bfinal           = bfinal;
    m_state.bout             = true;
    m_huff.set_bout(true);
    m_state.raw = raw;
    if (invalid)
        m_blktype = BT_INVALID;
    else if (fixed)
        m_blktype = BT_FIXED;
    else if (stored)
        m_blktype = BT_STORED;
    else
        m_blktype = BT_DYN;
}

void gen_c::fini() {
    if (m_state.in_block) {
        end_block();
        m_state.in_block = false;
    }

    m_binaryBitBuffer.flush(m_pad);
}

void gen_c::log(uint32_t type) {
    m_huff.log(type);
}

void gen_c::noeob() {
    m_huff.noeob();
}

////////////////////////////////////////////////////////////////////////

uint32_t gen_c::line_num() {
    return m_grammar.line_num();
}

void gen_c::die(const char* format, ...) {
    va_list argptr;

    fprintf(stderr, "Error at line %d: ", line_num());

    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);

    fprintf(stderr, "\n");
    throw std::exception();
}

void gen_c::warn(warn_t warn_id) {
    if (m_warn_printed[warn_id]) return;
    switch (warn_id) {
        case WARN_D_BEFORE_START: fprintf(stderr, "Generator info: distance before start of buffer"); break;
        case WARN_D_GREATER_HIST: fprintf(stderr, "Generator info: distance greater than history size"); break;
        case WARN_NO_HUFF_CODE: fprintf(stderr, "Generator info: missing Huffman code used"); break;
        default: fprintf(stderr, "Warning: unknown warning\n"); throw std::exception();
    }
    fprintf(stderr, " at or before line %d\n", line_num());
    m_warn_printed[warn_id] = true;
}

void gen_c::end_block() {
    switch (m_blktype) {
        case BT_DYN:
            if (m_state.raw) {
                m_huff.wr_dyn_hdr(NULL, m_state.bfinal, this);
            } else {
                m_huff.wr_dyn_hdr(&m_binaryBitBuffer, m_state.bfinal, this);
            }
            m_huff.wr_block(m_binaryBitBuffer);
            m_huff.end_block(m_binaryBitBuffer);
            break;
        case BT_FIXED:
            //        log(1);
            if (m_state.raw)
                m_huff.wr_fixed_hdr(NULL, m_state.bfinal);
            else
                m_huff.wr_fixed_hdr(&m_binaryBitBuffer, m_state.bfinal);

            m_huff.wr_block(m_binaryBitBuffer);
            m_huff.end_block(m_binaryBitBuffer);
            break;
        case BT_STORED:
            m_huff.wr_stored_blocks(m_binaryBitBuffer, m_state.bfinal, this, m_huff.get_noeob() ? 1U : 0U);
            break;
        case BT_INVALID: gz_generator::huffman_c::wr_invalid_block(m_binaryBitBuffer, m_state.bfinal); break;
    }
    m_huff.reset();
}

void gen_c::parse_lines() {
    m_grammar.parse_lines();
}

void gen_c::set_bout(bool bout) {
    m_state.bout = bout;
    m_huff.set_bout(bout);
}

void gen_c::set_pad(uint32_t pad) {
    m_pad = pad;
}

gen_c::gen_c(std::stringstream* config, std::vector<uint8_t>* pBinaryVector, std::vector<uint8_t>* pReferenceVector)
    : m_blktype(BT_DYN)
    , m_grammar(config)
    , m_binaryBitBuffer(pBinaryVector)
    , m_pReferenceBitBuffer(pReferenceVector)
    , m_cum_bytes(0U)
    , m_hist_ptr(0U)
    , m_byte_count(0U) {
    m_state = {false, false, true, false};

    m_num_ll_lens = m_num_d_lens = m_num_cl_lens = m_num_cl_lens_alt = 0;
    m_num_ll_enc_lens = m_num_d_enc_lens = m_num_cl_enc_lens = 0;
    m_testmode = m_testparam = 0;
    m_grammar.set_gen(this);

    for (uint32_t i = 0; i < NUM_WARN; i++) {
        m_warn_printed[i] = false;
    }
    m_pad       = 0U;
    m_extra_len = false;
}

std::vector<uint32_t> gen_c::getIndexes() {
    printf("TotalBitsWritten %d", m_binaryBitBuffer.getBitsWritten());
    return m_huff.getIndexes();
}

} // namespace gz_generator
