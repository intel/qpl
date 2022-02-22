/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <iostream>
#include <vector>

#include "ta_hl_common.hpp"
#include "compression_fixture.hpp"
#include "high_level_api_util.hpp"
#include "check_result.hpp"

#include "source_provider.hpp"

namespace qpl::test {

class CompressionOperationTest : public CompressionFixture {
public:
    void InitializeTestCases() {
        CompressionTestCase test_case;

        auto execution_path = this->GetExecutionPath();

        for (bool gzip : {false, true}) {
            for (auto &dataset: util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
                test_case.file_name = dataset.first;
                test_case.gzip_mode   = gzip;

                if (gzip == true && execution_path == qpl_path_hardware) {
                    break;
                }

                this->AddNewTestCase(test_case);
            }
        }
    }

    void SetUp() override {
        CompressionFixture::SetUp();
        InitializeTestCases();
    }
};

// Default-level compression

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression, default_dynamic, CompressionOperationTest) {
    // Variables
    uint32_t deflate_result_value = 0;

    const compression_levels compression_level = default_level;

    std::vector<uint8_t> compressed_source(source.size() + additional_bytes_for_compression);

    // Act
    auto deflate_operation = deflate_operation::builder()
            .compression_mode<dynamic_mode>()
            .compression_level(compression_level)
            .gzip_mode(current_test_case.gzip_mode)
            .build();

    ASSERT_NO_THROW(auto deflate_result = test::execute(deflate_operation, source, compressed_source);
                            deflate_result_value = handle_result(deflate_result));

    // Decompression
    auto inflate_operation = inflate_operation::builder()
            .gzip_mode(current_test_case.gzip_mode)
            .build();

    auto inflate_result = test::execute(inflate_operation,
                                        compressed_source.begin(),
                                        compressed_source.begin() + deflate_result_value,
                                        destination.begin(),
                                        destination.end());

    ASSERT_NO_THROW(auto inflate_result_value = handle_result(inflate_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, source));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression, default_fixed, CompressionOperationTest) {
    // Variables
    uint32_t deflate_result_value = 0;

    const compression_levels compression_level = default_level;

    std::vector<uint8_t> compressed_source(2 * source.size() + additional_bytes_for_compression);

    // Act
    auto deflate_operation = deflate_operation::builder()
            .compression_mode<fixed_mode>()
            .compression_level(compression_level)
            .gzip_mode(current_test_case.gzip_mode)
            .build();

    ASSERT_NO_THROW(auto deflate_result = test::execute(deflate_operation, source, compressed_source);
                            deflate_result_value = handle_result(deflate_result));

    // Decompression
    auto inflate_operation = inflate_operation::builder()
            .gzip_mode(current_test_case.gzip_mode)
            .build();

    auto inflate_result = test::execute(inflate_operation,
                                        compressed_source.begin(),
                                        compressed_source.begin() + deflate_result_value,
                                        destination.begin(),
                                        destination.end());

    ASSERT_NO_THROW(auto inflate_result_value = handle_result(inflate_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, source));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression, default_static, CompressionOperationTest) {
    // Variables
    uint32_t                 deflate_result_value = 0;
    const compression_levels compression_level    = default_level;

    std::vector<uint8_t> compressed_source(source.size() + additional_bytes_for_compression);

    // Build Huffman table
    deflate_histogram deflate_histogram;
    test::update_deflate_statistics(source.begin(), source.end(), deflate_histogram, compression_level);

    auto huffman_table = test::make_deflate_table(deflate_histogram);

    // Act
    auto deflate_operation = deflate_operation::builder()
            .compression_mode<static_mode>(huffman_table)
            .compression_level(compression_level)
            .gzip_mode(current_test_case.gzip_mode)
            .build();

    ASSERT_NO_THROW(auto deflate_result = test::execute(deflate_operation, source, compressed_source);
                            deflate_result_value = handle_result(deflate_result));

    // Decompression
    auto inflate_operation = inflate_operation::builder()
            .gzip_mode(current_test_case.gzip_mode)
            .build();

    auto inflate_result = test::execute(inflate_operation,
                                        compressed_source.begin(),
                                        compressed_source.begin() + deflate_result_value,
                                        destination.begin(),
                                        destination.end());

    ASSERT_NO_THROW(auto inflate_result_value = handle_result(inflate_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, source));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression, default_canned, CompressionOperationTest) {
    // Variables
    uint32_t deflate_result_value = 0;

    const compression_levels compression_level = default_level;

    std::vector<uint8_t> compressed_source(source.size() + additional_bytes_for_compression);

    // Build compression Huffman table
    deflate_histogram deflate_histogram;
    test::update_deflate_statistics(source.begin(), source.end(), deflate_histogram, compression_level);

    auto deflate_huffman_table = test::make_deflate_table(deflate_histogram);

    // Act
    auto deflate_operation = deflate_operation::builder()
            .compression_mode<canned_mode>(deflate_huffman_table)
            .compression_level(compression_level)
            .gzip_mode(current_test_case.gzip_mode)
            .build();

    ASSERT_NO_THROW(auto deflate_result = test::execute(deflate_operation, source, compressed_source);
                            deflate_result_value = handle_result(deflate_result));

    // Build decompression Huffman table
    auto inflate_huffman_table = test::make_inflate_table(deflate_huffman_table);

    // Decompression
    auto inflate_operation = inflate_operation::builder()
            .compression_mode<canned_mode>(inflate_huffman_table)
            .gzip_mode(current_test_case.gzip_mode)
            .build();

    auto inflate_result = test::execute(inflate_operation,
                                        compressed_source.begin(),
                                        compressed_source.begin() + deflate_result_value,
                                        destination.begin(),
                                        destination.end());

    ASSERT_NO_THROW(auto inflate_result_value = handle_result(inflate_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, source));
}

