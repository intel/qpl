/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/
#include "array"
#include "gtest/gtest.h"

#include "qpl_test_environment.hpp"
#include "random_generator.h"
#include "../t_common.hpp"
#include "check_result.hpp"

#include "qplc_api.h"
#include "dispatcher/dispatcher.hpp"

qplc_find_unique_t_ptr qplc_find_unique(uint32_t index) {
    static const auto &table = qpl::ml::dispatcher::kernels_dispatcher::get_instance().get_find_unique_table();

    return (qplc_find_unique_t_ptr) table[index];
}

static void fill_buffer_8u(uint8_t* src, uint8_t* dst, uint32_t length) {
    for (uint32_t indx = 0; indx < length; indx++)
        dst[indx] = src[indx];
}


static void ref_qplc_find_unique_8u(const uint8_t* src_ptr, uint8_t* dst_ptr,
    uint32_t length, uint32_t shift, uint32_t mask)
{
    uint32_t index;
    for (uint32_t idx = 0u; idx < length; idx++) {
        index = (src_ptr[idx] >> shift) & mask;
        dst_ptr[index] = 1u;
    }
}

constexpr uint32_t fun_indx_find_unique_8u = 0;

constexpr uint32_t TEST_BUFFER_SIZE = 64u;

namespace qpl::test {
using randomizer = qpl::test::random;
QPL_UNIT_API_ALGORITHMIC_TEST(qplc_find_unique_8u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE*sizeof(uint8_t)> source{};
    std::array<uint8_t, 256*sizeof(uint8_t)>              destination{};
    std::array<uint8_t, 256*sizeof(uint8_t)>              reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT8_MAX), seed);

    {
        uint8_t* p_source_8u = (uint8_t*)source.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_8u[indx] = static_cast<uint8_t>(random_value);
        }
    }
    uint32_t    shift = 0;
    uint32_t    mask  = 63;
    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        qplc_find_unique(fun_indx_find_unique_8u)(source.data(), destination.data(), length, shift, mask);
        ref_qplc_find_unique_8u(source.data(), reference.data(), length, shift, mask);
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_find_unique_8u!!! "));
    }
    mask = 127;
    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        qplc_find_unique(fun_indx_find_unique_8u)(source.data(), destination.data(), length, shift, mask);
        ref_qplc_find_unique_8u(source.data(), reference.data(), length, shift, mask);
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_find_unique_8u!!! "));
    }
    mask = 255;
    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);
        qplc_find_unique(fun_indx_find_unique_8u)(source.data(), destination.data(), length, shift, mask);
        ref_qplc_find_unique_8u(source.data(), reference.data(), length, shift, mask);
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_find_unique_8u!!! "));
    }
}
}
