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

qplc_expand_t_ptr qplc_expand(uint32_t index) {
    static const auto& table = qpl::core_sw::dispatcher::kernels_dispatcher::get_instance().get_expand_table();

    return (qplc_expand_t_ptr)table[index];
}

static void fill_buffer_8u(uint8_t* src, uint8_t* dst, uint32_t length) {
    uint8_t* p_src_8u = src;
    uint8_t* p_dst_8u = dst;
    for (uint32_t indx = 0U; indx < length; indx++)
        p_dst_8u[indx] = p_src_8u[indx];
}

static void fill_buffer_16u(uint8_t* src, uint8_t* dst, uint32_t length) {
    uint16_t* p_src_16u = (uint16_t*)src;
    uint16_t* p_dst_16u = (uint16_t*)dst;
    for (uint32_t indx = 0U; indx < length; indx++)
        p_dst_16u[indx] = p_src_16u[indx];
}

static void fill_buffer_32u(uint8_t* src, uint8_t* dst, uint32_t length) {
    uint32_t* p_src_32u = (uint32_t*)src;
    uint32_t* p_dst_32u = (uint32_t*)dst;
    for (uint32_t indx = 0U; indx < length; indx++)
        p_dst_32u[indx] = p_src_32u[indx];
}

static uint32_t ref_qplc_expand_8u(const uint8_t* src1_ptr, uint32_t length_1, const uint8_t* src2_ptr,
                                   uint32_t* length_2_ptr, uint8_t* dst_ptr) {
    uint32_t expanded = 0U;
    uint32_t idx      = 0U;
    for (; idx < *length_2_ptr; idx++) {
        if (src2_ptr[idx]) {
            if (expanded >= length_1) { break; }
            dst_ptr[idx] = src1_ptr[expanded++];
        } else {
            dst_ptr[idx] = 0U;
        }
    }
    *length_2_ptr -= idx;
    return expanded;
}

static uint32_t ref_qplc_expand_16u(const uint8_t* src1_ptr, uint32_t length_1, const uint8_t* src2_ptr,
                                    uint32_t* length_2_ptr, uint8_t* dst_ptr) {
    uint16_t* src_16u_ptr = (uint16_t*)src1_ptr;
    uint16_t* dst_16u_ptr = (uint16_t*)dst_ptr;
    uint32_t  expanded    = 0U;
    uint32_t  idx         = 0U;

    for (; idx < *length_2_ptr; idx++) {
        if (src2_ptr[idx]) {
            if (expanded >= length_1) { break; }
            dst_16u_ptr[idx] = src_16u_ptr[expanded++];
        } else {
            dst_16u_ptr[idx] = 0U;
        }
    }
    *length_2_ptr -= idx;
    return expanded;
}

static uint32_t ref_qplc_expand_32u(const uint8_t* src1_ptr, uint32_t length_1, const uint8_t* src2_ptr,
                                    uint32_t* length_2_ptr, uint8_t* dst_ptr) {
    uint32_t* src_32u_ptr = (uint32_t*)src1_ptr;
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;
    uint32_t  expanded    = 0U;
    uint32_t  idx         = 0U;

    for (; idx < *length_2_ptr; idx++) {
        if (src2_ptr[idx]) {
            if (expanded >= length_1) { break; }
            dst_32u_ptr[idx] = src_32u_ptr[expanded++];
        } else {
            dst_32u_ptr[idx] = 0U;
        }
    }
    *length_2_ptr -= idx;
    return expanded;
}

constexpr uint32_t fun_indx_expand_8u  = 0U;
constexpr uint32_t fun_indx_expand_16u = 1U;
constexpr uint32_t fun_indx_expand_32u = 2U;

constexpr uint32_t TEST_BUFFER_SIZE = 64U;

namespace qpl::test {
using randomizer = qpl::test::random;
QPL_UNIT_API_ALGORITHMIC_TEST(qplc_expand_8u, base) {
    const std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> buffer_src {};
    std::array<uint8_t, TEST_BUFFER_SIZE>                         buffer_mask {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)>       source {};
    std::array<uint8_t, TEST_BUFFER_SIZE>                         mask {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)>       destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)>       reference {};
    const uint64_t                                                seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer random_value(0U, static_cast<double>(UINT8_MAX), seed);

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

    fill_buffer_8u(buffer_mask.data(), mask.data(), TEST_BUFFER_SIZE);

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        uint32_t length2     = length;
        uint32_t ref_length2 = length;

