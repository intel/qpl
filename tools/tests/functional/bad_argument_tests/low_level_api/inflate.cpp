/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11.2.2018
 * @brief Bad argument tests for @ref qpl_op_decompress operation
 *
 */

#include <array>

#include "qpl/qpl.h"

#include "gtest/gtest.h"
#include "tb_ll_common.hpp"

// tests_common
#include "operation_test.hpp"

namespace qpl::test {

/**
 * @brief A basic @ref qpl_op_decompress operation bad argument test
 */
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(inflate, base) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    source.fill(0);
    destination.fill(0);

    // Preset
    job_ptr->op    = qpl_op_decompress;
    job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST;

    set_input_stream(job_ptr, source.data(), (uint32_t)source.size(), NOT_APPLICABLE_PARAMETER,
                     NOT_APPLICABLE_PARAMETER, static_cast<qpl_parser>(NOT_APPLICABLE_PARAMETER));

    set_output_stream(job_ptr, destination.data(), (uint32_t)destination.size(),
                      static_cast<qpl_out_format>(NOT_APPLICABLE_PARAMETER));

    // Job is null
    ASSERT_EQ(QPL_STS_NULL_PTR_ERR, run_job_api(nullptr)) << "NULL job check";

    // Input vector is nullptr
    job_ptr->next_in_ptr = nullptr;

    ASSERT_EQ(QPL_STS_NULL_PTR_ERR, run_job_api(job_ptr)) << "NULL next_in_ptr field of qpl_job";

    // Output vector is nullptr
    job_ptr->next_in_ptr  = source.data();
    job_ptr->next_out_ptr = nullptr;

    ASSERT_EQ(QPL_STS_NULL_PTR_ERR, run_job_api(job_ptr)) << "NULL next_out_ptr field of qpl_job";
}

/**
 * @brief Test a more specific @ref qpl_op_decompress operation bad arguments
 */
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(inflate, extended) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    source.fill(0);
    destination.fill(0);

    // Preset
    job_ptr->op    = qpl_op_decompress;
    job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST;
    set_input_stream(job_ptr, source.data(), (uint32_t)source.size(), NOT_APPLICABLE_PARAMETER,
                     NOT_APPLICABLE_PARAMETER, static_cast<qpl_parser>(NOT_APPLICABLE_PARAMETER));
    set_output_stream(job_ptr, destination.data(), (uint32_t)destination.size(),
                      static_cast<qpl_out_format>(NOT_APPLICABLE_PARAMETER));

    job_ptr->ignore_end_bits       = 8U;
    job_ptr->ignore_start_bits     = 0U;
    job_ptr->decomp_end_processing = qpl_stop_and_check_for_bfinal_eob;

    ASSERT_EQ(QPL_STS_INVALID_PARAM_ERR, run_job_api(job_ptr)) << "ignore_end_bits more than 7 bits";

    job_ptr->ignore_end_bits       = 0U;
    job_ptr->ignore_start_bits     = 8U;
    job_ptr->decomp_end_processing = qpl_stop_and_check_for_bfinal_eob;

    ASSERT_EQ(QPL_STS_INVALID_PARAM_ERR, run_job_api(job_ptr)) << "ignore_start_bits more than 7 bits";

    job_ptr->ignore_end_bits       = 0U;
    job_ptr->ignore_start_bits     = 0U;
    job_ptr->decomp_end_processing = 9U;

    ASSERT_EQ(QPL_STS_INVALID_DECOMP_END_PROC_ERR, run_job_api(job_ptr)) << "Incorrect DecompressEndProcessing ";

    job_ptr->ignore_end_bits       = 0U;
    job_ptr->ignore_start_bits     = 0U;
    job_ptr->decomp_end_processing = OWN_RESERVED_INFLATE_MANIPULATOR;

    ASSERT_EQ(QPL_STS_INVALID_DECOMP_END_PROC_ERR, run_job_api(job_ptr)) << "Incorrect DecompressEndProcessing ";

    // BE16 format
    if (qpl_path_hardware == job_ptr->data_ptr.path) {
        job_ptr->flags           = QPL_FLAG_NO_HDRS | QPL_FLAG_HUFFMAN_BE;
        uint32_t odd_source_size = source.size();
        if (odd_source_size % 2 == 0) { odd_source_size -= 1; }
        set_input_stream(job_ptr, source.data(), odd_source_size, NOT_APPLICABLE_PARAMETER, NOT_APPLICABLE_PARAMETER,
                         static_cast<qpl_parser>(NOT_APPLICABLE_PARAMETER));
        job_ptr->ignore_end_bits       = 0U;
        job_ptr->ignore_start_bits     = 0U;
        job_ptr->decomp_end_processing = qpl_stop_and_check_for_bfinal_eob;

        ASSERT_EQ(QPL_STS_HUFFMAN_BE_ODD_INPUT_SIZE_ERR, run_job_api(job_ptr)) << "Odd input stream size for BE16 ";
    }
}

