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
#include "util.hpp"

static inline qplc_pack_vector_t_ptr qplc_pack_vector(uint32_t index) {
    static const auto& table = qpl::core_sw::dispatcher::kernels_dispatcher::get_instance().get_pack_index_table();

    return (qplc_pack_vector_t_ptr)table[index];
}

static uint32_t ref_qplc_pack_index_8u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t** pp_dst,
                                       uint32_t dst_length, uint32_t* index_ptr) {
    uint32_t index   = *index_ptr;
    uint32_t status  = QPLC_STS_OK;
    uint8_t* dst_ptr = (uint8_t*)*pp_dst;
    uint8_t* end_ptr = dst_ptr + dst_length;

    for (uint32_t i = 0U; i < num_elements; i++) {
        if (0U < src_ptr[i]) {
            if (UINT8_MAX < index) {
                status = QPLC_STS_OUTPUT_OVERFLOW_ERR;
                break;
            }
            if (dst_ptr >= end_ptr) {
                status = QPLC_STS_DST_IS_SHORT_ERR;
                break;
            }
            *dst_ptr = (uint8_t)index;
            dst_ptr++;
        }
        index++;
    }

    *pp_dst    = dst_ptr;
    *index_ptr = index;
    return status;
}

static uint32_t ref_qplc_pack_index_8u16u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t** pp_dst,
                                          uint32_t dst_length, uint32_t* index_ptr) {
    uint32_t  index   = *index_ptr;
    uint32_t  status  = QPLC_STS_OK;
    uint16_t* dst_ptr = (uint16_t*)*pp_dst;
    uint16_t* end_ptr = dst_ptr + (dst_length >> 1);

    for (uint32_t i = 0U; i < num_elements; i++) {
        if (0U < src_ptr[i]) {
            if (UINT16_MAX < index) {
                status = QPLC_STS_OUTPUT_OVERFLOW_ERR;
                break;
            }
            if (dst_ptr >= end_ptr) {
                status = QPLC_STS_DST_IS_SHORT_ERR;
                break;
            }
            *dst_ptr = (uint16_t)index;
            dst_ptr++;
        }
        index++;
    }
    *pp_dst    = (uint8_t*)dst_ptr;
    *index_ptr = index;
    return status;
}

static uint32_t ref_qplc_pack_index_8u32u(const uint8_t* src_ptr, uint32_t num_elements, uint8_t** pp_dst,
                                          uint32_t dst_length, uint32_t* index_ptr) {
    uint64_t  index   = (uint64_t)*index_ptr;
    uint32_t  status  = QPLC_STS_OK;
    uint32_t* dst_ptr = (uint32_t*)*pp_dst;
    uint32_t* end_ptr = dst_ptr + (dst_length >> 2);

    for (uint32_t i = 0U; i < num_elements; i++) {
        if (0U < src_ptr[i]) {
            if (UINT32_MAX < index) {
                status = QPLC_STS_OUTPUT_OVERFLOW_ERR;
                break;
            }
            if (dst_ptr >= end_ptr) {
                status = QPLC_STS_DST_IS_SHORT_ERR;
                break;
            }
            *dst_ptr = (uint32_t)index;
            dst_ptr++;
        }
        index++;
    }
    *pp_dst    = (uint8_t*)dst_ptr;
    *index_ptr = (uint32_t)index;
    return status;
}

constexpr uint32_t TEST_BUFFER_SIZE        = 128U;
constexpr uint32_t fun_indx_pack_idx_8u    = 1U;
constexpr uint32_t fun_indx_pack_idx_8u16u = 2U;
constexpr uint32_t fun_indx_pack_idx_8u32u = 3U;

