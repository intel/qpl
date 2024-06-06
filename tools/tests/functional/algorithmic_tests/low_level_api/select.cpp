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

// utils_common
#include "iaa_features_checks.hpp"  // is_iaa_force_array_output_mod_supported()

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
#include <cerrno>
#endif

namespace qpl::test
{
    class SelectTest : public AnalyticMaskFixture
    {
    public:
        void InitializeTestCases()
        {
            std::vector<uint32_t> lengths = GenerateNumberOfElementsVector();

            for (uint32_t length : lengths)
            {
                for (uint32_t bit_width = 1; bit_width <= 32U; bit_width++)
                {
                    for (uint32_t destination_bit_width : {1, 8, 16, 32})
                    {
                        const uint32_t max_output_value = (1ULL << destination_bit_width) - 1;

                        if (destination_bit_width != 1U && max_output_value < length) {
                            continue;
                        } else if (destination_bit_width != 1U && bit_width > destination_bit_width) {
                            continue;
                        }

                        for (auto parser : {qpl_p_le_packed_array, qpl_p_be_packed_array, qpl_p_parquet_rle})
                        {
                            AnalyticTestCase test_case;
                            test_case.operation = qpl_op_select;
                            test_case.number_of_elements = length;
                            test_case.source_bit_width = bit_width;
                            test_case.destination_bit_width = destination_bit_width;
                            test_case.lower_bound = 0;
                            test_case.upper_bound = 1;
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
            AnalyticMaskFixture::SetUp();
            InitializeTestCases();
        }
    };

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(select, analytic_only, SelectTest)
    {
        auto status = run_job_api(job_ptr);

        auto reference_status = ref_select(reference_job_ptr);

        ASSERT_EQ(QPL_STS_OK, status);
        ASSERT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());
        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
        EXPECT_TRUE(CompareVectors(destination, reference_destination));
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(select, analytic_with_decompress, SelectTest)
    {
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

        auto reference_status = ref_select(reference_job_ptr);
        ASSERT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination));
    }

    // Force Array Output Modification Check For Select
    // This test checks if the Force Array Output Modification is supported for the Select operation.
    // This test creates a source array with 64 elements and a mask
    // The select operation is performed on the source array with the mask
    // The output is an array with values from the source array where the mask is set
    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(select, force_array_output_modification, SelectTest)
    {
        // Assert that Force Array Output Modification is supported
        QPL_SKIP_TEST_FOR_EXPR_VERBOSE(is_iaa_force_array_output_mod_supported() == false, "Force array output modification not available on device, skipping test.");

        // Skip test if on software path
        QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Force array output modification not available on software path");

        constexpr const uint32_t source_size                = 64;                  // Upper boundary for select
        constexpr const uint32_t input_vector_width         = 8;
        constexpr const uint32_t select_output_vector_width = 1;
        constexpr const uint32_t byte_bit_length            = 8;
        constexpr const uint64_t reference                  = 16045690984062438142ULL;  // Reference value for select

        // Source and output containers
        std::vector<uint8_t> source(source_size, 0);
        std::vector<uint8_t> select_mask(source_size / 8, 4);
        std::vector<uint8_t> destination(source_size * 4, 4);  // Destination are 32 bits which is 4 times larger than source

        std::unique_ptr<uint8_t[]> job_buffer;
        uint32_t   size = 0;

        // Filling source containers
        for (uint32_t i = 0; i < source_size; ++i) {
            if ((1UL << i) & reference) {
                source[i] = (i % 4) + 7;  // values in a cycle: 7, 8, 9, 10
            }
            else {
                source[i] = (i % 7);  // values in a cycle: 0, 1, 2, 3, 4, 5, 6
            }
        }

        // Fill mask after
        select_mask[0] = 254;
        select_mask[1] = 202;
        select_mask[2] = 186;
        select_mask[3] = 176;
        select_mask[4] = 239;
        select_mask[5] = 190;
        select_mask[6] = 173;
        select_mask[7] = 222;

        // Job initialization
        qpl_status status = qpl_get_job_size(qpl_path_hardware, &size);
        ASSERT_EQ(QPL_STS_OK, status) << "An error " << status << " acquired during job size getting.\n";
        

        job_buffer = std::make_unique<uint8_t[]>(size);
        qpl_job *job = reinterpret_cast<qpl_job *>(job_buffer.get());

        status = qpl_init_job(qpl_path_hardware, job);
        ASSERT_EQ(QPL_STS_OK, status) << "An error " << status << " acquired during job initializing.\n";

        // Performing a select operation
        job->next_in_ptr        = source.data();
        job->available_in       = source_size;
        job->next_out_ptr       = destination.data();
        job->available_out      = static_cast<uint32_t>(destination.size());
        job->op                 = qpl_op_select;
        job->src1_bit_width     = input_vector_width;
        job->num_input_elements = source_size;
        job->out_bit_width      = qpl_ow_32;
        job->next_src2_ptr      = select_mask.data();
        job->available_src2     = 8;
        job->src2_bit_width     = select_output_vector_width;

        job->flags             |= QPL_FLAG_FORCE_ARRAY_OUTPUT;

        status = run_job_api(job);
        ASSERT_EQ(QPL_STS_OK, status) << "An error " << status << " acquired during performing select.\n";

        const auto select_byte_size = job->total_out;

        // Check if select size is 43 * 4
        EXPECT_EQ(select_byte_size, 43 * 4);

        // Freeing resources
        status = qpl_fini_job(job);
        ASSERT_EQ(QPL_STS_OK, status) << "An error " << status << " acquired during job finalization.\n";

        // Compare with reference
        uint32_t idx = 0;

        for (uint32_t i = 0; i < source_size; ++i) {
            if ((1UL << i) & reference) {
                EXPECT_EQ(destination[idx], source[i]);
                idx += 4;
            }
        }
    }

#if defined(__linux__)
#ifdef MADV_PAGEOUT

    class SelectTestPageFault : public SelectTest
    {
    private:
        std::unique_ptr<uint8_t[], void(*)(void*)> aligned_src{nullptr, {}};
        std::unique_ptr<uint8_t[], void(*)(void*)> aligned_src2{nullptr, {}};
        std::unique_ptr<uint8_t[], void(*)(void*)> aligned_dst{nullptr, {}};

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

                test_case.operation                 = qpl_op_select;
                // For using madvise and aligned_alloc, buffer size should be multiple of page_size.
                // Note, that the resulting buffer size depends on bit_width and number_of_elements.
                // 8 multiplier here is due to element bit_width.
                test_case.number_of_elements        = multiplier * page_size * 8;
                test_case.source_bit_width          = source_bit_width;
                test_case.destination_bit_width     = destination_bit_width;
                test_case.lower_bound               = 0;
                test_case.upper_bound               = 1;
                test_case.parser                    = qpl_p_le_packed_array;
                test_case.flags                     = 0;
                test_case.second_input_bit_width    = 1;
                test_case.second_input_num_elements = multiplier * page_size * 8;

                AddNewTestCase(test_case);
            }
        }

