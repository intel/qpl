/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TEST_COMPRESSION_CHAIN_FIXTURE_HPP
#define QPL_TEST_COMPRESSION_CHAIN_FIXTURE_HPP

#include <vector>

#include "qpl/cpp_api/operations/compression_operation.hpp"

#include "gtest/gtest.h"
#include "gtest/gtest-param-test.h"

#include "qpl_test_environment.hpp"
#include "source_provider.hpp"
#include "test_cases.hpp"

namespace qpl::test {

class CompressionChainFixture : public testing::Test,
                                public TestCases<std::string> {
protected:
    void SetUp() override;

    void SetUpBeforeIteration() override;

    [[nodiscard]] auto GetSeed() const -> uint32_t;

    [[nodiscard]] auto GetExecutionPath() const -> qpl_path_t;

    virtual void SetBuffers();

    std::vector<uint8_t>     source;
    std::vector<uint8_t>     destination;
    std::vector<uint8_t>     reference_destination;
    std::vector<uint8_t>     compressed_destination;
    std::vector<uint8_t>     compressed_source;
    std::string              current_file_name;

private:
    uint32_t    m_seed;
    qpl_path_t m_execution_path;
};

}

#endif // QPL_TEST_COMPRESSION_CHAIN_FIXTURE_HPP
