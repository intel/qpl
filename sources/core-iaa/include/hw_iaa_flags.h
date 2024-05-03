/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_SOURCES_HW_PATH_INCLUDE_HW_IAA_FLAGS_H_
#define QPL_SOURCES_HW_PATH_INCLUDE_HW_IAA_FLAGS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t hw_operation_flags_t; /**< Represents operation's flags */

/* ################# COMMON FLAGS ################# */

/* ################# COMPRESSION FLAGS ################# */
/**
 * @name Operation codes for Intel® In-Memory Analytics Accelerator (Intel® IAA)
 * @anchor HW_OPCODES
 * @todo Opcode values
 * @{
 */
#define QPL_OPCODE_DECOMPRESS   0x42U    /**< Intel® IAA decompress operation code */
#define QPL_OPCODE_COMPRESS     0x43U    /**< Intel® IAA compress operation code */
#define QPL_OPCODE_CRC64        0x44U    /**< Intel® IAA crc64 operation code */
#define QPL_OPCODE_SCAN         0x50U    /**< Intel® IAA scan operation code */
#define QPL_OPCODE_EXTRACT      0x52U    /**< Intel® IAA extract operation code */
#define QPL_OPCODE_SELECT       0x53U    /**< Intel® IAA select operation code */
#define QPL_OPCODE_EXPAND       0x56U    /**< Intel® IAA expand operation code */

/** @} */

/* ################# FILTER FLAGS ################# */

typedef enum {
    hw_iaa_input_format_le   = 0U,
    hw_iaa_input_format_be   = 1U,
    hw_iaa_input_format_prle = 2U,
} hw_iaa_input_format;

typedef enum {
    hw_iaa_output_format_nominal       = 0U,
    hw_iaa_output_format_8U            = 1U,
    hw_iaa_output_format_16U           = 2U,
    hw_iaa_output_format_32U           = 3U,
    hw_iaa_output_modifier_big_endian  = (1U << 15U),
    hw_iaa_output_modifier_inverse     = (1U << 16U),
    hw_iaa_output_modifier_force_array = (1U << 27U),
} hw_iaa_output_format;

typedef enum {
    none                = 0U,
    end_of_block        = 1U,
    stored_end_of_block = 2U,
    final_end_of_block  = 3U
} hw_iaa_terminator_t;

typedef enum {
    mini_block_size_none = 0U,    /**< Disable indexing for the deflate stream */
    mini_block_size_512  = 1U,    /**< Put index into the deflate stream each 512   bytes */
    mini_block_size_1k   = 2U,    /**< Put index into the deflate stream each 1024  bytes */
    mini_block_size_2k   = 3U,    /**< Put index into the deflate stream each 2048  bytes */
    mini_block_size_4k   = 4U,    /**< Put index into the deflate stream each 4096  bytes */
    mini_block_size_8k   = 5U,    /**< Put index into the deflate stream each 8192  bytes */
    mini_block_size_16k  = 6U,    /**< Put index into the deflate stream each 16384 bytes */
    mini_block_size_32k  = 7U     /**< Put index into the deflate stream each 32768 bytes */
} hw_iaa_mini_block_size_t;

typedef enum {
    stop_and_check_for_bfinal_eob = 0U,    /**< Stop condition: b_final EOB; Check condition: b_final EOB */
    dont_stop_or_check,                    /**< Stop condition: none;       Check condition: none */
    stop_and_check_for_any_eob,            /**< Stop condition: EOB;        Check condition: EOB */
    stop_on_any_eob,                       /**< Stop condition: EOB;        Check condition: none */
    stop_on_bfinal_eob,                    /**< Stop condition: b_final EOB; Check condition: none */
    check_for_any_eob,                     /**< Stop condition: none;       Check condition: EOB */
    check_for_bfinal_eob,                  /**< Stop condition: none; Check condition: b_final EOB */
} hw_iaa_decompress_start_stop_rule_t;

#ifdef __cplusplus
}
#endif

#endif //QPL_SOURCES_HW_PATH_INCLUDE_HW_IAA_FLAGS_H_
