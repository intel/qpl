/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <array>

#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"
#include "random_generator.h"
#include "t_common.hpp"

// qpl_c_api
#include "compression_operations/bit_writer.h"

namespace qpl::test {

using randomizer = qpl::test::random;

constexpr uint32_t TEST_BUFFER_SIZE = 1U * 1024U;
constexpr uint32_t MIN_BIT_LENGTH   = 1U;
constexpr uint32_t MAX_BIT_LENGTH   = 64U;

QPL_UNIT_API_ALGORITHMIC_TEST(bit_writer_t, written_size) {
    std::array<uint8_t, TEST_BUFFER_SIZE> destination {};
    uint64_t                              stream_bit_length = 0U;
    const uint64_t                        seed              = util::TestEnvironment::GetInstance().GetSeed();

    bit_writer_t bit_writer = {};
    bit_writer_init(&bit_writer);
    bit_writer_set_buffer(&bit_writer, destination.data(), TEST_BUFFER_SIZE);

    randomizer         random_value(0U, static_cast<double>(UINT64_MAX), seed);
    randomizer         random_bit_length(MIN_BIT_LENGTH, MAX_BIT_LENGTH, seed);
    constexpr uint32_t record_count = TEST_BUFFER_SIZE / MAX_BIT_LENGTH;

    for (uint32_t records = 0; records < record_count; records++) {
        const auto value      = static_cast<uint64_t>(random_value);
        const auto bit_length = static_cast<uint32_t>(random_bit_length);

        bit_writer_write_bits(&bit_writer, value, bit_length);

        stream_bit_length += bit_length;
    }

    EXPECT_EQ(stream_bit_length, bit_writer_get_bits_used(&bit_writer));

    //  Todo: take design about expected behaviour of the bit_writer_get_available_bytes
    //  own_bit_writer_flush(&bit_writer);
    //  uint64_t available_bytes = TEST_BUFFER_SIZE - (stream_bit_length + 7u) / 8u;
    //  EXPECT_EQ( available_bytes, own_bit_writer_get_available_bytes(&bit_writer));
}
} // namespace qpl::test
