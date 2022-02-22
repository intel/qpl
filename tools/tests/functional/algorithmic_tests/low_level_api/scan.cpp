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
#include "qpl_api_ref.h"
#include "ta_ll_common.hpp"
#include "check_result.hpp"

namespace qpl::test
{
    class ScanTest : public AnalyticFixture
    {
    public:
        void InitializeTestCases()
        {
            std::vector<uint32_t>     destination_bit_widths = {1u, 8u, 16u, 32u};
            std::vector<uint32_t>     lengths                = GenerateNumberOfElementsVector();

            std::vector<uint32_t>     output_format_flags    = {0, QPL_FLAG_OUT_BE};

            for (uint32_t length : lengths)
            {
                for (uint32_t source_bit_width = 1; source_bit_width <= 32; source_bit_width++)
                {
                    for (auto destination_bit_width : destination_bit_widths)
                    {
                        const uint32_t max_output_value = (1ULL << destination_bit_width) - 1;

                        if (destination_bit_width != 1u && max_output_value < length)
                        {
                            continue;
                        }

                        for (auto parser : {qpl_p_le_packed_array, qpl_p_be_packed_array, qpl_p_parquet_rle})
                        {
                            for (auto flag : output_format_flags)
                            {
                                uint32_t max_input_value = (1ull << source_bit_width) - 1;
                                AnalyticTestCase test_case;
                                test_case.number_of_elements = length;
                                test_case.source_bit_width = source_bit_width;
                                test_case.destination_bit_width = destination_bit_width;
                                test_case.lower_bound = max_input_value / 4;
                                test_case.upper_bound = max_input_value / 4 * 3;
                                test_case.parser = parser;
                                test_case.flags = flag;

                                AddNewTestCase(test_case);
                            }
                        }
                    }
                }
            }
        }

