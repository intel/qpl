/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl_test_environment.hpp"

namespace qpl::test::util {
uint32_t TestEnvironment::GetSeed() const {
    return m_tests_seed;
}

qpl_path_t TestEnvironment::GetExecutionPath() {
    return m_execution_path;
}

std::filesystem::path TestEnvironment::GetDatasetPath() {
    return m_dataset_path;
}

uint32_t TestEnvironment::GetTestCaseId() const {
    return m_test_case_id;
}

bool TestEnvironment::IsTestCaseSpecified() const {
    return m_is_test_case_id_used;
}

bool TestEnvironment::IsAsynchronousApiTesting() const {
    return m_is_async;
}

auto TestEnvironment::GetAlgorithmicDataset() const -> const tools::algorithmic_dataset_t& {
    return *this->m_algorithmic_dataset_ptr.get();
}

auto TestEnvironment::GetCompleteDataset() const -> const tools::dataset_t& {
    return *this->m_complete_dataset_ptr.get();
}

void TestEnvironment::Initialize(const arguments_list_t& arguments_list) {
    if (!m_is_initialized) {
        m_tests_seed           = arguments_list.seed;
        m_execution_path       = arguments_list.execution_path;
        m_is_test_case_id_used = arguments_list.is_test_case_id_used;
        m_test_case_id         = arguments_list.test_case_id;
        m_is_async             = arguments_list.is_async_testing;

        m_is_initialized = true;

        m_dataset_path = arguments_list.path_to_dataset;

        m_complete_dataset_ptr    = std::make_shared<tools::dataset_t>(m_dataset_path.string());
        m_algorithmic_dataset_ptr = std::make_shared<tools::algorithmic_dataset_t>(m_dataset_path.string());
    }
}

TestEnvironment& TestEnvironment::GetInstance() {
    static TestEnvironment instance;
    return instance;
}
} // namespace qpl::test::util
