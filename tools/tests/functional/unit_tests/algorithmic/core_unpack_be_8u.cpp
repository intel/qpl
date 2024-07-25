/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/
#include <algorithm>
#include <array>

#include "dispatcher.hpp"
#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"
#include "qplc_api.h"
#include "random_generator.h"
#include "t_common.hpp"
#include "util.hpp"

static inline qplc_pack_bits_t_ptr qplc_pack_bits(uint32_t index) {
    static const auto& table = qpl::core_sw::dispatcher::kernels_dispatcher::get_instance().get_pack_table();

    return (qplc_pack_bits_t_ptr)table[index];
}

static inline qplc_unpack_bits_t_ptr qplc_unpack_bits(uint32_t index) {
    static const auto& table = qpl::core_sw::dispatcher::kernels_dispatcher::get_instance().get_unpack_table();

    return (qplc_unpack_bits_t_ptr)table[index];
}

static void fill_src_buffer_8u(uint8_t* src, uint8_t* dst, size_t length, uint32_t nbits) {
    const uint8_t mask = (1U << nbits) - 1U;
    for (uint32_t indx = 0; indx < length; indx++)
        dst[indx] = src[indx] & mask;
}

static void fill_reference_buffer_8u(uint8_t* src, uint8_t* dst, uint32_t length) {
    for (uint32_t indx = 0; indx < length; indx++)
        dst[indx] = src[indx];
}

constexpr uint32_t TEST_BUFFER_SIZE = 64U;

namespace qpl::test {
using randomizer = qpl::test::random;
QPL_UNIT_API_ALGORITHMIC_TEST(qplc_unpack_be_8u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE> buffer {};
    std::array<uint8_t, TEST_BUFFER_SIZE> source {};
    std::array<uint8_t, TEST_BUFFER_SIZE> source_pack {};
    std::array<uint8_t, TEST_BUFFER_SIZE> destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE> reference {};
    const uint64_t                        seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer                            random_value(0U, static_cast<double>(UINT8_MAX), seed);

    std::generate(buffer.begin(), buffer.end(), [&random_value]() { return static_cast<uint8_t>(random_value); });

    for (uint32_t nbits = 1; nbits <= 7; nbits++) {
        source.fill(0);
        fill_src_buffer_8u(buffer.data(), source.data(), buffer.size(), nbits);
        for (uint32_t length = TEST_BUFFER_SIZE; length <= TEST_BUFFER_SIZE; length++) {
            for (uint32_t start_bit = 0; start_bit < 8; start_bit++) {
                if ((2 == nbits) || (6 == nbits)) {
                    if (start_bit & 1) { continue; }
                }
                if (4 == nbits) {
                    if (start_bit & 3) { continue; }
                }
                destination.fill(0);
                reference.fill(0);
                source_pack.fill(0);

                qplc_pack_bits(32 + 3 + nbits - 1)(source.data(), length, source_pack.data(), start_bit);
                fill_reference_buffer_8u(source.data(), reference.data(), length);
                qplc_unpack_bits(32 + nbits - 1)(source_pack.data(), length, start_bit, destination.data());
                ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                            "FAIL qplc_unpack_be_8u!!! "));
            }
        }
    }
}
} // namespace qpl::test
