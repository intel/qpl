/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TEST_TEST_CASE_HPP
#define QPL_TEST_TEST_CASE_HPP

#include <vector>
#include <type_traits>

#include "gtest/gtest.h"

// tests_common
#include "operation_test.hpp"

namespace qpl::test
{
    /**
    * @briеf Internal class that holds all test cases and responsible for its managing
    */
    template<class TestCase>
    class TestCases
    {
    public:
        /**
        * @briеf Returns current test case
        *
        * @rеturn current test case
        */
        TestCase GetTestCase()
        {
            return *current_test_case_it;
        }

        /**
        * @briеf Adds new test case to the test case pool
        *
        * This method should be used to add new test case for the test
        *
        * @rеturn This method does not return anything
        */
        void AddNewTestCase(TestCase test_case)
        {
            test_cases.push_back(test_case);
            current_test_case_it = test_cases.begin();
        }

        /**
        * @briеf Virtual method to set up data before new test case
        *
        * This method is called right before running new test case,
        * so this should be used to do some preparing before running test
        *
        * @rеturn This method does not return anything
        */
        virtual void SetUpBeforeIteration()
        {
            // Do your setups for the current test case here
        }

    protected:
        TestCases() = default;

        /**
        * @briеf This method is called right after every test iteration
        *
        * Use this method to clean everything after test
        * and prepare your data for the next test iteration
        *
        * @rеturn This method does not return anything
        */
        virtual void CleanUpAfterIteration()
        {
            // Clean up the things you don't want to be seen in the next iteration here
        }

        /**
        * @briеf Switches test case state to the next
        *
        * @rеturn This method does not return anything
        */
        void SwitchToNextCase()
        {
            CleanUpAfterIteration();
            current_test_case_it++;
        }

        /**
        * @briеf Method returns number of all test cases
        *
        * @rеturn Returns count of all test cases
        */
        uint32_t TestCaseCount()
        {
            return static_cast<uint32_t>(test_cases.size());
        }

        void SetSpecifiedTestCase(uint32_t test_case_id)
        {
            current_test_case_it = test_cases.begin() + test_case_id;
        }

    private:
        std::vector<TestCase>                    test_cases;
        typename std::vector<TestCase>::iterator current_test_case_it;
    };

    /**
    * @briеf Class that extends JobFixture with test cases functionality
    *
    * To access JobFixture and test case features inherit this class
    *
    * @tparam TestCase type of single test case
    */
    template<class TestCase>
    class JobFixtureWithTestCases : public JobFixture,
                                    public TestCases<TestCase>
    {
    protected:
        /**
        * @briеf This method initializes qpl_job structure and test cases
        */
        void SetUp() override
        {
            JobFixture::SetUp();
            this->InitializeTestCases();
        }

        /**
        * @briеf Initializes list of test cases inside this method
        *
        * This method should be used BEFORE running test
        * for example you can place it in constructor
        * or SetUp() fixture method. Add all your test cases in this method
        *
        * @rеturn This method does not return anything
        */
        virtual void InitializeTestCases() = 0;

        /**
        * @briеf This method resets qpl_job structure for the next test case
        */
        virtual void CleanUpAfterIteration() override
        {
            JobFixture::TearDown();
            JobFixture::SetUp();
        }
    };

    /**
    * @briеf Class that extends ReferenceFixture with test cases functionality
    *
    * To access ReferenceFixture and test case features inherit this class
    *
    * @tparam TestCase type of single test case
    */
    template<class TestCase>
    class ReferenceFixtureWithTestCases : public ReferenceFixture,
                                          public TestCases<TestCase>
    {
    protected:
        /**
        * @briеf This method initializes qpl_job structures and test cases
        */
        void SetUp() override
        {
            ReferenceFixture::SetUp();
            this->InitializeTestCases();
        }

        /**
        * @briеf Initializes list of test cases inside this method
        *
        * This method should be used BEFORE running test
        * for example you can place it in constructor
        * or SetUp() fixture method. Add all your test cases in this method
        *
        * @rеturn This method does not return anything
        */
        virtual void InitializeTestCases() = 0;

        /**
        * @briеf This method resets qpl_job structure for the next test case
        */
        virtual void CleanUpAfterIteration() override
        {
            ReferenceFixture::TearDown();
            ReferenceFixture::SetUp();
        }
    };
}

