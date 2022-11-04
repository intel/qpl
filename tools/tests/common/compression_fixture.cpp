/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/cpp_api/operations/compression/inflate_operation.hpp"

#include "qpl_test_environment.hpp"
#include "source_provider.hpp"
#include "high_level_api_util.hpp"

#include "compression_fixture.hpp"

namespace qpl::test {

void CompressionFixture::SetUp() {
    m_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();
    m_seed           = util::TestEnvironment::GetInstance().GetSeed();

    chunk_size_generator = random(min_chunk_size, max_chunk_size, m_seed);
}

void CompressionFixture::SetUpBeforeIteration() {
    current_test_case = GetTestCase();

    SetBuffers();
}

auto CompressionFixture::GetSeed() const -> uint32_t {
    return m_seed;
}

auto CompressionFixture::GetExecutionPath() const -> qpl_path_t {
    return m_execution_path;
}

void CompressionFixture::SetBuffers() {
    auto dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();

    ASSERT_NO_THROW(source = dataset[current_test_case.file_name]);

    size_t destination_size = source.size();

    std::fill(destination.begin(), destination.end(), 0);
    std::fill(reference_destination.begin(), reference_destination.end(), 0);

    destination.resize(destination_size, 0);
    reference_destination.resize(destination_size, 0);
}

} // namespace qpl::test
