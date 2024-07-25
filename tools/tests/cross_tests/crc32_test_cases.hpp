/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_CRC32_TEST_CASES_HPP_
#define QPL_CRC32_TEST_CASES_HPP_

// tool_common
#include "source_provider.hpp"
#include "util.hpp"

namespace qpl::test {
struct SimpleCRC32TestCase {
    uint32_t source_size;
    uint64_t poly;
    bool     is_be_bit_order;
    bool     is_inverse;
};

std::ostream& operator<<(std::ostream& os, const SimpleCRC32TestCase& test_case);
} // namespace qpl::test

#endif //QPL_CRC32_TEST_CASES_HPP_
