/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/
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

static void fill_src_buffer_16u(uint8_t* src, uint8_t* dst, uint32_t length, uint32_t nbits) {
    uint16_t*      p_src_16u = (uint16_t*)src;
    uint16_t*      p_dst_16u = (uint16_t*)dst;
    const uint16_t mask      = (1U << nbits) - 1U;
    for (uint32_t indx = 0; indx < length; indx++) {
        p_dst_16u[indx] = p_src_16u[indx] & mask;
    }
}

static void fill_reference_buffer_16u(uint8_t* src, uint8_t* dst, uint32_t length) {
    uint16_t* p_src_16u = (uint16_t*)src;
    uint16_t* p_dst_16u = (uint16_t*)dst;
    for (uint32_t indx = 0; indx < length; indx++) {
        p_dst_16u[indx] = p_src_16u[indx];
    }
}

constexpr uint32_t TEST_BUFFER_SIZE = 64U;

namespace qpl::test {
using randomizer = qpl::test::random;
QPL_UNIT_API_ALGORITHMIC_TEST(qplc_unpack_16u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> buffer {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> source {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> source_pack {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> reference {};
    const uint64_t                                           seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer                                               random_value(0U, static_cast<double>(UINT16_MAX), seed);

    {
        uint16_t* p_buffer_16u = (uint16_t*)buffer.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_buffer_16u[indx] = static_cast<uint16_t>(random_value);
        }
    }

    for (uint32_t nbits = 9; nbits <= 15; nbits++) {
        source.fill(0);
        fill_src_buffer_16u(buffer.data(), source.data(), TEST_BUFFER_SIZE, nbits);
        for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
            for (uint32_t start_bit = 0; start_bit < 16; start_bit++) {
                if ((10 == nbits) || (14 == nbits)) {
                    if (start_bit & 1) { continue; }
                }
                if (12 == nbits) {
                    if (start_bit & 3) { continue; }
                }
                destination.fill(0);
                reference.fill(0);
                source_pack.fill(0);

                qplc_pack_bits(nbits - 1)(source.data(), length, source_pack.data(), start_bit);
                fill_reference_buffer_16u(source.data(), reference.data(), length);
                qplc_unpack_bits(nbits - 1)(source_pack.data(), length, start_bit, destination.data());
                ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                            "FAIL qplc_unpack_16u!!! "));
            }
        }
    }
}
} // namespace qpl::test
