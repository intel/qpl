/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "canned_test_cases.hpp"

// tool_common
#include "source_provider.hpp"

namespace qpl::test {
std::ostream& operator<<(std::ostream& os, const SimpleCannedOneChuckTestCase& test_case) {
    os << "File name: " << test_case.file_name << "\n";
    return os;
}
} // namespace qpl::test
