/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <iostream>
#include <vector>

#include "ta_hl_common.hpp"
#include "operation_chain_fixture.hpp"
#include "high_level_api_util.hpp"

#include "source_provider.hpp"
#include "format_generator.hpp"
#include "check_result.hpp"

constexpr uint32_t maximum_bit_set_size = 14;

namespace qpl::test {
    template<class... operations_t>
    class AnalyticChainTest : public OperationChainFixture<operations_t...> {
    public:
        void InitializeTestCases() {
            if (!filter_operation_exist_and_not_last<operations_t ...>()) {
                auto source_length = format_generator::generate_length_sequence();

                for (auto number_of_elements : source_length) {
                    for (uint32_t output_bit_width : {1, 8, 16, 32}) {
                        for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                            OperationChainTestCase test_case;

                            test_case.number_of_input_elements = number_of_elements;
                            test_case.input_bit_width          = input_bit_width;
                            test_case.output_bit_width         = output_bit_width;

                            if (!is_valid_test_case<operations_t ...>(test_case))
                            {
                                break;
                            }
                            
                            this->AddNewTestCase(test_case);
                        }
                    }
                }
            }

            auto dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();

            for (auto &data: dataset.get_data()) {
                OperationChainTestCase test_case;
                
                test_case.file_name                = data.first;
                test_case.number_of_input_elements = 0;
                test_case.input_bit_width          = byte_bit_length;
                test_case.output_bit_width         = 1;

                this->AddNewTestCase(test_case);
            }
        }

