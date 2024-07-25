/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "crc32_test_cases.hpp"

// tool_common
#include "source_provider.hpp"

namespace qpl::test {
std::ostream& operator<<(std::ostream& os, const SimpleCRC32TestCase& test_case) {
    os << "Polynomial: " << test_case.poly << ", is Big Endian bit order: " << test_case.is_be_bit_order
       << ", is inverse: " << test_case.is_inverse << "\n";
    return os;
}
} // namespace qpl::test
