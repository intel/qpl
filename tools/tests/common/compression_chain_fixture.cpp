/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/cpp_api/operations/compression/inflate_operation.hpp"
#include "qpl/cpp_api/operations/compression/deflate_operation.hpp"

#include "qpl_test_environment.hpp"
#include "source_provider.hpp"
#include "high_level_api_util.hpp"

#include "compression_chain_fixture.hpp"

namespace qpl::test {

void CompressionChainFixture::SetUp() {
    m_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();
    m_seed           = util::TestEnvironment::GetInstance().GetSeed();
}

void CompressionChainFixture::SetUpBeforeIteration() {
    current_file_name = GetTestCase();

    SetBuffers();
}

auto CompressionChainFixture::GetSeed() const -> uint32_t {
    return m_seed;
}

auto CompressionChainFixture::GetExecutionPath() const -> qpl_path_t {
    return m_execution_path;
}

void CompressionChainFixture::SetBuffers() {
    ASSERT_NO_THROW(source = util::TestEnvironment::GetInstance().GetAlgorithmicDataset()[current_file_name]);

    size_t destination_size = source.size();

    std::fill(destination.begin(), destination.end(), 0);
    std::fill(reference_destination.begin(), reference_destination.end(), 0);
    std::fill(compressed_destination.begin(), compressed_destination.end(), 0);
    std::fill(compressed_source.begin(), compressed_source.end(), 0);

    destination.resize(destination_size, 0);
    compressed_destination.resize(destination_size + additional_bytes_for_compression, 0);
    compressed_source.resize(destination_size, 0);
    reference_destination.resize(destination_size, 0);

    deflate_operation deflate_operation;
    auto compression_result = test::execute(deflate_operation, source, compressed_source);

    uint32_t compression_result_value = 0u;
    ASSERT_NO_THROW(compression_result_value = handle_result(compression_result));

    compressed_source.resize(compression_result_value);
}

} // namespace qpl::test
