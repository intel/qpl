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

#ifndef QPL_REF_CHECKSUM_H
#define QPL_REF_CHECKSUM_H

#include "own_ref_defs.h"
#include "qpl_api_ref.h"

#if defined(__cplusplus)
extern "C" {
#endif

void update_checksums(qpl_job* const qpl_job_ptr);

#if defined(__cplusplus)
}
#endif

#endif // QPL_REF_CHECKSUM_H

/** @} */
