/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <algorithm>
#include "../../../common/operation_test.hpp"
#include "qpl_api_ref.h"
#include "ta_ll_common.hpp"
#include "random_generator.h"
#include "source_provider.hpp"

namespace qpl::test
{
    class ZeroDecompressTest : public ReferenceFixture
    {
    public:
        template<typename DataType>
        testing::AssertionResult RunZeroSourceVectorTest()
        {
            encoded_source.resize(number_of_elements * sizeof(DataType));
            destination.resize(encoded_source.size() * 2);
            reference_destination.resize(encoded_source.size() * 2);

            std::fill(encoded_source.begin(), encoded_source.end(), 0u); // Fill vector with 0s

            try
            {
                source = ZeroCompressVector<DataType>(encoded_source); // Then, compress this vector
            } catch (...)
            {
                return testing::AssertionFailure() << "Failed to compress given source\n";
            }

            EXPECT_TRUE(ZeroDecompressAndCompareToReference<DataType>()); // Try to decompress it

            destination.resize(job_ptr->total_out);

            return CompareVectors(destination, encoded_source); // Compare the result with the source
        }

        template<typename DataType>
        testing::AssertionResult RunRandomValuesVectorTest()
        {
            encoded_source.resize(number_of_elements * sizeof(DataType));
            destination.resize(encoded_source.size() * 2);
            reference_destination.resize(encoded_source.size() * 2);

            qpl::test::random random_number(0, (DataType) UINT32_MAX, GetSeed());

            auto *source_ptr = reinterpret_cast<DataType *>(encoded_source.data());
            std::generate(source_ptr, source_ptr + number_of_elements,
                          [&random_number]()
                          {
                              return static_cast<DataType>(random_number);
                          });

            if ((GetSeed() & 1) == 0)
            {
                source_ptr[0] = 0;
            }

            for (uint32_t i = 1; i < number_of_elements; i++)
            {
                if ((source_ptr[i] & 3) == 0)
                {
                    //change type
                    if (source_ptr[i - 1] != 0)
                        source_ptr[i] = 0;
                } else
                {
                    // keep type
                    if (source_ptr[i - 1] == 0)
                        source_ptr[i] = 0;
                }
            }

            try {
                source = ZeroCompressVector<DataType>(encoded_source); // Then, compress this vector
            } catch (...) {
                return testing::AssertionFailure() << "Failed to compress given source\n";
            }

            EXPECT_TRUE(ZeroDecompressAndCompareToReference<DataType>()); // Try to decompress it

            destination.resize(job_ptr->total_out);

            return CompareVectors(destination, encoded_source);
        }

        template<typename DataType>
        testing::AssertionResult RunGeneratedDataTest()
        {
            uint32_t bit_width = 0;

            switch (sizeof(DataType))
            {
                case 2u:
                    bit_width = 16;
                    break;

                case 4u:
                    bit_width = 32;
                    break;

                default:
                    return testing::AssertionFailure();
            }

            source_provider source_generator(number_of_elements,
                                             bit_width,
                                             GetSeed());

            source = source_generator.get_zero_compressed_source(); // This produces random zero compressed stream

            destination.resize(source.size() * 3);
            reference_destination.resize(source.size() * 3);

            return ZeroDecompressAndCompareToReference<DataType>(); // Try to decompress it
        }

    private:
        template<typename DataType>
        testing::AssertionResult ZeroDecompressAndCompareToReference()
        {
            job_ptr->next_in_ptr  = source.data();
            job_ptr->available_in = static_cast<uint32_t>(source.size());

            job_ptr->next_out_ptr  = destination.data();
            job_ptr->available_out = static_cast<uint32_t>(destination.size());

            reference_job_ptr->next_in_ptr  = source.data();
            reference_job_ptr->available_in = static_cast<uint32_t>(source.size());

            reference_job_ptr->next_out_ptr  = reference_destination.data();
            reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());

            auto      job_status = run_job_api(job_ptr);
            qpl_status reference_status;

