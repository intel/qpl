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

qplc_select_t_ptr qplc_select(uint32_t index) {
    static const auto& table = qpl::core_sw::dispatcher::kernels_dispatcher::get_instance().get_select_table();

    return (qplc_select_t_ptr)table[index];
}

qplc_select_i_t_ptr qplc_select_i(uint32_t index) {
    static const auto& table = qpl::core_sw::dispatcher::kernels_dispatcher::get_instance().get_select_i_table();

    return (qplc_select_i_t_ptr)table[index];
}

static void fill_buffer_8u(uint8_t* src, uint8_t* dst, uint32_t length) {
    uint8_t* p_src_8u = src;
    uint8_t* p_dst_8u = dst;
    for (uint32_t indx = 0U; indx < length; indx++) {
        p_dst_8u[indx] = p_src_8u[indx];
    }
}

static uint32_t ref_qplc_select_8u(const uint8_t* src_ptr, const uint8_t* src2_ptr, uint8_t* dst_ptr, uint32_t length) {
    uint32_t selected = 0U;

    for (uint32_t idx = 0U; idx < length; idx++) {
        if (src2_ptr[idx] != 0U) { dst_ptr[selected++] = src_ptr[idx]; }
    }
    return selected;
}

static uint32_t ref_qplc_select_16u(const uint8_t* src_ptr, const uint8_t* src2_ptr, uint8_t* dst_ptr,
                                    uint32_t length) {
    uint16_t* src_16u_ptr = (uint16_t*)src_ptr;
    uint16_t* dst_16u_ptr = (uint16_t*)dst_ptr;
    uint32_t  selected    = 0U;

    for (uint32_t idx = 0U; idx < length; idx++) {
        if (src2_ptr[idx] != 0U) { dst_16u_ptr[selected++] = src_16u_ptr[idx]; }
    }
    return selected;
}

static uint32_t ref_qplc_select_32u(const uint8_t* src_ptr, const uint8_t* src2_ptr, uint8_t* dst_ptr,
                                    uint32_t length) {
    uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;
    uint32_t  selected    = 0U;

    for (uint32_t idx = 0U; idx < length; idx++) {
        if (src2_ptr[idx] != 0U) { dst_32u_ptr[selected++] = src_32u_ptr[idx]; }
    }
    return selected;
}

static uint32_t ref_qplc_select_8u_i(uint8_t* src_dst_ptr, const uint8_t* src2_ptr, uint32_t length) {
    return ref_qplc_select_8u(src_dst_ptr, src2_ptr, src_dst_ptr, length);
}

static uint32_t ref_qplc_select_16u_i(uint8_t* src_dst_ptr, const uint8_t* src2_ptr, uint32_t length) {
    return ref_qplc_select_16u(src_dst_ptr, src2_ptr, src_dst_ptr, length);
}

static uint32_t ref_qplc_select_32u_i(uint8_t* src_dst_ptr, const uint8_t* src2_ptr, uint32_t length) {
    return ref_qplc_select_32u(src_dst_ptr, src2_ptr, src_dst_ptr, length);
}

constexpr uint32_t fun_indx_select_8u  = 0U;
constexpr uint32_t fun_indx_select_16u = 1U;
constexpr uint32_t fun_indx_select_32u = 2U;

constexpr uint32_t TEST_BUFFER_SIZE = 64U;

namespace qpl::test {
using randomizer = qpl::test::random;
QPL_UNIT_API_ALGORITHMIC_TEST(qplc_select_8u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE>                   buffer_mask {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> source {};
    std::array<uint8_t, TEST_BUFFER_SIZE>                   mask {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> reference {};
    const uint64_t                                          seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer                                              random_value(0U, static_cast<double>(UINT8_MAX), seed);

    {
        uint8_t* p_source_8u   = (uint8_t*)source.data();
        uint8_t* p_buffer_mask = (uint8_t*)buffer_mask.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_8u[indx] = static_cast<uint8_t>(random_value);
        }
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_buffer_mask[indx] = 1U & static_cast<uint8_t>(random_value);
        }
    }
    fill_buffer_8u(buffer_mask.data(), mask.data(), (uint32_t)mask.size());
    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);

