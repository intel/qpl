;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Copyright (C) 2022 Intel Corporation
;
; SPDX-License-Identifier: MIT
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

default rel
[bits 64]

%include "reg_sizes.asm"

extern qpl_decode_huffman_code_block_stateless_base
extern qpl_decode_huffman_code_block_stateless_01
extern qpl_decode_huffman_code_block_stateless_04

section .text

%include "multibinary.asm"


mbin_interface		qpl_decode_huffman_code_block_stateless
mbin_dispatch_init5	qpl_decode_huffman_code_block_stateless, qpl_decode_huffman_code_block_stateless_base, qpl_decode_huffman_code_block_stateless_01, qpl_decode_huffman_code_block_stateless_01, qpl_decode_huffman_code_block_stateless_04
