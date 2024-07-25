/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <cstdlib> //aligned_alloc, free
#include <memory>  // unique_ptr
#include <string>
#include <vector>

#include "qpl/qpl.h"

#include "gtest/gtest.h"

// utils_common
#include "iaa_features_checks.hpp" // is_iaa_force_array_output_mod_supported()

// common
#include "analytic_fixture.hpp"
#include "common_defs.hpp"
#include "system_info.hpp"
#include "ta_ll_common.hpp"
#include "util.hpp"

// ref
#include "qpl_api_ref.h"

#if defined(__linux__)
#include <cerrno>
#include <sys/mman.h>
#include <unistd.h>
#endif

namespace qpl::test {
class ScanTest : public AnalyticFixture {
public:
    void InitializeTestCases() {
        const std::vector<uint32_t> destination_bit_widths = {1U, 8U, 16U, 32U};
        const std::vector<uint32_t> lengths                = GenerateNumberOfElementsVector();

        const std::vector<uint32_t> output_format_flags = {0, QPL_FLAG_OUT_BE};

        for (const uint32_t length : lengths) {
            for (uint32_t source_bit_width = 1; source_bit_width <= 32; source_bit_width++) {
                for (auto destination_bit_width : destination_bit_widths) {
                    const uint32_t max_output_value = (1ULL << destination_bit_width) - 1;

                    if (destination_bit_width != 1U && max_output_value < length) { continue; }

                    for (auto parser : {qpl_p_le_packed_array, qpl_p_be_packed_array, qpl_p_parquet_rle}) {
                        for (auto flag : output_format_flags) {
                            const uint32_t   max_input_value = (1ULL << source_bit_width) - 1;
                            AnalyticTestCase test_case;
                            test_case.number_of_elements    = length;
                            test_case.source_bit_width      = source_bit_width;
                            test_case.destination_bit_width = destination_bit_width;
                            test_case.lower_bound           = max_input_value / 4;
                            test_case.upper_bound           = max_input_value / 4 * 3;
                            test_case.parser                = parser;
                            test_case.flags                 = flag;

                            AddNewTestCase(test_case);
                        }
                    }
                }
            }
        }
    }

