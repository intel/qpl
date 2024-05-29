/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <cstdlib> //aligned_alloc, free
#include <memory> // unique_ptr
#include <vector>
#include <string>

#include "gtest/gtest.h"

#include "qpl/qpl.h"

// common
#include "analytic_fixture.hpp"
#include "ta_ll_common.hpp"
#include "util.hpp"
#include "system_info.hpp"
#include "common_defs.hpp"

// ref
#include "qpl_api_ref.h"

#if defined(__linux__)
#include <sys/mman.h>
#include <unistd.h>
#include <cerrno>
#endif

namespace qpl::test
{
    class ScanTest : public AnalyticFixture
    {
    public:
        void InitializeTestCases()
        {
            std::vector<uint32_t>     destination_bit_widths = {1U, 8U, 16U, 32U};
            std::vector<uint32_t>     lengths                = GenerateNumberOfElementsVector();

            std::vector<uint32_t>     output_format_flags    = {0, QPL_FLAG_OUT_BE};

            for (uint32_t length : lengths)
            {
                for (uint32_t source_bit_width = 1; source_bit_width <= 32; source_bit_width++)
                {
                    for (auto destination_bit_width : destination_bit_widths)
                    {
                        const uint32_t max_output_value = (1ULL << destination_bit_width) - 1;

                        if (destination_bit_width != 1U && max_output_value < length)
                        {
                            continue;
                        }

                        for (auto parser : {qpl_p_le_packed_array, qpl_p_be_packed_array, qpl_p_parquet_rle})
                        {
                            for (auto flag : output_format_flags)
                            {
                                uint32_t max_input_value = (1ULL << source_bit_width) - 1;
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

        ASSERT_EQ(QPL_STS_OK, status);
        ASSERT_EQ(QPL_STS_OK, reference_status);

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

        ASSERT_EQ(QPL_STS_OK, status);
        ASSERT_EQ(QPL_STS_OK, reference_status);

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

        ASSERT_EQ(QPL_STS_OK, status);
        ASSERT_EQ(QPL_STS_OK, reference_status);

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

        ASSERT_EQ(QPL_STS_OK, status);
        ASSERT_EQ(QPL_STS_OK, reference_status);

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

        ASSERT_EQ(QPL_STS_OK, status);
        ASSERT_EQ(QPL_STS_OK, reference_status);

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

        ASSERT_EQ(QPL_STS_OK, status);
        ASSERT_EQ(QPL_STS_OK, reference_status);

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

        ASSERT_EQ(QPL_STS_OK, status);
        ASSERT_EQ(QPL_STS_OK, reference_status);

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

        ASSERT_EQ(QPL_STS_OK, status);
        ASSERT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());
        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_eq_with_decompress, ScanTest)
    {
        job_ptr->op = qpl_op_scan_eq;
        reference_job_ptr->op = qpl_op_scan_eq;
        std::vector<uint8_t> compressed_source;
        ASSERT_NO_THROW(compressed_source = GetCompressedSource()); //NOLINT(cppcoreguidelines-avoid-goto)
        job_ptr->available_in = static_cast<uint32_t>(compressed_source.size());
        job_ptr->next_in_ptr  = compressed_source.data();
        job_ptr->flags   |= QPL_FLAG_DECOMPRESS_ENABLE;

        if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
            job_ptr->src1_bit_width = 0U;
        }

        auto status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        auto reference_status = ref_compare(reference_job_ptr);
        ASSERT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_ne_with_decompress, ScanTest)
    {
        job_ptr->op = qpl_op_scan_ne;
        reference_job_ptr->op = qpl_op_scan_ne;
        std::vector<uint8_t> compressed_source;
        ASSERT_NO_THROW(compressed_source = GetCompressedSource()); //NOLINT(cppcoreguidelines-avoid-goto)
        job_ptr->available_in = static_cast<uint32_t>(compressed_source.size());
        job_ptr->next_in_ptr  = compressed_source.data();
        job_ptr->flags   |= QPL_FLAG_DECOMPRESS_ENABLE;

        if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
            job_ptr->src1_bit_width = 0U;
        }

        auto status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        auto reference_status = ref_compare(reference_job_ptr);
        ASSERT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_lt_with_decompress, ScanTest)
    {
        job_ptr->op = qpl_op_scan_lt;
        reference_job_ptr->op = qpl_op_scan_lt;
        std::vector<uint8_t> compressed_source;
        ASSERT_NO_THROW(compressed_source = GetCompressedSource()); //NOLINT(cppcoreguidelines-avoid-goto)
        job_ptr->available_in = static_cast<uint32_t>(compressed_source.size());
        job_ptr->next_in_ptr  = compressed_source.data();
        job_ptr->flags   |= QPL_FLAG_DECOMPRESS_ENABLE;

        if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
            job_ptr->src1_bit_width = 0U;
        }

        auto status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        auto reference_status = ref_compare(reference_job_ptr);
        ASSERT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_le_with_decompress, ScanTest)
    {
        job_ptr->op = qpl_op_scan_le;
        reference_job_ptr->op = qpl_op_scan_le;
        std::vector<uint8_t> compressed_source;
        ASSERT_NO_THROW(compressed_source = GetCompressedSource()); //NOLINT(cppcoreguidelines-avoid-goto)
        job_ptr->available_in = static_cast<uint32_t>(compressed_source.size());
        job_ptr->next_in_ptr  = compressed_source.data();
        job_ptr->flags   |= QPL_FLAG_DECOMPRESS_ENABLE;

        if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
            job_ptr->src1_bit_width = 0U;
        }

        auto status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        auto reference_status = ref_compare(reference_job_ptr);
        ASSERT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_ge_with_decompress, ScanTest)
    {
        job_ptr->op = qpl_op_scan_ge;
        reference_job_ptr->op = qpl_op_scan_ge;
        std::vector<uint8_t> compressed_source;
        ASSERT_NO_THROW(compressed_source = GetCompressedSource()); //NOLINT(cppcoreguidelines-avoid-goto)
        job_ptr->available_in = static_cast<uint32_t>(compressed_source.size());
        job_ptr->next_in_ptr  = compressed_source.data();
        job_ptr->flags   |= QPL_FLAG_DECOMPRESS_ENABLE;

        if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
            job_ptr->src1_bit_width = 0U;
        }

