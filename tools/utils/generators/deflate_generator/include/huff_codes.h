/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

namespace gz_generator {
class gen_c;

class BitBuffer;

void create_huff_tree(uint32_t const* histogram, uint32_t hist_size, uint32_t* bl_count, uint32_t* codes,
                      uint32_t max_code_len);

void copy_lens(uint32_t const* code_lens, uint32_t num_lens, uint32_t* bl_count, uint32_t* codes, uint32_t num_codes);

void create_hufftables(struct bitbuffer* bb, uint32_t ll_codes[286], uint32_t d_codes[32], uint32_t end_of_block,
                       uint32_t* ll_hist, uint32_t* d_hist, gen_c* gen);

void create_hufftables(BitBuffer* bit_buffer, uint32_t ll_codes[286], uint32_t d_codes[32], uint32_t end_of_block,
                       uint32_t* ll_hist, uint32_t* d_hist, gen_c* gen);

void expand_len_tree(uint32_t* codes);
} // namespace gz_generator