    void SetUp() override {
        AnalyticFixture::SetUp();
        InitializeTestCases();
    }
};

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_eq_only, ScanTest) {
    job_ptr->op           = qpl_op_scan_eq;
    reference_job_ptr->op = qpl_op_scan_eq;

    auto status           = run_job_api(job_ptr);
    auto reference_status = ref_compare(reference_job_ptr);

    ASSERT_EQ(QPL_STS_OK, status);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareTotalInOutWithReference());
    EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_ne_only, ScanTest) {
    job_ptr->op           = qpl_op_scan_ne;
    reference_job_ptr->op = qpl_op_scan_ne;
    auto status           = run_job_api(job_ptr);
    auto reference_status = ref_compare(reference_job_ptr);

    ASSERT_EQ(QPL_STS_OK, status);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareTotalInOutWithReference());
    EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_lt_only, ScanTest) {
    job_ptr->op           = qpl_op_scan_lt;
    reference_job_ptr->op = qpl_op_scan_lt;
    auto status           = run_job_api(job_ptr);
    auto reference_status = ref_compare(reference_job_ptr);

    ASSERT_EQ(QPL_STS_OK, status);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareTotalInOutWithReference());
    EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_le_only, ScanTest) {
    job_ptr->op           = qpl_op_scan_le;
    reference_job_ptr->op = qpl_op_scan_le;
    auto status           = run_job_api(job_ptr);
    auto reference_status = ref_compare(reference_job_ptr);

    ASSERT_EQ(QPL_STS_OK, status);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareTotalInOutWithReference());
    EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_gt_only, ScanTest) {
    job_ptr->op           = qpl_op_scan_gt;
    reference_job_ptr->op = qpl_op_scan_gt;
    auto status           = run_job_api(job_ptr);
    auto reference_status = ref_compare(reference_job_ptr);

    ASSERT_EQ(QPL_STS_OK, status);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareTotalInOutWithReference());
    EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_ge_only, ScanTest) {
    job_ptr->op           = qpl_op_scan_ge;
    reference_job_ptr->op = qpl_op_scan_ge;
    auto status           = run_job_api(job_ptr);
    auto reference_status = ref_compare(reference_job_ptr);

    ASSERT_EQ(QPL_STS_OK, status);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareTotalInOutWithReference());
    EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_range_only, ScanTest) {
    job_ptr->op           = qpl_op_scan_range;
    reference_job_ptr->op = qpl_op_scan_range;
    auto status           = run_job_api(job_ptr);
    auto reference_status = ref_compare(reference_job_ptr);

    ASSERT_EQ(QPL_STS_OK, status);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareTotalInOutWithReference());
    EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_not_range_only, ScanTest) {
    job_ptr->op           = qpl_op_scan_not_range;
    reference_job_ptr->op = qpl_op_scan_not_range;
    auto status           = run_job_api(job_ptr);
    auto reference_status = ref_compare(reference_job_ptr);

    ASSERT_EQ(QPL_STS_OK, status);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareTotalInOutWithReference());
    EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_eq_with_decompress, ScanTest) {
    job_ptr->op           = qpl_op_scan_eq;
    reference_job_ptr->op = qpl_op_scan_eq;
    std::vector<uint8_t> compressed_source;
    ASSERT_NO_THROW(compressed_source = GetCompressedSource()); //NOLINT(cppcoreguidelines-avoid-goto)
    job_ptr->available_in = static_cast<uint32_t>(compressed_source.size());
    job_ptr->next_in_ptr  = compressed_source.data();
    job_ptr->flags |= QPL_FLAG_DECOMPRESS_ENABLE;

    if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
        job_ptr->src1_bit_width = 0U;
    }

    auto status = run_job_api(job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    auto reference_status = ref_compare(reference_job_ptr);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_ne_with_decompress, ScanTest) {
    job_ptr->op           = qpl_op_scan_ne;
    reference_job_ptr->op = qpl_op_scan_ne;
    std::vector<uint8_t> compressed_source;
    ASSERT_NO_THROW(compressed_source = GetCompressedSource()); //NOLINT(cppcoreguidelines-avoid-goto)
    job_ptr->available_in = static_cast<uint32_t>(compressed_source.size());
    job_ptr->next_in_ptr  = compressed_source.data();
    job_ptr->flags |= QPL_FLAG_DECOMPRESS_ENABLE;

    if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
        job_ptr->src1_bit_width = 0U;
    }

    auto status = run_job_api(job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    auto reference_status = ref_compare(reference_job_ptr);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_lt_with_decompress, ScanTest) {
    job_ptr->op           = qpl_op_scan_lt;
    reference_job_ptr->op = qpl_op_scan_lt;
    std::vector<uint8_t> compressed_source;
    ASSERT_NO_THROW(compressed_source = GetCompressedSource()); //NOLINT(cppcoreguidelines-avoid-goto)
    job_ptr->available_in = static_cast<uint32_t>(compressed_source.size());
    job_ptr->next_in_ptr  = compressed_source.data();
    job_ptr->flags |= QPL_FLAG_DECOMPRESS_ENABLE;

    if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
        job_ptr->src1_bit_width = 0U;
    }

    auto status = run_job_api(job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    auto reference_status = ref_compare(reference_job_ptr);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_le_with_decompress, ScanTest) {
    job_ptr->op           = qpl_op_scan_le;
    reference_job_ptr->op = qpl_op_scan_le;
    std::vector<uint8_t> compressed_source;
    ASSERT_NO_THROW(compressed_source = GetCompressedSource()); //NOLINT(cppcoreguidelines-avoid-goto)
    job_ptr->available_in = static_cast<uint32_t>(compressed_source.size());
    job_ptr->next_in_ptr  = compressed_source.data();
    job_ptr->flags |= QPL_FLAG_DECOMPRESS_ENABLE;

    if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
        job_ptr->src1_bit_width = 0U;
    }

    auto status = run_job_api(job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    auto reference_status = ref_compare(reference_job_ptr);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_ge_with_decompress, ScanTest) {
    job_ptr->op           = qpl_op_scan_ge;
    reference_job_ptr->op = qpl_op_scan_ge;
    std::vector<uint8_t> compressed_source;
    ASSERT_NO_THROW(compressed_source = GetCompressedSource()); //NOLINT(cppcoreguidelines-avoid-goto)
    job_ptr->available_in = static_cast<uint32_t>(compressed_source.size());
    job_ptr->next_in_ptr  = compressed_source.data();
    job_ptr->flags |= QPL_FLAG_DECOMPRESS_ENABLE;

    if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
        job_ptr->src1_bit_width = 0U;
    }

    auto status = run_job_api(job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    auto reference_status = ref_compare(reference_job_ptr);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_range_with_decompress, ScanTest) {
    job_ptr->op           = qpl_op_scan_range;
    reference_job_ptr->op = qpl_op_scan_range;
    std::vector<uint8_t> compressed_source;
    ASSERT_NO_THROW(compressed_source = GetCompressedSource()); //NOLINT(cppcoreguidelines-avoid-goto)
    job_ptr->available_in = static_cast<uint32_t>(compressed_source.size());
    job_ptr->next_in_ptr  = compressed_source.data();
    job_ptr->flags |= QPL_FLAG_DECOMPRESS_ENABLE;

    if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
        job_ptr->src1_bit_width = 0U;
    }

    auto status = run_job_api(job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    auto reference_status = ref_compare(reference_job_ptr);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_not_range_with_decompress, ScanTest) {
    job_ptr->op           = qpl_op_scan_not_range;
    reference_job_ptr->op = qpl_op_scan_not_range;
    std::vector<uint8_t> compressed_source;
    ASSERT_NO_THROW(compressed_source = GetCompressedSource()); //NOLINT(cppcoreguidelines-avoid-goto)
    job_ptr->available_in = static_cast<uint32_t>(compressed_source.size());
    job_ptr->next_in_ptr  = compressed_source.data();
    job_ptr->flags |= QPL_FLAG_DECOMPRESS_ENABLE;

    if (GetExecutionPath() == qpl_path_software && current_test_case.parser == qpl_p_parquet_rle) {
        job_ptr->src1_bit_width = 0U;
    }

    auto status = run_job_api(job_ptr);
    ASSERT_EQ(QPL_STS_OK, status);

    auto reference_status = ref_compare(reference_job_ptr);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_le_initial_output_index, ScanTest) {
    // Skip nominal output
    if (1U == current_test_case.destination_bit_width) { return; }

    const uint32_t max_available_index = (uint32_t)((1LLU << current_test_case.destination_bit_width) - 1U);
    if (current_test_case.number_of_elements > max_available_index) { return; }
    job_ptr->initial_output_index           = max_available_index - current_test_case.number_of_elements;
    reference_job_ptr->initial_output_index = max_available_index - current_test_case.number_of_elements;

    job_ptr->op           = qpl_op_scan_le;
    reference_job_ptr->op = qpl_op_scan_le;
    auto status           = run_job_api(job_ptr);
    auto reference_status = ref_compare(reference_job_ptr);

    ASSERT_EQ(QPL_STS_OK, status);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareTotalInOutWithReference());
    EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, scan_range_initial_output_index, ScanTest) {
    // Skip nominal output
    if (1U == current_test_case.destination_bit_width) { return; }

    const uint32_t max_available_index = (uint32_t)((1LLU << current_test_case.destination_bit_width) - 1U);
    if (current_test_case.number_of_elements > max_available_index) { return; }
    job_ptr->initial_output_index           = max_available_index - current_test_case.number_of_elements;
    reference_job_ptr->initial_output_index = max_available_index - current_test_case.number_of_elements;

    job_ptr->op           = qpl_op_scan_range;
    reference_job_ptr->op = qpl_op_scan_range;
    auto status           = run_job_api(job_ptr);
    auto reference_status = ref_compare(reference_job_ptr);

    ASSERT_EQ(QPL_STS_OK, status);
    ASSERT_EQ(QPL_STS_OK, reference_status);

    EXPECT_TRUE(CompareTotalInOutWithReference());
    EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
    EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
}

