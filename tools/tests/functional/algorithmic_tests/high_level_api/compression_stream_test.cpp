/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "iostream"
#include "vector"

#include "ta_hl_common.hpp"
#include "compression_fixture.hpp"
#include "high_level_api_util.hpp"
#include "source_provider.hpp"
#include "check_result.hpp"

#include "qpl/cpp_api/operations/compression/inflate_operation.hpp"
#include "qpl/cpp_api/results/deflate_stream.hpp"
#include "qpl/cpp_api/results/inflate_stream.hpp"

namespace qpl::test {

class CompressionStreamTest : public CompressionFixture {
public:
    void InitializeTestCases() {
        CompressionTestCase test_case;

        for (auto &dataset: util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
            test_case.file_name = dataset.first;

            test_case.chunk_size = static_cast<uint32_t>(chunk_size_generator);

            this->AddNewTestCase(test_case);
        }
    }

    void SetUp() override {
        CompressionFixture::SetUp();
        InitializeTestCases();
    }
};

// Default-level compression

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression_stream_test,default_dynamic, CompressionStreamTest) {
    // Variables
    const compression_levels compression_level = default_level;
    const compression_modes  compression_mode  = dynamic_mode;

    // Perform compression
    auto deflate_operation = deflate_operation::builder()
            .compression_level(compression_level)
            .compression_mode<compression_mode>()
            .build();

    auto deflate_stream    = qpl::deflate_stream(std::move(deflate_operation), destination.size());
    auto current_chunk     = source.begin();
    auto current_chunk_end = (current_test_case.chunk_size < source.size())
                             ? current_chunk + current_test_case.chunk_size
                             : source.end();

    while (std::distance(current_chunk_end, source.end()) > current_test_case.chunk_size) {
        deflate_stream.push(current_chunk, current_chunk_end);

        current_chunk += current_test_case.chunk_size;
        current_chunk_end += current_test_case.chunk_size;
    }

    deflate_stream.flush(current_chunk, source.end());

    // Perform decompression
    auto inflate_stream = qpl::inflate_stream(qpl::inflate_operation(),
                                              deflate_stream.begin(),
                                              deflate_stream.end());
    current_chunk     = destination.begin();
    current_chunk_end = (current_test_case.chunk_size < destination.size())
                        ? current_chunk + current_test_case.chunk_size
                        : destination.end();

    while (std::distance(current_chunk_end, destination.end()) > 0) {
        inflate_stream.extract(current_chunk, current_chunk_end);

        current_chunk += current_test_case.chunk_size;
        if (std::distance(current_chunk_end, destination.end()) < current_test_case.chunk_size) {
            current_chunk_end += std::distance(current_chunk_end, destination.end());
        } else {
            current_chunk_end += current_test_case.chunk_size;
        }
    }

    inflate_stream.extract(current_chunk, destination.end());

    EXPECT_TRUE(CompareVectors(source, destination));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression_stream_test, default_fixed, CompressionStreamTest) {
    // Variables
    const compression_levels compression_level = default_level;
    const compression_modes  compression_mode  = fixed_mode;

    // Perform compression
    auto deflate_operation = deflate_operation::builder()
            .compression_level(compression_level)
            .compression_mode<compression_mode>()
            .build();

    auto deflate_stream    = qpl::deflate_stream(std::move(deflate_operation), 2 * destination.size());
    auto current_chunk     = source.begin();
    auto current_chunk_end = (current_test_case.chunk_size < source.size())
                             ? current_chunk + current_test_case.chunk_size
                             : source.end();

    while (std::distance(current_chunk_end, source.end()) > current_test_case.chunk_size) {
        deflate_stream.push(current_chunk, current_chunk_end);

        current_chunk += current_test_case.chunk_size;
        current_chunk_end += current_test_case.chunk_size;
    }

    deflate_stream.flush(current_chunk, source.end());

    // Perform decompression
    auto inflate_stream = qpl::inflate_stream(qpl::inflate_operation(),
                                              deflate_stream.begin(),
                                              deflate_stream.end());
    current_chunk     = destination.begin();
    current_chunk_end = (current_test_case.chunk_size < destination.size())
                        ? current_chunk + current_test_case.chunk_size
                        : destination.end();

    while (std::distance(current_chunk_end, destination.end()) > 0) {
        inflate_stream.extract(current_chunk, current_chunk_end);

        current_chunk += current_test_case.chunk_size;
        if (std::distance(current_chunk_end, destination.end()) < current_test_case.chunk_size) {
            current_chunk_end += std::distance(current_chunk_end, destination.end());
        } else {
            current_chunk_end += current_test_case.chunk_size;
        }
    }

    inflate_stream.extract(current_chunk, destination.end());

    EXPECT_TRUE(CompareVectors(source, destination));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression_stream_test, default_static, CompressionStreamTest) {
    // Variables
    const compression_levels compression_level = default_level;
    const compression_modes  compression_mode  = static_mode;

    // Build Huffman table
    deflate_histogram deflate_histogram;
    test::update_deflate_statistics(source.begin(), source.end(), deflate_histogram, compression_level);

    auto huffman_table = test::make_deflate_table(deflate_histogram);

    // Perform compression
    auto deflate_operation = deflate_operation::builder()
            .compression_level(compression_level)
            .compression_mode<compression_mode>(huffman_table)
            .build();

    auto deflate_stream    = qpl::deflate_stream(std::move(deflate_operation), 2 * destination.size());
    auto current_chunk     = source.begin();
    auto current_chunk_end = (current_test_case.chunk_size < source.size())
                             ? current_chunk + current_test_case.chunk_size
                             : source.end();

    while (std::distance(current_chunk_end, source.end()) > current_test_case.chunk_size) {
        deflate_stream.push(current_chunk, current_chunk_end);

        current_chunk += current_test_case.chunk_size;
        current_chunk_end += current_test_case.chunk_size;
    }

    deflate_stream.flush(current_chunk, source.end());

    // Perform decompression
    auto inflate_stream = qpl::inflate_stream(qpl::inflate_operation(),
                                              deflate_stream.begin(),
                                              deflate_stream.end());
    current_chunk     = destination.begin();
    current_chunk_end = (current_test_case.chunk_size < destination.size())
                        ? current_chunk + current_test_case.chunk_size
                        : destination.end();

    while (std::distance(current_chunk_end, destination.end()) > 0) {
        inflate_stream.extract(current_chunk, current_chunk_end);

        current_chunk += current_test_case.chunk_size;
        if (std::distance(current_chunk_end, destination.end()) < current_test_case.chunk_size) {
            current_chunk_end += std::distance(current_chunk_end, destination.end());
        } else {
            current_chunk_end += current_test_case.chunk_size;
        }
    }

    inflate_stream.extract(current_chunk, destination.end());

    EXPECT_TRUE(CompareVectors(source, destination));
}

