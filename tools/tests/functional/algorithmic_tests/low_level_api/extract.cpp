/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/
#include <vector>
#include <string>
#include <numeric>
#include <cstdlib> //aligned_alloc, free
#include <memory> // unique_ptr

#include "gtest/gtest.h"

#include "qpl/qpl.h"

// utils_common
#include "iaa_features_checks.hpp"  // is_iaa_force_array_output_mod_supported()

// tests_common
#include "analytic_fixture.hpp"

#include "util.hpp"
#include "source_provider.hpp"
#include "ta_ll_common.hpp"
#include "system_info.hpp" // is_madv_pageout_available()
#include "common_defs.hpp" // PageFaultType

// ref
#include "qpl_api_ref.h"

// generator
#include "format_generator.hpp"

#if defined(__linux__)
#include <sys/mman.h>
#include <unistd.h>
#include <cerrno>
#endif

namespace qpl::test
{
    class ExtractTest : public AnalyticFixture
    {
    public:
        void InitializeTestCases()
        {
            std::vector<uint32_t> bit_widths(32U, 0U);
            std::iota(bit_widths.begin(), bit_widths.end(), 1U);
            auto number_of_elements = format_generator::generate_length_sequence();
            std::vector<uint64_t> output_format_flags = {0U, QPL_FLAG_OUT_BE};

            uint32_t test_cases_counter = 0;

            for (uint32_t length_in_elements : number_of_elements)
            {
                for (auto source_bit_width : bit_widths)
                {
                    for (auto destination_bit_width : {1U, 8U, 16U, 32U})
                    {
                        if (destination_bit_width != 1U)
                        {
                            if ((1ULL << destination_bit_width) <= length_in_elements)
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
                                test_case.lower_bound = length_in_elements / 4U;
                                test_case.upper_bound = (length_in_elements / 4U) * 3U;
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

        if (0U == test_case_counter % 5000U)
        {
            std::cout << " Running test case number " << test_case_counter << std::endl;
        }

        auto status = run_job_api(job_ptr);

        auto reference_status = ref_extract(reference_job_ptr);

        ASSERT_EQ(QPL_STS_OK, status);
        ASSERT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());

        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));

        EXPECT_TRUE(CompareVectors(destination, reference_destination));
        test_case_counter++;
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(extract, analytic_with_decompress, ExtractTest)
    {
        static uint32_t test_case_counter = 0;

        if (0U == test_case_counter % 5000U)
        {
            std::cout << " Running test case number " << test_case_counter << std::endl;
        }

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

        auto reference_status = ref_extract(reference_job_ptr);
        ASSERT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination));
        test_case_counter++;
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(extract, DISABLED_analytic_with_random_decompress, ExtractTest)
    {
        static uint32_t test_case_counter = 0;

        if (0U == test_case_counter % 5000U)
        {
            std::cout << " Running test case number " << test_case_counter << std::endl;
        }

        std::vector<uint8_t> compressed_source;
        ASSERT_NO_THROW(compressed_source = GetCompressedSource(true)); //NOLINT(cppcoreguidelines-avoid-goto)

        if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
            job_ptr->src1_bit_width = 0U;
        }

        auto saved_in_ptr = compressed_source.data();
        auto saved_flags  = job_ptr->flags;
        auto saved_op     = job_ptr->op;

        auto header_index = index_table.findHeaderBlockIndex(0);
        auto header_index_start  = index_table[header_index];
        auto header_index_finish = index_table[header_index + 1U];

        auto bit_start = header_index_start.bit_offset;
        auto bit_end   = header_index_finish.bit_offset;

        uint8_t *start = saved_in_ptr + bit_start / 8U;

        // Decompress header for random access
        job_ptr->op    = qpl_op_decompress;
        job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_RND_ACCESS;

        job_ptr->ignore_start_bits = bit_start & 7U;
        job_ptr->ignore_end_bits   = 7U & (0U - bit_end);
        job_ptr->available_in      = ((bit_end + 7U) / 8U) - (bit_start / 8U);
        job_ptr->next_in_ptr       = start;

        auto status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        // Filter with random access
        auto mini_block_index = index_table.findMiniBlockIndex(0);

        bit_start = index_table[mini_block_index].bit_offset;
        bit_end   = index_table[mini_block_index + 1U].bit_offset;

        job_ptr->next_in_ptr       = saved_in_ptr + bit_start / 8U;
        job_ptr->ignore_start_bits = bit_start & 7U;
        job_ptr->ignore_end_bits   = 7U & (0U - bit_end);
        job_ptr->available_in      = ((bit_end + 7U) / 8U) - (bit_start / 8U);
        job_ptr->crc               = index_table[mini_block_index].crc;

        job_ptr->op    = saved_op;
        job_ptr->flags = saved_flags | QPL_FLAG_DECOMPRESS_ENABLE | QPL_FLAG_RND_ACCESS;

        status = run_job_api(job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        auto reference_status = ref_extract(reference_job_ptr);
        ASSERT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareVectors(destination, reference_destination));
        test_case_counter++;
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(extract, initial_output_index, ExtractTest)
    {
        static uint32_t test_case_counter = 0;

        // Skip nominal and extended output
        if ((1U == current_test_case.destination_bit_width) || (1U != current_test_case.source_bit_width)) {
            return;
        }

        uint32_t max_available_index = (uint32_t)((1LLU << current_test_case.destination_bit_width) - 1U);
        if (current_test_case.number_of_elements > max_available_index) {
            return;
        }
        job_ptr->initial_output_index = max_available_index - current_test_case.number_of_elements;
        reference_job_ptr->initial_output_index = max_available_index - current_test_case.number_of_elements;

        auto status = run_job_api(job_ptr);

        auto reference_status = ref_extract(reference_job_ptr);

        ASSERT_EQ(QPL_STS_OK, status);
        ASSERT_EQ(QPL_STS_OK, reference_status);

        EXPECT_TRUE(CompareTotalInOutWithReference());

        EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));