/**
* @briеf Internal helper to register new tests
*/
#define QPL_TEST_TC_(test_suite_name, test_name, parent_class, parent_id)                                                   \
    static_assert(sizeof(GTEST_STRINGIFY_(test_suite_name)) > 1,                                                            \
                  "test_suite_name must not be empty");                                                                     \
    static_assert(sizeof(GTEST_STRINGIFY_(test_name)) > 1,                                                                  \
                  "test_name must not be empty");                                                                           \
    class GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)                                                                \
        : public parent_class                                                                                               \
    {                                                                                                                       \
    public:                                                                                                                 \
        GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)                                                                  \
        () {}                                                                                                               \
        ~GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)() override = default;                                           \
        GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) (const GTEST_TEST_CLASS_NAME_(test_suite_name,                   \
                                                                                         test_name) &) = delete;            \
        GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) & operator=(const GTEST_TEST_CLASS_NAME_(test_suite_name,        \
                                                                                                    test_name) &) = delete; \
        GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) & operator=(const GTEST_TEST_CLASS_NAME_(test_suite_name,         \
                                                                                                    test_name) &&) = delete;\
                                                                                                                            \
    private:                                                                                                                \
        void RunSingleIteration();                                                                                          \
        void TestBody() override                                                                                            \
        {                                                                                                                   \
            using environment = util::TestEnvironment;                                                                      \
            uint32_t test_case_id = environment::GetInstance().GetTestCaseId();                                             \
            if (environment::GetInstance().IsTestCaseSpecified())                                                           \
            {                                                                                                               \
                if (test_case_id < TestCaseCount())                                                                         \
                {                                                                                                           \
                    SetSpecifiedTestCase(test_case_id);                                                                     \
                    SetUpBeforeIteration();                                                                                 \
                    RunSingleIteration();                                                                                   \
                    return;                                                                                                 \
                } else                                                                                                      \
                {                                                                                                           \
                    FAIL() << "Note: test case id = "                                                                       \
                           << environment::GetInstance().GetTestCaseId()                                                    \
                           << " was specified, but this test "                                                              \
                           << "contains only " << TestCaseCount()                                                           \
                           << " test cases\n";                                                                              \
                }                                                                                                           \
            }                                                                                                               \
            for (uint32_t i = 0; i < TestCaseCount(); i++)                                                                  \
            {                                                                                                               \
                SetUpBeforeIteration();                                                                                     \
                RunSingleIteration();                                                                                       \
                if (HasFailure()) {                                                                                         \
                    std::cout << "The test case is: " << GetTestCase() << "\n";                                             \
                    std::cout << "Test case id: " << i << "\n";                                                             \
                    std::cout << "To reproduce the fail, run this test with"                                                \
                              << " flag --testid=" << i << "\n";                                                            \
                    break;                                                                                                  \
                } else if (IsSkipped()) {                                                                                   \
                    break;                                                                                                  \
                } else {                                                                                                    \
                    SwitchToNextCase();                                                                                     \
                }                                                                                                           \
            }                                                                                                               \
        }                                                                                                                   \
        static testing::TestInfo *const test_info_ GTEST_ATTRIBUTE_UNUSED_;                                                 \
    };                                                                                                                      \
                                                                                                                            \
    testing::TestInfo *const GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)::test_info_ =                               \
        testing::internal::MakeAndRegisterTestInfo(#test_suite_name, #test_name, nullptr, nullptr,                          \
            testing::internal::CodeLocation(__FILE__, __LINE__), (parent_id),                                               \
            testing::internal::SuiteApiResolver<parent_class>::GetSetUpCaseOrSuite(__FILE__, __LINE__),                     \
            testing::internal::SuiteApiResolver<parent_class>::GetTearDownCaseOrSuite(__FILE__, __LINE__),                  \
            new testing::internal::TestFactoryImpl<GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)>);                    \
    void GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)::RunSingleIteration()

#endif //QPL_TEST_TEST_CASE_HPP
