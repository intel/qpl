/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TEST_OPERATION_TEST
#define QPL_TEST_OPERATION_TEST

#include "gtest/gtest.h"
#include "gtest/gtest-param-test.h"

#include "qpl_test_environment.hpp"

#include "qpl/qpl.h"

namespace qpl::test
{
    /**
    * @briеf Auxiliary class for JobAPI testing 
    * 
    * This class is capable of allocating/deallocating and initializing qpl_job structure,
    * it also covers basic test function needs.
    */
    class JobFixture : public testing::Test
    {
    public:
        int32_t num_test = 0;

    protected:
        /**
        * @briеf Allocates and initializes qpl_job structure, sets seed and execution path
        */
        void SetUp() override
        {
            m_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();
            m_seed           = util::TestEnvironment::GetInstance().GetSeed();

            uint32_t job_size = 0;

            auto status = qpl_get_job_size(m_execution_path, &job_size);
            ASSERT_EQ(QPL_STS_OK, status);

            m_job_buffer = new uint8_t[job_size];
            job_ptr      = reinterpret_cast<qpl_job *>(m_job_buffer);

            status = qpl_init_job(m_execution_path, job_ptr);
            ASSERT_EQ(QPL_STS_OK, status);
        }

        /**
        * @briеf Deallocates qpl_job structure
        */
        void TearDown() override
        {
            qpl_fini_job(job_ptr);
            delete[] m_job_buffer;
        }

        /**
        * @briеf Returns seed
        */
        [[nodiscard]] uint32_t GetSeed() const
        {
            return m_seed;
        }

        /**
        * @briеf Returns execution path
        */
        qpl_path_t GetExecutionPath()
        {
            return m_execution_path;
        }

        /*
        * @brief Function to compare two vectors
        *
        * @param[in] first                         - First vector to compare
        * @param[in] second                        - Second vector to compare
        * @param[in] number_of_elements_to_compare - Only this number of elements will be compare, default value 0 means compare all vector elements
        * @param[in] fail_message                  - Message that should provide user with some information about what 
        * @return 
        * - AssertionSuccess - success
        * - AssertionFailure - failure
        */
        template<typename DataType>
        testing::AssertionResult CompareVectors(const std::vector<DataType> &first,
                                                const std::vector<DataType> &second,
                                                const uint32_t number_of_elements_to_compare = 0u,
                                                const std::string &fail_message = "")
        {
            if (0 == number_of_elements_to_compare)
            {
                if (first.size() != second.size())
                {
                    return testing::AssertionFailure() << fail_message << ", "
                                                       << "vectors have different length";
                }
            } else
            {
                if (first.size() < number_of_elements_to_compare)
                {
                    return testing::AssertionFailure() << fail_message << ", "
                                                       << "first vector has less elements, that required";
                }

                if (second.size() < number_of_elements_to_compare)
                {
                    return testing::AssertionFailure() << fail_message << ", "
                                                       << "second vector has less elements, that required";
                }
            }

            uint32_t index_bound = (number_of_elements_to_compare == 0u) ? static_cast<uint32_t>(first.size())
                                                                         : number_of_elements_to_compare;

            for (uint32_t i = 0; i < index_bound; i++)
            {
                if (first[i] != second[i])
                {
                    return testing::AssertionFailure() << fail_message << ", "
                                                       << "vectors differ at " << i << " index (" << (uint32_t) first[i] << " , "
                                                       << (uint32_t) second[i] << ")";
                }
            }

            return testing::AssertionSuccess();
        }

        /**
        * Following fields are visible from test level
        */
        qpl_job               *job_ptr;
        std::vector<uint8_t> source;
        std::vector<uint8_t> destination;

    private:
        uint8_t     *m_job_buffer = nullptr;
        uint32_t    m_seed;
        qpl_path_t m_execution_path;
    };

    /**
    * @brief Extends JobFixture for testing with reference code
    */
    class ReferenceFixture : public JobFixture
    {
    protected:
        /**
        * @briеf Allocates and initializes qpl_job structure for reference
        */
        void SetUp() override
        {
            JobFixture::SetUp();

            uint32_t job_size = sizeof(qpl_job);

            m_reference_job_buffer = new uint8_t[job_size];
            memset(m_reference_job_buffer, 0u, job_size);
            reference_job_ptr = reinterpret_cast<qpl_job *>(m_reference_job_buffer);
        }

        /**
        * @briеf Deallocates qpl_job structure for reference
        */
        void TearDown() override
        {
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
        testing::AssertionResult CompareTotalInOutWithReference()
        {
            if (job_ptr->total_out != reference_job_ptr->total_out)
            {
                return testing::AssertionFailure() << "Reference total out " << reference_job_ptr->total_out
                                                   << " library total out " << job_ptr->total_out;
            }

            if (job_ptr->total_in != reference_job_ptr->total_in)
            {
                return testing::AssertionFailure() << "Reference total in " << reference_job_ptr->total_in
                                                   << " library total in " << job_ptr->total_in;
            }

            return testing::AssertionSuccess();
        }

        /**
        * Following fields are visible from test level
        */
        qpl_job               *reference_job_ptr;
        std::vector<uint8_t> reference_destination;

    private:
        uint8_t *m_reference_job_buffer;
    };

#define QPL_FIXTURE_TEST(test_suite, test_name, fixture) \
        GTEST_TEST_(test_suite, test_name, fixture, \
                    testing::internal::GetTypeId<fixture>())
}

#endif //QPL_TEST_OPERATION_TEST