        auto status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        auto reference_status = ref_compare(reference_job_ptr);
        ASSERT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_range_with_decompress, ScanTest)
    {
        job_ptr->op = qpl_op_scan_range;
        reference_job_ptr->op = qpl_op_scan_range;
        std::vector<uint8_t> compressed_source;
        ASSERT_NO_THROW(compressed_source = GetCompressedSource()); //NOLINT(cppcoreguidelines-avoid-goto)
        job_ptr->available_in = static_cast<uint32_t>(compressed_source.size());
        job_ptr->next_in_ptr  = compressed_source.data();
        job_ptr->flags   |= QPL_FLAG_DECOMPRESS_ENABLE;

        if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
            job_ptr->src1_bit_width = 0U;
        }

        auto status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        auto reference_status = ref_compare(reference_job_ptr);
        ASSERT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_not_range_with_decompress, ScanTest)
    {
        job_ptr->op = qpl_op_scan_not_range;
        reference_job_ptr->op = qpl_op_scan_not_range;
        std::vector<uint8_t> compressed_source;
        ASSERT_NO_THROW(compressed_source = GetCompressedSource()); //NOLINT(cppcoreguidelines-avoid-goto)
        job_ptr->available_in = static_cast<uint32_t>(compressed_source.size());
        job_ptr->next_in_ptr  = compressed_source.data();
        job_ptr->flags   |= QPL_FLAG_DECOMPRESS_ENABLE;

        if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
            job_ptr->src1_bit_width = 0U;
        }

        auto status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        auto reference_status = ref_compare(reference_job_ptr);
        ASSERT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_le_initial_output_index, ScanTest)
    {
        // Skip nominal output
        if (1U == current_test_case.destination_bit_width) {
            return;
        }

        uint32_t max_available_index = (uint32_t)((1LLU << current_test_case.destination_bit_width) - 1U);
        if (current_test_case.number_of_elements > max_available_index) {
            return;
        }
        job_ptr->initial_output_index = max_available_index - current_test_case.number_of_elements;
        reference_job_ptr->initial_output_index = max_available_index - current_test_case.number_of_elements;

        job_ptr->op = qpl_op_scan_le;
        reference_job_ptr->op = qpl_op_scan_le;
        auto status = run_job_api(job_ptr);
        auto reference_status = ref_compare(reference_job_ptr);

        ASSERT_EQ(QPL_STS_OK, status);
        ASSERT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());
        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_range_initial_output_index, ScanTest)
    {
        // Skip nominal output
        if (1U == current_test_case.destination_bit_width) {
            return;
        }

        uint32_t max_available_index = (uint32_t)((1LLU << current_test_case.destination_bit_width) - 1U);
        if (current_test_case.number_of_elements > max_available_index) {
            return;
        }
        job_ptr->initial_output_index = max_available_index - current_test_case.number_of_elements;
        reference_job_ptr->initial_output_index = max_available_index - current_test_case.number_of_elements;

        job_ptr->op = qpl_op_scan_range;
        reference_job_ptr->op = qpl_op_scan_range;
        auto status = run_job_api(job_ptr);
        auto reference_status = ref_compare(reference_job_ptr);

        ASSERT_EQ(QPL_STS_OK, status);
        ASSERT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());
        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
        EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
    }