/**
 * @brief Test @ref qpl_op_decompress operation behaviour in case if incompatible flags were set
 */
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(inflate, flags_conflict) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    source.fill(0);
    destination.fill(0);

    // Preset
    job_ptr->op = qpl_op_decompress;
    set_input_stream(job_ptr, source.data(), (uint32_t)source.size(), NOT_APPLICABLE_PARAMETER,
                     NOT_APPLICABLE_PARAMETER, static_cast<qpl_parser>(NOT_APPLICABLE_PARAMETER));
    set_output_stream(job_ptr, destination.data(), (uint32_t)destination.size(),
                      static_cast<qpl_out_format>(NOT_APPLICABLE_PARAMETER));

    job_ptr->flags = QPL_FLAG_RND_ACCESS | QPL_FLAG_NO_HDRS | QPL_FLAG_FIRST | QPL_FLAG_LAST;

    ASSERT_EQ(QPL_STS_FLAG_CONFLICT_ERR, run_job_api(job_ptr))
            << "Don't found flag conflict: user try decompress huffman_only in random mode";

    job_ptr->flags = QPL_FLAG_GZIP_MODE | QPL_FLAG_NO_HDRS | QPL_FLAG_FIRST | QPL_FLAG_LAST;

    ASSERT_EQ(QPL_STS_FLAG_CONFLICT_ERR, run_job_api(job_ptr))
            << "Don't found flag conflict: user try decompress huffman_only in with gzip header";

    job_ptr->flags = QPL_FLAG_ZLIB_MODE | QPL_FLAG_NO_HDRS | QPL_FLAG_FIRST | QPL_FLAG_LAST;

    ASSERT_EQ(QPL_STS_FLAG_CONFLICT_ERR, run_job_api(job_ptr))
            << "Don't found flag conflict: user try decompress huffman_only in with zlib header";

    job_ptr->flags = QPL_FLAG_ZLIB_MODE | QPL_FLAG_GZIP_MODE | QPL_FLAG_FIRST | QPL_FLAG_LAST;

    ASSERT_EQ(QPL_STS_FLAG_CONFLICT_ERR, run_job_api(job_ptr))
            << "Don't found flag conflict: user try decompress with zlib and gzip header both";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(inflate, buffers_overlap) {
    check_buffer_overlap(job_ptr, qpl_op_decompress, QPL_FLAG_FIRST | QPL_FLAG_LAST);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(decompress_huffman_only, fixed) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);

    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);

    set_operation_properties(job_ptr, DROP_INITIAL_BYTES,
                             QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_NO_HDRS | QPL_FLAG_GEN_LITERALS,
                             qpl_op_decompress);

    job_ptr->huffman_table = nullptr;

    ASSERT_EQ(run_job_api(job_ptr), QPL_STS_NOT_SUPPORTED_MODE_ERR);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(inflate_canned, nullptr_huffman_table) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);

    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);

    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE,
                             qpl_op_decompress);

    job_ptr->huffman_table = nullptr;

    ASSERT_EQ(run_job_api(job_ptr), QPL_STS_NULL_PTR_ERR);
}

/**
 * @brief Tests inflate operation for Canned Mode and Indexing Mode flags and expects the
 * mode not supported error.
 */
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(inflate, canned_indexing) {
    const qpl_path_t execution_path = qpl::test::util::TestEnvironment::GetInstance().GetExecutionPath();

    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);

    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);

    set_operation_properties(job_ptr, DROP_INITIAL_BYTES,
                             QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_RND_ACCESS | QPL_FLAG_CANNED_MODE,
                             qpl_op_decompress);

    qpl_huffman_table_t huffman_table = nullptr;

    ASSERT_EQ(
            qpl_deflate_huffman_table_create(combined_table_type, execution_path, DEFAULT_ALLOCATOR_C, &huffman_table),
            QPL_STS_OK);

    job_ptr->huffman_table = huffman_table;

    const qpl_status status = run_job_api(job_ptr);

    qpl_huffman_table_destroy(huffman_table);

    ASSERT_EQ(status, QPL_STS_NOT_SUPPORTED_MODE_ERR);
}

} // namespace qpl::test
