/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/
#include <array>
#include "algorithm"

#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"
#include "random_generator.h"
#include "t_common.hpp"

#include "qplc_api.h"
#include "dispatcher.hpp"

static inline qplc_pack_bits_t_ptr qplc_pack_bits(uint32_t index) {
    static const auto &table = qpl::core_sw::dispatcher::kernels_dispatcher::get_instance().get_pack_table();

    return (qplc_pack_bits_t_ptr) table[index];
}

typedef void (*qplc_pack_8u_type)(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit);

void static ref_qplc_pack_8u1u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit)
{
    for (uint32_t indx = 0; indx < num_elements; indx++) {
        *dst_ptr |= *src_ptr++ << start_bit++;
        start_bit &= 7u;
        if (start_bit == 0)
            dst_ptr++;
    }
}

void static ref_qplc_pack_8unu(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit, uint32_t nbits)
{
    uint32_t    data_in;
    for (uint32_t indx = 0; indx < num_elements; indx++) {
        data_in = *src_ptr++;
        *dst_ptr |= data_in << start_bit;
        start_bit += nbits;
        if (start_bit >= 8) {
            dst_ptr++;
            start_bit &= 7u;
            if (start_bit != 0) {
                *dst_ptr = data_in >> (nbits - start_bit);
            }
        }
    }
}

void static ref_qplc_pack_8u2u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_8unu(src_ptr, num_elements, dst_ptr, start_bit, 2u);
}

void static ref_qplc_pack_8u3u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_8unu(src_ptr, num_elements, dst_ptr, start_bit, 3u);
}

void static ref_qplc_pack_8u4u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_8unu(src_ptr, num_elements, dst_ptr, start_bit, 4u);
}

void static ref_qplc_pack_8u5u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_8unu(src_ptr, num_elements, dst_ptr, start_bit, 5u);
}

void static ref_qplc_pack_8u6u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_8unu(src_ptr, num_elements, dst_ptr, start_bit, 6u);
}

void static ref_qplc_pack_8u7u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t* dst_ptr, uint32_t start_bit) {
    ref_qplc_pack_8unu(src_ptr, num_elements, dst_ptr, start_bit, 7u);
}

static qplc_pack_8u_type ref_qplc_pack_8u_tabl[7] =
{
    ref_qplc_pack_8u1u,
    ref_qplc_pack_8u2u,
    ref_qplc_pack_8u3u,
    ref_qplc_pack_8u4u,
    ref_qplc_pack_8u5u,
    ref_qplc_pack_8u6u,
    ref_qplc_pack_8u7u
};


static void fill_src_buffer_8u(uint8_t* src, uint8_t* dst, size_t length, uint32_t nbits) {
    uint8_t mask = (1u << nbits) - 1u;
    for (uint32_t indx = 0; indx < length; indx++)
        dst[indx] = src[indx] & mask;
}

constexpr uint32_t TEST_BUFFER_SIZE = 64u;

namespace qpl::test {
using randomizer = qpl::test::random;
QPL_UNIT_API_ALGORITHMIC_TEST(qplc_pack_8u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE> buffer{};
    std::array<uint8_t, TEST_BUFFER_SIZE> source{};
    std::array<uint8_t, TEST_BUFFER_SIZE> destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT8_MAX), seed);

    std::generate(buffer.begin(), buffer.end(), [&random_value](){return static_cast<uint8_t>(random_value);});

    for (uint32_t nbits = 1; nbits <= 7; nbits++) {
        source.fill(0);
        fill_src_buffer_8u(buffer.data(), source.data(), buffer.size(), nbits);
        for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
            for (uint32_t start_bit = 0; start_bit < 8; start_bit++) {
                if ((2 == nbits) || (6 == nbits)) {
                    if (start_bit & 1) {
                        continue;
                    }
                }
                if (4 == nbits) {
                    if (start_bit & 3) {
                        continue;
                    }
                }
                destination.fill(0);
                reference.fill(0);
                qplc_pack_bits(nbits - 1)(source.data(), length, destination.data(), start_bit);
                ref_qplc_pack_8u_tabl[nbits - 1](source.data(), length, reference.data(), start_bit);
                ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
                    destination.begin(), destination.end(), "FAIL qplc_pack_8u!!! "));
            }
        }
    }
}
}
