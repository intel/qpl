/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "iostream"
#include "vector"

#include "ta_hl_common.hpp"
#include "compression_chain_fixture.hpp"
#include "high_level_api_util.hpp"
#include "source_provider.hpp"
#include "check_result.hpp"

#include "qpl/cpp_api/operations/compression/inflate_operation.hpp"

namespace qpl::test {

class CompressionChainTest : public CompressionChainFixture {
public:
    void InitializeTestCases() {
        std::cout << "InitializeTestCases ->" << std::endl;

        auto execution_path = this->GetExecutionPath(); // todo doing nothing???

        for (auto file: util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
            this->AddNewTestCase(file.first);
        }
        std::cout << "InitializeTestCases <-" << std::endl;
    }

    void SetUp() override {
        printf("SetUp\n");
        CompressionChainFixture::SetUp();
        InitializeTestCases();
    }
};

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression_chain_test,
                                       inflate_extract_deflate,
                                       CompressionChainTest) {
    // Variables
    const uint32_t lower_boundary = static_cast<uint32_t>(source.size()) / 3;
    const uint32_t upper_boundary = lower_boundary * 2;
    uint32_t chain_result_value   = 0;
    uint32_t extract_result_value = 0;
    uint32_t inflate_result_value = 0;

    // Act
    auto chain = inflate_operation() |
                 extract_operation(lower_boundary, upper_boundary) |
                 deflate_operation();

    ASSERT_NO_THROW(auto chain_result = test::execute(chain, compressed_source, compressed_destination);
                    chain_result_value = handle_result(chain_result));

    inflate_operation inflate_operation;

    auto inflate_result = test::execute(inflate_operation,
                                        compressed_destination.begin(),
                                        compressed_destination.begin() + chain_result_value,
                                        destination.begin(),
                                        destination.end());
    ASSERT_NO_THROW(inflate_result_value = handle_result(inflate_result));

    // Reference Act
    extract_operation extract_operation(lower_boundary, upper_boundary);

    ASSERT_NO_THROW(auto extract_result = test::execute(extract_operation, source, reference_destination);
                    extract_result_value = handle_result(extract_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, reference_destination));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression_chain_test,
                                       inflate_scan_select_deflate,
                                       CompressionChainTest) {
    // Variables
    const uint32_t boundary       = (1u << (byte_bit_length - 1u));
    uint32_t chain_result_value   = 0;
    uint32_t scan_result_value    = 0;
    uint32_t select_result_value  = 0;
    uint32_t inflate_result_value = 0;

    // Act
    auto chain = inflate_operation() |
                 scan_operation(less, boundary) |
                 select_operation() |
                 deflate_operation();

    ASSERT_NO_THROW(auto chain_result = test::execute(chain, compressed_source, compressed_destination);
                    chain_result_value = handle_result(chain_result));

    inflate_operation inflate_operation;

    auto inflate_result = test::execute(inflate_operation,
                                        compressed_destination.begin(),
                                        compressed_destination.begin() + chain_result_value,
                                        destination.begin(),
                                        destination.end());
    ASSERT_NO_THROW(inflate_result_value = handle_result(inflate_result));

    // Reference Act
    scan_operation scan_operation(less, boundary);

    // Run Scan operation
    std::vector<uint8_t> scan_destination((source.size() + 7) >> 3, 0);

    auto scan_result = test::execute(scan_operation, source, scan_destination);
    ASSERT_NO_THROW(scan_result_value = handle_result(scan_result));

    // Prepare Select operation
    select_operation select_operation(scan_destination.data(), scan_destination.size());

    // Run Select operation
    auto select_result = test::execute(select_operation, source, reference_destination);        
    ASSERT_NO_THROW(select_result_value = handle_result(select_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, reference_destination));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression_chain_test, inflate_expand_deflate, CompressionChainTest) {
    // Variables
    const uint32_t source_size          = static_cast<uint32_t>(source.size());
    uint32_t       chain_result_value   = 0;
    uint32_t       expand_result_value  = 0;
    uint32_t       inflate_result_value = 0;

    auto mask = generate_mask(source_size - (source_size % byte_bit_length));

    // Act
    auto chain = inflate_operation() |
                 expand_operation(mask.data(), mask.size()) |
                 deflate_operation();

    ASSERT_NO_THROW(auto chain_result = test::execute(chain, compressed_source, compressed_destination);
                    chain_result_value = handle_result(chain_result));

    inflate_operation inflate_operation;

    auto inflate_result = test::execute(inflate_operation,
                                        compressed_destination.begin(),
                                        compressed_destination.begin() + chain_result_value,
                                        destination.begin(),
                                        destination.end());
    ASSERT_NO_THROW(inflate_result_value = handle_result(inflate_result));

    // Reference Act
    expand_operation expand_operation(mask.data(), mask.size());

    ASSERT_NO_THROW(auto expand_result = test::execute(expand_operation, source, reference_destination);
                    expand_result_value = handle_result(expand_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, reference_destination));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression_chain_test,
                                       inflate_find_unique,
                                       CompressionChainTest) {
    // Variables
    const uint32_t bits_to_igonre     = 0;
    uint32_t chain_result_value       = 0;
    uint32_t find_unique_result_value = 0;

    // Act
    auto chain = inflate_operation() |
                 find_unique_operation(bits_to_igonre, bits_to_igonre);

    ASSERT_NO_THROW(auto chain_result = test::execute(chain, compressed_source, destination);
                    chain_result_value = handle_result(chain_result));

    // Reference Act
    find_unique_operation find_unique_operation(bits_to_igonre, bits_to_igonre);

    ASSERT_NO_THROW(auto find_unique_result = test::execute(find_unique_operation, source, reference_destination);
                    find_unique_result_value = handle_result(find_unique_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, reference_destination));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(compression_chain_test,
                                       extract_deflate,
                                       CompressionChainTest) {
    // Variables
    const uint32_t lower_boundary = static_cast<uint32_t>(source.size()) / 3;
    const uint32_t upper_boundary = lower_boundary * 2;
    uint32_t chain_result_value   = 0;
    uint32_t extract_result_value = 0;
    uint32_t inflate_result_value = 0;

    std::cout << "extract_deflate <-" << std::endl;

    // Act
    auto chain = extract_operation(lower_boundary, upper_boundary) |
                 deflate_operation();

    ASSERT_NO_THROW(auto chain_result = test::execute(chain, source, compressed_destination);
                    chain_result_value = handle_result(chain_result));

    std::cout << "extract_deflate ->" << std::endl;

    inflate_operation inflate_operation;

    auto inflate_result = test::execute(inflate_operation,
                                        compressed_destination.begin(),
                                        compressed_destination.begin() + chain_result_value,
                                        destination.begin(),
                                        destination.end());
    ASSERT_NO_THROW(inflate_result_value = handle_result(inflate_result));

    // Reference Act
    extract_operation extract_operation(lower_boundary, upper_boundary);

    ASSERT_NO_THROW(auto extract_result = test::execute(extract_operation, source, reference_destination);
                    extract_result_value = handle_result(extract_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, reference_destination));
}

} // namespace qpl::test