        void SetUp() override {
            OperationChainFixture<operations_t ...>::SetUp();
            InitializeTestCases();
        }
    };

    using scan_select_test_t = AnalyticChainTest<scan_operation, select_operation>;

    QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(analytic_chain_scan_select,
                                           execute,
                                           scan_select_test_t) {
        // Variables
        const uint32_t boundary            = (1u << (current_test_case.input_bit_width - 1u));
        uint32_t       chain_result_value  = 0;
        uint32_t       scan_result_value   = 0;
        uint32_t       select_result_value = 0;

        // Act
        auto chain = scan_operation::builder(less, boundary)
                             .is_inclusive(true)
                             .number_of_input_elements(current_test_case.number_of_input_elements)
                             .input_vector_width(current_test_case.input_bit_width)
                             .output_vector_width(1)
                             .build() |
                     select_operation::builder()
                             .output_vector_width(current_test_case.output_bit_width)
                             .build();

        auto chain_result = test::execute(chain, source, destination);
        ASSERT_NO_THROW(chain_result_value = handle_result(chain_result));

        // Reference Act

        // Prepare Scan operation
        auto scan_operation = scan_operation::builder(less, boundary)
                .is_inclusive(true)
                .number_of_input_elements(current_test_case.number_of_input_elements)
                .input_vector_width(current_test_case.input_bit_width)
                .output_vector_width(1)
                .build();

        // Run Scan operation
        std::vector<uint8_t> scan_destination((current_test_case.number_of_input_elements + 7) >> 3, 0);

        auto scan_result = test::execute(scan_operation, source, scan_destination);
        ASSERT_NO_THROW(scan_result_value = handle_result(scan_result));

        // Prepare Select operation
        auto select_operation = select_operation::builder(scan_destination.data(), scan_destination.size())
                .number_of_input_elements(scan_result_value)
                .input_vector_width(current_test_case.input_bit_width)
                .output_vector_width(current_test_case.output_bit_width)
                .build();

        // Run Select operation
        auto select_result = test::execute(select_operation, source, reference_destination);
        ASSERT_NO_THROW(select_result_value = handle_result(select_result));

        // Assert
        EXPECT_TRUE(CompareVectors(destination, reference_destination));
    }

    using scan_range_select_test_t = AnalyticChainTest<scan_range_operation, select_operation>;

    QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(analytic_chain_scan_range_select,
                                           execute,
                                           scan_range_select_test_t) {
        // Variables
        const uint32_t lower_boundary          = (1ull << (current_test_case.input_bit_width)) / 3;
        const uint32_t upper_boundary          = lower_boundary * 2;
        uint32_t       chain_result_value      = 0;
        uint32_t       scan_range_result_value = 0;
        uint32_t       select_result_value     = 0;
        
        // Act
        auto chain = scan_range_operation::builder(lower_boundary, upper_boundary)
                             .is_inclusive(true)
                             .number_of_input_elements(current_test_case.number_of_input_elements)
                             .input_vector_width(current_test_case.input_bit_width)
                             .output_vector_width(1)
                             .build() |
                     select_operation::builder()
                             .output_vector_width(current_test_case.output_bit_width)
                             .build();

        auto chain_result = test::execute(chain, source, destination);
        ASSERT_NO_THROW(chain_result_value = handle_result(chain_result));
        
        // Reference Act
        
        // Prepare ScanRange operation
        auto scan_range_operation = scan_range_operation::builder(lower_boundary, upper_boundary)
                .is_inclusive(true)
                .number_of_input_elements(current_test_case.number_of_input_elements)
                .input_vector_width(current_test_case.input_bit_width)
                .output_vector_width(1)
                .build();

        // Run ScanRange operation
        std::vector<uint8_t> scan_range_destination((current_test_case.number_of_input_elements + 7) >> 3, 0);

        auto scan_range_result = test::execute(scan_range_operation, source, scan_range_destination);
        ASSERT_NO_THROW(scan_range_result_value = handle_result(scan_range_result));

        // Prepare Select operation
        auto select_operation = select_operation::builder(scan_range_destination.data(), scan_range_destination.size())
                .number_of_input_elements(scan_range_result_value)
                .input_vector_width(current_test_case.input_bit_width)
                .output_vector_width(current_test_case.output_bit_width)
                .build();

        // Run Select operation
        auto select_result = test::execute(select_operation, source, reference_destination);        
        ASSERT_NO_THROW(select_result_value = handle_result(select_result));

        // Assert
        EXPECT_TRUE(CompareVectors(destination, reference_destination));
    }

    using scan_range_select_find_unique_test_t = AnalyticChainTest<scan_range_operation,
                                                                   select_operation,
                                                                   find_unique_operation>;

    QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(analytic_chain_scan_range_select_find_unique,
                                           execute,
                                           scan_range_select_find_unique_test_t) {
        // Variables
        const uint32_t lower_boundary           = (1ull << (current_test_case.input_bit_width)) / 3;
        const uint32_t upper_boundary           = lower_boundary * 2;
        const uint32_t bits_to_ignore           = current_test_case.input_bit_width / 3;
        uint32_t       chain_result_value       = 0;
        uint32_t       scan_range_result_value  = 0;
        uint32_t       select_result_value      = 0;
        uint32_t       find_unique_result_value = 0;
        
        // Act
        auto chain = scan_range_operation::builder(lower_boundary, upper_boundary)
                             .is_inclusive(true)
                             .number_of_input_elements(current_test_case.number_of_input_elements)
                             .input_vector_width(current_test_case.input_bit_width)
                             .output_vector_width(1)
                             .build() |
                     select_operation::builder()
                             .output_vector_width(1)
                             .build() |
                     find_unique_operation::builder(bits_to_ignore, bits_to_ignore)
                             .output_vector_width(current_test_case.output_bit_width)
                             .build();

        auto chain_result = test::execute(chain, source, destination);
        ASSERT_NO_THROW(chain_result_value = handle_result(chain_result));
        
        // Reference Act
        
        // Prepare ScanRange operation
        auto scan_range_operation = scan_range_operation::builder(lower_boundary, upper_boundary)
                .is_inclusive(true)
                .number_of_input_elements(current_test_case.number_of_input_elements)
                .input_vector_width(current_test_case.input_bit_width)
                .output_vector_width(1)
                .build();

        // Run ScanRange operation
        std::vector<uint8_t> scan_range_destination((current_test_case.number_of_input_elements + 7) >> 3, 0);

        auto scan_range_result = test::execute(scan_range_operation, source, scan_range_destination);
        ASSERT_NO_THROW(scan_range_result_value = handle_result(scan_range_result));

        // Prepare Select operation
        auto select_operation = select_operation::builder(scan_range_destination.data(), scan_range_destination.size())
                .number_of_input_elements(scan_range_result_value)
                .input_vector_width(current_test_case.input_bit_width)
                .output_vector_width(current_test_case.output_bit_width)
                .build();

        // Run Select operation
        std::vector<uint8_t> select_destination((current_test_case.number_of_input_elements *
                                                 current_test_case.input_bit_width + 7) >> 3, 0);

        auto select_result = test::execute(select_operation, source, select_destination);        
        ASSERT_NO_THROW(select_result_value = handle_result(select_result));

        // Prepare FindUnique operation
        auto find_unique_operation = find_unique_operation::builder(bits_to_ignore, bits_to_ignore)
                .number_of_input_elements(select_result_value)
                .input_vector_width(current_test_case.input_bit_width)
                .output_vector_width(current_test_case.output_bit_width)
                .build();

        // Run FindUnique operation
        auto find_unique_result = test::execute(find_unique_operation, select_destination, reference_destination);        
        ASSERT_NO_THROW(find_unique_result_value = handle_result(find_unique_result));

        // Assert
        EXPECT_TRUE(CompareVectors(destination, reference_destination));
    }

    using scan_select_expand_test_t = AnalyticChainTest<scan_operation,
                                                        select_operation,
                                                        expand_operation>;

    QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(analytic_chain_scan_select_expand,
                                           execute,
                                           scan_select_expand_test_t) {
        // Variables
        const uint32_t boundary            = (1u << (current_test_case.input_bit_width - 1u));
        const uint32_t mask_bit_size       = current_test_case.number_of_input_elements >> 2;
        uint32_t       chain_result_value  = 0;
        uint32_t       scan_result_value   = 0;
        uint32_t       select_result_value = 0;
        uint32_t       expand_result_value = 0;

        auto expand_mask = generate_mask(mask_bit_size);
        
        // Act
        auto chain = scan_operation::builder(less, boundary)
                             .is_inclusive(true)
                             .number_of_input_elements(current_test_case.number_of_input_elements)
                             .input_vector_width(current_test_case.input_bit_width)
                             .output_vector_width(1)
                             .build() |
                     select_operation::builder()
                             .output_vector_width(1)
                             .build() |
                     expand_operation::builder(expand_mask.data(), expand_mask.size())
                             .number_of_input_elements(mask_bit_size)
                             .output_vector_width(current_test_case.output_bit_width)
                             .build();

        auto chain_result = test::execute(chain, source, destination);
        ASSERT_NO_THROW(chain_result_value = handle_result(chain_result));
        
        // Reference Act
        
        // Prepare ScanRange operation
        auto scan_operation = scan_operation::builder(less, boundary)
                .is_inclusive(true)
                .number_of_input_elements(current_test_case.number_of_input_elements)
                .input_vector_width(current_test_case.input_bit_width)
                .output_vector_width(1)
                .build();

        // Run ScanRange operation
        std::vector<uint8_t> scan_destination((current_test_case.number_of_input_elements + 7) >> 3, 0);

        auto scan_result = test::execute(scan_operation, source, scan_destination);
        ASSERT_NO_THROW(scan_result_value = handle_result(scan_result));

        // Prepare Select operation
        auto select_operation = select_operation::builder(scan_destination.data(), scan_destination.size())
                .number_of_input_elements(scan_result_value)
                .input_vector_width(current_test_case.input_bit_width)
                .output_vector_width(current_test_case.output_bit_width)
                .build();

        // Run Select operation
        std::vector<uint8_t> select_destination((current_test_case.number_of_input_elements *
                                                 current_test_case.input_bit_width + 7) >> 3, 0);

        auto select_result = test::execute(select_operation, source, select_destination);        
        ASSERT_NO_THROW(select_result_value = handle_result(select_result));

        // Prepare Expand operation
        auto expand_operation = expand_operation::builder(expand_mask.data(), expand_mask.size())
                .number_of_input_elements(mask_bit_size)
                .input_vector_width(current_test_case.input_bit_width)
                .output_vector_width(current_test_case.output_bit_width)
                .build();

        // Run Expand operation
        auto expand_result = test::execute(expand_operation, select_destination, reference_destination);        
        ASSERT_NO_THROW(expand_result_value = handle_result(expand_result));

        // Assert
        EXPECT_TRUE(CompareVectors(destination, reference_destination));
    }

    using extract_find_unique_test_t = AnalyticChainTest<extract_operation, find_unique_operation>;

    QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(analytic_chain_extract_find_unique,
                                           execute,
                                           extract_find_unique_test_t) {
        // Variables
        const uint32_t lower_boundary           = current_test_case.number_of_input_elements / 3;
        const uint32_t upper_boundary           = lower_boundary * 2;
        const uint32_t bits_to_ignore           = current_test_case.input_bit_width / 3;
        uint32_t       chain_result_value       = 0;
        uint32_t       extract_result_value     = 0;
        uint32_t       find_unique_result_value = 0;
        
        // Act
        auto chain = extract_operation::builder(lower_boundary, upper_boundary)
                             .number_of_input_elements(current_test_case.number_of_input_elements)
                             .input_vector_width(current_test_case.input_bit_width)
                             .output_vector_width(1)
                             .build() |
                     find_unique_operation::builder(bits_to_ignore, bits_to_ignore)
                             .output_vector_width(current_test_case.output_bit_width)
                             .build();

        auto chain_result = test::execute(chain, source, destination);
        ASSERT_NO_THROW(chain_result_value = handle_result(chain_result));
        
        // Reference Act
        
        // Prepare Extract operation
        auto extract_operation = extract_operation::builder(lower_boundary, upper_boundary)
                .number_of_input_elements(current_test_case.number_of_input_elements)
                .input_vector_width(current_test_case.input_bit_width)
                .output_vector_width(1)
                .build();

        // Run Extract operation
        std::vector<uint8_t> extract_destination(source.size());

        auto extract_result = test::execute(extract_operation, source, extract_destination);
        ASSERT_NO_THROW(extract_result_value = handle_result(extract_result));

        // Prepare FindUnique operation
        auto find_unique_operation = find_unique_operation::builder(bits_to_ignore, bits_to_ignore)
                .number_of_input_elements(extract_result_value)
                .input_vector_width(current_test_case.input_bit_width)
                .output_vector_width(current_test_case.output_bit_width)
                .build();

        // Run FindUnique operation
        auto find_unique_result = test::execute(find_unique_operation, extract_destination, reference_destination);        
        ASSERT_NO_THROW(find_unique_result_value = handle_result(find_unique_result));

        // Assert
        EXPECT_TRUE(CompareVectors(destination, reference_destination));
    }

    using extract_set_membership_test_t = AnalyticChainTest<extract_operation, set_membership_operation>;

    QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(analytic_chain_extract_set_membership,
                                           execute,
                                           extract_set_membership_test_t) {
        // Variables
        const uint32_t lower_boundary              = current_test_case.number_of_input_elements / 3;
        const uint32_t upper_boundary              = lower_boundary;

        uint32_t lower_bits_to_ignore = current_test_case.input_bit_width / 3;
        uint32_t upper_bits_to_ignore = current_test_case.input_bit_width / 3;

        const uint32_t actual_bit_width = current_test_case.input_bit_width - (lower_bits_to_ignore + upper_bits_to_ignore);

        if (actual_bit_width > maximum_bit_set_size) {
            upper_bits_to_ignore = current_test_case.input_bit_width - lower_bits_to_ignore - maximum_bit_set_size;
        }

        uint32_t       chain_result_value          = 0;
        uint32_t       extract_result_value        = 0;
        uint32_t       set_membership_result_value = 0;
        
        auto set_membership_mask = generate_mask(1ull << (current_test_case.input_bit_width - (lower_bits_to_ignore + upper_bits_to_ignore)));

        // Act
        auto chain = extract_operation::builder(lower_boundary, upper_boundary)
                             .number_of_input_elements(current_test_case.number_of_input_elements)
                             .input_vector_width(current_test_case.input_bit_width)
                             .output_vector_width(1)
                             .build() |
                     set_membership_operation::builder(set_membership_mask.data(), set_membership_mask.size())
                             .number_low_order_bits_to_ignore(lower_bits_to_ignore)
                             .number_high_order_bits_to_ignore(upper_bits_to_ignore)
                             .output_vector_width(current_test_case.output_bit_width)
                             .build();

        auto chain_result = test::execute(chain, source, destination);
        ASSERT_NO_THROW(chain_result_value = handle_result(chain_result));
        
        // Reference Act
        
        // Prepare Extract operation
        auto extract_operation = extract_operation::builder(lower_boundary, upper_boundary)
                .number_of_input_elements(current_test_case.number_of_input_elements)
                .input_vector_width(current_test_case.input_bit_width)
                .output_vector_width(1)
                .build();

        // Run Extract operation
        std::vector<uint8_t> extract_destination(source.size());

        auto extract_result = test::execute(extract_operation, source, extract_destination);
        ASSERT_NO_THROW(extract_result_value = handle_result(extract_result));

        // Prepare FindUnique operation
        auto set_membership_operation = set_membership_operation::builder(set_membership_mask.data(), set_membership_mask.size())
                .number_low_order_bits_to_ignore(lower_bits_to_ignore)
                .number_high_order_bits_to_ignore(upper_bits_to_ignore)
                .number_of_input_elements(extract_result_value)
                .input_vector_width(current_test_case.input_bit_width)
                .output_vector_width(current_test_case.output_bit_width)
                .build();

        // Run FindUnique operation
        auto set_membership_result = test::execute(set_membership_operation, extract_destination, reference_destination);        
        ASSERT_NO_THROW(set_membership_result_value = handle_result(set_membership_result));

        // Assert
        EXPECT_TRUE(CompareVectors(destination, reference_destination));
    }

    using extract_scan_select_test_t = AnalyticChainTest<extract_operation, scan_operation, select_operation>;

    QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(analytic_chain_extract_scan_select,
                                           execute,
                                           extract_scan_select_test_t) {
        // Variables
        const uint32_t boundary             = (1u << (current_test_case.input_bit_width - 1u));
        const uint32_t lower_boundary       = current_test_case.number_of_input_elements / 3;
        const uint32_t upper_boundary       = lower_boundary * 2;
        uint32_t       chain_result_value   = 0;
        uint32_t       extract_result_value = 0;
        uint32_t       scan_result_value    = 0;
        uint32_t       select_result_value  = 0;
        
        // Act
        auto chain = extract_operation::builder(lower_boundary, upper_boundary)
                             .number_of_input_elements(current_test_case.number_of_input_elements)
                             .input_vector_width(current_test_case.input_bit_width)
                             .output_vector_width(1)
                             .build() |
                     scan_operation::builder(less, boundary)
                             .is_inclusive(true)
                             .output_vector_width(1)
                             .build() |
                     select_operation::builder()
                             .output_vector_width(current_test_case.output_bit_width)
                             .build();

        auto chain_result = test::execute(chain, source, destination);
        ASSERT_NO_THROW(chain_result_value = handle_result(chain_result));
        
        // Reference Act
        
        // Prepare Extract operation
        auto extract_operation = extract_operation::builder(lower_boundary, upper_boundary)
                .number_of_input_elements(current_test_case.number_of_input_elements)
                .input_vector_width(current_test_case.input_bit_width)
                .output_vector_width(1)
                .build();

        // Run Extract operation
        std::vector<uint8_t> extract_destination(source.size());

        auto extract_result = test::execute(extract_operation, source, extract_destination);
        ASSERT_NO_THROW(extract_result_value = handle_result(extract_result));

        // Prepare Scan operation
        auto scan_operation = scan_operation::builder(less, boundary)
                .is_inclusive(true)
                .number_of_input_elements(extract_result_value)
                .input_vector_width(current_test_case.input_bit_width)
                .output_vector_width(1)
                .build();

        // Run Scan operation
        std::vector<uint8_t> scan_destination((extract_result_value + 7) >> 3, 0);

        auto scan_result = test::execute(scan_operation, extract_destination, scan_destination);
        ASSERT_NO_THROW(scan_result_value = handle_result(scan_result));

        // Prepare Select operation
        auto select_operation = select_operation::builder(scan_destination.data(), scan_destination.size())
                .number_of_input_elements(scan_result_value)
                .input_vector_width(current_test_case.input_bit_width)
                .output_vector_width(current_test_case.output_bit_width)
                .build();

        // Run Select operation
        auto select_result = test::execute(select_operation, extract_destination, reference_destination);        
        ASSERT_NO_THROW(select_result_value = handle_result(select_result));

        // Assert
        EXPECT_TRUE(CompareVectors(destination, reference_destination));
   }
}
