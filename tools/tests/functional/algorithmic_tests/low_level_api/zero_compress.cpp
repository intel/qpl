/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <algorithm>
#include "../../../common/test_cases.hpp"
#include "../../../common/operation_test.hpp"
#include "qpl_api_ref.h"
#include "ta_ll_common.hpp"
#include "random_generator.h"

namespace qpl::test
{
    class ZeroCompressTest : public ReferenceFixtureWithTestCases<uint32_t>
    {
    public:
        template<typename DataType>
        testing::AssertionResult RunZeroSourceVectorTest()
        {
            source.resize(GetTestCase() * sizeof(DataType));
            destination.resize(source.size() * 2);
            reference_destination.resize(source.size() * 2);

            std::fill(source.begin(), source.end(), 0u);

            return ZeroCompressAndCompareToReference<DataType>();
        }

        template<typename DataType>
        testing::AssertionResult RunGeneratedSourceVectorTest()
        {
            source.resize(GetTestCase() * sizeof(DataType));
            destination.resize(source.size() * 2);
            reference_destination.resize(source.size() * 2);

            qpl::test::random random_number(0, (DataType) UINT32_MAX, GetSeed());

            auto *source_ptr = reinterpret_cast<DataType *>(source.data());

            std::generate(source_ptr, source_ptr + GetTestCase(),
                          [&random_number]()
                          {
                              return static_cast<DataType>(random_number);
                          });

            if ((GetSeed() & 1) == 0)
            {
                source_ptr[0] = 0;
            }

            for (uint32_t i = 1; i < GetTestCase(); i++)
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

            return ZeroCompressAndCompareToReference<DataType>();
        }

        void InitializeTestCases() override
        {
            for (uint32_t number_of_elements = 100; number_of_elements < 1000; number_of_elements++)
            {
                AddNewTestCase(number_of_elements);
            }
        }

    private:
        template<typename DataType>
        testing::AssertionResult ZeroCompressAndCompareToReference()
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
                    reference_status = ref_zero_compress_16u(reference_job_ptr);
                    break;

                case 4u:
                    reference_status = ref_zero_compress_32u(reference_job_ptr);
                    break;

                default:
                    return testing::AssertionFailure();
            }

            EXPECT_EQ(QPL_STS_OK, job_status);
            EXPECT_EQ(job_status, reference_status);

            EXPECT_TRUE(CompareTotalInOutWithReference());

            EXPECT_TRUE(CompareVectors(destination, reference_destination));

            destination.resize(job_ptr->total_out);
            std::vector<uint8_t> decompressed_destination;

            decompressed_destination = DecompressData<DataType>(destination,
                                                                static_cast<uint32_t>(source.size()));


            return CompareVectors(decompressed_destination, source);
        }

        template<typename DataType>
        std::vector<uint8_t> DecompressData(std::vector<uint8_t> &input,
                                            uint32_t expectedLength)
        {
            std::vector<uint8_t> result(expectedLength);

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
                    compress_job_ptr->op = qpl_op_z_decompress16;
                    break;

                case 4u:
                    compress_job_ptr->op = qpl_op_z_decompress32;
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
    };

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(zero_compress, compress_16u_zero_vector, ZeroCompressTest)
    {
        job_ptr->op           = qpl_op_z_compress16;
        reference_job_ptr->op = qpl_op_z_compress16;

        EXPECT_TRUE(RunZeroSourceVectorTest<uint16_t>());
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(zero_compress, compress_32u_zero_vector, ZeroCompressTest)
    {
        job_ptr->op           = qpl_op_z_compress32;
        reference_job_ptr->op = qpl_op_z_compress32;

        EXPECT_TRUE(RunZeroSourceVectorTest<uint32_t>());
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(zero_compress, compress_32_generated_vector, ZeroCompressTest)
    {
        job_ptr->op           = qpl_op_z_compress32;
        reference_job_ptr->op = qpl_op_z_compress32;

        EXPECT_TRUE(RunGeneratedSourceVectorTest<uint32_t>());
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(zero_compress, compress_16_generated_vector, ZeroCompressTest)
    {
        job_ptr->op           = qpl_op_z_compress16;
        reference_job_ptr->op = qpl_op_z_compress16;

        EXPECT_TRUE(RunGeneratedSourceVectorTest<uint16_t>());
    }
}
