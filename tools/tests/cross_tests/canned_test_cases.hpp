/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_CANNED_TEST_CASES_HPP_
#define QPL_CANNED_TEST_CASES_HPP_

// tool_common
#include "source_provider.hpp"
#include "util.hpp"

namespace qpl::test {

struct SimpleCannedOneChuckTestCase {
    std::string file_name;
};

std::ostream& operator<<(std::ostream& os, const SimpleCannedOneChuckTestCase& test_case);
} // namespace qpl::test

#endif //QPL_CANNED_TEST_CASES_HPP_
