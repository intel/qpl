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

#define QPL_TEST_MAX_8U  0xFFU
#define QPL_TEST_MAX_16U 0xFFFFU
#define QPL_TEST_MAX_32U 0xFFFFFFFFU

#include "dispatcher.hpp"
#include "qplc_api.h"

qplc_aggregates_t_ptr qplc_aggregates(uint32_t index) {
    static const auto& table = qpl::core_sw::dispatcher::kernels_dispatcher::get_instance().get_aggregates_table();

    return (qplc_aggregates_t_ptr)table[index];
}

static void ref_qplc_bit_aggregates_8u(const uint8_t* src_ptr, uint32_t length, uint32_t* min_value_ptr,
                                       uint32_t* max_value_ptr, uint32_t* sum_ptr, uint32_t* index_ptr) {

    for (uint32_t idx = 0U; idx < length; idx++) {
        *sum_ptr += src_ptr[idx];
        if (QPL_TEST_MAX_32U == *min_value_ptr) {
            *min_value_ptr = (0U == src_ptr[idx]) ? *min_value_ptr : idx + *index_ptr;
        }
        *max_value_ptr = (0U == src_ptr[idx]) ? *max_value_ptr : idx + *index_ptr;
    }
    *index_ptr += length;
}

static void ref_qplc_aggregates_8u(const uint8_t* src_ptr, uint32_t length, uint32_t* min_value_ptr,
                                   uint32_t* max_value_ptr, uint32_t* sum_ptr) {
    for (uint32_t idx = 0U; idx < length; idx++) {
        *sum_ptr += src_ptr[idx];
        *min_value_ptr = (src_ptr[idx] < *min_value_ptr) ? src_ptr[idx] : *min_value_ptr;
        *max_value_ptr = (src_ptr[idx] > *max_value_ptr) ? src_ptr[idx] : *max_value_ptr;
    }
}

static void ref_qplc_aggregates_16u(const uint8_t* src_ptr, uint32_t length, uint32_t* min_value_ptr,
                                    uint32_t* max_value_ptr, uint32_t* sum_ptr) {
    const uint16_t* src_16u_ptr = (uint16_t*)src_ptr;
    for (uint32_t idx = 0U; idx < length; idx++) {
        *sum_ptr += src_16u_ptr[idx];
        *min_value_ptr = (src_16u_ptr[idx] < *min_value_ptr) ? src_16u_ptr[idx] : *min_value_ptr;
        *max_value_ptr = (src_16u_ptr[idx] > *max_value_ptr) ? src_16u_ptr[idx] : *max_value_ptr;
    }
}

static void ref_qplc_aggregates_32u(const uint8_t* src_ptr, uint32_t length, uint32_t* min_value_ptr,
                                    uint32_t* max_value_ptr, uint32_t* sum_ptr) {
    const uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    for (uint32_t idx = 0U; idx < length; idx++) {
        *sum_ptr += src_32u_ptr[idx];
        *min_value_ptr = (src_32u_ptr[idx] < *min_value_ptr) ? src_32u_ptr[idx] : *min_value_ptr;
        *max_value_ptr = (src_32u_ptr[idx] > *max_value_ptr) ? src_32u_ptr[idx] : *max_value_ptr;
    }
}

constexpr uint32_t fun_indx_bit_aggregates_8u = 0U;
constexpr uint32_t fun_indx_aggregates_8u     = 1U;
constexpr uint32_t fun_indx_aggregates_16u    = 2U;
constexpr uint32_t fun_indx_aggregates_32u    = 3U;

constexpr uint32_t TEST_BUFFER_SIZE = 128U;