        destination.fill(0);
        reference.fill(0);
        const uint32_t expanded = qplc_expand(0)(source.data(), length, mask.data(), &length2, destination.data());
        const uint32_t ref_expanded =
                ref_qplc_expand_8u(source.data(), length, mask.data(), &ref_length2, reference.data());
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                    "FAIL qplc_expand_8u!!! "));
        ASSERT_EQ(length2, ref_length2);
        ASSERT_EQ(expanded, ref_expanded);
    }

    {
        uint8_t* p_buffer_mask = (uint8_t*)mask.data();
        for (uint32_t indx = 0U; indx < 16U; indx++) {
            p_buffer_mask[indx] = 0U;
        }
    }

    for (uint32_t length = 16U; length <= TEST_BUFFER_SIZE; length++) {
        uint32_t length2     = length;
        uint32_t ref_length2 = length;

        destination.fill(0);
        reference.fill(0);
        const uint32_t expanded = qplc_expand(0)(source.data(), length, mask.data(), &length2, destination.data());
        const uint32_t ref_expanded =
                ref_qplc_expand_8u(source.data(), length, mask.data(), &ref_length2, reference.data());
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                    "FAIL qplc_expand_8u!!! "));
        ASSERT_EQ(length2, ref_length2);
        ASSERT_EQ(expanded, ref_expanded);
    }

    fill_buffer_8u(buffer_mask.data(), mask.data(), TEST_BUFFER_SIZE);

    for (uint32_t length = 2U; length <= TEST_BUFFER_SIZE; length++) {
        uint32_t length2     = length;
        uint32_t ref_length2 = length;

        fill_buffer_8u(buffer_mask.data(), mask.data(), TEST_BUFFER_SIZE);
        destination.fill(0);
        reference.fill(0);
        const uint32_t expanded = qplc_expand(0)(source.data(), length - 1, mask.data(), &length2, destination.data());
        const uint32_t ref_expanded =
                ref_qplc_expand_8u(source.data(), length - 1, mask.data(), &ref_length2, reference.data());
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                    "FAIL qplc_expand_8u!!! "));
        ASSERT_EQ(length2, ref_length2);
        ASSERT_EQ(expanded, ref_expanded);
    }

    {
        uint8_t* p_buffer_mask = (uint8_t*)mask.data();
        for (uint32_t indx = 0U; indx < 32U; indx++) {
            p_buffer_mask[indx] = 0U;
        }
    }

    for (uint32_t length = 33U; length <= TEST_BUFFER_SIZE; length++) {
        uint32_t length2     = length;
        uint32_t ref_length2 = length;

        destination.fill(0);
        reference.fill(0);
        const uint32_t expanded =
                qplc_expand(0)(source.data(), length - 32U, mask.data(), &length2, destination.data());
        const uint32_t ref_expanded =
                ref_qplc_expand_8u(source.data(), length - 32U, mask.data(), &ref_length2, reference.data());
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                    "FAIL qplc_expand_8u!!! "));
        ASSERT_EQ(length2, ref_length2);
        ASSERT_EQ(expanded, ref_expanded);
    }

    {
        uint8_t* p_buffer_mask = (uint8_t*)mask.data();
        for (uint32_t indx = 0U; indx < 32U; indx++) {
            p_buffer_mask[indx] = 1U;
        }
    }

    for (uint32_t length = 33U; length <= TEST_BUFFER_SIZE; length++) {
        uint32_t length2     = length;
        uint32_t ref_length2 = length;

        destination.fill(0);
        reference.fill(0);
        const uint32_t expanded =
                qplc_expand(0)(source.data(), length - 32U, mask.data(), &length2, destination.data());
        const uint32_t ref_expanded =
                ref_qplc_expand_8u(source.data(), length - 32U, mask.data(), &ref_length2, reference.data());
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                    "FAIL qplc_expand_8u!!! "));
        ASSERT_EQ(length2, ref_length2);
        ASSERT_EQ(expanded, ref_expanded);
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_expand_16u, base) {
    const std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> buffer_src {};
    std::array<uint8_t, TEST_BUFFER_SIZE>                          buffer_mask {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)>       source {};
    std::array<uint8_t, TEST_BUFFER_SIZE>                          mask {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)>       destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)>       reference {};
    const uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer     random_value(0U, static_cast<double>(UINT16_MAX), seed);

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

    fill_buffer_8u(buffer_mask.data(), mask.data(), TEST_BUFFER_SIZE);

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        uint32_t length2     = length;
        uint32_t ref_length2 = length;

        destination.fill(0);
        reference.fill(0);
        const uint32_t expanded = qplc_expand(1)(source.data(), length, mask.data(), &length2, destination.data());
        const uint32_t ref_expanded =
                ref_qplc_expand_16u(source.data(), length, mask.data(), &ref_length2, reference.data());
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                    "FAIL qplc_expand_16u!!! "));
        ASSERT_EQ(length2, ref_length2);
        ASSERT_EQ(expanded, ref_expanded);
    }

    {
        uint8_t* p_buffer_mask = (uint8_t*)mask.data();
        for (uint32_t indx = 0U; indx < 16U; indx++) {
            p_buffer_mask[indx] = 0U;
        }
    }

    for (uint32_t length = 16U; length <= TEST_BUFFER_SIZE; length++) {
        uint32_t length2     = length;
        uint32_t ref_length2 = length;

        destination.fill(0);
        reference.fill(0);
        const uint32_t expanded = qplc_expand(1)(source.data(), length, mask.data(), &length2, destination.data());
        const uint32_t ref_expanded =
                ref_qplc_expand_16u(source.data(), length, mask.data(), &ref_length2, reference.data());
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                    "FAIL qplc_expand_16u!!! "));
        ASSERT_EQ(length2, ref_length2);
        ASSERT_EQ(expanded, ref_expanded);
    }

    fill_buffer_8u(buffer_mask.data(), mask.data(), TEST_BUFFER_SIZE);

    for (uint32_t length = 2U; length <= TEST_BUFFER_SIZE; length++) {
        uint32_t length2     = length;
        uint32_t ref_length2 = length;

        fill_buffer_8u(buffer_mask.data(), mask.data(), TEST_BUFFER_SIZE);
        destination.fill(0);
        reference.fill(0);
        const uint32_t expanded = qplc_expand(1)(source.data(), length - 1U, mask.data(), &length2, destination.data());
        const uint32_t ref_expanded =
                ref_qplc_expand_16u(source.data(), length - 1U, mask.data(), &ref_length2, reference.data());
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                    "FAIL qplc_expand_16u!!! "));
        ASSERT_EQ(length2, ref_length2);
        ASSERT_EQ(expanded, ref_expanded);
    }

    {
        uint8_t* p_buffer_mask = (uint8_t*)mask.data();
        for (uint32_t indx = 0U; indx < 32U; indx++) {
            p_buffer_mask[indx] = 0U;
        }
    }

    for (uint32_t length = 33U; length <= TEST_BUFFER_SIZE; length++) {
        uint32_t length2     = length;
        uint32_t ref_length2 = length;

        destination.fill(0);
        reference.fill(0);
        const uint32_t expanded =
                qplc_expand(1)(source.data(), length - 32U, mask.data(), &length2, destination.data());
        const uint32_t ref_expanded =
                ref_qplc_expand_16u(source.data(), length - 32U, mask.data(), &ref_length2, reference.data());
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                    "FAIL qplc_expand_16u!!! "));
        ASSERT_EQ(length2, ref_length2);
        ASSERT_EQ(expanded, ref_expanded);
    }

    {
        uint8_t* p_buffer_mask = (uint8_t*)mask.data();
        for (uint32_t indx = 0U; indx < 32U; indx++) {
            p_buffer_mask[indx] = 1U;
        }
    }

    for (uint32_t length = 33U; length <= TEST_BUFFER_SIZE; length++) {
        uint32_t length2     = length;
        uint32_t ref_length2 = length;

        destination.fill(0);
        reference.fill(0);
        const uint32_t expanded =
                qplc_expand(1)(source.data(), length - 32U, mask.data(), &length2, destination.data());
        const uint32_t ref_expanded =
                ref_qplc_expand_16u(source.data(), length - 32U, mask.data(), &ref_length2, reference.data());
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                    "FAIL qplc_expand_16u!!! "));
        ASSERT_EQ(length2, ref_length2);
        ASSERT_EQ(expanded, ref_expanded);
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_expand_32u, base) {
    const std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> buffer_src {};
    std::array<uint8_t, TEST_BUFFER_SIZE>                          buffer_mask {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)>       source {};
    std::array<uint8_t, TEST_BUFFER_SIZE>                          mask {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)>       destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)>       reference {};
    const uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer     random_value(0U, static_cast<double>(UINT32_MAX), seed);

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

    fill_buffer_8u(buffer_mask.data(), mask.data(), TEST_BUFFER_SIZE);

    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        uint32_t length2     = length;
        uint32_t ref_length2 = length;

        destination.fill(0);
        reference.fill(0);
        const uint32_t expanded = qplc_expand(2)(source.data(), length, mask.data(), &length2, destination.data());
        const uint32_t ref_expanded =
                ref_qplc_expand_32u(source.data(), length, mask.data(), &ref_length2, reference.data());
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                    "FAIL qplc_expand_32u!!! "));
        ASSERT_EQ(length2, ref_length2);
        ASSERT_EQ(expanded, ref_expanded);
    }

    {
        uint8_t* p_buffer_mask = (uint8_t*)mask.data();
        for (uint32_t indx = 0U; indx < 16U; indx++) {
            p_buffer_mask[indx] = 0U;
        }
    }

    for (uint32_t length = 16U; length <= TEST_BUFFER_SIZE; length++) {
        uint32_t length2     = length;
        uint32_t ref_length2 = length;

        destination.fill(0);
        reference.fill(0);
        const uint32_t expanded = qplc_expand(2)(source.data(), length, mask.data(), &length2, destination.data());
        const uint32_t ref_expanded =
                ref_qplc_expand_32u(source.data(), length, mask.data(), &ref_length2, reference.data());
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                    "FAIL qplc_expand_32u!!! "));
        ASSERT_EQ(length2, ref_length2);
        ASSERT_EQ(expanded, ref_expanded);
    }

    fill_buffer_8u(buffer_mask.data(), mask.data(), TEST_BUFFER_SIZE);

    for (uint32_t length = 2U; length <= TEST_BUFFER_SIZE; length++) {
        uint32_t length2     = length;
        uint32_t ref_length2 = length;

        fill_buffer_8u(buffer_mask.data(), mask.data(), TEST_BUFFER_SIZE);
        destination.fill(0);
        reference.fill(0);
        const uint32_t expanded = qplc_expand(2)(source.data(), length - 1U, mask.data(), &length2, destination.data());
        const uint32_t ref_expanded =
                ref_qplc_expand_32u(source.data(), length - 1U, mask.data(), &ref_length2, reference.data());
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                    "FAIL qplc_expand_32u!!! "));
        ASSERT_EQ(length2, ref_length2);
        ASSERT_EQ(expanded, ref_expanded);
    }

    {
        uint8_t* p_buffer_mask = (uint8_t*)mask.data();
        for (uint32_t indx = 0U; indx < 32U; indx++) {
            p_buffer_mask[indx] = 0U;
        }
    }

    for (uint32_t length = 33U; length <= TEST_BUFFER_SIZE; length++) {
        uint32_t length2     = length;
        uint32_t ref_length2 = length;

        destination.fill(0);
        reference.fill(0);
        const uint32_t expanded =
                qplc_expand(2)(source.data(), length - 32U, mask.data(), &length2, destination.data());
        const uint32_t ref_expanded =
                ref_qplc_expand_32u(source.data(), length - 32U, mask.data(), &ref_length2, reference.data());
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                    "FAIL qplc_expand_32u!!! "));
        ASSERT_EQ(length2, ref_length2);
        ASSERT_EQ(expanded, ref_expanded);
    }

    {
        uint8_t* p_buffer_mask = (uint8_t*)mask.data();
        for (uint32_t indx = 0U; indx < 32U; indx++) {
            p_buffer_mask[indx] = 1U;
        }
    }

    for (uint32_t length = 33U; length <= TEST_BUFFER_SIZE; length++) {
        uint32_t length2     = length;
        uint32_t ref_length2 = length;

        destination.fill(0);
        reference.fill(0);
        const uint32_t expanded =
                qplc_expand(2)(source.data(), length - 32U, mask.data(), &length2, destination.data());
        const uint32_t ref_expanded =
                ref_qplc_expand_32u(source.data(), length - 32U, mask.data(), &ref_length2, reference.data());
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                    "FAIL qplc_expand_32u!!! "));
        ASSERT_EQ(length2, ref_length2);
        ASSERT_EQ(expanded, ref_expanded);
    }
}
} // namespace qpl::test
