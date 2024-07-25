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

static void fill_src_buffer_32u(uint8_t* src, uint8_t* dst, uint32_t length, uint32_t nbits) {
    uint32_t*      p_src_32u = (uint32_t*)src;
    uint32_t*      p_dst_32u = (uint32_t*)dst;
    const uint32_t mask      = (1U << nbits) - 1U;
    for (uint32_t indx = 0; indx < length; indx++) {
        p_dst_32u[indx] = p_src_32u[indx] & mask;
    }
}

static void fill_reference_buffer_32u(uint8_t* src, uint8_t* dst, uint32_t length) {
    uint32_t* p_src_32u = (uint32_t*)src;
    uint32_t* p_dst_32u = (uint32_t*)dst;
    for (uint32_t indx = 0; indx < length; indx++) {
        p_dst_32u[indx] = p_src_32u[indx];
    }
}

constexpr uint32_t TEST_BUFFER_SIZE = 64U;

namespace qpl::test {
using randomizer = qpl::test::random;
QPL_UNIT_API_ALGORITHMIC_TEST(qplc_unpack_32u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> buffer {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> source {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> source_pack {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> reference {};
    const uint64_t                                           seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer                                               random_value(0U, static_cast<double>(UINT32_MAX), seed);

    {
        uint32_t* p_buffer_32u = (uint32_t*)buffer.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_buffer_32u[indx] = static_cast<uint32_t>(random_value);
        }
    }

    for (uint32_t nbits = 17; nbits <= 31; nbits++) {
        source.fill(0);
        fill_src_buffer_32u(buffer.data(), source.data(), TEST_BUFFER_SIZE, nbits);
        for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
            for (uint32_t start_bit = 0; start_bit < 32; start_bit++) {
                if ((18 == nbits) || (22 == nbits) || (26 == nbits) || (30 == nbits)) {
                    if (start_bit & 1) { continue; }
                }
                if ((20 == nbits) || (28 == nbits)) {
                    if (start_bit & 3) { continue; }
                }
                if (24 == nbits) {
                    if (start_bit & 7) { continue; }
                }
                destination.fill(0);
                reference.fill(0);
                source_pack.fill(0);

                qplc_pack_bits(nbits - 1)(source.data(), length, source_pack.data(), start_bit);
                fill_reference_buffer_32u(source.data(), reference.data(), length);
                qplc_unpack_bits(nbits - 1)(source_pack.data(), length, start_bit, destination.data());
                ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                            "FAIL qplc_unpack_32u!!! "));
            }
        }
    }
}
} // namespace qpl::test
