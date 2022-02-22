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
 * @name Operation codes
 * @anchor HW_OPCODES
 * @todo Opcode values
 * @{
 */
#define QPL_OPCODE_DECOMPRESS   0x42u    /**< @todo */
#define QPL_OPCODE_COMPRESS     0x43u    /**< @todo */

#define QPL_OPCODE_Z_DECOMP32   0x48u    /**< @todo */
#define QPL_OPCODE_Z_DECOMP16   0x49u    /**< @todo */
#define QPL_OPCODE_Z_COMP32     0x4Cu    /**< @todo */
#define QPL_OPCODE_Z_COMP16     0x4Du    /**< @todo */
/** @} */

/* ################# FILTER FLAGS ################# */

typedef enum {
    hw_iaa_input_format_le   = 0u,
    hw_iaa_input_format_be   = 1u,
    hw_iaa_input_format_prle = 2u,
} hw_iaa_input_format;

typedef enum {
    hw_iaa_output_format_nominal      = 0u,
    hw_iaa_output_format_8u           = 1u,
    hw_iaa_output_format_16u          = 2u,
    hw_iaa_output_format_32u          = 3u,
    hw_iaa_output_modifier_big_endian = (1u << 15u),
    hw_iaa_output_modifier_inverse    = (1u << 16u),
} hw_iaa_output_format;

typedef enum {
    none                = 0u,
    end_of_block        = 1u,
    stored_end_of_block = 2u,
    final_end_of_block  = 3u
} hw_iaa_terminator_t;

typedef enum {
    mini_block_size_none = 0u,    /**< Disable indexing for the deflate stream */
    mini_block_size_512  = 1u,    /**< Put index into the deflate stream each 512   bytes */
    mini_block_size_1k   = 2u,    /**< Put index into the deflate stream each 1024  bytes */
    mini_block_size_2k   = 3u,    /**< Put index into the deflate stream each 2048  bytes */
    mini_block_size_4k   = 4u,    /**< Put index into the deflate stream each 4096  bytes */
    mini_block_size_8k   = 5u,    /**< Put index into the deflate stream each 8192  bytes */
    mini_block_size_16k  = 6u,    /**< Put index into the deflate stream each 16384 bytes */
    mini_block_size_32k  = 7u     /**< Put index into the deflate stream each 32768 bytes */
} hw_iaa_mini_block_size_t;

typedef enum {
    stop_and_check_for_bfinal_eob = 0u,    /**< Stop condition: b_final EOB; Check condition: b_final EOB */
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