namespace qpl::test {
using randomizer = qpl::test::random;
QPL_UNIT_API_ALGORITHMIC_TEST(qplc_bit_aggregates_8u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> source {};
    const uint64_t                                          seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer                                              random_value(0U, static_cast<double>(UINT8_MAX), seed);
    uint32_t                                                min_value_ptr     = 0U;
    uint32_t                                                max_value_ptr     = 0U;
    uint32_t                                                sum_ptr           = 0U;
    uint32_t                                                index_ptr         = 0U;
    uint32_t                                                ref_min_value_ptr = 0U;
    uint32_t                                                ref_max_value_ptr = 0U;
    uint32_t                                                ref_sum_ptr       = 0U;
    uint32_t                                                ref_index_ptr     = 0U;

    {
        uint8_t* p_source_8u = (uint8_t*)source.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_8u[indx] = 1U & static_cast<uint8_t>(random_value);
        }
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = QPL_TEST_MAX_32U;
        max_value_ptr     = 0U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = QPL_TEST_MAX_32U;
        ref_max_value_ptr = 0U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_bit_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                    &index_ptr);
        ref_qplc_bit_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr,
                                   &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = 0U;
        max_value_ptr     = 0U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = 0U;
        ref_max_value_ptr = 0U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_bit_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                    &index_ptr);
        ref_qplc_bit_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr,
                                   &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    {
        uint8_t* p_source_8u = (uint8_t*)source.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_8u[indx] = 0U;
        }
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = QPL_TEST_MAX_32U;
        max_value_ptr     = 0U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = QPL_TEST_MAX_32U;
        ref_max_value_ptr = 0U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_bit_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                    &index_ptr);
        ref_qplc_bit_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr,
                                   &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = 0U;
        max_value_ptr     = 0U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = 0U;
        ref_max_value_ptr = 0U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_bit_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                    &index_ptr);
        ref_qplc_bit_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr,
                                   &ref_index_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_aggregates_8u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> source {};
    const uint64_t                                          seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer                                              random_value(0U, static_cast<double>(UINT8_MAX), seed);
    uint32_t                                                min_value_ptr     = 0U;
    uint32_t                                                max_value_ptr     = 0U;
    uint32_t                                                sum_ptr           = 0U;
    uint32_t                                                index_ptr         = 0U;
    uint32_t                                                ref_min_value_ptr = 0U;
    uint32_t                                                ref_max_value_ptr = 0U;
    uint32_t                                                ref_sum_ptr       = 0U;
    uint32_t                                                ref_index_ptr     = 0U;

    {
        uint8_t* p_source_8u = (uint8_t*)source.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_8u[indx] = static_cast<uint8_t>(random_value);
        }
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = QPL_TEST_MAX_32U;
        max_value_ptr     = 0U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = QPL_TEST_MAX_32U;
        ref_max_value_ptr = 0U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                &index_ptr);
        ref_qplc_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = 0U;
        max_value_ptr     = 0U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = 0U;
        ref_max_value_ptr = 0U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                &index_ptr);
        ref_qplc_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = 0U;
        max_value_ptr     = QPL_TEST_MAX_8U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = 0U;
        ref_max_value_ptr = QPL_TEST_MAX_8U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                &index_ptr);
        ref_qplc_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = 1U;
        max_value_ptr     = QPL_TEST_MAX_8U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = 1U;
        ref_max_value_ptr = QPL_TEST_MAX_8U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                &index_ptr);
        ref_qplc_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    {
        uint8_t* p_source_8u = (uint8_t*)source.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_8u[indx] = 0U;
        }
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = QPL_TEST_MAX_32U;
        max_value_ptr     = 0U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = QPL_TEST_MAX_32U;
        ref_max_value_ptr = 0U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                &index_ptr);
        ref_qplc_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = 0U;
        max_value_ptr     = 0U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = 0U;
        ref_max_value_ptr = 0U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_aggregates_8u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                &index_ptr);
        ref_qplc_aggregates_8u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_aggregates_16u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> source {};
    const uint64_t                                           seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer                                               random_value(0U, static_cast<double>(UINT16_MAX), seed);
    uint32_t                                                 min_value_ptr     = 0U;
    uint32_t                                                 max_value_ptr     = 0U;
    uint32_t                                                 sum_ptr           = 0U;
    uint32_t                                                 index_ptr         = 0U;
    uint32_t                                                 ref_min_value_ptr = 0U;
    uint32_t                                                 ref_max_value_ptr = 0U;
    uint32_t                                                 ref_sum_ptr       = 0U;
    uint32_t                                                 ref_index_ptr     = 0U;

    {
        uint16_t* p_source_16u = (uint16_t*)source.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_16u[indx] = static_cast<uint16_t>(random_value);
        }
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = QPL_TEST_MAX_32U;
        max_value_ptr     = 0U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = QPL_TEST_MAX_32U;
        ref_max_value_ptr = 0U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_aggregates_16u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                 &index_ptr);
        ref_qplc_aggregates_16u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = 0U;
        max_value_ptr     = 0U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = 0U;
        ref_max_value_ptr = 0U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_aggregates_16u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                 &index_ptr);
        ref_qplc_aggregates_16u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = 0U;
        max_value_ptr     = QPL_TEST_MAX_16U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = 0U;
        ref_max_value_ptr = QPL_TEST_MAX_16U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_aggregates_16u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                 &index_ptr);
        ref_qplc_aggregates_16u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = 1U;
        max_value_ptr     = QPL_TEST_MAX_16U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = 1U;
        ref_max_value_ptr = QPL_TEST_MAX_16U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_aggregates_16u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                 &index_ptr);
        ref_qplc_aggregates_16u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    {
        uint16_t* p_source_16u = (uint16_t*)source.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_16u[indx] = 0U;
        }
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = QPL_TEST_MAX_32U;
        max_value_ptr     = 0U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = QPL_TEST_MAX_32U;
        ref_max_value_ptr = 0U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_aggregates_16u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                 &index_ptr);
        ref_qplc_aggregates_16u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = 0U;
        max_value_ptr     = 0U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = 0U;
        ref_max_value_ptr = 0U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_aggregates_16u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                 &index_ptr);
        ref_qplc_aggregates_16u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_aggregates_32u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> source {};
    const uint64_t                                           seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer                                               random_value(0U, static_cast<double>(UINT32_MAX), seed);
    uint32_t                                                 min_value_ptr     = 0U;
    uint32_t                                                 max_value_ptr     = 0U;
    uint32_t                                                 sum_ptr           = 0U;
    uint32_t                                                 index_ptr         = 0U;
    uint32_t                                                 ref_min_value_ptr = 0U;
    uint32_t                                                 ref_max_value_ptr = 0U;
    uint32_t                                                 ref_sum_ptr       = 0U;
    uint32_t                                                 ref_index_ptr     = 0U;

    {
        uint32_t* p_source_32u = (uint32_t*)source.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_32u[indx] = 0x1FFFFU & static_cast<uint32_t>(random_value);
        }
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = QPL_TEST_MAX_32U;
        max_value_ptr     = 0U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = QPL_TEST_MAX_32U;
        ref_max_value_ptr = 0U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_aggregates_32u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                 &index_ptr);
        ref_qplc_aggregates_32u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = 0U;
        max_value_ptr     = 0U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = 0U;
        ref_max_value_ptr = 0U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_aggregates_32u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                 &index_ptr);
        ref_qplc_aggregates_32u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = 0U;
        max_value_ptr     = QPL_TEST_MAX_32U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = 0U;
        ref_max_value_ptr = QPL_TEST_MAX_32U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_aggregates_32u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                 &index_ptr);
        ref_qplc_aggregates_32u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = 1U;
        max_value_ptr     = QPL_TEST_MAX_32U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = 1U;
        ref_max_value_ptr = QPL_TEST_MAX_32U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_aggregates_32u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                 &index_ptr);
        ref_qplc_aggregates_32u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    {
        uint32_t* p_source_32u = (uint32_t*)source.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_32u[indx] = 0U;
        }
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = QPL_TEST_MAX_32U;
        max_value_ptr     = 0U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = QPL_TEST_MAX_32U;
        ref_max_value_ptr = 0U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_aggregates_32u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                 &index_ptr);
        ref_qplc_aggregates_32u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        min_value_ptr     = 0U;
        max_value_ptr     = 0U;
        sum_ptr           = 0U;
        index_ptr         = 0U;
        ref_min_value_ptr = 0U;
        ref_max_value_ptr = 0U;
        ref_sum_ptr       = 0U;
        ref_index_ptr     = 0U;

        qplc_aggregates(fun_indx_aggregates_32u)(source.data(), length, &min_value_ptr, &max_value_ptr, &sum_ptr,
                                                 &index_ptr);
        ref_qplc_aggregates_32u(source.data(), length, &ref_min_value_ptr, &ref_max_value_ptr, &ref_sum_ptr);
        ASSERT_EQ(min_value_ptr, ref_min_value_ptr);
        ASSERT_EQ(max_value_ptr, ref_max_value_ptr);
        ASSERT_EQ(sum_ptr, ref_sum_ptr);
        ASSERT_EQ(index_ptr, ref_index_ptr);
    }
}
} // namespace qpl::test
