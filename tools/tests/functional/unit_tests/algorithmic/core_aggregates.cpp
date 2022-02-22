/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/
#include <array>

#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"
#include "random_generator.h"
#include "../t_common.hpp"

#define QPL_TEST_MAX_8U      0xffu
#define QPL_TEST_MAX_16U     0xffffu
#define QPL_TEST_MAX_32U     0xffffffffu

#include "qplc_api.h"
#include "dispatcher/dispatcher.hpp"

qplc_aggregates_t_ptr qplc_aggregates(uint32_t index) {
    static const auto &table = qpl::ml::dispatcher::kernels_dispatcher::get_instance().get_aggregates_table();

    return (qplc_aggregates_t_ptr) table[index];
}

static void ref_qplc_bit_aggregates_8u(const uint8_t* src_ptr, uint32_t length,
    uint32_t* min_value_ptr, uint32_t* max_value_ptr,
    uint32_t* sum_ptr, uint32_t* index_ptr) {

    for (uint32_t idx = 0u; idx < length; idx++) {
        *sum_ptr += src_ptr[idx];
        if (QPL_TEST_MAX_32U == *min_value_ptr) {
            *min_value_ptr = (0u == src_ptr[idx]) ? *min_value_ptr : idx + *index_ptr;
        }
        *max_value_ptr = (0u == src_ptr[idx]) ? *max_value_ptr : idx + *index_ptr;
    }
    *index_ptr += length;
}

static void ref_qplc_aggregates_8u(const uint8_t* src_ptr, uint32_t length,
    uint32_t* min_value_ptr, uint32_t* max_value_ptr,
    uint32_t* sum_ptr, uint32_t* index_ptr) {
    for (uint32_t idx = 0u; idx < length; idx++) {
        *sum_ptr += src_ptr[idx];
        *min_value_ptr = (src_ptr[idx] < *min_value_ptr) ? src_ptr[idx] : *min_value_ptr;
        *max_value_ptr = (src_ptr[idx] > *max_value_ptr) ? src_ptr[idx] : *max_value_ptr;
    }
}

static void ref_qplc_aggregates_16u(const uint8_t* src_ptr, uint32_t length,
    uint32_t* min_value_ptr, uint32_t* max_value_ptr,
    uint32_t* sum_ptr, uint32_t* index_ptr) {
    const uint16_t* src_16u_ptr = (uint16_t*)src_ptr;
    for (uint32_t idx = 0u; idx < length; idx++) {
        *sum_ptr += src_16u_ptr[idx];
        *min_value_ptr = (src_16u_ptr[idx] < *min_value_ptr) ? src_16u_ptr[idx] : *min_value_ptr;
        *max_value_ptr = (src_16u_ptr[idx] > *max_value_ptr) ? src_16u_ptr[idx] : *max_value_ptr;
    }
}

static void ref_qplc_aggregates_32u(const uint8_t* src_ptr, uint32_t length,
    uint32_t* min_value_ptr, uint32_t* max_value_ptr,
    uint32_t* sum_ptr, uint32_t* index_ptr) {
    const uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    for (uint32_t idx = 0u; idx < length; idx++) {
        *sum_ptr += src_32u_ptr[idx];
        *min_value_ptr = (src_32u_ptr[idx] < *min_value_ptr) ? src_32u_ptr[idx] : *min_value_ptr;
        *max_value_ptr = (src_32u_ptr[idx] > *max_value_ptr) ? src_32u_ptr[idx] : *max_value_ptr;
    }
}


constexpr uint32_t fun_indx_bit_aggregates_8u = 0;
constexpr uint32_t fun_indx_aggregates_8u = 1;
constexpr uint32_t fun_indx_aggregates_16u = 2;
constexpr uint32_t fun_indx_aggregates_32u = 3;

constexpr uint32_t TEST_BUFFER_SIZE = 128u;

namespace qpl::test {
using randomizer = qpl::test::random;
QPL_UNIT_API_ALGORITHMIC_TEST(qplc_bit_aggregates_8u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE*sizeof(uint8_t)> source{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT8_MAX), seed);
    uint32_t    min_value_ptr;
    uint32_t    max_value_ptr;
    uint32_t    sum_ptr;
    uint32_t    index_ptr;
    uint32_t    ref_min_value_ptr;
    uint32_t    ref_max_value_ptr;
    uint32_t    ref_sum_ptr;
    uint32_t    ref_index_ptr;