        void SetUp() override
        {
            AnalyticFixture::SetUp();
            InitializeTestCases();
        }
    };

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_eq_only, ScanTest)
    {
        job_ptr->op = qpl_op_scan_eq;
        reference_job_ptr->op = qpl_op_scan_eq;
        auto status           = run_job_api(job_ptr);
        auto reference_status = ref_compare(reference_job_ptr);

        EXPECT_EQ(QPL_STS_OK, status);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());
        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_ne_only, ScanTest)
    {
        job_ptr->op = qpl_op_scan_ne;
        reference_job_ptr->op = qpl_op_scan_ne;
        auto status           = run_job_api(job_ptr);
        auto reference_status = ref_compare(reference_job_ptr);

        EXPECT_EQ(QPL_STS_OK, status);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());
        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_lt_only, ScanTest)
    {
        job_ptr->op = qpl_op_scan_lt;
        reference_job_ptr->op = qpl_op_scan_lt;
        auto status           = run_job_api(job_ptr);
        auto reference_status = ref_compare(reference_job_ptr);

        EXPECT_EQ(QPL_STS_OK, status);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());
        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_le_only, ScanTest)
    {
        job_ptr->op = qpl_op_scan_le;
        reference_job_ptr->op = qpl_op_scan_le;
        auto status           = run_job_api(job_ptr);
        auto reference_status = ref_compare(reference_job_ptr);

        EXPECT_EQ(QPL_STS_OK, status);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());
        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_gt_only, ScanTest)
    {
        job_ptr->op = qpl_op_scan_gt;
        reference_job_ptr->op = qpl_op_scan_gt;
        auto status           = run_job_api(job_ptr);
        auto reference_status = ref_compare(reference_job_ptr);

        EXPECT_EQ(QPL_STS_OK, status);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());
        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_ge_only, ScanTest)
    {
        job_ptr->op = qpl_op_scan_ge;
        reference_job_ptr->op = qpl_op_scan_ge;
        auto status           = run_job_api(job_ptr);
        auto reference_status = ref_compare(reference_job_ptr);

        EXPECT_EQ(QPL_STS_OK, status);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());
        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_range_only, ScanTest)
    {
        job_ptr->op = qpl_op_scan_range;
        reference_job_ptr->op = qpl_op_scan_range;
        auto status           = run_job_api(job_ptr);
        auto reference_status = ref_compare(reference_job_ptr);

        EXPECT_EQ(QPL_STS_OK, status);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());
        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_not_range_only, ScanTest)
    {
        job_ptr->op = qpl_op_scan_not_range;
        reference_job_ptr->op = qpl_op_scan_not_range;
        auto status           = run_job_api(job_ptr);
        auto reference_status = ref_compare(reference_job_ptr);

        EXPECT_EQ(QPL_STS_OK, status);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());
        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_eq_with_decompress, ScanTest)
    {
        job_ptr->op = qpl_op_scan_eq;
        reference_job_ptr->op = qpl_op_scan_eq;
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

        auto reference_status = ref_compare(reference_job_ptr);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_ne_with_decompress, ScanTest)
    {
        job_ptr->op = qpl_op_scan_ne;
        reference_job_ptr->op = qpl_op_scan_ne;
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

        auto reference_status = ref_compare(reference_job_ptr);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_lt_with_decompress, ScanTest)
    {
        job_ptr->op = qpl_op_scan_lt;
        reference_job_ptr->op = qpl_op_scan_lt;
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

        auto reference_status = ref_compare(reference_job_ptr);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_le_with_decompress, ScanTest)
    {
        job_ptr->op = qpl_op_scan_le;
        reference_job_ptr->op = qpl_op_scan_le;
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

        auto reference_status = ref_compare(reference_job_ptr);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_ge_with_decompress, ScanTest)
    {
        job_ptr->op = qpl_op_scan_ge;
        reference_job_ptr->op = qpl_op_scan_ge;
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

        auto reference_status = ref_compare(reference_job_ptr);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_range_with_decompress, ScanTest)
    {
        job_ptr->op = qpl_op_scan_range;
        reference_job_ptr->op = qpl_op_scan_range;
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

        auto reference_status = ref_compare(reference_job_ptr);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_not_range_with_decompress, ScanTest)
    {
        job_ptr->op = qpl_op_scan_not_range;
        reference_job_ptr->op = qpl_op_scan_not_range;
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

        auto reference_status = ref_compare(reference_job_ptr);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_le_initial_output_index, ScanTest)
    {
        // Skip nominal output
        if (1u == current_test_case.destination_bit_width) {
            return;
        }

        uint32_t max_available_index = (uint32_t)((1llu << current_test_case.destination_bit_width) - 1u);
        if (current_test_case.number_of_elements > max_available_index) {
            return;
        }
        job_ptr->initial_output_index = max_available_index - current_test_case.number_of_elements;
        reference_job_ptr->initial_output_index = max_available_index - current_test_case.number_of_elements;

        job_ptr->op = qpl_op_scan_le;
        reference_job_ptr->op = qpl_op_scan_le;
        auto status = run_job_api(job_ptr);
        auto reference_status = ref_compare(reference_job_ptr);

        EXPECT_EQ(QPL_STS_OK, status);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());
        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_range_initial_output_index, ScanTest)
    {
        // Skip nominal output
        if (1u == current_test_case.destination_bit_width) {
            return;
        }

        uint32_t max_available_index = (uint32_t)((1llu << current_test_case.destination_bit_width) - 1u);
        if (current_test_case.number_of_elements > max_available_index) {
            return;
        }
        job_ptr->initial_output_index = max_available_index - current_test_case.number_of_elements;
        reference_job_ptr->initial_output_index = max_available_index - current_test_case.number_of_elements;

        job_ptr->op = qpl_op_scan_range;
        reference_job_ptr->op = qpl_op_scan_range;
        auto status = run_job_api(job_ptr);
        auto reference_status = ref_compare(reference_job_ptr);

        EXPECT_EQ(QPL_STS_OK, status);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());
        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }
}
