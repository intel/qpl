/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <algorithm>
#include <array>

#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"
#include "random_generator.h"
#include "t_common.hpp"

#define OWN_BYTE_WIDTH 8U  /**< Byte width in bits */
#define OWN_WORD_WIDTH 16U /**< Word width in bits */

#include "dispatcher.hpp"
#include "qplc_api.h"

static inline qplc_pack_bits_t_ptr qplc_pack_bits(uint32_t index) {
    static const auto& table = qpl::core_sw::dispatcher::kernels_dispatcher::get_instance().get_pack_table();

    return (qplc_pack_bits_t_ptr)table[index];
}

typedef void (*qplc_pack_16u_type)(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit);

void static ref_qplc_pack_16unu(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit,
                                uint32_t nbits) {
    int32_t   bits_in_buf = (int32_t)(nbits + start_bit);
    uint16_t* src_16u_ptr = (uint16_t*)src_ptr;
    uint16_t* dst_16u_ptr = (uint16_t*)dst_ptr;
    uint32_t  src         = (uint32_t)(*dst_16u_ptr) & ((1U << start_bit) - 1);
    ;

    src |= ((uint32_t)(*src_16u_ptr)) << start_bit;
    src_16u_ptr++;
    num_elements--;

    while (0U < num_elements) {
        if (OWN_WORD_WIDTH <= bits_in_buf) {
            *dst_16u_ptr = (uint16_t)(src);
            dst_16u_ptr++;
            src = src >> OWN_WORD_WIDTH;
            bits_in_buf -= OWN_WORD_WIDTH;
        }
        src = src | (((uint32_t)(*src_16u_ptr)) << bits_in_buf);
        src_16u_ptr++;
        num_elements--;
        bits_in_buf += nbits;
    }
    dst_ptr = (uint8_t*)dst_16u_ptr;
    while (0 < bits_in_buf) {
        *dst_ptr = (uint8_t)(src);
        bits_in_buf -= OWN_BYTE_WIDTH;
        dst_ptr++;
        src >>= OWN_BYTE_WIDTH;
    }
}

void static ref_qplc_pack_16u9u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_16unu(src_ptr, num_elements, dst_ptr, start_bit, 9U);
}

void static ref_qplc_pack_16u10u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_16unu(src_ptr, num_elements, dst_ptr, start_bit, 10U);
}

void static ref_qplc_pack_16u11u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_16unu(src_ptr, num_elements, dst_ptr, start_bit, 11U);
}

void static ref_qplc_pack_16u12u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_16unu(src_ptr, num_elements, dst_ptr, start_bit, 12U);
}

void static ref_qplc_pack_16u13u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_16unu(src_ptr, num_elements, dst_ptr, start_bit, 13U);
}

void static ref_qplc_pack_16u14u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_16unu(src_ptr, num_elements, dst_ptr, start_bit, 14U);
}

void static ref_qplc_pack_16u15u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_16unu(src_ptr, num_elements, dst_ptr, start_bit, 15U);
}

static qplc_pack_16u_type ref_qplc_pack_16u_tabl[7] = {ref_qplc_pack_16u9u,  ref_qplc_pack_16u10u, ref_qplc_pack_16u11u,
                                                       ref_qplc_pack_16u12u, ref_qplc_pack_16u13u, ref_qplc_pack_16u14u,
                                                       ref_qplc_pack_16u15u};

static void fill_src_buffer_16u(uint8_t* src, uint8_t* dst, size_t length, uint32_t nbits) {
    uint16_t*      p_src_16u = (uint16_t*)src;
    uint16_t*      p_dst_16u = (uint16_t*)dst;
    const uint16_t mask      = (1U << nbits) - 1U;
    for (uint32_t indx = 0; indx < length; indx++)
        p_dst_16u[indx] = p_src_16u[indx] & mask;
}

constexpr uint32_t TEST_BUFFER_SIZE = 64U;

namespace qpl::test {
using randomizer = qpl::test::random;
QPL_UNIT_API_ALGORITHMIC_TEST(qplc_pack_16u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> buffer {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> source {};
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
                qplc_pack_bits(nbits - 1)(source.data(), length, destination.data(), start_bit);
                ref_qplc_pack_16u_tabl[nbits - 1 - 8](source.data(), length, reference.data(), start_bit);
                ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                            "FAIL qplc_pack_16u!!! "));
            }
        }
    }
}
} // namespace qpl::test
