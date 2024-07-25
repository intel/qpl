/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TEST_ENVIRONMENT
#define QPL_TEST_ENVIRONMENT

#include <cstdint>
#include <filesystem>

#include "qpl/c_api/defs.h"

#include "algorithmic_dataset.hpp"
#include "arguments_list.hpp"
#include "dataset.hpp"

namespace qpl::test::util {
/**
    * @briеf Singleton object that is capable of sharing common resources at program level
    */
class TestEnvironment {
public:
    /**
        * @briеf Method to get singleton object
        *
        * @return instance of this object
        */
    static TestEnvironment& GetInstance();

    /**
        * @briеf This method returns seed
        *
        * Seed is set once and then is the same for all tests
        */
    uint32_t GetSeed() const;

    uint32_t GetTestCaseId() const;

    bool IsTestCaseSpecified() const;

    bool IsAsynchronousApiTesting() const;

    /**
        * @briеf Returns dataset used for algorithmic test
        */
    auto GetAlgorithmicDataset() const -> const tools::algorithmic_dataset_t&;

    /**
        * @briеf Returns dataset which contains all files from specified dataset folder
        */
    auto GetCompleteDataset() const -> const tools::dataset_t&;

    /**
        * @briеf This method returns execution path
        */
    qpl_path_t GetExecutionPath();

    /**
        * @briеf This method returns path to dataset
        */
    std::filesystem::path GetDatasetPath();

    /**
        * @briеf This method initializes variables that can be received through command line
        */
    void Initialize(const arguments_list_t& arguments_list);

    TestEnvironment(TestEnvironment const&) = delete;

    TestEnvironment(TestEnvironment&&) = delete;

    TestEnvironment& operator=(TestEnvironment const&) = delete;

    TestEnvironment& operator=(TestEnvironment&&) = delete;

private:
    TestEnvironment() = default;

    bool                                          m_is_initialized       = false;
    bool                                          m_is_test_case_id_used = false;
    bool                                          m_is_async             = false;
    uint32_t                                      m_tests_seed           = 0;
    uint32_t                                      m_test_case_id         = 0;
    qpl_path_t                                    m_execution_path       = qpl_path_auto;
    std::filesystem::path                         m_dataset_path         = "";
    std::shared_ptr<tools::dataset_t>             m_complete_dataset_ptr;
    std::shared_ptr<tools::algorithmic_dataset_t> m_algorithmic_dataset_ptr;
};
} // namespace qpl::test::util

#endif //QPL_TEST_ENVIRONMENT
