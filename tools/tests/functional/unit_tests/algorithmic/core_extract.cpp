/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/
#include "array"
#include "vector"
#include "gtest/gtest.h"

#include "qpl_test_environment.hpp"
#include "t_common.hpp"
#include "random_generator.h"

#include "qplc_api.h"
#include "dispatcher.hpp"


qplc_extract_t_ptr qplc_extract(uint32_t index) {
    static const auto &table = qpl::core_sw::dispatcher::kernels_dispatcher::get_instance().get_extract_table();

    return (qplc_extract_t_ptr) table[index];
}

qplc_extract_i_t_ptr qplc_extract_i(uint32_t index) {
    static const auto &table = qpl::core_sw::dispatcher::kernels_dispatcher::get_instance().get_extract_i_table();

    return (qplc_extract_i_t_ptr) table[index];
}

static void fill_buffer_8u(uint8_t* src, uint8_t* dst, uint32_t length) {
    uint8_t* p_src_8u = src;
    uint8_t* p_dst_8u = dst;
    for (uint32_t indx = 0; indx < length; indx++) {
        p_dst_8u[indx] = p_src_8u[indx];
    }
}

static uint32_t ref_qplc_extract_8u(const uint8_t* src_ptr,
    uint8_t* dst_ptr,
    uint32_t length,
    uint32_t* index_ptr,
    uint32_t low_value,
    uint32_t high_value)
{
    uint32_t start;
    uint32_t stop;

    if ((*index_ptr + length) < low_value) {
        *index_ptr += length;
        return 0u;
    }
    if (*index_ptr > high_value) {
        return 0u;
    }

    start = (*index_ptr < low_value) ? (low_value - *index_ptr) : 0u;
    stop = ((*index_ptr + length) > high_value) ? (high_value + 1u - *index_ptr) : length;

    src_ptr += start;
    for (uint32_t idx = 0; idx < (stop - start); idx++) {
        dst_ptr[idx] = src_ptr[idx];
    }
    *index_ptr += length;
    return (stop - start);
}

static uint32_t ref_qplc_extract_16u(const uint8_t* src_ptr,
    uint8_t* dst_ptr,
    uint32_t length,
    uint32_t* index_ptr,
    uint32_t low_value,
    uint32_t high_value)
{
    uint32_t       start;
    uint32_t       stop;
    const uint16_t* src_16u_ptr = (uint16_t*)src_ptr;
    uint16_t* dst_16u_ptr = (uint16_t*)dst_ptr;

    if ((*index_ptr + length) < low_value) {
        *index_ptr += length;
        return 0u;
    }
    if (*index_ptr > high_value) {
        return 0u;
    }

    start = (*index_ptr < low_value) ? (low_value - *index_ptr) : 0u;
    stop = ((*index_ptr + length) > high_value) ? (high_value + 1u - *index_ptr) : length;

    src_16u_ptr += start;
    for (uint32_t idx = 0; idx < (stop - start); idx++) {
        dst_16u_ptr[idx] = src_16u_ptr[idx];
    }
    *index_ptr += length;
    return (stop - start);
}

static uint32_t ref_qplc_extract_32u(const uint8_t* src_ptr,
    uint8_t* dst_ptr,
    uint32_t length,
    uint32_t* index_ptr,
    uint32_t low_value,
    uint32_t high_value)
{
    uint32_t       start;
    uint32_t       stop;
    const uint32_t* src_32u_ptr = (uint32_t*)src_ptr;
    uint32_t* dst_32u_ptr = (uint32_t*)dst_ptr;

    if ((*index_ptr + length) < low_value) {
        *index_ptr += length;
        return 0u;
    }
    if (*index_ptr > high_value) {
        return 0u;
    }

    start = (*index_ptr < low_value) ? (low_value - *index_ptr) : 0u;
    stop = ((*index_ptr + length) > high_value) ? (high_value + 1u - *index_ptr) : length;

    src_32u_ptr += start;
    for (uint32_t idx = 0; idx < (stop - start); idx++) {
        dst_32u_ptr[idx] = src_32u_ptr[idx];
    }
    *index_ptr += length;
    return (stop - start);
}

