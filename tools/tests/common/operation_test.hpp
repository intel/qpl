/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TEST_OPERATION_TEST
#define QPL_TEST_OPERATION_TEST

#include "qpl/qpl.h"

#include "gtest/gtest-param-test.h"
#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"

namespace qpl::test {
/**
    * @briеf Auxiliary class for JobAPI testing
    *
    * This class is capable of allocating/deallocating and initializing qpl_job structure,
    * it also covers basic test function needs.
    */
class JobFixture : public testing::Test {
public:
    int32_t num_test = 0;

protected:
    /**
        * @briеf Allocates and initializes qpl_job structure, sets seed and execution path
        */
    void SetUp() override {
        m_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();
        m_seed           = util::TestEnvironment::GetInstance().GetSeed();

        uint32_t job_size = 0U;

        auto status = qpl_get_job_size(m_execution_path, &job_size);
        ASSERT_EQ(QPL_STS_OK, status);

        m_job_buffer = new uint8_t[job_size];
        job_ptr      = reinterpret_cast<qpl_job*>(m_job_buffer);

        status = qpl_init_job(m_execution_path, job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);
    }

    /**
        * @briеf Deallocates qpl_job structure
        */
    void TearDown() override {
        qpl_fini_job(job_ptr);
        delete[] m_job_buffer;
    }

    /**
        * @briеf Returns seed
        */
    [[nodiscard]] uint32_t GetSeed() const { return m_seed; }

    /**
        * @briеf Returns execution path
        */
    qpl_path_t GetExecutionPath() { return m_execution_path; }

    /**
        * Following fields are visible from test level
        */
    qpl_job*             job_ptr;
    std::vector<uint8_t> source;
    std::vector<uint8_t> destination;

private:
    uint8_t*   m_job_buffer = nullptr;
    uint32_t   m_seed;
    qpl_path_t m_execution_path;
};

/**
    * @brief Extends JobFixture for testing with reference code
    */
class ReferenceFixture : public JobFixture {
protected:
    /**
        * @briеf Allocates and initializes qpl_job structure for reference
        */
    void SetUp() override {
        JobFixture::SetUp();

        const uint32_t job_size = sizeof(qpl_job);

        m_reference_job_buffer = new uint8_t[job_size];
        memset(m_reference_job_buffer, 0U, job_size);
        reference_job_ptr = reinterpret_cast<qpl_job*>(m_reference_job_buffer);
    }

    /**
        * @briеf Deallocates qpl_job structure for reference
        */
    void TearDown() override {
        JobFixture::TearDown();
        delete[] m_reference_job_buffer;
    }

    /**
        * @briеf Compares total_in and total_out fields for main and reference job
        *
        * @return
        * - AssertionSuccess - success
        * - AssertionFailure - failure
        */
    testing::AssertionResult CompareTotalInOutWithReference() {
        if (job_ptr->total_out != reference_job_ptr->total_out) {
            return testing::AssertionFailure() << "Reference total out " << reference_job_ptr->total_out
                                               << " library total out " << job_ptr->total_out;
        }

        if (job_ptr->total_in != reference_job_ptr->total_in) {
            return testing::AssertionFailure()
                   << "Reference total in " << reference_job_ptr->total_in << " library total in " << job_ptr->total_in;
        }

        return testing::AssertionSuccess();
    }

    /**
        * Following fields are visible from test level
        */
    qpl_job*             reference_job_ptr;
    std::vector<uint8_t> reference_destination;

private:
    uint8_t* m_reference_job_buffer;
};

#define QPL_FIXTURE_TEST(test_suite, test_name, fixture) \
    GTEST_TEST_(test_suite, test_name, fixture, testing::internal::GetTypeId<fixture>())
} // namespace qpl::test

#endif //QPL_TEST_OPERATION_TEST