// Force Array Output Modification Test
// This test checks if the Force Array Output Modification is supported and works as expected.
// The test generates a source array with 64 elements, where each element is a value from a cycle of these values: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10.
// The test generates a reference value, which is a bitmask of the elements that should be in the scan range.
// The test performs a scan operation with the Force Array Output Modification enabled.
// The test checks if the output matches the reference value.
QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan, force_array_output_modification, ScanTest) {
    // Assert that Force Array Output Modification is supported
    QPL_SKIP_TEST_FOR_EXPR_VERBOSE(is_iaa_force_array_output_mod_supported() == false,
                                   "Force array output modification not available on device, skipping test.");

    // Skip test if on software path
    QPL_SKIP_TEST_FOR_VERBOSE(qpl_path_software, "Force array output modification not available on software path");

    constexpr const uint32_t source_size         = 64; // 64 elements
    constexpr const uint32_t input_vector_width  = 8;  // 8-bit values
    constexpr const uint32_t output_vector_width = 32; // 32-bit values (because of Force Array Output Mod)
    constexpr const uint32_t lower_boundary      = 7;  // Lower boundary for scan range
    constexpr const uint32_t upper_boundary      = 10; // Upper boundary for scan range
    constexpr const uint32_t byte_bit_length     = 8;  // 8 bits in a byte
    constexpr const uint64_t reference           = 16045690984062438142ULL; // Reference value for scan range

    // Source and output containers
    std::vector<uint8_t> source(source_size, 0);          // 64 bytes of data initialized with zeros
    std::vector<uint8_t> destination(source_size * 4, 1); // 256 bytes of data initialized with ones

    std::unique_ptr<uint8_t[]> job_buffer;
    uint32_t                   size   = 0;
    const auto*                output = reinterpret_cast<const uint32_t*>(destination.data());

    // Filling source containers
    for (uint32_t i = 0; i < source_size; ++i) {
        if ((1UL << i) & reference) {
            source[i] = (i % 4) + 7; // values in a cycle: 7, 8, 9, 10
        } else {
            source[i] = (i % 7); // values in a cycle: 0, 1, 2, 3, 4, 5, 6
        }
    }

    // Job initialization
    qpl_status status = qpl_get_job_size(qpl_path_hardware, &size);
    ASSERT_EQ(QPL_STS_OK, status) << "An error " << status << " acquired during job size getting.\n";

    job_buffer   = std::make_unique<uint8_t[]>(size);
    qpl_job* job = reinterpret_cast<qpl_job*>(job_buffer.get());

    status = qpl_init_job(qpl_path_hardware, job);
    ASSERT_EQ(QPL_STS_OK, status) << "An error " << status << " acquired during job initializing.\n";

    // Performing an operation
    job->next_in_ptr        = source.data();
    job->available_in       = static_cast<uint32_t>(source.size());
    job->next_out_ptr       = destination.data();
    job->available_out      = static_cast<uint32_t>(destination.size());
    job->op                 = qpl_op_scan_range;
    job->src1_bit_width     = input_vector_width;
    job->num_input_elements = static_cast<uint32_t>(source.size());
    job->out_bit_width =
            qpl_ow_32; // without Force Array Output Mod, this will causes scan to return an array of indices
    job->param_low  = lower_boundary;
    job->param_high = upper_boundary;

    // Enable Force Array Output Modification
    job->flags |= QPL_FLAG_FORCE_ARRAY_OUTPUT;

    status = run_job_api(job);

    const auto bitmask_size_in_bytes    = job->total_out;
    const auto bitmask_size_in_elements = bitmask_size_in_bytes * byte_bit_length / output_vector_width;

    // Check if bitmask size matches the expected size
    EXPECT_EQ(bitmask_size_in_elements, source_size) << "Bitmask size does not match the expected size.\n";

    // Freeing resources
    status = qpl_fini_job(job);
    ASSERT_EQ(QPL_STS_OK, status) << "An error " << status << " acquired during job finalizing.\n";

    // Check if output matches the reference
    for (uint32_t i = 0; i < source_size; ++i) {
        // If the value should be in the scan range and it is not, return an error
        ASSERT_EQ((1UL << i) & reference ? 1U : 0U, output[i])
                << "Output value at index " << i << " does not match the expected value.\n";
    }
}

