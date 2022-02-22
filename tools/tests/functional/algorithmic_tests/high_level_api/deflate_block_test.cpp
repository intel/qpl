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

#include "source_provider.hpp"

namespace qpl::test {

class DeflateBlockTest : public CompressionFixture {
public:
    void InitializeTestCases() {
        CompressionTestCase test_case;

        for (auto &dataset: util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
            for (uint32_t mini_block_size = 1; mini_block_size <= 7; mini_block_size++) {
                test_case.file_name       = dataset.first;
                test_case.mini_block_size = static_cast<mini_block_sizes>(mini_block_size);

                this->AddNewTestCase(test_case);
            }
        }
    }

    void SetUp() override {
        CompressionFixture::SetUp();
        num_test = 0;
        InitializeTestCases();
    }

    int32_t num_test = 0;
};

// Default-level compression

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(deflate_block, default_dynamic, DeflateBlockTest) {
    // Variables
    const compression_levels compression_level = default_level;
    const compression_modes  compression_mode  = dynamic_mode;

    // Act
    auto deflate_operation = deflate_operation::builder()
            .compression_mode<compression_mode>()
            .compression_level(compression_level)
            .build();

    auto deflate_block = test::build_deflate_block(deflate_operation, source, current_test_case.mini_block_size);

    // Assert
    EXPECT_TRUE(check_deflate_block(source.begin(), source.end(), deflate_block));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(deflate_block, default_fixed, DeflateBlockTest) {
    // Variables
    const compression_levels compression_level = default_level;
    const compression_modes  compression_mode  = fixed_mode;

    // Act
    auto deflate_operation = deflate_operation::builder()
            .compression_mode<compression_mode>()
            .compression_level(compression_level)
            .build();

    auto deflate_block = test::build_deflate_block(deflate_operation, source, current_test_case.mini_block_size);

    // Assert
    EXPECT_TRUE(check_deflate_block(source.begin(), source.end(), deflate_block));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(deflate_block, default_static, DeflateBlockTest) {
    // Variables
    const compression_levels compression_level = default_level;
    const compression_modes  compression_mode  = static_mode;

    // Build Huffman table
    deflate_histogram deflate_histogram;
    test::update_deflate_statistics(source.begin(), source.end(), deflate_histogram, compression_level);

    auto huffman_table = test::make_deflate_table(deflate_histogram);

    // Act
    auto deflate_operation = deflate_operation::builder()
            .compression_mode<compression_mode>(huffman_table)
            .compression_level(compression_level)
            .build();

    auto deflate_block = test::build_deflate_block(deflate_operation, source, current_test_case.mini_block_size);

    // Assert
    EXPECT_TRUE(check_deflate_block(source.begin(), source.end(), deflate_block));
}

// High-level compression

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(deflate_block, high_dynamic, DeflateBlockTest) {
    // Variables
    const compression_levels compression_level = high_level;
    const compression_modes  compression_mode  = dynamic_mode;

    if (GetExecutionPath() == qpl_path_hardware) {
        if (0 == DeflateBlockTest::num_test++) {
            GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path.";
        }
        return;
    }

    // Act
    auto deflate_operation = deflate_operation::builder()
            .compression_mode<compression_mode>()
            .compression_level(compression_level)
            .build();

    auto deflate_block = test::build_deflate_block(deflate_operation, source, current_test_case.mini_block_size);

    // Assert
    EXPECT_TRUE(check_deflate_block(source.begin(), source.end(), deflate_block));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(deflate_block, high_static, DeflateBlockTest) {
    // Variables
    const compression_levels compression_level = high_level;
    const compression_modes  compression_mode  = static_mode;

    if (GetExecutionPath() == qpl_path_hardware) {
        if (0 == DeflateBlockTest::num_test++) {
            GTEST_SKIP() << "Deflate operation doesn't support high compression level on the hardware path.";
        }
        return;
    }

    // Build Huffman table
    deflate_histogram deflate_histogram;
    test::update_deflate_statistics(source.begin(), source.end(), deflate_histogram, compression_level);

    auto huffman_table = test::make_deflate_table(deflate_histogram);

    // Act
    auto deflate_operation = deflate_operation::builder()
            .compression_mode<compression_mode>(huffman_table)
            .compression_level(compression_level)
            .build();

    auto deflate_block = test::build_deflate_block(deflate_operation, source, current_test_case.mini_block_size);

    // Assert
    EXPECT_TRUE(check_deflate_block(source.begin(), source.end(), deflate_block));
}

} // namespace qpl::test