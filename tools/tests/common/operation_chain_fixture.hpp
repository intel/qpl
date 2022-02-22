/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef QPL_TEST_OPERATION_CHAIN_FIXTURE_HPP
#define QPL_TEST_OPERATION_CHAIN_FIXTURE_HPP

#include "qpl/cpp_api/chaining/operation_chain.hpp"

#include <vector>

#include "gtest/gtest.h"
#include "gtest/gtest-param-test.h"

#include "qpl_test_environment.hpp"
#include "source_provider.hpp"
#include "test_cases.hpp"

namespace qpl::test {

struct OperationChainTestCase {
    std::string   file_name = "";
    uint32_t      number_of_input_elements;
    uint32_t      input_bit_width;
    uint32_t      output_bit_width;
};

static std::ostream &operator<<(std::ostream &os, const OperationChainTestCase &test_case) {
    os << "file name: " << test_case.file_name << "\n";
    os << "Number of input elements: " << test_case.number_of_input_elements << "\n";
    os << "Input bit width: " << test_case.input_bit_width << "\n";
    os << "Output bit width: " << test_case.output_bit_width << "\n";
    return os;
}

template<class ...Operations>
class OperationChainFixture : public testing::Test,
                              public TestCases<OperationChainTestCase> {
protected:
    void SetUp() override;

    void SetUpBeforeIteration() override;

    [[nodiscard]] auto GetSeed() const -> uint32_t;

    [[nodiscard]] auto GetExecutionPath() const -> qpl_path_t;

    virtual void SetBuffers();

    void SetParser(parsers newParser);

    std::vector<uint8_t>   source;
    std::vector<uint8_t>   compressed_source;
    std::vector<uint8_t>   destination;
    std::vector<uint8_t>   reference_destination;
    OperationChainTestCase current_test_case;
    parsers                parser = little_endian_packed_array;

private:
    uint32_t    m_seed;
    qpl_path_t m_execution_path;
};

}

#include "operation_chain_fixture.cxx"

#endif // QPL_TEST_OPERATION_CHAIN_FIXTURE_HPP