static uint32_t ref_qplc_extract_8u_i(uint8_t* src_dst_ptr,
    uint32_t length,
    uint32_t* index_ptr,
    uint32_t low_value,
    uint32_t high_value)
{
    return ref_qplc_extract_8u(src_dst_ptr, src_dst_ptr, length,
        index_ptr, low_value, high_value);
}

static uint32_t ref_qplc_extract_16u_i(uint8_t* src_dst_ptr,
    uint32_t length,
    uint32_t* index_ptr,
    uint32_t low_value,
    uint32_t high_value)
{
    return ref_qplc_extract_16u(src_dst_ptr, src_dst_ptr, length,
        index_ptr, low_value, high_value);
}

static uint32_t ref_qplc_extract_32u_i(uint8_t* src_dst_ptr,
    uint32_t length,
    uint32_t* index_ptr,
    uint32_t low_value,
    uint32_t high_value)
{
    return ref_qplc_extract_32u(src_dst_ptr, src_dst_ptr, length,
        index_ptr, low_value, high_value);
}

constexpr uint32_t fun_indx_extract_8u = 0;
constexpr uint32_t fun_indx_extract_16u = 1;
constexpr uint32_t fun_indx_extract_32u = 2;

constexpr uint32_t TEST_BUFFER_SIZE = 64u;

