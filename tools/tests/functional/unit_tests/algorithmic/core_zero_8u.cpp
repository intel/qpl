/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <algorithm>

#include "dispatcher.hpp"
#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"
#include "qplc_memop.h"
#include "random_generator.h"
#include "t_common.hpp"

namespace qpl::test {

qplc_zero_t_ptr qplc_zero() {
    static const auto& table = qpl::core_sw::dispatcher::kernels_dispatcher::get_instance().get_zero_table();

    return (qplc_zero_t_ptr)table[0U];
}

using TestEnviroment = qpl::test::util::TestEnvironment;
using randomizer     = qpl::test::random;

constexpr uint32_t KB                    = 1024U;
constexpr uint32_t TEST_ARRAY_SIZE_BEGIN = 1U;
constexpr uint32_t TEST_ARRAY_SIZE_END   = 63U;

QPL_UNIT_API_ALGORITHMIC_TEST(qpl_zero_8u, base) {
    randomizer random(TEST_ARRAY_SIZE_BEGIN, TEST_ARRAY_SIZE_END, TestEnviroment::GetInstance().GetSeed());

    std::vector<uint8_t> vector(static_cast<uint32_t>(random) * KB + static_cast<uint32_t>(random));
    std::generate(vector.begin(), vector.end(), [&random]() { return static_cast<uint32_t>(random); });

    qplc_zero()(vector.data(), static_cast<uint32_t>(vector.size()));

    const bool all_elements_is_zero = std::all_of(vector.begin(), vector.end(), [](int i) { return i == 0; });

    EXPECT_TRUE(all_elements_is_zero);
}
} // namespace qpl::test
