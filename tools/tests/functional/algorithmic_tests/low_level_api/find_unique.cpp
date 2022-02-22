/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <vector>
#include <string>
#include "gtest/gtest.h"
#include "qpl/qpl.h"
#include "../../../common/analytic_fixture.hpp"
#include "util.hpp"
#include "source_provider.hpp"
#include "qpl_api_ref.h"
#include "ta_ll_common.hpp"
#include "check_result.hpp"

namespace qpl::test
{
    class FindUniqueTest : public AnalyticFixture
    {
    public:
        void InitializeTestCases()
        {
            std::vector<uint32_t> lengths = GenerateNumberOfElementsVector();

            for (uint32_t length : lengths)
            {
                for (uint32_t source_bit_width = 1; source_bit_width <= 32; source_bit_width++)
                {
                    for (auto parser : {qpl_p_le_packed_array, qpl_p_be_packed_array, qpl_p_parquet_rle})
                    {
                            AnalyticTestCase test_case;
                            test_case.operation             = qpl_op_find_unique;
                            test_case.number_of_elements    = length;
                            test_case.source_bit_width      = source_bit_width;
                            test_case.destination_bit_width = 1;
                            test_case.lower_bound           = source_bit_width / 4;
                            test_case.upper_bound           = source_bit_width / 4;

                           // Lower/upper bound means drop hither or lower order bits
                           // Find unique results vector of size 2^N, N = actual bit width
                           // Maximum destination size is 2 ^ 14
                            uint32_t actual_bit_width =
                                             source_bit_width - test_case.upper_bound - test_case.lower_bound;

                            if (actual_bit_width > 15)
                            {
                                test_case.upper_bound = source_bit_width - test_case.lower_bound - 15;
                            }

                            test_case.parser = parser;
                            test_case.flags  = 0;
                            AddNewTestCase(test_case);
                    }
                }
            }
        }

        void SetBuffers() override
        {
            source_provider source_gen(current_test_case.number_of_elements,
                                       current_test_case.source_bit_width,
                                       GetSeed(),
                                       current_test_case.parser);

            source = source_gen.get_source();

            uint32_t drop_bits = current_test_case.lower_bound + current_test_case.upper_bound;

            uint32_t destination_size_bits = 1ULL << (current_test_case.source_bit_width - drop_bits);
            uint32_t destination_size_bytes = (destination_size_bits + max_bit_index) >> bit_to_byte_shift_offset;
            destination.resize(destination_size_bytes);
            reference_destination.resize(destination_size_bytes);

            std::fill(destination.begin(), destination.end(), 0u);
            std::fill(reference_destination.begin(), reference_destination.end(), 0u);

            job_ptr->available_in           = static_cast<uint32_t>(source.size());
            job_ptr->next_in_ptr            = source.data();
            reference_job_ptr->available_in = static_cast<uint32_t>(source.size());
            reference_job_ptr->next_in_ptr  = source.data();

            job_ptr->available_out           = static_cast<uint32_t>(destination.size());
            job_ptr->next_out_ptr            = destination.data();
            reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
            reference_job_ptr->next_out_ptr  = reference_destination.data();
        }

        void SetUp() override
        {
            AnalyticFixture::SetUp();
            InitializeTestCases();
        }
    };

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(find_unique, analytic_only, FindUniqueTest)
    {
        auto status = run_job_api(job_ptr);

        auto reference_status = ref_find_unique(reference_job_ptr);

        EXPECT_EQ(QPL_STS_OK, status);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());

        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));

        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(find_unique, analytic_with_decompress, FindUniqueTest)
    {
        size_t source_bit_width = current_test_case.source_bit_width -
                                  (current_test_case.lower_bound + current_test_case.upper_bound);

        if (source_bit_width > 8)
        {
            // When decompression is enabled there's a maximum number of bits for source 1 elements
            SUCCEED();
            return;
        }

        std::vector<uint8_t> compressed_source;
        ASSERT_NO_THROW(compressed_source = GetCompressedSource());
        job_ptr->available_in = static_cast<uint32_t>(compressed_source.size());
        job_ptr->next_in_ptr  = compressed_source.data();
        job_ptr->flags   |= QPL_FLAG_DECOMPRESS_ENABLE;

        if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
            job_ptr->src1_bit_width = 0u;
        }

        auto status = run_job_api(job_ptr);
        EXPECT_EQ(QPL_STS_OK, status);

        auto reference_status = ref_find_unique(reference_job_ptr);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }
}
