/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "huffman_only_test_cases.hpp"

// tool_common
#include "source_provider.hpp"

namespace qpl::test {
std::ostream& operator<<(std::ostream& os, const SimpleHuffmanOnlyTestCase& test_case) {
    os << "is Huffman BE: " << test_case.is_huffman_be << ", file name: " << test_case.file_name << "\n";
    return os;
}
} // namespace qpl::test
