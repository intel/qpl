/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <vector>
#include <string>

#include "gtest/gtest.h"
#include "../../../common/compression_fixture.hpp"
#include "high_level_api_util.hpp"
#include "qpl/cpp_api/operations/compression/zero_compress_operation.hpp"
#include "qpl/cpp_api/operations/compression/zero_decompress_operation.hpp"

#include "ta_hl_common.hpp"
#include "source_provider.hpp"
#include "qpl_api_ref.h"

namespace qpl::test {

class ZeroCompressionOperationTest : public ZeroCompressionFixture {
public:
    void InitializeTestCases() override {
        ZeroCompressionTestCase test_case;

        for (zero_vector_generation_type vector_generation_type: {zero_vector_generation_type::zero_vector,
                                                                  zero_vector_generation_type::random_vector,
                                                                  zero_vector_generation_type::generated_vector}) {
            for (zero_input_format input_format: {zero_input_format::word_16_bit,
                                                  zero_input_format::word_32_bit}) {
                for (uint32_t number_of_elements = 100; number_of_elements < 1000; number_of_elements++) {
                    test_case.number_of_input_elements = number_of_elements;
                    test_case.input_format             = input_format;
                    test_case.vector_generation_type   = vector_generation_type;

                    this->AddNewTestCase(test_case);
                }
            }
        }
    }
};

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST_TC(zero_compression, common, ZeroCompressionOperationTest) {
    // Variables
    uint32_t compressed_size   = 0;
    uint32_t decompressed_size = 0;

    // Act
    auto zero_compress_operation = qpl::zero_compress_operation(current_test_case.input_format);

    job_ptr->next_in_ptr   = source.data();
    job_ptr->next_out_ptr  = reference_compressed_data.data();
    job_ptr->available_in  = static_cast<uint32_t>(source.size());
    job_ptr->available_out = static_cast<uint32_t>(reference_compressed_data.size());

    ASSERT_NO_THROW(auto zero_compress_result = test::execute(zero_compress_operation, source, compressed_data);
                            compressed_size = handle_result(zero_compress_result));

    auto status = QPL_STS_OK;
    if (current_test_case.input_format == zero_input_format::word_16_bit) {
        job_ptr->op = qpl_op_z_compress16;
        status = ref_zero_compress_16u(job_ptr);
    } else {
        job_ptr->op = qpl_op_z_compress32;
        status = ref_zero_compress_32u(job_ptr);
    }

    // Asserts
    ASSERT_EQ(QPL_STS_OK, status);

    ASSERT_EQ(compressed_size, job_ptr->total_out);

    EXPECT_TRUE(CompareVectors(compressed_data,
                               reference_compressed_data,
                               compressed_size,
                               "Incorrect compressed vectors"));

    // Decompression
    auto zero_decompress_operation = qpl::zero_decompress_operation(current_test_case.input_format);

    job_ptr->next_in_ptr   = reference_compressed_data.data();
    job_ptr->next_out_ptr  = reference_destination.data();
    job_ptr->available_in  = compressed_size;
    job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());

    auto zero_decompress_result = test::execute(zero_decompress_operation,
                                                compressed_data.begin(),
                                                compressed_data.begin() + compressed_size,
                                                destination.begin(),
                                                destination.end());

    ASSERT_NO_THROW(decompressed_size = handle_result(zero_decompress_result));

    if (current_test_case.input_format == zero_input_format::word_16_bit) {
        job_ptr->op = qpl_op_z_decompress16;
        status = ref_zero_decompress_16u(job_ptr);
    } else {
        job_ptr->op = qpl_op_z_decompress32;
        status = ref_zero_decompress_32u(job_ptr);
    }

    // Asserts
    ASSERT_EQ(QPL_STS_OK, status);

    ASSERT_EQ(decompressed_size, job_ptr->total_out);
    ASSERT_EQ(decompressed_size, static_cast<uint32_t>(source.size()));

    EXPECT_TRUE(CompareVectors(destination,
                               reference_destination,
                               0,
                               "Incorrect decompressed vectors"));

    EXPECT_TRUE(CompareVectors(destination,
                               source,
                               0,
                               "The source and decompressed vectors do not match"));
} // TEST

} // namespace qpl::test