// High-level compression

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression_stream_test, high_dynamic, CompressionStreamTest) {
    // Variables
    const compression_levels compression_level = high_level;
    const compression_modes  compression_mode  = dynamic_mode;

    // Perform compression
    auto deflate_operation = deflate_operation::builder()
            .compression_level(compression_level)
            .compression_mode<compression_mode>()
            .build();

    auto deflate_stream    = qpl::deflate_stream(std::move(deflate_operation), destination.size());
    auto current_chunk     = source.begin();
    auto current_chunk_end = (current_test_case.chunk_size < source.size())
                             ? current_chunk + current_test_case.chunk_size
                             : source.end();

    while (std::distance(current_chunk_end, source.end()) > current_test_case.chunk_size) {
        deflate_stream.push(current_chunk, current_chunk_end);

        current_chunk += current_test_case.chunk_size;
        current_chunk_end += current_test_case.chunk_size;
    }

    deflate_stream.flush(current_chunk, source.end());

    // Perform decompression
    auto inflate_stream = qpl::inflate_stream(qpl::inflate_operation(),
                                              deflate_stream.begin(),
                                              deflate_stream.end());
    current_chunk     = destination.begin();
    current_chunk_end = (current_test_case.chunk_size < destination.size())
                        ? current_chunk + current_test_case.chunk_size
                        : destination.end();

    while (std::distance(current_chunk_end, destination.end()) > 0) {
        inflate_stream.extract(current_chunk, current_chunk_end);

        current_chunk += current_test_case.chunk_size;
        if (std::distance(current_chunk_end, destination.end()) < current_test_case.chunk_size) {
            current_chunk_end += std::distance(current_chunk_end, destination.end());
        } else {
            current_chunk_end += current_test_case.chunk_size;
        }
    }

    inflate_stream.extract(current_chunk, destination.end());

    EXPECT_TRUE(CompareVectors(source, destination));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression_stream_test, high_static, CompressionStreamTest) {
    // Variables
    const compression_levels compression_level = high_level;
    const compression_modes  compression_mode  = static_mode;

    // Build Huffman table
    deflate_histogram deflate_histogram;
    test::update_deflate_statistics(source.begin(), source.end(), deflate_histogram, compression_level);

    auto huffman_table = test::make_deflate_table(deflate_histogram);

    // Perform compression
    auto deflate_operation = deflate_operation::builder()
            .compression_level(compression_level)
            .compression_mode<compression_mode>(huffman_table)
            .build();

    auto deflate_stream    = qpl::deflate_stream(std::move(deflate_operation), 2 * destination.size());
    auto current_chunk     = source.begin();
    auto current_chunk_end = (current_test_case.chunk_size < source.size())
                             ? current_chunk + current_test_case.chunk_size
                             : source.end();

    while (std::distance(current_chunk_end, source.end()) > current_test_case.chunk_size) {
        deflate_stream.push(current_chunk, current_chunk_end);

        current_chunk += current_test_case.chunk_size;
        current_chunk_end += current_test_case.chunk_size;
    }

    deflate_stream.flush(current_chunk, source.end());

    // Perform decompression
    auto inflate_stream = qpl::inflate_stream(qpl::inflate_operation(),
                                              deflate_stream.begin(),
                                              deflate_stream.end());
    current_chunk     = destination.begin();
    current_chunk_end = (current_test_case.chunk_size < destination.size())
                        ? current_chunk + current_test_case.chunk_size
                        : destination.end();

    while (std::distance(current_chunk_end, destination.end()) > 0) {
        inflate_stream.extract(current_chunk, current_chunk_end);

        current_chunk += current_test_case.chunk_size;
        if (std::distance(current_chunk_end, destination.end()) < current_test_case.chunk_size) {
            current_chunk_end += std::distance(current_chunk_end, destination.end());
        } else {
            current_chunk_end += current_test_case.chunk_size;
        }
    }

    inflate_stream.extract(current_chunk, destination.end());

    EXPECT_TRUE(CompareVectors(source, destination));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression_stream_test, high_fixed, CompressionStreamTest) {
    // Variables
    const compression_levels compression_level = high_level;
    const compression_modes  compression_mode  = fixed_mode;

    // Perform compression
    auto deflate_operation = deflate_operation::builder()
            .compression_level(compression_level)
            .compression_mode<compression_mode>()
            .build();

    auto deflate_stream    = qpl::deflate_stream(std::move(deflate_operation), 2 * destination.size());
    auto current_chunk     = source.begin();
    auto current_chunk_end = (current_test_case.chunk_size < source.size())
                             ? current_chunk + current_test_case.chunk_size
                             : source.end();

    while (std::distance(current_chunk_end, source.end()) > current_test_case.chunk_size) {
        deflate_stream.push(current_chunk, current_chunk_end);

        current_chunk += current_test_case.chunk_size;
        current_chunk_end += current_test_case.chunk_size;
    }

    deflate_stream.flush(current_chunk, source.end());

    // Perform decompression
    auto inflate_stream = qpl::inflate_stream(qpl::inflate_operation(),
                                              deflate_stream.begin(),
                                              deflate_stream.end());
    current_chunk     = destination.begin();
    current_chunk_end = (current_test_case.chunk_size < destination.size())
                        ? current_chunk + current_test_case.chunk_size
                        : destination.end();

    while (std::distance(current_chunk_end, destination.end()) > 0) {
        inflate_stream.extract(current_chunk, current_chunk_end);

        current_chunk += current_test_case.chunk_size;
        if (std::distance(current_chunk_end, destination.end()) < current_test_case.chunk_size) {
            current_chunk_end += std::distance(current_chunk_end, destination.end());
        } else {
            current_chunk_end += current_test_case.chunk_size;
        }
    }

    inflate_stream.extract(current_chunk, destination.end());

    EXPECT_TRUE(CompareVectors(source, destination));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(decompression_stream, incorrect_input) {
    constexpr uint32_t source_size = 100;
    constexpr uint32_t destination_size = 100;
    std::array<uint8_t, source_size> source = {};
    std::array<uint8_t, destination_size> destination = {};

    // Perform decompression
    auto inflate_stream = qpl::inflate_stream(qpl::inflate_operation(),
                                              source.begin(),
                                              source.end());

    // Expect any exception appear due incorrect decompression data
    EXPECT_ANY_THROW(inflate_stream.extract(destination.begin(), destination.end()));
}

} // namespace qpl::test
