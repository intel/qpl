/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "hw_definitions.h"
#include "hw_descriptors_api.h"
#include "simple_memory_ops_c_bind.h"

HW_PATH_IAA_API(void, descriptor_reset, (hw_descriptor* const descriptor_ptr)) {
    call_c_set_zeros_uint8_t((uint8_t*)descriptor_ptr, sizeof(hw_descriptor));
}

HW_PATH_IAA_API(void, descriptor_set_completion_record,
                (hw_descriptor* const descriptor_ptr, HW_PATH_VOLATILE hw_completion_record* const completion_record)) {
    const uint32_t FLAG_REQ_COMP   = 0x08U;
    const uint32_t FLAG_COMP_VALID = 0x04U;

    hw_decompress_analytics_descriptor* const this_ptr = (hw_decompress_analytics_descriptor*)descriptor_ptr;

    this_ptr->op_code_op_flags |= FLAG_REQ_COMP | FLAG_COMP_VALID;
    this_ptr->completion_record_ptr = (uint8_t*)completion_record;
}

HW_PATH_IAA_API(void, descriptor_init_noop_operation, (hw_descriptor* const descriptor_ptr)) {
    hw_iaa_descriptor_reset(descriptor_ptr);

    hw_decompress_analytics_descriptor* const this_ptr = (hw_decompress_analytics_descriptor*)descriptor_ptr;
    this_ptr->op_code_op_flags |= ADOF_OPCODE(QPL_OPCODE_NOOP);
}
