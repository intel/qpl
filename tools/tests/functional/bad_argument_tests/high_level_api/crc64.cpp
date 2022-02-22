/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "tb_hl_common.hpp"

#include "qpl/cpp_api/operations/other/crc_operation.hpp"

namespace qpl::test {

constexpr uint32_t source_size = 1024u;
constexpr uint64_t poly        = 0x04C11DB700000000;

QPL_HIGH_LEVEL_API_BAD_ARGUMENT_TEST(crc64, small_destination_size) {
    std::vector<uint8_t> source(source_size);
    std::vector<uint8_t> destination(7);

    auto crc_operation = qpl::crc_operation(poly);

    auto status = test::get_execute_status(crc_operation,
                                           source.begin(),
                                           source.end(),
                                           destination.begin(),
                                           destination.end());

    EXPECT_EQ(status, QPL_STS_DST_IS_SHORT_ERR) << "Fail on: destination size should be >= 64 bit";
}

}
