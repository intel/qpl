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

#define OWN_BYTE_WIDTH  8U  /**< Byte width in bits */
#define OWN_DWORD_WIDTH 32U /**< Dword width in bits */

#include "dispatcher.hpp"
#include "qplc_api.h"

static inline qplc_pack_bits_t_ptr qplc_pack_bits(uint32_t index) {
    static const auto& table = qpl::core_sw::dispatcher::kernels_dispatcher::get_instance().get_pack_table();

    return (qplc_pack_bits_t_ptr)table[index];
}

typedef void (*qplc_pack_32u_type)(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit);

void static ref_qplc_pack_32unu(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit,
                                uint32_t nbits) {
    if (num_elements > 1U) {
        int32_t   bits_in_buf = (int32_t)(nbits + start_bit);
        uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
        uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;
        uint64_t  src         = (uint64_t)(*dst_32u_ptr) & ((1U << start_bit) - 1U);

        src |= ((uint64_t)(*src_32u_ptr)) << start_bit;
        src_32u_ptr++;
        num_elements--;

        while (0U < num_elements) {
            if (OWN_DWORD_WIDTH <= bits_in_buf) {
                *dst_32u_ptr = (uint32_t)(src);
                dst_32u_ptr++;
                src = src >> OWN_DWORD_WIDTH;
                bits_in_buf -= OWN_DWORD_WIDTH;
            }
            src = src | (((uint64_t)(*src_32u_ptr)) << bits_in_buf);
            src_32u_ptr++;
            num_elements--;
            bits_in_buf += (int32_t)nbits;
        }
        dst_ptr = (uint8_t*)dst_32u_ptr;
        while (0 < bits_in_buf) {
            *dst_ptr = (uint8_t)(src);
            bits_in_buf -= OWN_BYTE_WIDTH;
            dst_ptr++;
            src >>= OWN_BYTE_WIDTH;
        }
    } else {
        // In case when there's only one element to pack
        // output buffer size can be less than 32 bits,
        // the following code performs packing byte by byte
        uint64_t mask   = (uint64_t)((1U << nbits) - 1) << start_bit;
        uint64_t source = (uint64_t)(*(uint32_t*)src_ptr) << start_bit;

        while (mask) {
            const uint8_t mask_8u   = (uint8_t)mask;
            const uint8_t source_8u = (uint8_t)source;
            if (mask_8u) {
                uint8_t dst_8u = *dst_ptr & (~mask_8u);
                dst_8u |= source_8u;
                *dst_ptr = dst_8u;
            }
            dst_ptr++;
            mask >>= 8;
            source >>= 8;
        }
    }
}

void static ref_qplc_pack_32u17u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_32unu(src_ptr, num_elements, dst_ptr, start_bit, 17U);
}

void static ref_qplc_pack_32u18u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_32unu(src_ptr, num_elements, dst_ptr, start_bit, 18U);
}

void static ref_qplc_pack_32u19u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_32unu(src_ptr, num_elements, dst_ptr, start_bit, 19U);
}

void static ref_qplc_pack_32u20u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_32unu(src_ptr, num_elements, dst_ptr, start_bit, 20U);
}

void static ref_qplc_pack_32u21u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_32unu(src_ptr, num_elements, dst_ptr, start_bit, 21U);
}

void static ref_qplc_pack_32u22u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_32unu(src_ptr, num_elements, dst_ptr, start_bit, 22U);
}

void static ref_qplc_pack_32u23u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_32unu(src_ptr, num_elements, dst_ptr, start_bit, 23U);
}

void static ref_qplc_pack_32u24u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_32unu(src_ptr, num_elements, dst_ptr, start_bit, 24U);
}

void static ref_qplc_pack_32u25u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_32unu(src_ptr, num_elements, dst_ptr, start_bit, 25U);
}

void static ref_qplc_pack_32u26u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_32unu(src_ptr, num_elements, dst_ptr, start_bit, 26U);
}

void static ref_qplc_pack_32u27u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_32unu(src_ptr, num_elements, dst_ptr, start_bit, 27U);
}

void static ref_qplc_pack_32u28u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_32unu(src_ptr, num_elements, dst_ptr, start_bit, 28U);
}

void static ref_qplc_pack_32u29u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_32unu(src_ptr, num_elements, dst_ptr, start_bit, 29U);
}

void static ref_qplc_pack_32u30u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_32unu(src_ptr, num_elements, dst_ptr, start_bit, 30U);
}

void static ref_qplc_pack_32u31u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_32unu(src_ptr, num_elements, dst_ptr, start_bit, 31U);
}

static qplc_pack_32u_type ref_qplc_pack_32u_tabl[15U] = {
        ref_qplc_pack_32u17u, ref_qplc_pack_32u18u, ref_qplc_pack_32u19u, ref_qplc_pack_32u20u, ref_qplc_pack_32u21u,
        ref_qplc_pack_32u22u, ref_qplc_pack_32u23u, ref_qplc_pack_32u24u, ref_qplc_pack_32u25u, ref_qplc_pack_32u26u,
        ref_qplc_pack_32u27u, ref_qplc_pack_32u28u, ref_qplc_pack_32u29u, ref_qplc_pack_32u30u, ref_qplc_pack_32u31u};

static void fill_src_buffer_32u(uint8_t* src, uint8_t* dst, size_t length, uint32_t nbits) {
    uint32_t*      p_src_32u = (uint32_t*)src;
    uint32_t*      p_dst_32u = (uint32_t*)dst;
    const uint32_t mask      = (1U << nbits) - 1U;
    for (uint32_t indx = 0U; indx < length; indx++)
        p_dst_32u[indx] = p_src_32u[indx] & mask;
}

constexpr uint32_t TEST_BUFFER_SIZE = 64U;

namespace qpl::test {
using randomizer = qpl::test::random;
QPL_UNIT_API_ALGORITHMIC_TEST(qplc_pack_32u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> buffer {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> source {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> reference {};
    const uint64_t                                           seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer                                               random_value(0U, static_cast<double>(UINT32_MAX), seed);

    {
        uint32_t* p_buffer_32u = (uint32_t*)buffer.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_buffer_32u[indx] = static_cast<uint32_t>(random_value);
        }
    }

    for (uint32_t nbits = 17U; nbits <= 31U; nbits++) {
        source.fill(0);
        fill_src_buffer_32u(buffer.data(), source.data(), TEST_BUFFER_SIZE, nbits);
        for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
            for (uint32_t start_bit = 0U; start_bit < 16U; start_bit++) {
                if ((18U == nbits) || (22U == nbits) || (26U == nbits) || (30U == nbits)) {
                    if (start_bit & 1) { continue; }
                }
                if ((20U == nbits) || (28U == nbits)) {
                    if (start_bit & 3) { continue; }
                }
                if (24U == nbits) {
                    if (start_bit & 7) { continue; }
                }
                destination.fill(0);
                reference.fill(0);
                qplc_pack_bits(nbits - 1U)(source.data(), length, destination.data(), start_bit);
                ref_qplc_pack_32u_tabl[nbits - 1U - 16U](source.data(), length, reference.data(), start_bit);
                ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                            "FAIL qplc_pack_32u!!! "));
            }
        }
    }
}
} // namespace qpl::test
