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
#include "util.hpp"

#include "qplc_api.h"
#include "dispatcher/dispatcher.hpp"
#include "check_result.hpp"

qplc_set_membership_i_t_ptr qplc_set_membership_i(uint32_t index) {
    static const auto &table = qpl::ml::dispatcher::kernels_dispatcher::get_instance().get_set_membership_i_table();

    return (qplc_set_membership_i_t_ptr) table[index];
}

static void fill_buffer_8u(uint8_t* src, uint8_t* dst, uint32_t length) {
    for (uint32_t indx = 0; indx < length; indx++) {
        dst[indx] = src[indx];
    }
}
static void fill_buffer_16u(uint8_t* src, uint8_t* dst, uint32_t length) {
    uint16_t* p_src_16u = (uint16_t*)src;
    uint16_t* p_dst_16u = (uint16_t*)dst;
    for (uint32_t indx = 0; indx < length; indx++) {
        p_dst_16u[indx] = p_src_16u[indx];
    }
}
static void fill_buffer_32u(uint8_t* src, uint8_t* dst, uint32_t length) {
    uint32_t* p_src_32u = (uint32_t*)src;
    uint32_t* p_dst_32u = (uint32_t*)dst;
    for (uint32_t indx = 0; indx < length; indx++) {
        p_dst_32u[indx] = p_src_32u[indx];
        }
}


static void ref_qplc_set_membership_8u_i(uint8_t* src_dst_ptr, const uint8_t* src2_ptr,
    uint32_t length, uint32_t shift, uint32_t mask)
{
    uint32_t index;
    for (uint32_t idx = 0u; idx < length; idx++) {
        index = (src_dst_ptr[idx] >> shift) & mask;
        src_dst_ptr[idx] = src2_ptr[index];
    }
}
static void ref_qplc_set_membership_16u_i(uint8_t* src_dst_ptr, const uint8_t* src2_ptr,
    uint32_t length, uint32_t shift, uint32_t mask)
{
    uint16_t* src_16u_ptr = (uint16_t*)src_dst_ptr;
    uint8_t* dst_ptr = src_dst_ptr;
    uint32_t index;
    for (uint32_t idx = 0u; idx < length; idx++) {
        index = (src_16u_ptr[idx] >> shift) & mask;
        dst_ptr[idx] = src2_ptr[index];
    }
}
static void ref_qplc_set_membership_32u_i(uint8_t* src_dst_ptr, const uint8_t* src2_ptr,
    uint32_t length, uint32_t shift, uint32_t mask)
{
    uint32_t* src_32u_ptr = (uint32_t*)src_dst_ptr;
    uint8_t* dst_ptr = src_dst_ptr;
    uint32_t index;

    for (uint32_t idx = 0u; idx < length; idx++) {
        index = (src_32u_ptr[idx] >> shift) & mask;
        dst_ptr[idx] = src2_ptr[index];
    }
}

constexpr uint32_t fun_indx_set_membership_8u  = 0;
constexpr uint32_t fun_indx_set_membership_16u = 1;
constexpr uint32_t fun_indx_set_membership_32u = 2;


constexpr uint32_t TEST_BUFFER_SIZE = 64u;
constexpr uint32_t TEST_BUFFER_SIZE_32U = 128u;

namespace qpl::test {
using randomizer = qpl::test::random;
QPL_UNIT_API_ALGORITHMIC_TEST(qplc_set_membership_8u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE*sizeof(uint8_t)> buffer{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> source_index{};
    std::array<uint8_t, TEST_BUFFER_SIZE*sizeof(uint8_t)> destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE*sizeof(uint8_t)> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT8_MAX), seed);

