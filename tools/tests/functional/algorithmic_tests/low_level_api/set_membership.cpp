/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/


#include <vector>
#include <string>
#include "gtest/gtest.h"
#include "qpl/qpl.h"
#include "../../../common/operation_test.hpp"
#include "../../../common/analytic_mask_fixture.hpp"
#include "util.hpp"
#include "qpl_api_ref.h"
#include "ta_ll_common.hpp"
#include "check_result.hpp"

namespace qpl::test
{
    class SetMembershipTest : public AnalyticMaskFixture
    {
    public:
        void InitializeTestCases()
        {
            std::vector<uint32_t> lengths = GenerateNumberOfElementsVector();

            for (uint32_t length : lengths)
            {
                for (uint32_t source_bit_width = 1u; source_bit_width <= 32u; source_bit_width++)
                {
                    for (uint32_t output_bit_width : {1u, 8u, 16u, 32u})
                    {
                        if (output_bit_width != 1u && (1u << output_bit_width) <= length)
                        {
                            continue;
                        }

                        for (auto parser : {qpl_p_le_packed_array, qpl_p_be_packed_array, qpl_p_parquet_rle})
                        {

                            AnalyticTestCase test_case;
                            test_case.operation = qpl_op_set_membership;
                            test_case.number_of_elements = length;
                            test_case.lower_bound = source_bit_width / 4;
                            test_case.upper_bound = source_bit_width / 4;

                            uint32_t actual_bit_width = source_bit_width - test_case.upper_bound - test_case.lower_bound;

                            if (actual_bit_width > 14)
                            {
                                test_case.upper_bound = source_bit_width - test_case.lower_bound - 14;
                            }

                            test_case.second_input_bit_width = 1;
                            test_case.second_input_num_elements =
                                1ULL << (source_bit_width - test_case.lower_bound - test_case.upper_bound);
                            test_case.source_bit_width = source_bit_width;
                            test_case.destination_bit_width = 1;

                            test_case.parser = parser;
                            test_case.flags = 0;

                            AddNewTestCase(test_case);

                            test_case.flags = QPL_FLAG_SRC2_BE;
                            AddNewTestCase(test_case);

                            test_case.flags = QPL_FLAG_OUT_BE;
                            AddNewTestCase(test_case);
                        }
                    }
                }
            }
        }

    void
    SetUp() override
        {
            AnalyticMaskFixture::SetUp();
            InitializeTestCases();
        }
    };

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(set_membership, analytic_only, SetMembershipTest)
    {
        static uint32_t test_case_counter = 0;

        if (0u == test_case_counter % 5000u)
        {
            std::cout << "Running test case number " << test_case_counter << std::endl;
        }

        auto status = run_job_api(job_ptr);

        auto reference_status = ref_set_membership(reference_job_ptr);

        EXPECT_EQ(QPL_STS_OK, status);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        if (!(current_test_case.flags & QPL_FLAG_OUT_BE))
        {
            // Temporary fix for test @todo
            // Reference output is not correct
            EXPECT_TRUE(CompareTotalInOutWithReference());
        }

        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
        test_case_counter++;
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(set_membership, analytic_with_decompress, SetMembershipTest)
    {
        static uint32_t test_case_counter = 0;

        if (0u == test_case_counter % 5000u)
        {
            std::cout << "Running test case number " << test_case_counter << std::endl;
        }

        size_t source_bit_width = current_test_case.source_bit_width -
                                  (current_test_case.lower_bound + current_test_case.upper_bound);

        if (source_bit_width > 8 && GetExecutionPath() == qpl_path_hardware)
        {
            // When decompression is enabled there's a maximum number of bits for source 1 elements
            SUCCEED();
            return;
        }

        if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
            job_ptr->src1_bit_width = 0u;
        }

        std::vector<uint8_t> compressed_source;
        ASSERT_NO_THROW(compressed_source = GetCompressedSource());
        job_ptr->available_in = static_cast<uint32_t>(compressed_source.size());
        job_ptr->next_in_ptr  = compressed_source.data();
        job_ptr->flags   |= QPL_FLAG_DECOMPRESS_ENABLE;

        auto status = run_job_api(job_ptr);
        EXPECT_EQ(QPL_STS_OK, status);

        auto reference_status = ref_set_membership(reference_job_ptr);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination));
        test_case_counter++;
    }
}