    {
        uint8_t* p_source_8u = (uint8_t*)source.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_8u[indx] = 1u & static_cast<uint8_t>(random_value);
        }
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = QPL_TEST_MAX_32U;
        max_value_ptr = 0;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = QPL_TEST_MAX_32U;
        ref_max_value_ptr = 0;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_bit_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_bit_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = 0;
        max_value_ptr = 0;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = 0;
        ref_max_value_ptr = 0;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_bit_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_bit_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    {
        uint8_t* p_source_8u = (uint8_t*)source.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_8u[indx] = 0;
        }
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = QPL_TEST_MAX_32U;
        max_value_ptr = 0;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = QPL_TEST_MAX_32U;
        ref_max_value_ptr = 0;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_bit_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_bit_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = 0;
        max_value_ptr = 0;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = 0;
        ref_max_value_ptr = 0;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_bit_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_bit_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_aggregates_8u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> source{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT8_MAX), seed);
    uint32_t    min_value_ptr;
    uint32_t    max_value_ptr;
    uint32_t    sum_ptr;
    uint32_t    index_ptr;
    uint32_t    ref_min_value_ptr;
    uint32_t    ref_max_value_ptr;
    uint32_t    ref_sum_ptr;
    uint32_t    ref_index_ptr;

    {
        uint8_t* p_source_8u = (uint8_t*)source.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_8u[indx] = static_cast<uint8_t>(random_value);
        }
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = QPL_TEST_MAX_32U;
        max_value_ptr = 0;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = QPL_TEST_MAX_32U;
        ref_max_value_ptr = 0;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = 0;
        max_value_ptr = 0;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = 0;
        ref_max_value_ptr = 0;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = 0;
        max_value_ptr = QPL_TEST_MAX_8U;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = 0;
        ref_max_value_ptr = QPL_TEST_MAX_8U;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = 1;
        max_value_ptr = QPL_TEST_MAX_8U;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = 1;
        ref_max_value_ptr = QPL_TEST_MAX_8U;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    {
        uint8_t* p_source_8u = (uint8_t*)source.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_8u[indx] = 0;
        }
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = QPL_TEST_MAX_32U;
        max_value_ptr = 0;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = QPL_TEST_MAX_32U;
        ref_max_value_ptr = 0;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = 0;
        max_value_ptr = 0;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = 0;
        ref_max_value_ptr = 0;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_aggregates_16u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> source{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT16_MAX), seed);
    uint32_t    min_value_ptr;
    uint32_t    max_value_ptr;
    uint32_t    sum_ptr;
    uint32_t    index_ptr;
    uint32_t    ref_min_value_ptr;
    uint32_t    ref_max_value_ptr;
    uint32_t    ref_sum_ptr;
    uint32_t    ref_index_ptr;

    {
        uint16_t* p_source_16u = (uint16_t*)source.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_16u[indx] = static_cast<uint16_t>(random_value);
        }
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = QPL_TEST_MAX_32U;
        max_value_ptr = 0;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = QPL_TEST_MAX_32U;
        ref_max_value_ptr = 0;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_aggregates_16u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_aggregates_16u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = 0;
        max_value_ptr = 0;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = 0;
        ref_max_value_ptr = 0;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_aggregates_16u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_aggregates_16u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = 0;
        max_value_ptr = QPL_TEST_MAX_16U;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = 0;
        ref_max_value_ptr = QPL_TEST_MAX_16U;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_aggregates_16u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_aggregates_16u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = 1;
        max_value_ptr = QPL_TEST_MAX_16U;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = 1;
        ref_max_value_ptr = QPL_TEST_MAX_16U;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_aggregates_16u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_aggregates_16u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    {
        uint16_t* p_source_16u = (uint16_t*)source.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_16u[indx] = 0;
        }
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = QPL_TEST_MAX_32U;
        max_value_ptr = 0;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = QPL_TEST_MAX_32U;
        ref_max_value_ptr = 0;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_aggregates_16u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_aggregates_16u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = 0;
        max_value_ptr = 0;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = 0;
        ref_max_value_ptr = 0;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_aggregates_16u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_aggregates_16u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_aggregates_32u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> source{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT32_MAX), seed);
    uint32_t    min_value_ptr;
    uint32_t    max_value_ptr;
    uint32_t    sum_ptr;
    uint32_t    index_ptr;
    uint32_t    ref_min_value_ptr;
    uint32_t    ref_max_value_ptr;
    uint32_t    ref_sum_ptr;
    uint32_t    ref_index_ptr;

    {
        uint32_t* p_source_32u = (uint32_t*)source.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_32u[indx] = 0x1ffffu & static_cast<uint32_t>(random_value);
        }
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = QPL_TEST_MAX_32U;
        max_value_ptr = 0;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = QPL_TEST_MAX_32U;
        ref_max_value_ptr = 0;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_aggregates_32u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_aggregates_32u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = 0;
        max_value_ptr = 0;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = 0;
        ref_max_value_ptr = 0;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_aggregates_32u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_aggregates_32u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = 0;
        max_value_ptr = QPL_TEST_MAX_32U;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = 0;
        ref_max_value_ptr = QPL_TEST_MAX_32U;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_aggregates_32u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_aggregates_32u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = 1;
        max_value_ptr = QPL_TEST_MAX_32U;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = 1;
        ref_max_value_ptr = QPL_TEST_MAX_32U;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_aggregates_32u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_aggregates_32u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    {
        uint32_t* p_source_32u = (uint32_t*)source.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_32u[indx] = 0;
        }
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = QPL_TEST_MAX_32U;
        max_value_ptr = 0;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = QPL_TEST_MAX_32U;
        ref_max_value_ptr = 0;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_aggregates_32u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_aggregates_32u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr = 0;
        max_value_ptr = 0;
        sum_ptr = 0;
        index_ptr = 0;
        ref_min_value_ptr = 0;
        ref_max_value_ptr = 0;
        ref_sum_ptr = 0;
        ref_index_ptr = 0;

        qplc_aggregates(fun_indx_aggregates_32u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr, &index_ptr);
        ref_qplc_aggregates_32u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr, &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }
}
}