    {
        uint8_t* p_buffer_8u = (uint8_t*)buffer.data();
        uint8_t* p_source_index_8u = (uint8_t*)source_index.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_buffer_8u[indx] = static_cast<uint8_t>(random_value);
        }
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_index_8u[indx] = static_cast<uint8_t>(random_value);
        }
    }
    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        fill_buffer_8u(buffer.data(), destination.data(), TEST_BUFFER_SIZE);
        fill_buffer_8u(buffer.data(), reference.data(), TEST_BUFFER_SIZE);
        qplc_set_membership_i(fun_indx_set_membership_8u)(destination.data(), source_index.data(), length, 0u, 0x1e);
        ref_qplc_set_membership_8u_i(reference.data(), source_index.data(), length, 0u, 0x1e);
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_set_membership_8u!!! "));
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_set_membership_16u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> buffer{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> source_index{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT16_MAX), seed);

    {
        uint16_t* p_buffer_16u = (uint16_t*)buffer.data();
        uint16_t* p_source_index_16u = (uint16_t*)source_index.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_buffer_16u[indx] = static_cast<uint16_t>(random_value);
        }
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_index_16u[indx] = static_cast<uint16_t>(random_value);
        }
    }
    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        fill_buffer_16u(buffer.data(), destination.data(), TEST_BUFFER_SIZE);
        fill_buffer_16u(buffer.data(), reference.data(), TEST_BUFFER_SIZE);
        qplc_set_membership_i(fun_indx_set_membership_16u)(destination.data(), source_index.data(), length, 0u, 0x1e);
        ref_qplc_set_membership_16u_i(reference.data(), source_index.data(), length, 0u, 0x1e);
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_set_membership_16u!!! "));
    }
    for (uint32_t offset = 0; offset <= 32; offset += sizeof(uint16_t)) {
        for (uint32_t length = 1; length <= (TEST_BUFFER_SIZE >> 1); length++) {
            fill_buffer_16u(buffer.data(), destination.data(), TEST_BUFFER_SIZE);
            fill_buffer_16u(buffer.data(), reference.data(), TEST_BUFFER_SIZE);
            qplc_set_membership_i(fun_indx_set_membership_16u)(destination.data() + offset, source_index.data(), length, 0u, 0xe);
            ref_qplc_set_membership_16u_i(reference.data() + offset, source_index.data(), length, 0u, 0xe);
            ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
                destination.begin(), destination.end(), "FAIL qplc_set_membership_16u!!! "));
        }
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_set_membership_32u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE_32U * sizeof(uint32_t)> buffer{};
    std::array<uint8_t, TEST_BUFFER_SIZE_32U * sizeof(uint32_t)> source_index{};
    std::array<uint8_t, TEST_BUFFER_SIZE_32U * sizeof(uint32_t)> destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE_32U * sizeof(uint32_t)> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT32_MAX), seed);

    {
        uint32_t* p_buffer_32u = (uint32_t*)buffer.data();
        uint32_t* p_source_index_32u = (uint32_t*)source_index.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE_32U; indx++) {
            p_buffer_32u[indx] = static_cast<uint32_t>(random_value);
        }
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE_32U; indx++) {
            p_source_index_32u[indx] = static_cast<uint32_t>(random_value);
        }
    }
    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE_32U; length++) {
        fill_buffer_32u(buffer.data(), destination.data(), TEST_BUFFER_SIZE_32U);
        fill_buffer_32u(buffer.data(), reference.data(), TEST_BUFFER_SIZE_32U);
        qplc_set_membership_i(fun_indx_set_membership_32u)(destination.data(), source_index.data(), length, 0u, 0x1e);
        ref_qplc_set_membership_32u_i(reference.data(), source_index.data(), length, 0u, 0x1e);
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_set_membership_32u!!! "));
    }
    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE_32U; length++) {
        fill_buffer_32u(buffer.data(), destination.data(), TEST_BUFFER_SIZE_32U);
        fill_buffer_32u(buffer.data(), reference.data(), TEST_BUFFER_SIZE_32U);
        qplc_set_membership_i(fun_indx_set_membership_32u)(destination.data(), source_index.data(), length, 0u, 0xe);
        ref_qplc_set_membership_32u_i(reference.data(), source_index.data(), length, 0u, 0xe);
        ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
            destination.begin(), destination.end(), "FAIL qplc_set_membership_32u!!! "));
    }
    for (uint32_t offset = 0; offset <= 64; offset += sizeof(uint32_t)) {
        for (uint32_t length = 1; length <= (TEST_BUFFER_SIZE_32U >> 4); length++) {
            fill_buffer_32u(buffer.data(), destination.data(), TEST_BUFFER_SIZE_32U);
            fill_buffer_32u(buffer.data(), reference.data(), TEST_BUFFER_SIZE_32U);
            qplc_set_membership_i(fun_indx_set_membership_32u)(destination.data() + offset, source_index.data(), length, 0u, 0xe);
            ref_qplc_set_membership_32u_i(reference.data() + offset, source_index.data(), length, 0u, 0xe);
            ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
                destination.begin(), destination.end(), "FAIL qplc_set_membership_32u!!! "));
        }
    }
}
}
