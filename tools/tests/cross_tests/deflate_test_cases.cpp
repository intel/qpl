/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "deflate_test_cases.hpp"

// tool_common
#include "source_provider.hpp"

namespace qpl::test {
std::ostream& operator<<(std::ostream& os, const SimpleDeflateTestCase& test_case) {
    std::string header;
    std::string block_type;

    if (no_header == test_case.header) {
        header = "No header";
    } else if (gzip_header == test_case.header) {
        header = "Gzip header";
    } else if (zlib_header == test_case.header) {
        header = "Zlib header";
    }

    if (block_static == test_case.block_type) {
        block_type = "Stored";
    } else if (block_fixed == test_case.block_type) {
        block_type = "Fixed Block";
    } else {
        block_type = "Dynamic Block";
    }

    os << "Header type: " << header << ", block type: " << block_type << ", file name: " << test_case.file_name << "\n";
    return os;
}
} // namespace qpl::test
