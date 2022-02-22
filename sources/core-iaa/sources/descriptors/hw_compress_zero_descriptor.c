/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <assert.h>

#include "own_hw_definitions.h"
#include "hw_descriptors_api.h"

#define PLATFORM 2
#include "qplc_memop.h"

/**
 * @brief Defines a type of the Intel® In-Memory Analytics Accelerator (Intel® IAA) compress_zero descriptor
 */
HW_PATH_BYTE_PACKED_STRUCTURE_BEGIN {
    uint32_t trusted_fields;
    uint32_t op_code_op_flags;          /**< Opcode 31:24, opflags 23:0 */
    uint8_t  *completion_record_ptr;    /**< Completion record address */
    uint8_t  *source_ptr;               /**< Source address */
    uint8_t  *destination_ptr;          /**< Destination address */
    uint32_t processed_bytes;           /**< Transfer size */
    uint16_t interruption_handle;       /**< Completion interrupt handle */
    uint8_t  reserved_1[10];            /**< Reserved */
    uint32_t max_output_bytes;          /**< Transfer size */
    uint8_t  reserved_2[12];            /**< Reserved */
} own_hw_compress_zero_descriptor;
HW_PATH_BYTE_PACKED_STRUCTURE_END

/*
 * Check that descriptor has a correct size
 */
static_assert(sizeof(own_hw_compress_zero_descriptor) == HW_PATH_DESCRIPTOR_SIZE, "Descriptor size is not correct");

HW_PATH_IAA_API(void, descriptor_init_zero_compress, (hw_descriptor *const descriptor_ptr,
                                                      uint32_t zero_opcode,
                                                      const uint8_t *const source_ptr,
                                                      uint8_t *const destination_ptr,
                                                      const uint32_t input_size,
                                                      const uint32_t output_size,
                                                      const hw_operation_flags_t operation_flags)) {
    avx512_qplc_zero_8u((uint8_t *) descriptor_ptr, sizeof(hw_descriptor));

    own_hw_compress_zero_descriptor *const this_ptr = (own_hw_compress_zero_descriptor *) descriptor_ptr;

    this_ptr->op_code_op_flags      = ADOF_OPCODE(zero_opcode);
    this_ptr->source_ptr            = (uint8_t *) source_ptr;
    this_ptr->destination_ptr       = destination_ptr;
    this_ptr->processed_bytes       = input_size;
    this_ptr->max_output_bytes      = output_size;
}
