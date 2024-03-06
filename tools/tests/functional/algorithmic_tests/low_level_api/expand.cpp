/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <vector>
#include <string>
#include <cstdlib> //aligned_alloc, free
#include <memory> // unique_ptr

#include "gtest/gtest.h"

#include "qpl/qpl.h"

// tests_common
#include "analytic_mask_fixture.hpp"

#include "util.hpp"
#include "ta_ll_common.hpp"
#include "system_info.hpp" // is_madv_pageout_available()
#include "common_defs.hpp" // PageFaultType

// ref
#include "qpl_api_ref.h"

#if defined(__linux__)
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#endif

namespace qpl::test
{
    class ExpandTest : public AnalyticMaskFixture
    {
    public:
        void InitializeTestCases()
        {
            std::vector<uint32_t> lengths = GenerateNumberOfElementsVector();

            for (uint32_t length : lengths)
            {
                for (uint32_t bit_width = 1u; bit_width <= 32u; bit_width++)
                {
                    for (uint32_t destination_bit_width : {1u, 8u, 16u, 32u})
                    {
                        const uint32_t max_output_value = (1ULL << destination_bit_width) - 1;

                        if (destination_bit_width != 1u && max_output_value < length) {
                            continue;
                        } else if (destination_bit_width != 1u && bit_width > destination_bit_width) {
                            continue;
                        }

                        for (auto parser : {qpl_p_le_packed_array, qpl_p_be_packed_array, qpl_p_parquet_rle})
                        {
                            AnalyticTestCase test_case;
                            test_case.operation = qpl_op_expand;
                            test_case.number_of_elements = length;
                            test_case.source_bit_width = bit_width;
                            test_case.destination_bit_width = destination_bit_width;
                            test_case.parser = parser;
                            test_case.flags = 0;
                            test_case.second_input_bit_width = 1;
                            test_case.second_input_num_elements = length;

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

        void SetUp() override
        {
            AnalyticFixture::SetUp();
            InitializeTestCases();
        }
    };

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(expand, analytic_only, ExpandTest)
    {
        auto status = run_job_api(job_ptr);

        auto reference_status = ref_expand(reference_job_ptr);

        EXPECT_EQ(QPL_STS_OK, status);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());
        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
        EXPECT_TRUE(CompareVectors(destination, reference_destination));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(expand, analytic_with_decompression, ExpandTest)
    {
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

        auto reference_status = ref_expand(reference_job_ptr);
        EXPECT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination));
    }

#if defined(__linux__)
#ifdef MADV_PAGEOUT

    class ExpandTestPageFault : public ExpandTest
    {
    public:
        void InitializeTestCases()
        {
            std::vector<uint32_t> lengths_multiplier;
            // @todo Extend testing to generate several pages of input/output
            // when logic on touching multiple Pages in case of PF is extended the library.
            for (uint32_t i = 1U; i <= 1U; i++)
                lengths_multiplier.push_back(i);

            // Following bit widths values are hard coded for simplicity.
            uint32_t source_bit_width      = 1U;
            uint32_t destination_bit_width = 1U;

            const uint32_t page_size = getpagesize();

            for (uint32_t multiplier : lengths_multiplier)
            {
                AnalyticTestCase test_case;

                test_case.operation                 = qpl_op_expand;
                test_case.source_bit_width          = source_bit_width;
                // For using madvise and aligned_alloc, buffer size should be multiple of page_size.
                // Note, that the resulting buffer size depends on bit_width and number_of_elements.
                // 8 multiplier here is due to element bit_width.
                test_case.number_of_elements        = multiplier * page_size * 8U;
                test_case.destination_bit_width     = destination_bit_width;
                test_case.parser                    = qpl_p_le_packed_array;
                test_case.flags                     = 0U;
                test_case.second_input_bit_width    = 1U;
                test_case.second_input_num_elements = multiplier * page_size * 8U;

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
            reference_job_ptr->available_in   = static_cast<uint32_t>(source.size());
            reference_job_ptr->next_in_ptr    = source.data();
            reference_job_ptr->available_src2 = static_cast<uint32_t>(mask.size());
            reference_job_ptr->next_src2_ptr  = mask.data();
            reference_job_ptr->available_out  = static_cast<uint32_t>(reference_destination.size());
            reference_job_ptr->next_out_ptr   = reference_destination.data();

            // Align and set job buffers
            // Ensure, we initialize to default values,
            // so that in the case of failed allocation below, library exits right away.
            job_ptr->available_in   = 0U;
            job_ptr->available_src2 = 0U;
            job_ptr->available_out  = 0U;
            job_ptr->next_in_ptr    = nullptr;
            job_ptr->next_src2_ptr  = nullptr;
            job_ptr->next_out_ptr   = nullptr;

            const uint32_t psize = getpagesize();

            uint8_t *aligned_src_buffer  = static_cast<uint8_t*>(std::aligned_alloc(psize, source.size()));
            uint8_t *aligned_src2_buffer = static_cast<uint8_t*>(std::aligned_alloc(psize, mask.size()));
            uint8_t *aligned_dst_buffer  = static_cast<uint8_t*>(std::aligned_alloc(psize, destination.size()));

            if (aligned_src_buffer == nullptr || aligned_dst_buffer == nullptr ||
                aligned_src2_buffer == nullptr) {
                std::free(aligned_src_buffer);
                std::free(aligned_src2_buffer);
                std::free(aligned_dst_buffer);

                return;
            }

            std::copy(source.begin(), source.end(), aligned_src_buffer);
            std::copy(mask.begin(), mask.end(), aligned_src2_buffer);
            std::copy(destination.begin(), destination.end(), aligned_dst_buffer);

            aligned_src = std::unique_ptr<uint8_t[], void(*)(void*)>(aligned_src_buffer,
                                                                     std::free);

            aligned_src2 = std::unique_ptr<uint8_t[], void(*)(void*)>(aligned_src2_buffer,
                                                                     std::free);

            aligned_dst = std::unique_ptr<uint8_t[], void(*)(void*)>(aligned_dst_buffer,
                                                                     std::free);

            job_ptr->available_in   = static_cast<uint32_t>(source.size());
            job_ptr->next_in_ptr    = aligned_src.get();

            job_ptr->available_src2 = static_cast<uint32_t>(mask.size());
            job_ptr->next_src2_ptr  = aligned_src2.get();

            job_ptr->available_out  = static_cast<uint32_t>(destination.size());
            job_ptr->next_out_ptr   = aligned_dst.get();
        }

        void RunTestExpandPageFaults(PageFaultType type) {
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
                else if (type == READ_SRC_2_PAGE_FAULT) {
                    // Check that we at least have a single page of data
                    // to avoid swapping out the memory that we don't own.
                    ASSERT_GE(job_ptr->available_src2, psize);

                    int err = madvise(job_ptr->next_src2_ptr, psize, MADV_PAGEOUT);
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
                ASSERT_EQ(QPL_STS_OK, ref_expand(reference_job_ptr));

                // Copy results back to destination to ensure correct postprocessing.
                std::copy(aligned_dst.get(), aligned_dst.get() + job_ptr->total_out, destination.data());

                EXPECT_TRUE(CompareTotalInOutWithReference());
                EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
                EXPECT_TRUE(CompareVectors(destination, reference_destination));
            }
        }

        std::unique_ptr<uint8_t[], void(*)(void*)> aligned_src{nullptr, {}};
        std::unique_ptr<uint8_t[], void(*)(void*)> aligned_src2{nullptr, {}};
        std::unique_ptr<uint8_t[], void(*)(void*)> aligned_dst{nullptr, {}};
    };

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(expand_with_page_fault_read_src1, analytic_only, ExpandTestPageFault)
    {
        QPL_SKIP_TEST_FOR(qpl_path_software);
        RunTestExpandPageFaults(READ_SRC_1_PAGE_FAULT);
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(expand_with_page_fault_read_src2, analytic_only, ExpandTestPageFault)
    {
        QPL_SKIP_TEST_FOR(qpl_path_software);
        RunTestExpandPageFaults(READ_SRC_2_PAGE_FAULT);
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(expand_with_page_fault_write, analytic_only, ExpandTestPageFault)
    {
        QPL_SKIP_TEST_FOR(qpl_path_software);
        RunTestExpandPageFaults(WRITE_PAGE_FAULT);
    }

#endif // #ifdef MADV_PAGEOUT
#endif // #if defined(__linux__)

}