namespace qpl::test {
using randomizer = qpl::test::random;
QPL_UNIT_API_ALGORITHMIC_TEST(qplc_pack_idx_8u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> source {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)> reference {};
    const uint64_t                                          seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer                                              random_value(0U, static_cast<double>(UINT8_MAX), seed);

    {
        uint8_t* p_source_8u = (uint8_t*)source.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_8u[indx] = 1 & static_cast<uint8_t>(random_value);
        }
    }
    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        for (uint32_t length_dst = 1U; length_dst <= TEST_BUFFER_SIZE; length_dst++) {
            destination.fill(0);
            reference.fill(0);
            uint8_t* p_dst_8u     = destination.data();
            uint8_t* p_ref_dst_8u = reference.data();
            uint32_t index        = 0U;
            uint32_t ref_index    = 0U;

            const uint32_t status = (uint32_t)qplc_pack_vector(fun_indx_pack_idx_8u)(source.data(), length, &p_dst_8u,
                                                                                     length_dst, &index);
            const uint32_t ref_status =
                    (uint32_t)ref_qplc_pack_index_8u(source.data(), length, &p_ref_dst_8u, length_dst, &ref_index);
            ASSERT_EQ(status, ref_status);
            ASSERT_EQ(index, ref_index);
            ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
            ASSERT_TRUE(std::equal(reference.begin(), reference.end(), destination.data()));
            ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                        "FAIL qplc_pack_idx_8u!!! "));
        }
        for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
            for (uint32_t length_dst = 1U; length_dst <= TEST_BUFFER_SIZE; length_dst++) {
                destination.fill(0);
                reference.fill(0);
                uint8_t* p_dst_16u     = destination.data();
                uint8_t* p_ref_dst_16u = reference.data();
                uint32_t index         = UINT8_MAX - 32U;
                uint32_t ref_index     = UINT8_MAX - 32U;

                const uint32_t status = (uint32_t)qplc_pack_vector(fun_indx_pack_idx_8u)(
                        source.data(), length, &p_dst_16u, length_dst, &index);
                const uint32_t ref_status =
                        (uint32_t)ref_qplc_pack_index_8u(source.data(), length, &p_ref_dst_16u, length_dst, &ref_index);
                ASSERT_EQ(status, ref_status);
                ASSERT_EQ(index, ref_index);
                ASSERT_EQ((p_dst_16u - (uint8_t*)destination.data()), (p_ref_dst_16u - (uint8_t*)reference.data()));
                ASSERT_TRUE(std::equal(reference.begin(), reference.end(), destination.data()));
                ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                            "FAIL qplc_pack_idx_8u!!! "));
            }
        }
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_pack_idx_8u16u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)>  source {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint16_t)> reference {};
    const uint64_t                                           seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer                                               random_value(0U, static_cast<double>(UINT8_MAX), seed);

    {
        uint8_t* p_source_8u = (uint8_t*)source.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_8u[indx] = 1 & static_cast<uint8_t>(random_value);
        }
    }
    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        for (uint32_t length_dst = 1U; length_dst <= TEST_BUFFER_SIZE; length_dst++) {
            destination.fill(0);
            reference.fill(0);
            uint8_t* p_dst_8u     = destination.data();
            uint8_t* p_ref_dst_8u = reference.data();
            uint32_t index        = 0U;
            uint32_t ref_index    = 0U;

            const uint32_t status = (uint32_t)qplc_pack_vector(fun_indx_pack_idx_8u16u)(source.data(), length,
                                                                                        &p_dst_8u, length_dst, &index);
            const uint32_t ref_status =
                    (uint32_t)ref_qplc_pack_index_8u16u(source.data(), length, &p_ref_dst_8u, length_dst, &ref_index);
            ASSERT_EQ(status, ref_status);
            ASSERT_EQ(index, ref_index);
            ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
            ASSERT_TRUE(std::equal(reference.begin(), reference.end(), destination.data()));
            ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                        "FAIL qplc_pack_idx_8u16u!!! "));
        }
    }
    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        for (uint32_t length_dst = 1U; length_dst <= TEST_BUFFER_SIZE; length_dst++) {
            destination.fill(0);
            reference.fill(0);
            uint8_t* p_dst_8u     = destination.data();
            uint8_t* p_ref_dst_8u = reference.data();
            uint32_t index        = UINT16_MAX - 32U;
            uint32_t ref_index    = UINT16_MAX - 32U;

            const uint32_t status = (uint32_t)qplc_pack_vector(fun_indx_pack_idx_8u16u)(source.data(), length,
                                                                                        &p_dst_8u, length_dst, &index);
            const uint32_t ref_status =
                    (uint32_t)ref_qplc_pack_index_8u16u(source.data(), length, &p_ref_dst_8u, length_dst, &ref_index);
            ASSERT_EQ(status, ref_status);
            ASSERT_EQ(index, ref_index);
            ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
            ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                        "FAIL qplc_pack_idx_8u16u!!! "));
        }
    }
}