        const uint32_t select = qplc_select(fun_indx_select_8u)(source.data(), mask.data(), destination.data(), length);
        const uint32_t ref_select = ref_qplc_select_8u(source.data(), mask.data(), reference.data(), length);
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                    "FAIL qplc_select_8u!!! "));
        ASSERT_EQ(select, ref_select);
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_select_16u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE>                    buffer_mask {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> source {};
    std::array<uint8_t, TEST_BUFFER_SIZE>                    mask {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> reference {};
    const uint64_t                                           seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer                                               random_value(0U, static_cast<double>(UINT16_MAX), seed);

    {
        uint16_t* p_source_16u  = (uint16_t*)source.data();
        uint8_t*  p_buffer_mask = (uint8_t*)buffer_mask.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_16u[indx] = static_cast<uint16_t>(random_value);
        }
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_buffer_mask[indx] = (uint8_t)(1U & static_cast<uint16_t>(random_value));
        }
    }

    fill_buffer_8u(buffer_mask.data(), mask.data(), (uint32_t)mask.size());

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);

        const uint32_t select =
                qplc_select(fun_indx_select_16u)(source.data(), mask.data(), destination.data(), length);
        const uint32_t ref_select = ref_qplc_select_16u(source.data(), mask.data(), reference.data(), length);
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                    "FAIL qplc_select_16u!!! "));
        ASSERT_EQ(select, ref_select);
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_select_32u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE>                    buffer_mask {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> source {};
    std::array<uint8_t, TEST_BUFFER_SIZE>                    mask {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> reference {};
    const uint64_t                                           seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer                                               random_value(0U, static_cast<double>(UINT32_MAX), seed);

    {
        uint32_t* p_source_32u  = (uint32_t*)source.data();
        uint8_t*  p_buffer_mask = (uint8_t*)buffer_mask.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_32u[indx] = static_cast<uint16_t>(random_value);
        }
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_buffer_mask[indx] = (uint8_t)(1U & static_cast<uint32_t>(random_value));
        }
    }

    fill_buffer_8u(buffer_mask.data(), mask.data(), (uint32_t)mask.size());

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        destination.fill(0);
        reference.fill(0);

        const uint32_t select =
                qplc_select(fun_indx_select_32u)(source.data(), mask.data(), destination.data(), length);
        const uint32_t ref_select = ref_qplc_select_32u(source.data(), mask.data(), reference.data(), length);
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                    "FAIL qplc_select_32u!!! "));
        ASSERT_EQ(select, ref_select);
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_select_8u_i, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> source_destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE>                   mask {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> reference {};
    const uint64_t                                          seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer                                              random_value(0U, static_cast<double>(UINT8_MAX), seed);

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        {
            uint8_t* p_source_8u   = (uint8_t*)source_destination.data();
            uint8_t* p_buffer_mask = (uint8_t*)mask.data();
            for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
                p_source_8u[indx] = static_cast<uint8_t>(random_value);
            }
            for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
                p_buffer_mask[indx] = 1U & static_cast<uint8_t>(random_value);
            }
        }
        fill_buffer_8u(source_destination.data(), reference.data(), (uint32_t)source_destination.size());
        const uint32_t select     = qplc_select_i(fun_indx_select_8u)(source_destination.data(), mask.data(), length);
        const uint32_t ref_select = ref_qplc_select_8u_i(reference.data(), mask.data(), length);
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), source_destination.begin(),
                                    source_destination.end(), "FAIL qplc_select_8u_i!!! "));
        ASSERT_EQ(select, ref_select);
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_select_16u_i, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> source_destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE>                    mask {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> reference {};
    const uint64_t                                           seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer                                               random_value(0U, static_cast<double>(UINT16_MAX), seed);

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        {
            uint16_t* p_source_16u  = (uint16_t*)source_destination.data();
            uint8_t*  p_buffer_mask = (uint8_t*)mask.data();
            for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
                p_source_16u[indx] = static_cast<uint16_t>(random_value);
            }
            for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
                p_buffer_mask[indx] = (uint8_t)(1U & static_cast<uint16_t>(random_value));
            }
        }
        fill_buffer_8u(source_destination.data(), reference.data(), (uint32_t)source_destination.size());
        const uint32_t select     = qplc_select_i(fun_indx_select_16u)(source_destination.data(), mask.data(), length);
        const uint32_t ref_select = ref_qplc_select_16u_i(reference.data(), mask.data(), length);
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), source_destination.begin(),
                                    source_destination.end(), "FAIL qplc_select_16u_i!!! "));
        ASSERT_EQ(select, ref_select);
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_select_32u_i, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> source_destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE>                    mask {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> reference {};
    const uint64_t                                           seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer                                               random_value(0U, static_cast<double>(UINT32_MAX), seed);

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        {
            uint32_t* p_source_32u  = (uint32_t*)source_destination.data();
            uint8_t*  p_buffer_mask = (uint8_t*)mask.data();
            for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
                p_source_32u[indx] = static_cast<uint16_t>(random_value);
            }
            for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
                p_buffer_mask[indx] = (uint8_t)(1U & static_cast<uint32_t>(random_value));
            }
        }
        fill_buffer_8u(source_destination.data(), reference.data(), (uint32_t)source_destination.size());
        const uint32_t select     = qplc_select_i(fun_indx_select_32u)(source_destination.data(), mask.data(), length);
        const uint32_t ref_select = ref_qplc_select_32u_i(reference.data(), mask.data(), length);
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), source_destination.begin(),
                                    source_destination.end(), "FAIL qplc_select_32u_i!!! "));
        ASSERT_EQ(select, ref_select);
    }
}
} // namespace qpl::test
