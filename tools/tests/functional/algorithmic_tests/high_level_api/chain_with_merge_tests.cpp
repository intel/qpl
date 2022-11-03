/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "iostream"
#include "vector"

#include "ta_hl_common.hpp"
#include "operation_chain_fixture.hpp"
#include "high_level_api_util.hpp"
#include "format_generator.hpp"
#include "check_result.hpp"

namespace qpl::test {
template <parsers Parser, class ...Operations>
class MergeChainTest : public OperationChainFixture<Operations...> {
public:
    void InitializeTestCases() {
        OperationChainTestCase test_case;

        auto source_length = format_generator::generate_length_sequence();

        for (auto number_of_elements : source_length) {
            for (uint32_t output_bit_width : {1, 8, 16, 32}) {
                for (uint32_t input_bit_width = 1; input_bit_width <= max_input_bit_width; input_bit_width++) {
                    test_case.number_of_input_elements = number_of_elements;
                    test_case.input_bit_width          = input_bit_width;
                    test_case.output_bit_width         = output_bit_width;

                    if (!is_valid_test_case<Operations ...>(test_case)) {
                        break;
                    }

                    this->AddNewTestCase(test_case);
                }
            }
        }

        if constexpr (Parser != parquet_rle) {
            for (auto &dataset: util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
                test_case.file_name                = dataset.first;
                test_case.number_of_input_elements = 0;
                test_case.input_bit_width          = BYTE_BIT_LENGTH;
                test_case.output_bit_width         = 1;

                this->AddNewTestCase(test_case);
            }
        }
    }

