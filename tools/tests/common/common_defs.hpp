/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TESTS_COMMON_COMMON_DEFS_HPP_
#define QPL_TESTS_COMMON_COMMON_DEFS_HPP_

// Required for PF testing.
// Based on the value, we choose to swap out the pages
// in src-1/src-2 (READ PF) or dst (WRITE PF).
typedef enum
{
    READ_SRC_1_PAGE_FAULT,
    READ_SRC_2_PAGE_FAULT,
    WRITE_PAGE_FAULT
} PageFaultType;

// Required for version checking in PF tests
// to ensure that MADV_PAGEOUT is available.
#define QPL_PF_TESTS_REQ_MAJOR 5U
#define QPL_PF_TESTS_REQ_MINOR 4U

#endif // QPL_TESTS_COMMON_COMMON_DEFS_HPP_