// High-level compression

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression, high_dynamic, CompressionOperationTest) {
    // Variables
    uint32_t deflate_result_value = 0;

    if (GetExecutionPath() == qpl_path_hardware) {
        GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path.";
    }

    const compression_levels compression_level = high_level;

    std::vector<uint8_t> compressed_source(source.size() + additional_bytes_for_compression);

    // Act
    auto deflate_operation = deflate_operation::builder()
            .compression_mode<dynamic_mode>()
            .compression_level(compression_level)
            .gzip_mode(current_test_case.gzip_mode)
            .build();

    ASSERT_NO_THROW(auto deflate_result = test::execute(deflate_operation, source, compressed_source);
                            deflate_result_value = handle_result(deflate_result));

    // Decompression
    auto inflate_operation = inflate_operation::builder()
            .gzip_mode(current_test_case.gzip_mode)
            .build();

    auto inflate_result = test::execute(inflate_operation,
                                        compressed_source.begin(),
                                        compressed_source.begin() + deflate_result_value,
                                        destination.begin(),
                                        destination.end());

    ASSERT_NO_THROW(auto inflate_result_value = handle_result(inflate_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, source));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression, high_static, CompressionOperationTest) {
    // Variables
    uint32_t deflate_result_value = 0;

    if (GetExecutionPath() == qpl_path_hardware) {
        GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path.";
    }

    const compression_levels compression_level = high_level;

    std::vector<uint8_t> compressed_source(source.size() + additional_bytes_for_compression);

    // Build Huffman table
    deflate_histogram deflate_histogram;
    test::update_deflate_statistics(source.begin(), source.end(), deflate_histogram, compression_level);

    auto huffman_table = test::make_deflate_table(deflate_histogram);

    // Act
    auto deflate_operation = deflate_operation::builder()
            .compression_mode<static_mode>(huffman_table)
            .compression_level(compression_level)
            .gzip_mode(current_test_case.gzip_mode)
            .build();

    ASSERT_NO_THROW(auto deflate_result = test::execute(deflate_operation, source, compressed_source);
                            deflate_result_value = handle_result(deflate_result));

    // Decompression
    auto inflate_operation = inflate_operation::builder()
            .gzip_mode(current_test_case.gzip_mode)
            .build();

    auto inflate_result = test::execute(inflate_operation,
                                        compressed_source.begin(),
                                        compressed_source.begin() + deflate_result_value,
                                        destination.begin(),
                                        destination.end());

    ASSERT_NO_THROW(auto inflate_result_value = handle_result(inflate_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, source));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression, high_fixed, CompressionOperationTest) {
    // Variables
    uint32_t deflate_result_value = 0;

    if (GetExecutionPath() == qpl_path_hardware) {
        GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path.";
    }

    const compression_levels compression_level = high_level;

    std::vector<uint8_t> compressed_source(source.size() * 2 + additional_bytes_for_compression);

    // Act
    auto deflate_operation = deflate_operation::builder()
            .compression_mode<fixed_mode>()
            .compression_level(compression_level)
            .gzip_mode(current_test_case.gzip_mode)
            .build();

    ASSERT_NO_THROW(auto deflate_result = test::execute(deflate_operation, source, compressed_source);
                            deflate_result_value = handle_result(deflate_result));

    // Decompression
    auto inflate_operation = inflate_operation::builder()
            .gzip_mode(current_test_case.gzip_mode)
            .build();

    auto inflate_result = test::execute(inflate_operation,
                                        compressed_source.begin(),
                                        compressed_source.begin() + deflate_result_value,
                                        destination.begin(),
                                        destination.end());

    ASSERT_NO_THROW(auto inflate_result_value = handle_result(inflate_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, source));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression, high_canned, CompressionOperationTest) {
    // Variables
    uint32_t deflate_result_value = 0;

    if (GetExecutionPath() == qpl_path_hardware) {
        GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path.";
    }

    const compression_levels compression_level = high_level;

    std::vector<uint8_t> compressed_source(source.size() + additional_bytes_for_compression);

    // Build compression Huffman table
    deflate_histogram deflate_histogram;
    test::update_deflate_statistics(source.begin(), source.end(), deflate_histogram, compression_level);

    auto deflate_huffman_table = test::make_deflate_table(deflate_histogram);

    // Act
    auto deflate_operation = deflate_operation::builder()
            .compression_mode<canned_mode>(deflate_huffman_table)
            .compression_level(compression_level)
            .gzip_mode(current_test_case.gzip_mode)
            .build();

    ASSERT_NO_THROW(auto deflate_result = test::execute(deflate_operation, source, compressed_source);
                            deflate_result_value = handle_result(deflate_result));

    // Build decompression Huffman table
    auto inflate_huffman_table = test::make_inflate_table(deflate_huffman_table);

    // Decompression
    auto inflate_operation = inflate_operation::builder()
            .compression_mode<canned_mode>(inflate_huffman_table)
            .gzip_mode(current_test_case.gzip_mode)
            .build();

    auto inflate_result = test::execute(inflate_operation,
                                        compressed_source.begin(),
                                        compressed_source.begin() + deflate_result_value,
                                        destination.begin(),
                                        destination.end());

    ASSERT_NO_THROW(auto inflate_result_value = handle_result(inflate_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, source));
}

} // namespace qpl::test
