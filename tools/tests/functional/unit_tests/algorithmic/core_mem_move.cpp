/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <algorithm>
#include <array>

#include "dispatcher.hpp"
#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"
#include "qplc_memop.h"
#include "random_generator.h"
#include "t_common.hpp"

using TestEnviroment = qpl::test::util::TestEnvironment;
using randomizer     = qpl::test::random;

constexpr uint32_t TEST_ARRAY_SIZE  = 4U * 1024U;
constexpr uint32_t TEST_SOURCE_SIZE = TEST_ARRAY_SIZE / 2U;
constexpr uint32_t TEST_SPAN_SIZE   = TEST_ARRAY_SIZE / 4U;

static inline qplc_move_t_ptr move() {
    static const auto& table = qpl::core_sw::dispatcher::kernels_dispatcher::get_instance().get_move_table();

    return (qplc_move_t_ptr)table[0U];
}

QPL_UNIT_API_ALGORITHMIC_TEST(own_move_8u, forward_direction) {
    auto       seed = TestEnviroment::GetInstance().GetSeed();
    randomizer random_8u(0, UINT8_MAX, seed);

    std::array<uint8_t, TEST_SOURCE_SIZE> reference {};
    std::array<uint8_t, TEST_ARRAY_SIZE>  buffer {};
    std::generate(reference.begin(), reference.end(), [&random_8u]() { return static_cast<uint8_t>(random_8u); });

    uint8_t* source_ptr      = buffer.data();
    uint8_t* destination_ptr = buffer.data() + TEST_SPAN_SIZE;

    std::copy(reference.begin(), reference.end(), source_ptr);

    move()(source_ptr, destination_ptr, TEST_SOURCE_SIZE);

    ASSERT_TRUE(std::equal(reference.begin(), reference.end(), destination_ptr));
}

QPL_UNIT_API_ALGORITHMIC_TEST(own_move_8u, backward_direction) {
    auto       seed = TestEnviroment::GetInstance().GetSeed();
    randomizer random_8u(0, UINT8_MAX, seed);

    std::array<uint8_t, TEST_SOURCE_SIZE> reference {};
    std::array<uint8_t, TEST_ARRAY_SIZE>  buffer {};
    std::generate(reference.begin(), reference.end(), [&random_8u]() { return static_cast<uint8_t>(random_8u); });

    uint8_t* destination_ptr = buffer.data();
    uint8_t* source_ptr      = buffer.data() + TEST_SPAN_SIZE;

    std::copy(reference.begin(), reference.end(), source_ptr);

    move()(source_ptr, destination_ptr, TEST_SOURCE_SIZE);

    ASSERT_TRUE(std::equal(reference.begin(), reference.end(), destination_ptr));
}