        EXPECT_TRUE(CompareVectors(destination, reference_destination));
        test_case_counter++;
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(parquet_extract, bitwidth_mismatch_non_octa_group)
    {
        /*
            This tests the case where input and output bit-widths do not match
            while ending in the middle of a literal octa-group
            for extract operation on parquet format
        */
        uint32_t    size = 0;

        auto execution_path = util::TestEnvironment::GetInstance().GetExecutionPath();

        qpl_status status = qpl_get_job_size(execution_path, &size);
        ASSERT_EQ(QPL_STS_OK, status);

        auto job_buffer = std::make_unique<uint8_t[]>(size);
        auto job        = reinterpret_cast<qpl_job *>(job_buffer.get());


        status = qpl_init_job(execution_path, job);
        ASSERT_EQ(QPL_STS_OK, status);

        // testing only the 4-bit input width, only case creatable without creating new testing features (parquet creation)
        const uint8_t input_bit_width = 4U;

        // parquet_num_values must be 16 (divisible by 8, > 8, and (parquet_num_values - 1) fits in 4-bit)
        const uint8_t parquet_num_values = 16U;

        // Reference vector is a an ascending vector of values between [0 .. 15]
        std::vector<uint8_t> reference_vector(parquet_num_values);
        std::iota(reference_vector.begin(), reference_vector.end(), 0);

        // Parquet size is 1/2 of number of values (8-bit width to 4-bit width) + 2 (1 byte for bit width, and 1 for format/count)
        const uint8_t parquet_size = (parquet_num_values / 2) + 2;
        std::vector<uint8_t> source(parquet_size, 0);
        // first byte of parquet is input_bit_width
        source[0] = input_bit_width;
        // second byte of parquet is (count << 1) | [format {0 for rle, 1 for literals}]
        source[1] = (2U << 1) | 1;

        // Packing 8-bit numbers into 4-bit width literals
        for (int i = 0; i < reference_vector.size(); i++) {
            if (i % 2 == 0) {
                source[2 + i / 2] |= reference_vector[i];
            }
            else {
                source[2 + i / 2] |= (reference_vector[i] << 4);
            }
        }

        // Arbitrarily picked value (must be 8 < values < 16)
        const uint8_t values_to_extract = 10U;

        // Testing all potential output bit widths
        std::vector<qpl_out_format> output_bit_widths = {qpl_ow_8, qpl_ow_16, qpl_ow_32};

        // Output vector is of size 40 to fit {10 (items) * 4 (bytes per 32-bit value)}
        std::vector<uint8_t> destination(values_to_extract * 4);

        for (auto &output_bit_width: output_bit_widths) {
            // Performing an operation
            job->op                 = qpl_op_extract;
            job->src1_bit_width     = input_bit_width;
            job->out_bit_width      = output_bit_width;
            // Start extracting
            job->param_low          = 0;
            job->param_high         = values_to_extract - 1;
            job->num_input_elements = values_to_extract;
            job->parser             = qpl_p_parquet_rle;

            job->next_in_ptr        = source.data();
            job->available_in       = parquet_size;
            job->next_out_ptr       = destination.data();
            job->available_out      = static_cast<uint32_t>(destination.size());

            job->mini_block_size    = qpl_mblk_size_none;
            job->statistics_mode    = qpl_compression_mode;

            status = qpl_execute_job(job);
            ASSERT_EQ(QPL_STS_OK, status);


            for (int i = 0; i < 10; i++){
                if (output_bit_width == qpl_ow_8){
                    EXPECT_EQ(reference_vector[i], destination[i]);
                }
                else if(output_bit_width == qpl_ow_16){
                    EXPECT_EQ(reference_vector[i], ((uint16_t*)(destination.data()))[i]);
                }
                else { //output_bit_width == qpl_ow_32
                    EXPECT_EQ(reference_vector[i], ((uint32_t*)(destination.data()))[i]);
                }
            }
        }
        status = qpl_fini_job(job);
        ASSERT_EQ(QPL_STS_OK, status);
    }