#if defined(__linux__)
#ifdef MADV_PAGEOUT

    class ScanTestPageFault : public ScanTest
    {
    private:
        std::unique_ptr<uint8_t[], void(*)(void*)> aligned_src{nullptr, {}};
        std::unique_ptr<uint8_t[], void(*)(void*)> aligned_dst{nullptr, {}};

    public:
        void InitializeTestCases()
        {
            std::vector<uint32_t> lengths_multiplier;

            // @todo Extend testing to generate several pages of input/output
            // when logic on touching multiple Pages in case of PF is extended the library.
            for (uint32_t i = 1U; i <= 1U; i++)
                lengths_multiplier.push_back(i);

            // Following bit widths values are chosen for simplicity.
            // This way the resulting input buffer (next_in_ptr) would be exactly of length size.
            uint32_t source_bit_width = 8U;
            uint32_t destination_bit_width = 1U;

            const uint32_t page_size = getpagesize();
            for (uint32_t multiplier : lengths_multiplier)
            {
                uint32_t max_input_value = (1ULL << source_bit_width) - 1;

                AnalyticTestCase test_case;
                // For using madvise and aligned_alloc, buffer size should be multiple of page_size.
                test_case.number_of_elements    = multiplier * page_size;
                test_case.source_bit_width      = source_bit_width;
                test_case.destination_bit_width = destination_bit_width;
                test_case.lower_bound           = max_input_value / 4;
                test_case.upper_bound           = max_input_value / 4 * 3;
                test_case.parser                = qpl_p_le_packed_array;
                test_case.flags                 = 0;

                AddNewTestCase(test_case);
            }
        }

        void SetUp() override
        {
            AnalyticFixture::SetUp();
            InitializeTestCases();
        }

        void SetBuffers() override
        {
            GenerateBuffers();

            // Set reference job buffers
            reference_job_ptr->available_in = static_cast<uint32_t>(source.size());
            reference_job_ptr->next_in_ptr  = source.data();

            reference_job_ptr->available_out = static_cast<uint32_t>(reference_destination.size());
            reference_job_ptr->next_out_ptr  = reference_destination.data();

            // Align and set job buffers
            // Ensure, we initialize to default values,
            // so that in the case of failed allocation below, library exits right away.
            job_ptr->next_in_ptr   = nullptr;
            job_ptr->next_out_ptr  = nullptr;
            job_ptr->available_in  = 0;
            job_ptr->available_out = 0;

            const uint32_t psize = getpagesize();

            uint8_t *aligned_src_buffer = static_cast<uint8_t*>(std::aligned_alloc(psize, source.size()));
            uint8_t *aligned_dst_buffer = static_cast<uint8_t*>(std::aligned_alloc(psize, destination.size()));

            if (aligned_src_buffer == nullptr || aligned_dst_buffer == nullptr) {
                std::free(aligned_src_buffer); //NOLINT(cppcoreguidelines-no-malloc)
                std::free(aligned_dst_buffer); //NOLINT(cppcoreguidelines-no-malloc)

                return;
            }

            std::copy(source.begin(), source.end(), aligned_src_buffer);
            std::copy(destination.begin(), destination.end(), aligned_dst_buffer);

            aligned_src = std::unique_ptr<uint8_t[], void(*)(void*)>(aligned_src_buffer,
                                                                     std::free);

            aligned_dst = std::unique_ptr<uint8_t[], void(*)(void*)>(aligned_dst_buffer,
                                                                     std::free);

            job_ptr->next_in_ptr  = aligned_src.get();
            job_ptr->available_in = static_cast<uint32_t>(source.size());

            job_ptr->next_out_ptr  = aligned_dst.get();
            job_ptr->available_out = static_cast<uint32_t>(destination.size());
        }

        void RunTestScanPageFaults(PageFaultType type) {
            if (is_madv_pageout_available()) {
                const uint32_t psize = getpagesize();

                if (type == READ_SRC_1_PAGE_FAULT) {
                    // Check that we at least have a single page of data
                    // to avoid swapping out the memory that we don't own.
                    ASSERT_GE(job_ptr->available_in, psize);

                    int err = madvise(job_ptr->next_in_ptr, psize, MADV_PAGEOUT);
                    if (err) {
                        int errsv = errno;
                        ASSERT_EQ(err, 0) << "madvise failed, error code is " << errsv << "\n";
                    }
                }
                else if (type == WRITE_PAGE_FAULT) {
                    // Check that we at least have a single page of data
                    // to avoid swapping out the memory that we don't own.
                    ASSERT_GE(job_ptr->available_out, psize);

                    int err = madvise(job_ptr->next_out_ptr, psize, MADV_PAGEOUT);
                    if (err) {
                        int errsv = errno;
                        ASSERT_EQ(err, 0) << "madvise failed, error code is " << errsv << "\n";
                    }
                }
                else { // not supported
                    return;
                }

                ASSERT_EQ(QPL_STS_OK, run_job_api(job_ptr));
                ASSERT_EQ(QPL_STS_OK, ref_compare(reference_job_ptr));

                // Copy results back to destination to ensure correct postprocessing.
                std::copy(aligned_dst.get(), aligned_dst.get()+job_ptr->total_out, destination.data());

                EXPECT_TRUE(CompareTotalInOutWithReference());
                EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
                EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
            }
        }

    };

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan_with_page_fault_read, scan_eq_only, ScanTestPageFault)
    {
        QPL_SKIP_TEST_FOR(qpl_path_software);

        job_ptr->op = qpl_op_scan_eq;
        reference_job_ptr->op = qpl_op_scan_eq;

        RunTestScanPageFaults(READ_SRC_1_PAGE_FAULT);
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan_with_page_fault_write, scan_eq_only, ScanTestPageFault)
    {
        QPL_SKIP_TEST_FOR(qpl_path_software);

        job_ptr->op = qpl_op_scan_eq;
        reference_job_ptr->op = qpl_op_scan_eq;

        RunTestScanPageFaults(WRITE_PAGE_FAULT);
    }

#endif // #ifdef MADV_PAGEOUT
#endif // #if defined(__linux__)

}