#if defined(__linux__)
#ifdef MADV_PAGEOUT

class ScanTestPageFault : public ScanTest {
private:
    std::unique_ptr<uint8_t[], void (*)(void*)> aligned_src {nullptr, {}};
    std::unique_ptr<uint8_t[], void (*)(void*)> aligned_dst {nullptr, {}};

public:
    void InitializeTestCases() {
        std::vector<uint32_t> lengths_multiplier;

        // @todo Extend testing to generate several pages of input/output
        // when logic on touching multiple Pages in case of PF is extended the library.
        for (uint32_t i = 1U; i <= 1U; i++)
            lengths_multiplier.push_back(i);

        // Following bit widths values are chosen for simplicity.
        // This way the resulting input buffer (next_in_ptr) would be exactly of length size.
        const uint32_t source_bit_width      = 8U;
        const uint32_t destination_bit_width = 1U;

        const uint32_t page_size = getpagesize();
        for (const uint32_t multiplier : lengths_multiplier) {
            const uint32_t max_input_value = (1ULL << source_bit_width) - 1;

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

    void SetUp() override {
        AnalyticFixture::SetUp();
        InitializeTestCases();
    }

    void SetBuffers() override {
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

        uint8_t* aligned_src_buffer = static_cast<uint8_t*>(std::aligned_alloc(psize, source.size()));
        uint8_t* aligned_dst_buffer = static_cast<uint8_t*>(std::aligned_alloc(psize, destination.size()));

        if (aligned_src_buffer == nullptr || aligned_dst_buffer == nullptr) {
            std::free(aligned_src_buffer); //NOLINT(cppcoreguidelines-no-malloc)
            std::free(aligned_dst_buffer); //NOLINT(cppcoreguidelines-no-malloc)

            return;
        }

        std::copy(source.begin(), source.end(), aligned_src_buffer);
        std::copy(destination.begin(), destination.end(), aligned_dst_buffer);

        aligned_src = std::unique_ptr<uint8_t[], void (*)(void*)>(aligned_src_buffer, std::free);

        aligned_dst = std::unique_ptr<uint8_t[], void (*)(void*)>(aligned_dst_buffer, std::free);

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

                const int err = madvise(job_ptr->next_in_ptr, psize, MADV_PAGEOUT);
                if (err) {
                    const int errsv = errno;
                    ASSERT_EQ(err, 0) << "madvise failed, error code is " << errsv << "\n";
                }
            } else if (type == WRITE_PAGE_FAULT) {
                // Check that we at least have a single page of data
                // to avoid swapping out the memory that we don't own.
                ASSERT_GE(job_ptr->available_out, psize);

                const int err = madvise(job_ptr->next_out_ptr, psize, MADV_PAGEOUT);
                if (err) {
                    const int errsv = errno;
                    ASSERT_EQ(err, 0) << "madvise failed, error code is " << errsv << "\n";
                }
            } else { // not supported
                return;
            }

            ASSERT_EQ(QPL_STS_OK, run_job_api(job_ptr));
            ASSERT_EQ(QPL_STS_OK, ref_compare(reference_job_ptr));

            // Copy results back to destination to ensure correct postprocessing.
            std::copy(aligned_dst.get(), aligned_dst.get() + job_ptr->total_out, destination.data());

            EXPECT_TRUE(CompareTotalInOutWithReference());
            EXPECT_TRUE(compare_checksum_fields(job_ptr, reference_job_ptr));
            EXPECT_TRUE(CompareVectors(destination, reference_destination, job_ptr->total_out));
        }
    }
};

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan_with_page_fault_read, scan_eq_only, ScanTestPageFault) {
    QPL_SKIP_TEST_FOR(qpl_path_software);

    job_ptr->op           = qpl_op_scan_eq;
    reference_job_ptr->op = qpl_op_scan_eq;

    RunTestScanPageFaults(READ_SRC_1_PAGE_FAULT);
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST_TC(scan_with_page_fault_write, scan_eq_only, ScanTestPageFault) {
    QPL_SKIP_TEST_FOR(qpl_path_software);

    job_ptr->op           = qpl_op_scan_eq;
    reference_job_ptr->op = qpl_op_scan_eq;

    RunTestScanPageFaults(WRITE_PAGE_FAULT);
}

#endif // #ifdef MADV_PAGEOUT
#endif // #if defined(__linux__)

} // namespace qpl::test
