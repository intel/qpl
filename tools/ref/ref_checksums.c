/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11/12/2020
 * @brief Internal helper functions for reference calculation of checksum for analytics operations
 *
 * @ingroup REFERENCE_PRIVATE
 * @{
 *
 */

#include "ref_checksums.h"

void update_checksums(qpl_job* const qpl_job_ptr) {
    // Update crc32 field
    uint32_t used_poly = (qpl_job_ptr->flags & QPL_FLAG_CRC32C) ? 0x1EDC6F41 : 0x04C11DB7;

    qpl_job_ptr->crc =
            ref_crc32(qpl_job_ptr->next_in_ptr - qpl_job_ptr->drop_initial_bytes,
                      qpl_job_ptr->available_in + qpl_job_ptr->drop_initial_bytes, used_poly, qpl_job_ptr->crc);

    // Update xor checksum field
    qpl_job_ptr->xor_checksum =
            ref_xor_checksum(qpl_job_ptr->next_in_ptr - qpl_job_ptr->drop_initial_bytes,
                             qpl_job_ptr->available_in + qpl_job_ptr->drop_initial_bytes, qpl_job_ptr->xor_checksum);
}

/** @} */