QPL_UNIT_API_ALGORITHMIC_TEST(qplc_pack_idx_8u32u, base) {
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint8_t)>  source {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> destination {};
    std::array<uint8_t, TEST_BUFFER_SIZE * sizeof(uint32_t)> reference {};
    const uint64_t                                           seed = util::TestEnvironment::GetInstance().GetSeed();
    randomizer                                               random_value(0U, static_cast<double>(UINT8_MAX), seed);

    {
        uint8_t* p_source_8u = (uint8_t*)source.data();
        for (uint32_t indx = 0U; indx < TEST_BUFFER_SIZE; indx++) {
            p_source_8u[indx] = 1 & static_cast<uint8_t>(random_value);
        }
    }
    for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
        for (uint32_t length_dst = 1U; length_dst <= TEST_BUFFER_SIZE; length_dst++) {
            destination.fill(0);
            reference.fill(0);
            uint8_t* p_dst_8u     = destination.data();
            uint8_t* p_ref_dst_8u = reference.data();
            uint32_t index        = 0U;
            uint32_t ref_index    = 0U;

            const uint32_t status = (uint32_t)qplc_pack_vector(fun_indx_pack_idx_8u32u)(source.data(), length,
                                                                                        &p_dst_8u, length_dst, &index);
            const uint32_t ref_status =
                    (uint32_t)ref_qplc_pack_index_8u32u(source.data(), length, &p_ref_dst_8u, length_dst, &ref_index);
            ASSERT_EQ(status, ref_status);
            ASSERT_EQ(index, ref_index);
            ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
            ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                        "FAIL qplc_pack_idx_8u32u!!! "));
        }
        for (uint32_t length = 1U; length <= TEST_BUFFER_SIZE; length++) {
            for (uint32_t length_dst = 1U; length_dst <= TEST_BUFFER_SIZE; length_dst++) {
                destination.fill(0);
                reference.fill(0);
                uint8_t* p_dst_8u     = destination.data();
                uint8_t* p_ref_dst_8u = reference.data();
                uint32_t index        = UINT32_MAX - 32U;
                uint32_t ref_index    = UINT32_MAX - 32U;

                const uint32_t status = (uint32_t)qplc_pack_vector(fun_indx_pack_idx_8u32u)(
                        source.data(), length, &p_dst_8u, length_dst, &index);
                const uint32_t ref_status = (uint32_t)ref_qplc_pack_index_8u32u(source.data(), length, &p_ref_dst_8u,
                                                                                length_dst, &ref_index);
                ASSERT_EQ(status, ref_status);
                ASSERT_EQ(index, ref_index);
                ASSERT_EQ((p_dst_8u - (uint8_t*)destination.data()), (p_ref_dst_8u - (uint8_t*)reference.data()));
                ASSERT_TRUE(CompareSegments(reference.begin(), reference.end(), destination.begin(), destination.end(),
                                            "FAIL qplc_pack_idx_8u32u!!! "));
            }
        }
    }
}
} // namespace qpl::test
