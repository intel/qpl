/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TESTS_CRC_TESTS_BASE_CRC_TEST_FIXTURE_HPP_
#define QPL_TESTS_CRC_TESTS_BASE_CRC_TEST_FIXTURE_HPP_

#include <stdexcept>

#include "qpl/qpl.h"

// tests_common
#include "execution_wrapper.hpp"
#include "operation_test.hpp"
#include "test_cases.hpp"

namespace qpl::test {
class BaseCRCTestFixture : public testing::Test {
protected:
    /**
         * @briеf Allocates and initializes qpl_job structures for HW and SW and sets seed
         */
    void SetUp() override;

    /**
        * @briеf Deallocates qpl_job structure
        */
    void TearDown() override;

    uint32_t GetSeed() const;

    qpl_job*             sw_job_ptr;
    std::vector<uint8_t> sw_source;
    std::vector<uint8_t> sw_destination;

    qpl_job*             hw_job_ptr;
    std::vector<uint8_t> hw_source;

private:
    void SetUpSoftwareJob();

    void SetUpHardwareJob();

    uint32_t m_seed;
    uint8_t* m_sw_job_buffer;
    uint8_t* m_hw_job_buffer;
};

#define ASSERT_ERR_STATUS(status) \
    if (status != QPL_STS_OK) return testing::AssertionFailure()

#define QPL_LOW_LEVEL_API_CRC_TEST(test_suite_name, test_fixture, test_name) \
    GTEST_TEST_(ll_##test_suite_name, tc_##test_name, test_fixture, testing::internal::GetTypeId<test_fixture>())

#define QPL_LOW_LEVEL_API_CRC_TEST_TC(test_suite_name, test_fixture, test_name) \
    QPL_TEST_TC_(ll_##test_suite_name, test_name, test_fixture, testing::internal::GetTypeId<test_fixture>())

} // namespace qpl::test

#endif //QPL_TESTS_CRC_TESTS_BASE_CRC_TEST_FIXTURE_HPP_
