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

void ZeroCompressionFixture::SetUp() {
    JobFixture::SetUp();
    this->InitializeTestCases();
}

void ZeroCompressionFixture::SetUpBeforeIteration() {
    current_test_case = GetTestCase();

    SetBuffers();
}

void ZeroCompressionFixture::SetBuffers() {
    uint32_t source_size = 0;
    if (current_test_case.input_format == zero_input_format::word_16_bit) {
        source_size = current_test_case.number_of_input_elements * sizeof(uint16_t);
    } else {
        source_size = current_test_case.number_of_input_elements * sizeof(uint32_t);
    }

    source.resize(source_size);

    auto compressed_buffer_size = source_size * 2;

    switch (current_test_case.vector_generation_type) {
        case zero_vector_generation_type::generated_vector: {
            source_provider source_gen(source_size,
                                       8,
                                       GetSeed());

            ASSERT_NO_THROW(source = source_gen.get_source());

            break;
        }
        case zero_vector_generation_type::random_vector: {
            auto elements_count = current_test_case.number_of_input_elements;

            ASSERT_FALSE(source.empty());

            if (current_test_case.input_format == zero_input_format::word_16_bit) {
                qpl::test::random random_number(0, std::numeric_limits<uint16_t>::max(), GetSeed());

                auto source_ptr = reinterpret_cast<uint16_t *>(source.data());

                std::generate(source_ptr, source_ptr + elements_count,
                              [&random_number]() {
                                  return static_cast<uint16_t>(random_number);
                              });

                if ((GetSeed() & 1) == 0) {
                    source_ptr[0] = 0;
                }

                for (uint32_t i = 1; i < elements_count; i++) {
                    if ((source_ptr[i] & 3) == 0) {
                        //change type
                        if (source_ptr[i - 1] != 0) {
                            source_ptr[i] = 0;
                        }
                    } else {
                        // keep type
                        if (source_ptr[i - 1] == 0) {
                            source_ptr[i] = 0;
                        }
                    }
                }
            } else {
                qpl::test::random random_number(0, std::numeric_limits<uint32_t>::max(), GetSeed());

                auto source_ptr = reinterpret_cast<uint32_t *>(source.data());

                std::generate(source_ptr, source_ptr + elements_count,
                              [&random_number]() {
                                  return static_cast<uint32_t>(random_number);
                              });

                if ((GetSeed() & 1) == 0) {
                    source_ptr[0] = 0;
                }

                for (uint32_t i = 1; i < elements_count; i++) {
                    if ((source_ptr[i] & 3) == 0) {
                        //change type
                        if (source_ptr[i - 1] != 0) {
                            source_ptr[i] = 0;
                        }
                    } else {
                        // keep type
                        if (source_ptr[i - 1] == 0) {
                            source_ptr[i] = 0;
                        }
                    }
                }
            }

            break;
        }
        default: {
            std::fill(source.begin(), source.end(), 0);

            break;
        }
    }

    destination.resize(source_size);
    reference_destination.resize(source_size);
    compressed_data.resize(compressed_buffer_size);
    reference_compressed_data.resize(compressed_buffer_size);

    std::fill(destination.begin(), destination.end(), 4);
    std::fill(reference_destination.begin(), reference_destination.end(), 5);
    std::fill(compressed_data.begin(), compressed_data.end(), 6);
    std::fill(reference_compressed_data.begin(), reference_compressed_data.end(), 7);
}

void ZeroCompressionFixture::CleanUpAfterIteration() {
    JobFixture::TearDown();
    JobFixture::SetUp();
}

} // namespace qpl::test