namespace qpl::test {
using randomizer = qpl::test::random;

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_extract_8u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> source{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT8_MAX), seed);
    uint32_t    low_value = 21u;
    uint32_t    high_value = 42u;

    {
        uint8_t* p_source_8u = (uint8_t*)source.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_8u[indx] = static_cast<uint8_t>(random_value);
        }
    }

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        for (uint32_t index = 0; index < length; index++) {
            uint32_t index_current = index;
            uint32_t ref_index_current = index;
            destination.fill(0);
            reference.fill(0);
            uint32_t elements_extracted = qplc_extract(fun_indx_extract_8u)(source.data(), destination.data(),
                length, &index_current, low_value, high_value);
            uint32_t ref_elements_extracted = ref_qplc_extract_8u(source.data(), reference.data(),
                length, &ref_index_current, low_value, high_value);
            ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
                destination.begin(), destination.end(), "FAIL qplc_extract_8u!!! "));
            ASSERT_EQ(index_current, ref_index_current);
            ASSERT_EQ(elements_extracted, ref_elements_extracted);
        }
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_extract_16u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> source{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT16_MAX), seed);
    uint32_t    low_value = 21u;
    uint32_t    high_value = 42u;

    {
        uint16_t* p_source_16u = (uint16_t*)source.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_16u[indx] = static_cast<uint16_t>(random_value);
        }
        for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
            for (uint32_t index = 0; index < length; index++) {
                uint32_t index_current = index;
                uint32_t ref_index_current = index;
                destination.fill(0);
                reference.fill(0);
                uint32_t elements_extracted = qplc_extract(fun_indx_extract_16u)(source.data(), destination.data(),
                    length, &index_current, low_value, high_value);
                uint32_t ref_elements_extracted = ref_qplc_extract_16u(source.data(), reference.data(),
                    length, &ref_index_current, low_value, high_value);
                ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
                    destination.begin(), destination.end(), "FAIL qplc_extract_16u!!! "));
                ASSERT_EQ(index_current, ref_index_current);
                ASSERT_EQ(elements_extracted, ref_elements_extracted);
            }
        }
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_extract_32u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> source{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT32_MAX), seed);
    uint32_t    low_value = 21u;
    uint32_t    high_value = 42u;

    {
        uint32_t* p_source_32u = (uint32_t*)source.data();
        for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_32u[indx] = static_cast<uint32_t>(random_value);
        }
        for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
            for (uint32_t index = 0; index < length; index++) {
                uint32_t index_current = index;
                uint32_t ref_index_current = index;
                destination.fill(0);
                reference.fill(0);
                uint32_t elements_extracted = qplc_extract(fun_indx_extract_32u)(source.data(), destination.data(),
                    length, &index_current, low_value, high_value);
                uint32_t ref_elements_extracted = ref_qplc_extract_32u(source.data(), reference.data(),
                    length, &ref_index_current, low_value, high_value);
                ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
                    destination.begin(), destination.end(), "FAIL qplc_extract_32u!!! "));
                ASSERT_EQ(index_current, ref_index_current);
                ASSERT_EQ(elements_extracted, ref_elements_extracted);
            }
        }
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_extract_8u_i, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> source_destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT8_MAX), seed);
    uint32_t    low_value = 21u;
    uint32_t    high_value = 42u;

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        for (uint32_t index = 0; index < length; index++) {
            uint32_t index_current = index;
            uint32_t ref_index_current = index;
            {
                uint8_t* p_source_8u = (uint8_t*)source_destination.data();
                for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
                    p_source_8u[indx] = static_cast<uint8_t>(random_value);
                }
            }
            fill_buffer_8u(source_destination.data(), reference.data(), (uint32_t)source_destination.size());
            uint32_t elements_extracted = qplc_extract_i(fun_indx_extract_8u)(source_destination.data(),
                length, &index_current, low_value, high_value);
            uint32_t ref_elements_extracted = ref_qplc_extract_8u_i(reference.data(),
                length, &ref_index_current, low_value, high_value);
            ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
                source_destination.begin(), source_destination.end(), "FAIL qplc_extract_8u_i!!! "));
            ASSERT_EQ(index_current, ref_index_current);
            ASSERT_EQ(elements_extracted, ref_elements_extracted);
        }
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_extract_16u_i, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> source_destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT16_MAX), seed);
    uint32_t    low_value = 21u;
    uint32_t    high_value = 42u;

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        for (uint32_t index = 0; index < length; index++) {
            uint32_t index_current = index;
            uint32_t ref_index_current = index;
            {
                uint16_t* p_source_16u = (uint16_t*)source_destination.data();
                for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
                    p_source_16u[indx] = static_cast<uint16_t>(random_value);
                }
            }
            fill_buffer_8u(source_destination.data(), reference.data(), (uint32_t)source_destination.size());
            uint32_t elements_extracted = qplc_extract_i(fun_indx_extract_16u)(source_destination.data(),
                length, &index_current, low_value, high_value);
            uint32_t ref_elements_extracted = ref_qplc_extract_16u_i(reference.data(),
                length, &ref_index_current, low_value, high_value);
            ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
                source_destination.begin(), source_destination.end(), "FAIL qplc_extract_16u_i!!! "));
            ASSERT_EQ(index_current, ref_index_current);
            ASSERT_EQ(elements_extracted, ref_elements_extracted);
        }
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_extract_32u_i, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> source_destination{};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> reference{};
    uint64_t seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer         random_value(0u, static_cast<double>(UINT32_MAX), seed);
    uint32_t    low_value = 21u;
    uint32_t    high_value = 42u;

    for (uint32_t length = 1; length <= TEST_BUFFER_SIZE; length++) {
        for (uint32_t index = 0; index < length; index++) {
            uint32_t index_current = index;
            uint32_t ref_index_current = index;
            {
                uint32_t* p_source_32u = (uint32_t*)source_destination.data();
                for (uint32_t indx = 0; indx < TEST_BUFFER_SIZE; indx++) {
                    p_source_32u[indx] = static_cast<uint32_t>(random_value);
                }
            }
            fill_buffer_8u(source_destination.data(), reference.data(), (uint32_t)source_destination.size());
            uint32_t elements_extracted = qplc_extract_i(fun_indx_extract_32u)(source_destination.data(),
                length, &index_current, low_value, high_value);
            uint32_t ref_elements_extracted = ref_qplc_extract_32u_i(reference.data(),
                length, &ref_index_current, low_value, high_value);
            ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(),
                source_destination.begin(), source_destination.end(), "FAIL qplc_extract_32u_i!!! "));
            ASSERT_EQ(index_current, ref_index_current);
            ASSERT_EQ(elements_extracted, ref_elements_extracted);
        }
    }
}
}
