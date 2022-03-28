/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "zero_test_cases.hpp"
#include "source_provider.hpp"

namespace qpl::test {
    std::ostream &operator<<(std::ostream &os, const SimpleZeroTestCase &test_case) {
        os << "Reference size: "
           << test_case.ref_size
           << ", using flag QPL_FLAG_CRC32C: "
           << test_case.flag_crc
           << "\n";
        return os;
    }
}
