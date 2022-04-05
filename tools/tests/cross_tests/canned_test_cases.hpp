/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_CANNED_TEST_CASES_HPP_
#define QPL_CANNED_TEST_CASES_HPP_

#include "util.hpp"
#include "source_provider.hpp"

namespace qpl::test {

    struct SimpleCannedOneChuckTestCase {
        std::string     file_name;
    };

    std::ostream &operator<<(std::ostream &os, const SimpleCannedOneChuckTestCase &test_case);
}

#endif //QPL_CANNED_TEST_CASES_HPP_
