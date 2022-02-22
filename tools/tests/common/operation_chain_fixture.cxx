/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/cpp_api/operations/compression/inflate_operation.hpp"
#include "qpl/cpp_api/operations/compression/deflate_operation.hpp"

#include "high_level_api_util.hpp"

#include "operation_chain_fixture.hpp"

namespace qpl::test {

template<class ...Operations>
void OperationChainFixture<Operations ...>::SetUp() {
    m_execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();
    m_seed           = util::TestEnvironment::GetInstance().GetSeed();
}

template<class ...Operations>
void OperationChainFixture<Operations ...>::SetUpBeforeIteration() {
    current_test_case = GetTestCase();

    SetBuffers();
}

template<class ...Operations>
auto OperationChainFixture<Operations ...>::GetSeed() const -> uint32_t {
    return m_seed;
}

template<class ...Operations>
auto OperationChainFixture<Operations ...>::GetExecutionPath() const -> qpl_path_t {
    return m_execution_path;
}

template<class ...Operations>
void OperationChainFixture<Operations ...>::SetParser(parsers newParser) {
    parser = newParser;
}

template<class ...Operations>
void OperationChainFixture<Operations ...>::SetBuffers() {
    if (current_test_case.file_name != "") {
        ASSERT_NO_THROW(source = util::TestEnvironment::GetInstance().GetAlgorithmicDataset()[current_test_case.file_name]);
        current_test_case.number_of_input_elements = static_cast<uint32_t>(source.size());
    } else {
        source_provider source_gen(current_test_case.number_of_input_elements,
                                   current_test_case.input_bit_width,
                                   GetSeed(),
                                   convert_parser(parser));
        ASSERT_NO_THROW(source = source_gen.get_source());
    }


    if constexpr (contains_operation<inflate_operation, Operations ...>()) {
        std::vector<uint8_t> temporary_buffer(source.size() + additional_bytes_for_compression);
        auto compression_operation = deflate_operation::builder()
                .compression_level(high_level)
                .build();
    
        auto compression_result = test::execute(compression_operation, source, temporary_buffer);

        uint32_t compression_result_value = 0u;
        ASSERT_NO_THROW(compression_result_value = handle_result(compression_result));

        compressed_source.resize(compression_result_value);
        std::memcpy(compressed_source.data(), temporary_buffer.data(), compression_result_value);
    }

    size_t destination_size = get_destination_size<Operations ...>(current_test_case);

    std::fill(destination.begin(), destination.end(), 0);
    std::fill(reference_destination.begin(), reference_destination.end(), 0);

    destination.resize(destination_size, 0);
    reference_destination.resize(destination_size, 0);
}

} // namespace qpl::test
