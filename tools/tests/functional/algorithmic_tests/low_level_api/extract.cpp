/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <vector>
#include <string>
#include <numeric>
#include "gtest/gtest.h"
#include "qpl/qpl.h"
#include "../../../common/analytic_fixture.hpp"
#include "util.hpp"
#include "source_provider.hpp"
#include "qpl_api_ref.h"
#include "ta_ll_common.hpp"
#include "format_generator.hpp"
#include "check_result.hpp"

namespace qpl::test
{
    class ExtractTest : public AnalyticFixture
    {
    public:
        void InitializeTestCases()
        {
            std::vector<uint32_t> bit_widths(32u, 0u);
            std::iota(bit_widths.begin(), bit_widths.end(), 1u);
            auto number_of_elements = format_generator::generate_length_sequence();
            std::vector<uint64_t> output_format_flags = {0u, QPL_FLAG_OUT_BE};

            uint32_t test_cases_counter = 0;

            for (uint32_t length_in_elements : number_of_elements)
            {
                for (auto source_bit_width : bit_widths)
                {
                    for (auto destination_bit_width : {1u, 8u, 16u, 32u})
                    {
                        if (destination_bit_width != 1u)
                        {
                            if ((1ull << destination_bit_width) <= length_in_elements)
                            {
                                continue;
                            } else if (source_bit_width > destination_bit_width)
                            {
                                continue;
                            }
                        }

                        for (auto parser : {qpl_p_le_packed_array, qpl_p_be_packed_array, qpl_p_parquet_rle})
                        {
                            for (uint64_t output_flag : output_format_flags)
                            {
                                AnalyticTestCase test_case;
                                test_case.operation = qpl_op_extract;
                                test_case.number_of_elements = length_in_elements;
                                test_case.source_bit_width = source_bit_width;
                                test_case.destination_bit_width = destination_bit_width;
                                test_case.lower_bound = length_in_elements / 4u;
                                test_case.upper_bound = (length_in_elements / 4u) * 3u;
                                test_case.parser = parser;
                                test_case.flags = output_flag;

                                AddNewTestCase(test_case);

                                test_cases_counter++;
                            }
                        }
                    }
                }
            }

            std::cout << "Total number of test cases: " << test_cases_counter << std::endl;
        }

        void SetUp() override
        {
            AnalyticFixture::SetUp();
            InitializeTestCases();
        }
    };

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(extract, analytic_only, ExtractTest)
    {
        static uint32_t test_case_counter = 0;

        if (0u == test_case_counter % 5000u)
        {
            std::cout << " Running test case number " << test_case_counter << std::endl;
        }

        auto status = run_job_api(job_ptr);

        auto reference_status = ref_extract(reference_job_ptr);

        EXPECT_EQ(QPL_STS_OK, status);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());

        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));

        EXPECT_TRUE(CompareVectors(destination, reference_destination));
        test_case_counter++;
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(extract, analytic_with_decompress, ExtractTest)
    {
        static uint32_t test_case_counter = 0;

        if (0u == test_case_counter % 5000u)
        {
            std::cout << " Running test case number " << test_case_counter << std::endl;
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

        auto reference_status = ref_extract(reference_job_ptr);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination));
        test_case_counter++;
    }


    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(extract, DISABLED_analytic_with_random_decompress, ExtractTest)
    {
        static uint32_t test_case_counter = 0;

        if (0u == test_case_counter % 5000u)
        {
            std::cout << " Running test case number " << test_case_counter << std::endl;
        }

        std::vector<uint8_t> compressed_source;
        ASSERT_NO_THROW(compressed_source = GetCompressedSource(true));

        if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
            job_ptr->src1_bit_width = 0u;
        }

        auto saved_in_ptr = compressed_source.data();
        auto saved_flags  = job_ptr->flags;
        auto saved_op     = job_ptr->op;

        auto header_index = index_table.findHeaderBlockIndex(0);
        auto header_index_start  = index_table[header_index];
        auto header_index_finish = index_table[header_index + 1u];

        auto bit_start = header_index_start.bit_offset;
        auto bit_end   = header_index_finish.bit_offset;

        uint8_t *start = saved_in_ptr + bit_start / 8u;

        // Decompress header for random access
        job_ptr->op    = qpl_op_decompress;
        job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_NO_BUFFERING;

        job_ptr->ignore_start_bits = bit_start & 7u;
        job_ptr->ignore_end_bits   = 7u & (0u - bit_end);
        job_ptr->available_in      = ((bit_end + 7u) / 8u) - (bit_start / 8u);
        job_ptr->next_in_ptr       = start;

        auto status = run_job_api(job_ptr);
        EXPECT_EQ(QPL_STS_OK, status);

        // Filter with random access
        auto mini_block_index = index_table.findMiniBlockIndex(0);

        bit_start = index_table[mini_block_index].bit_offset;
        bit_end   = index_table[mini_block_index + 1u].bit_offset;

        job_ptr->next_in_ptr       = saved_in_ptr + bit_start / 8u;
        job_ptr->ignore_start_bits = bit_start & 7u;
        job_ptr->ignore_end_bits   = 7u & (0u - bit_end);
        job_ptr->available_in      = ((bit_end + 7u) / 8u) - (bit_start / 8u);
        job_ptr->crc               = index_table[mini_block_index].crc;

        job_ptr->op    = saved_op;
        job_ptr->flags = saved_flags | QPL_FLAG_DECOMPRESS_ENABLE | QPL_FLAG_NO_BUFFERING | QPL_FLAG_RND_ACCESS;

        status = run_job_api(job_ptr);
        EXPECT_EQ(QPL_STS_OK, status);

        auto reference_status = ref_extract(reference_job_ptr);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination));
        test_case_counter++;
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(extract, initial_output_index, ExtractTest)
    {
        static uint32_t test_case_counter = 0;

        // Skip nominal and extended output
        if ((1u == current_test_case.destination_bit_width) || (1u != current_test_case.source_bit_width)) {
            return;
        }

        uint32_t max_available_index = (uint32_t)((1llu << current_test_case.destination_bit_width) - 1u);
        if (current_test_case.number_of_elements > max_available_index) {
            return;
        }
        job_ptr->initial_output_index = max_available_index - current_test_case.number_of_elements;
        reference_job_ptr->initial_output_index = max_available_index - current_test_case.number_of_elements;

        auto status = run_job_api(job_ptr);

        auto reference_status = ref_extract(reference_job_ptr);

        EXPECT_EQ(QPL_STS_OK, status);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());

        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));

        EXPECT_TRUE(CompareVectors(destination, reference_destination));
        test_case_counter++;
    }
}