            switch (sizeof(DataType))
            {
                case 2u:
                    reference_status = ref_zero_decompress_16u(reference_job_ptr);
                    break;

                case 4u:
                    reference_status = ref_zero_decompress_32u(reference_job_ptr);
                    break;

                default:
                    return testing::AssertionFailure();
            }

            EXPECT_EQ(QPL_STS_OK, job_status);
            EXPECT_EQ(job_status, reference_status);

            EXPECT_TRUE(CompareTotalInOutWithReference());

            return CompareVectors(destination, reference_destination);
        }

        template<typename DataType>
        std::vector<uint8_t> ZeroCompressVector(std::vector<uint8_t> &input)
        {
            std::vector<uint8_t> result(input.size() * 2);

            uint32_t job_size = 0;
            auto     status   = qpl_get_job_size(GetExecutionPath(), &job_size);
            EXPECT_EQ(QPL_STS_OK, status);

            auto job_buffer       = std::make_unique<uint8_t[]>(job_size);
            auto compress_job_ptr = reinterpret_cast<qpl_job *>(job_buffer.get());

            status = qpl_init_job(GetExecutionPath(), compress_job_ptr);
            EXPECT_EQ(QPL_STS_OK, status);

            switch (sizeof(DataType))
            {
                case 2u:
                    compress_job_ptr->op = qpl_op_z_compress16;
                    break;

                case 4u:
                    compress_job_ptr->op = qpl_op_z_compress32;
                    break;

                default:
                    throw std::exception();
            }

            compress_job_ptr->next_in_ptr  = input.data();
            compress_job_ptr->available_in = static_cast<uint32_t>(input.size());

            compress_job_ptr->next_out_ptr  = result.data();
            compress_job_ptr->available_out = static_cast<uint32_t>(result.size());

            status = run_job_api(compress_job_ptr);

            if (QPL_STS_OK != status)
            {
                throw std::exception();
            }

            result.resize(compress_job_ptr->total_out);

            return result;
        }

        std::vector<uint8_t> encoded_source;
        const uint32_t       number_of_elements = 1000u;
    };

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(zero_decompress, decompress_16u_zero_vector, ZeroDecompressTest)
    {
        job_ptr->op           = qpl_op_z_decompress16;
        reference_job_ptr->op = qpl_op_z_decompress16;

        EXPECT_TRUE(RunZeroSourceVectorTest<uint16_t>());
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(zero_decompress, decompress_32u_zero_vector, ZeroDecompressTest)
    {
        job_ptr->op           = qpl_op_z_decompress32;
        reference_job_ptr->op = qpl_op_z_decompress32;

        EXPECT_TRUE(RunZeroSourceVectorTest<uint32_t>());
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(zero_decompress, decompress_32u_random_vector, ZeroDecompressTest)
    {
        job_ptr->op           = qpl_op_z_decompress32;
        reference_job_ptr->op = qpl_op_z_decompress32;

        EXPECT_TRUE(RunRandomValuesVectorTest<uint32_t>());
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(zero_decompress, decompress_16u_random_vector, ZeroDecompressTest)
    {
        job_ptr->op           = qpl_op_z_decompress16;
        reference_job_ptr->op = qpl_op_z_decompress16;

        EXPECT_TRUE(RunRandomValuesVectorTest<uint16_t>());
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(zero_decompress, decompress_32u_generated_vector, ZeroDecompressTest)
    {
        job_ptr->op           = qpl_op_z_decompress32;
        reference_job_ptr->op = qpl_op_z_decompress32;

        EXPECT_TRUE(RunGeneratedDataTest<uint32_t>());
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_F(zero_decompress, decompress_16u_generated_vector, ZeroDecompressTest)
    {
        job_ptr->op           = qpl_op_z_decompress16;
        reference_job_ptr->op = qpl_op_z_decompress16;

        EXPECT_TRUE(RunGeneratedDataTest<uint16_t>());
    }
}
