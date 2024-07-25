/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_HUFFMAN_ONLY_TEST_CASES_HPP_
#define QPL_HUFFMAN_ONLY_TEST_CASES_HPP_

// tool_common
#include "source_provider.hpp"
#include "util.hpp"

namespace qpl::test {

struct SimpleHuffmanOnlyTestCase {
    bool        is_huffman_be;
    std::string file_name;
};

std::ostream& operator<<(std::ostream& os, const SimpleHuffmanOnlyTestCase& test_case);
} // namespace qpl::test

#endif //QPL_HUFFMAN_ONLY_TEST_CASES_HPP_