        void SetUp() override
        {
            AnalyticMaskFixture::SetUp();
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
                std::free(aligned_src_buffer); //NOLINT(cppcoreguidelines-no-malloc)
                std::free(aligned_src2_buffer); //NOLINT(cppcoreguidelines-no-malloc)
                std::free(aligned_dst_buffer); //NOLINT(cppcoreguidelines-no-malloc)

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

        void RunTestSelectPageFaults(PageFaultType type) {
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
                ASSERT_EQ(QPL_STS_OK, ref_select(reference_job_ptr));

                // Copy results back to destination to ensure correct postprocessing.
                std::copy(aligned_dst.get(), aligned_dst.get() + job_ptr->total_out, destination.data());

                EXPECT_TRUE(CompareTotalInOutWithReference());
                EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
                EXPECT_TRUE(CompareVectors(destination, reference_destination));
            }
        }
    };

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(select_with_page_fault_read, analytic_only, SelectTestPageFault)
    {
        QPL_SKIP_TEST_FOR(qpl_path_software);
        RunTestSelectPageFaults(READ_SRC_1_PAGE_FAULT);
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(select_with_page_fault_read_src2, analytic_only, SelectTestPageFault)
    {
        QPL_SKIP_TEST_FOR(qpl_path_software);
        RunTestSelectPageFaults(READ_SRC_2_PAGE_FAULT);
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(select_with_page_fault_write, analytic_only, SelectTestPageFault)
    {
        QPL_SKIP_TEST_FOR(qpl_path_software);
        RunTestSelectPageFaults(WRITE_PAGE_FAULT);
    }

#endif // #ifdef MADV_PAGEOUT
#endif // #if defined(__linux__)

}