    void SetUp() override {
        OperationChainFixture<Operations...>::SetUp();
        OperationChainFixture<Operations...>::SetParser(Parser);
        InitializeTestCases();
    }
};

template <parsers parser>
using inflate_scan_t = MergeChainTest<parser, inflate_operation, scan_operation>;

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(merge_chain_inflate_scan_le,
                                       DISABLED_little_endian,  // TODO: Will be enabled when compression for middle layer is implemented
                                       inflate_scan_t<little_endian_packed_array>) {
    // Variables
    const uint32_t boundary           = (1u << (current_test_case.input_bit_width - 1u));
    uint32_t       chain_result_value = 0;
    uint32_t       scan_result_value  = 0;
    const parsers  parser             = parsers::little_endian_packed_array;

    // Act
    auto chain = inflate_operation() |
                 merge(current_test_case.number_of_input_elements) |
                 scan_operation::builder(less, boundary)
                         .is_inclusive(true)
                         .parser<parser>(current_test_case.number_of_input_elements)
                         .input_vector_width(current_test_case.input_bit_width)
                         .output_vector_width(current_test_case.output_bit_width)
                         .build();

    ASSERT_NO_THROW(auto chain_result = test::execute(chain, compressed_source, destination);
                            chain_result_value = handle_result(chain_result));

    // Reference Act

    // Prepare Scan operation
    auto scan_operation = scan_operation::builder(less, boundary)
            .is_inclusive(true)
            .parser<parser>(current_test_case.number_of_input_elements)
            .input_vector_width(current_test_case.input_bit_width)
            .output_vector_width(current_test_case.output_bit_width)
            .build();

    // Run Scan operation

    auto scan_result = test::execute(scan_operation, source, reference_destination);
    ASSERT_NO_THROW(scan_result_value = handle_result(scan_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, reference_destination));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(merge_chain_inflate_scan_be,
                                       DISABLED_big_endian,  // TODO: Will be enabled when compression for middle layer is implemented
                                       inflate_scan_t<big_endian_packed_array>) {
    // Variables
    const uint32_t boundary           = (1u << (current_test_case.input_bit_width - 1u));
    uint32_t       chain_result_value = 0;
    uint32_t       scan_result_value  = 0;
    const parsers  parser             = parsers::big_endian_packed_array;

    // Act
    auto chain = inflate_operation() |
                 merge(current_test_case.number_of_input_elements) |
                 scan_operation::builder(less, boundary)
                         .is_inclusive(true)
                         .parser<parser>(current_test_case.number_of_input_elements)
                         .input_vector_width(current_test_case.input_bit_width)
                         .output_vector_width(current_test_case.output_bit_width)
                         .build();

    ASSERT_NO_THROW(auto chain_result = test::execute(chain, compressed_source, destination);
                            chain_result_value = handle_result(chain_result));

    // Reference Act

    // Prepare Scan operation
    auto scan_operation = scan_operation::builder(less, boundary)
            .is_inclusive(true)
            .parser<parser>(current_test_case.number_of_input_elements)
            .input_vector_width(current_test_case.input_bit_width)
            .output_vector_width(current_test_case.output_bit_width)
            .build();

    // Run Scan operation

    auto scan_result = test::execute(scan_operation, source, reference_destination);
    ASSERT_NO_THROW(scan_result_value = handle_result(scan_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, reference_destination));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(merge_chain_inflate_scan_prle,
                                       DISABLED_parquet_rle,  // TODO: Will be enabled when compression for middle layer is implemented
                                       inflate_scan_t<parquet_rle>) {
    // Variables
    const uint32_t boundary           = (1u << (current_test_case.input_bit_width - 1u));
    uint32_t       chain_result_value = 0;
    uint32_t       scan_result_value  = 0;
    const parsers  parser             = parsers::parquet_rle;

    // Act
    auto chain = inflate_operation() |
                 merge(current_test_case.number_of_input_elements) |
                 scan_operation::builder(less, boundary)
                         .is_inclusive(true)
                         .parser<parser>(current_test_case.number_of_input_elements)
                         .input_vector_width(current_test_case.input_bit_width)
                         .output_vector_width(current_test_case.output_bit_width)
                         .build();

    ASSERT_NO_THROW(auto chain_result = test::execute(chain, compressed_source, destination);
                            chain_result_value = handle_result(chain_result));

    // Reference Act

    // Prepare Scan operation
    auto scan_operation = scan_operation::builder(less, boundary)
            .is_inclusive(true)
            .parser<parser>(current_test_case.number_of_input_elements)
            .input_vector_width(current_test_case.input_bit_width)
            .output_vector_width(current_test_case.output_bit_width)
            .build();

    // Run Scan operation

    auto scan_result = test::execute(scan_operation, source, reference_destination);
    ASSERT_NO_THROW(scan_result_value = handle_result(scan_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, reference_destination));
}

template <parsers parser>
using inflate_scan_range_t = MergeChainTest<parser, inflate_operation, scan_range_operation>;

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(merge_chain_inflate_scan_range_le,
                                       DISABLED_little_endian,  // TODO: Will be enabled when compression for middle layer is implemented
                                       inflate_scan_range_t<little_endian_packed_array>) {
    // Variables
    const uint32_t lower_boundary          = (1ull << (current_test_case.input_bit_width)) / 3;
    const uint32_t upper_boundary          = lower_boundary * 2;
    uint32_t       chain_result_value      = 0;
    uint32_t       scan_range_result_value = 0;
    const parsers  parser                  = parsers::little_endian_packed_array;

    // Act
    auto chain = inflate_operation() |
                 merge(current_test_case.number_of_input_elements) |
                 scan_range_operation::builder(lower_boundary, upper_boundary)
                         .is_inclusive(true)
                         .parser<parser>(current_test_case.number_of_input_elements)
                         .input_vector_width(current_test_case.input_bit_width)
                         .output_vector_width(current_test_case.output_bit_width)
                         .build();

    ASSERT_NO_THROW(auto chain_result = test::execute(chain, compressed_source, destination);
                            chain_result_value = handle_result(chain_result));

    // Reference Act

    // Prepare Scan_Range operation
    auto scan_range_operation = scan_range_operation::builder(lower_boundary, upper_boundary)
            .is_inclusive(true)
            .parser<parser>(current_test_case.number_of_input_elements)
            .input_vector_width(current_test_case.input_bit_width)
            .output_vector_width(current_test_case.output_bit_width)
            .build();

    // Run Scan_Range operation

    auto scan_range_result = test::execute(scan_range_operation, source, reference_destination);
    ASSERT_NO_THROW(scan_range_result_value = handle_result(scan_range_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, reference_destination));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(merge_chain_inflate_scan_range_be,
                                       DISABLED_big_endian,  // TODO: Will be enabled when compression for middle layer is implemented
                                       inflate_scan_range_t<big_endian_packed_array>) {
    // Variables
    const uint32_t lower_boundary          = (1ull << (current_test_case.input_bit_width)) / 3;
    const uint32_t upper_boundary          = lower_boundary * 2;
    uint32_t       chain_result_value      = 0;
    uint32_t       scan_range_result_value = 0;
    const parsers  parser                  = parsers::big_endian_packed_array;

    // Act
    auto chain = inflate_operation() |
                 merge(current_test_case.number_of_input_elements) |
                 scan_range_operation::builder(lower_boundary, upper_boundary)
                         .is_inclusive(true)
                         .parser<parser>(current_test_case.number_of_input_elements)
                         .input_vector_width(current_test_case.input_bit_width)
                         .output_vector_width(current_test_case.output_bit_width)
                         .build();

    ASSERT_NO_THROW(auto chain_result = test::execute(chain, compressed_source, destination);
                            chain_result_value = handle_result(chain_result));

    // Reference Act

    // Prepare Scan_Range operation
    auto scan_range_operation = scan_range_operation::builder(lower_boundary, upper_boundary)
            .is_inclusive(true)
            .parser<parser>(current_test_case.number_of_input_elements)
            .input_vector_width(current_test_case.input_bit_width)
            .output_vector_width(current_test_case.output_bit_width)
            .build();

    // Run Scan_Range operation

    auto scan_range_result = test::execute(scan_range_operation, source, reference_destination);
    ASSERT_NO_THROW(scan_range_result_value = handle_result(scan_range_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, reference_destination));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(merge_chain_inflate_scan_range_prle,
                                       DISABLED_parquet_rle,  // TODO: Will be enabled when compression for middle layer is implemented
                                       inflate_scan_range_t<parquet_rle>) {
    // Variables
    const uint32_t lower_boundary          = (1ull << (current_test_case.input_bit_width)) / 3;
    const uint32_t upper_boundary          = lower_boundary * 2;
    uint32_t       chain_result_value      = 0;
    uint32_t       scan_range_result_value = 0;
    const parsers  parser                  = parsers::parquet_rle;

    // Act
    auto chain = inflate_operation() |
                 merge(current_test_case.number_of_input_elements) |
                 scan_range_operation::builder(lower_boundary, upper_boundary)
                         .is_inclusive(true)
                         .parser<parser>(current_test_case.number_of_input_elements)
                         .input_vector_width(current_test_case.input_bit_width)
                         .output_vector_width(current_test_case.output_bit_width)
                         .build();

    ASSERT_NO_THROW(auto chain_result = test::execute(chain, compressed_source, destination);
                            chain_result_value = handle_result(chain_result));

    // Reference Act

    // Prepare Scan_Range operation
    auto scan_range_operation = scan_range_operation::builder(lower_boundary, upper_boundary)
            .is_inclusive(true)
            .parser<parser>(current_test_case.number_of_input_elements)
            .input_vector_width(current_test_case.input_bit_width)
            .output_vector_width(current_test_case.output_bit_width)
            .build();

    // Run Scan_Range operation

    auto scan_range_result = test::execute(scan_range_operation, source, reference_destination);
    ASSERT_NO_THROW(scan_range_result_value = handle_result(scan_range_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, reference_destination));
}

template <parsers parser>
using inflate_extract_t = MergeChainTest<parser, inflate_operation, extract_operation>;

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(merge_chain_inflate_extract_le,
                                       DISABLED_little_endian,  // TODO: Will be enabled when compression for middle layer is implemented
                                       inflate_extract_t<little_endian_packed_array>) {
    // Variables
    const uint32_t lower_boundary       = current_test_case.number_of_input_elements / 3;
    const uint32_t upper_boundary       = lower_boundary * 2;
    uint32_t       chain_result_value   = 0;
    uint32_t       extract_result_value = 0;
    const parsers  parser               = parsers::little_endian_packed_array;

    // Act
    auto chain = inflate_operation() |
                 merge(current_test_case.number_of_input_elements) |
                 extract_operation::builder(lower_boundary, upper_boundary)
                         .parser<parser>(current_test_case.number_of_input_elements)
                         .input_vector_width(current_test_case.input_bit_width)
                         .output_vector_width(current_test_case.output_bit_width)
                         .build();

    ASSERT_NO_THROW(auto chain_result = test::execute(chain, compressed_source, destination);
                            chain_result_value = handle_result(chain_result));

    // Reference Act

    // Prepare Extract operation
    auto extract_operation = extract_operation::builder(lower_boundary, upper_boundary)
            .parser<parser>(current_test_case.number_of_input_elements)
            .input_vector_width(current_test_case.input_bit_width)
            .output_vector_width(current_test_case.output_bit_width)
            .build();

    // Run Extract operation

    auto extract_result = test::execute(extract_operation, source, reference_destination);
    ASSERT_NO_THROW(extract_result_value = handle_result(extract_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, reference_destination));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(merge_chain_inflate_extract_be,
                                       DISABLED_big_endian,  // TODO: Will be enabled when compression for middle layer is implemented
                                       inflate_extract_t<big_endian_packed_array>) {
    // Variables
    const uint32_t lower_boundary       = current_test_case.number_of_input_elements / 3;
    const uint32_t upper_boundary       = lower_boundary * 2;
    uint32_t       chain_result_value   = 0;
    uint32_t       extract_result_value = 0;
    const parsers  parser               = parsers::big_endian_packed_array;

    // Act
    auto chain = inflate_operation() |
                 merge(current_test_case.number_of_input_elements) |
                 extract_operation::builder(lower_boundary, upper_boundary)
                         .parser<parser>(current_test_case.number_of_input_elements)
                         .input_vector_width(current_test_case.input_bit_width)
                         .output_vector_width(current_test_case.output_bit_width)
                         .build();

    ASSERT_NO_THROW(auto chain_result = test::execute(chain, compressed_source, destination);
                            chain_result_value = handle_result(chain_result));

    // Reference Act

    // Prepare Extract operation
    auto extract_operation = extract_operation::builder(lower_boundary, upper_boundary)
            .parser<parser>(current_test_case.number_of_input_elements)
            .input_vector_width(current_test_case.input_bit_width)
            .output_vector_width(current_test_case.output_bit_width)
            .build();

    // Run Extract operation

    auto extract_result = test::execute(extract_operation, source, reference_destination);
    ASSERT_NO_THROW(extract_result_value = handle_result(extract_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, reference_destination));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(merge_chain_inflate_extract_prle,
                                       DISABLED_parquet_rle,  // TODO: Will be enabled when compression for middle layer is implemented
                                       inflate_extract_t<parquet_rle>) {
    // Variables
    const uint32_t lower_boundary       = current_test_case.number_of_input_elements / 3;
    const uint32_t upper_boundary       = lower_boundary * 2;
    uint32_t       chain_result_value   = 0;
    uint32_t       extract_result_value = 0;
    const parsers  parser               = parsers::parquet_rle;

    // Act
    auto chain = inflate_operation() |
                 merge(current_test_case.number_of_input_elements) |
                 extract_operation::builder(lower_boundary, upper_boundary)
                         .parser<parser>(current_test_case.number_of_input_elements)
                         .input_vector_width(current_test_case.input_bit_width)
                         .output_vector_width(current_test_case.output_bit_width)
                         .build();

    ASSERT_NO_THROW(auto chain_result = test::execute(chain, compressed_source, destination);
                            chain_result_value = handle_result(chain_result));

    // Reference Act

    // Prepare Extract operation
    auto extract_operation = extract_operation::builder(lower_boundary, upper_boundary)
            .parser<parser>(current_test_case.number_of_input_elements)
            .input_vector_width(current_test_case.input_bit_width)
            .output_vector_width(current_test_case.output_bit_width)
            .build();

    // Run Extract operation

    auto extract_result = test::execute(extract_operation, source, reference_destination);
    ASSERT_NO_THROW(extract_result_value = handle_result(extract_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, reference_destination));
}

template <parsers parser>
using inflate_expand_t = MergeChainTest<parser, inflate_operation, expand_operation>;

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(merge_chain_inflate_expand_le,
                                       DISABLED_little_endian, // TODO: Will be enabled when compression for middle layer is implemented
                                       inflate_expand_t<little_endian_packed_array>) {
    // Variables
    uint32_t      chain_result_value  = 0;
    uint32_t      expand_result_value = 0;
    const parsers parser              = parsers::little_endian_packed_array;

    auto expand_mask = generate_mask(current_test_case.number_of_input_elements);

    // Act
    auto chain = inflate_operation() |
                 merge(current_test_case.number_of_input_elements) |
                 expand_operation::builder(expand_mask.data(), expand_mask.size())
                         .parser<parser>(current_test_case.number_of_input_elements)
                         .input_vector_width(current_test_case.input_bit_width)
                         .output_vector_width(current_test_case.output_bit_width)
                         .build();

    ASSERT_NO_THROW(auto chain_result = test::execute(chain, compressed_source, destination);
                            chain_result_value = handle_result(chain_result));

    // Reference Act

    // Prepare Expand operation
    auto expand_operation = expand_operation::builder(expand_mask.data(), expand_mask.size())
            .parser<parser>(current_test_case.number_of_input_elements)
            .input_vector_width(current_test_case.input_bit_width)
            .output_vector_width(current_test_case.output_bit_width)
            .build();

    // Run Expand operation

    auto expand_result = test::execute(expand_operation, source, reference_destination);
    ASSERT_NO_THROW(expand_result_value = handle_result(expand_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, reference_destination));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(merge_chain_inflate_expand_be,
                                       DISABLED_big_endian, // TODO: Will be enabled when compression for middle layer is implemented
                                       inflate_expand_t<big_endian_packed_array>) {
    // Variables
    uint32_t      chain_result_value  = 0;
    uint32_t      expand_result_value = 0;
    const parsers parser              = parsers::big_endian_packed_array;

    auto expand_mask = generate_mask(current_test_case.number_of_input_elements);

    // Act
    auto chain = inflate_operation() |
                 merge(current_test_case.number_of_input_elements) |
                 expand_operation::builder(expand_mask.data(), expand_mask.size())
                         .parser<parser>(current_test_case.number_of_input_elements)
                         .input_vector_width(current_test_case.input_bit_width)
                         .output_vector_width(current_test_case.output_bit_width)
                         .build();

    ASSERT_NO_THROW(auto chain_result = test::execute(chain, compressed_source, destination);
                            chain_result_value = handle_result(chain_result));

    // Reference Act

    // Prepare Expand operation
    auto expand_operation = expand_operation::builder(expand_mask.data(), expand_mask.size())
            .parser<parser>(current_test_case.number_of_input_elements)
            .input_vector_width(current_test_case.input_bit_width)
            .output_vector_width(current_test_case.output_bit_width)
            .build();

    // Run Expand operation

    auto expand_result = test::execute(expand_operation, source, reference_destination);
    ASSERT_NO_THROW(expand_result_value = handle_result(expand_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, reference_destination));
}

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(merge_chain_inflate_expand_prle,
                                       DISABLED_parquet_rle, // TODO: Will be enabled when compression for middle layer is implemented
                                       inflate_expand_t<parquet_rle>) {
    // Variables
    uint32_t      chain_result_value  = 0;
    uint32_t      expand_result_value = 0;
    const parsers parser              = parsers::parquet_rle;

    auto expand_mask = generate_mask(current_test_case.number_of_input_elements);

    // Act
    auto chain = inflate_operation() |
                 merge(current_test_case.number_of_input_elements) |
                 expand_operation::builder(expand_mask.data(), expand_mask.size())
                         .parser<parser>(current_test_case.number_of_input_elements)
                         .input_vector_width(current_test_case.input_bit_width)
                         .output_vector_width(current_test_case.output_bit_width)
                         .build();

    ASSERT_NO_THROW(auto chain_result = test::execute(chain, compressed_source, destination);
                            chain_result_value = handle_result(chain_result));

    // Reference Act

    // Prepare Expand operation
    auto expand_operation = expand_operation::builder(expand_mask.data(), expand_mask.size())
            .parser<parser>(current_test_case.number_of_input_elements)
            .input_vector_width(current_test_case.input_bit_width)
            .output_vector_width(current_test_case.output_bit_width)
            .build();

    // Run Expand operation

    auto expand_result = test::execute(expand_operation, source, reference_destination);
    ASSERT_NO_THROW(expand_result_value = handle_result(expand_result));

    // Assert
    EXPECT_TRUE(CompareVectors(destination, reference_destination));
}

} // namespace qpl::test