    // Force Array Output Modification Extract Test
    // This test is to verify that the force array output modification is working as expected
    // The test creates a source vector with 64 elements, and extracts a range of elements from index 16 to 47
    // The output will be extended to the force array output modification, and the output will be verified
    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(extract, force_array_output_modification, ExtractTest)
    {
        // Assert that Force Array Output Modification is supported
        QPL_SKIP_TEST_FOR_EXPR_VERBOSE(is_iaa_force_array_output_mod_supported() == false, "Force array output modification not available on device, skipping test.");

        // Skip test if on software path
        QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Force array output modification not available on software path");

        constexpr const uint32_t source_size        = 64;
        constexpr const uint32_t input_vector_width = 8;
        constexpr const uint32_t lower_index        = 16;
        constexpr const uint32_t upper_index        = 47;
        constexpr const uint64_t reference          = 16045690984062438142ULL;  // Reference value for extract range

        // Source and output containers
        std::vector<uint8_t> source(source_size, 0);
        std::vector<uint8_t> destination(source_size * 4, 4);  // Destination are 32 bits which is 4 times larger than source

        std::unique_ptr<uint8_t[]> job_buffer;
        uint32_t   size = 0;

        // Filling source container
        for (uint32_t i = 0; i < source_size; ++i) {
            if ((1UL << i) & reference) {
                source[i] = (i % 4) + 7;  // values in a cycle: 7, 8, 9, 10
            }
            else {
                source[i] = (i % 7);  // values in a cycle: 0, 1, 2, 3, 4, 5, 6
            }
        }

        // Job initialization
        qpl_status status = qpl_get_job_size(qpl_path_hardware, &size);
        ASSERT_EQ(QPL_STS_OK, status) << "An error " << status << " acquired during job size getting.\n";

        job_buffer = std::make_unique<uint8_t[]>(size);
        qpl_job *job = reinterpret_cast<qpl_job *>(job_buffer.get());

        status = qpl_init_job(qpl_path_hardware, job);
        ASSERT_EQ(QPL_STS_OK, status) << "An error " << status << " acquired during job initializing.\n";

        // Performing an operation
        job->next_in_ptr        = source.data();
        job->available_in       = source_size;
        job->next_out_ptr       = destination.data();
        job->available_out      = static_cast<uint32_t>(destination.size());
        job->op                 = qpl_op_extract;
        job->src1_bit_width     = input_vector_width;
        job->param_low          = lower_index;
        job->param_high         = upper_index;
        job->num_input_elements = source_size;
        job->out_bit_width      = qpl_ow_32;

        // Enable force array output mod
        job->flags             |= QPL_FLAG_FORCE_ARRAY_OUTPUT;

        status = run_job_api(job);
        ASSERT_EQ(QPL_STS_OK, status) << "An error " << status << " acquired during performing extract.\n";

        const auto extract_size = job->total_out;

        EXPECT_EQ(extract_size, (upper_index - lower_index + 1) * 4);

        // Freeing resources
        status = qpl_fini_job(job);
        ASSERT_EQ(QPL_STS_OK, status) << "An error " << status << " acquired during job finalizing.\n";

        // Verify the output
        for (size_t i = 0; i < extract_size / 4; i++) {
            ASSERT_EQ(destination[i * 4], source[i + lower_index]) << "Mismatch at index " << i;
        }
    }

#if defined(__linux__)
#ifdef MADV_PAGEOUT

    class ExtractTestPageFault : public ExtractTest
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
            uint32_t source_bit_width      = 8U;
            uint32_t destination_bit_width = 1U;

            const uint32_t page_size = getpagesize();
            for (uint32_t multiplier : lengths_multiplier)
            {
                AnalyticTestCase test_case;

                test_case.operation             = qpl_op_extract;
                // For using madvise and aligned_alloc, buffer size should be multiple of page_size.
                test_case.number_of_elements    = multiplier * page_size;
                test_case.source_bit_width      = source_bit_width;
                test_case.destination_bit_width = destination_bit_width;
                test_case.lower_bound           = test_case.number_of_elements  / 4U;
                test_case.upper_bound           = (test_case.number_of_elements / 4U) * 3U;
                test_case.parser                = qpl_p_le_packed_array;

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
            job_ptr->available_in  = 0U;
            job_ptr->available_out = 0U;

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

        void RunTestExtractPageFaults(PageFaultType type) {
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
                ASSERT_EQ(QPL_STS_OK, ref_extract(reference_job_ptr));

                // Copy results back to destination to ensure correct postprocessing.
                std::copy(aligned_dst.get(), aligned_dst.get()+job_ptr->total_out, destination.data());

                EXPECT_TRUE(CompareTotalInOutWithReference());
                EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
                EXPECT_TRUE(CompareVectors(destination, reference_destination));
            }
        }
    };

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(extract_with_page_fault_read, analytic_only, ExtractTestPageFault)
    {
        QPL_SKIP_TEST_FOR(qpl_path_software);
        RunTestExtractPageFaults(READ_SRC_1_PAGE_FAULT);
    }

    QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(extract_with_page_fault_write, analytic_only, ExtractTestPageFault)
    {
        QPL_SKIP_TEST_FOR(qpl_path_software);
        RunTestExtractPageFaults(WRITE_PAGE_FAULT);
    }

#endif // #ifdef MADV_PAGEOUT
#endif // #if defined(__linux__)

}
