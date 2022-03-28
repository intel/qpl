/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_ZERO_TEST_CASES_HPP_
#define QPL_ZERO_TEST_CASES_HPP_

#include "util.hpp"
#include "source_provider.hpp"

namespace qpl::test {
    struct SimpleZeroTestCase {
        size_t  ref_size;
        bool    flag_crc;
    };

    std::ostream &operator<<(std::ostream &os, const SimpleZeroTestCase &test_case);
}

#endif // QPL_ZERO_TEST_CASES_HPP_
