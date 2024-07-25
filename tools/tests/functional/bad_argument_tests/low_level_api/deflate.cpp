/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @date 11.2.2018
 * @brief Bad argument tests for @ref qpl_op_compress operation
 *
 */

#include <array>

#include "qpl/qpl.h"

#include "gtest/gtest.h"
#include "tb_ll_common.hpp"

// tests_common
#include "operation_test.hpp"

// tool_common
#include "util.hpp"

namespace qpl::test {

/**
 * @brief Bad argument test for @ref qpl_op_compress operation
 */
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(deflate, base) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    source.fill(0);
    destination.fill(0);

    // Preset
    job_ptr->op    = qpl_op_compress;
    job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST;

    set_input_stream(job_ptr, source.data(), (uint32_t)source.size(), NOT_APPLICABLE_PARAMETER,
                     NOT_APPLICABLE_PARAMETER, static_cast<qpl_parser>(NOT_APPLICABLE_PARAMETER));
    set_output_stream(job_ptr, destination.data(), (uint32_t)destination.size(),
                      static_cast<qpl_out_format>(NOT_APPLICABLE_PARAMETER));

    ASSERT_EQ(QPL_STS_NULL_PTR_ERR, run_job_api(nullptr));

    job_ptr->next_in_ptr = nullptr;
    ASSERT_EQ(QPL_STS_NULL_PTR_ERR, run_job_api(job_ptr));
    job_ptr->next_in_ptr = source.data();

    job_ptr->next_out_ptr = nullptr;
    ASSERT_EQ(QPL_STS_NULL_PTR_ERR, run_job_api(job_ptr));
    job_ptr->next_out_ptr = destination.data();

    job_ptr->available_in = 0U;
    ASSERT_EQ(QPL_STS_SIZE_ERR, run_job_api(job_ptr));
    job_ptr->available_in = SOURCE_ARRAY_SIZE;

    job_ptr->available_out = 0U;
    ASSERT_EQ(QPL_STS_SIZE_ERR, run_job_api(job_ptr));
}

/**
 * @brief Tests on missed flags
 */
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(deflate, missed_flags) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    source.fill(0);
    destination.fill(0);

    // Preset
    job_ptr->op = qpl_op_compress;
    set_input_stream(job_ptr, source.data(), (uint32_t)source.size(), NOT_APPLICABLE_PARAMETER,
                     NOT_APPLICABLE_PARAMETER, static_cast<qpl_parser>(NOT_APPLICABLE_PARAMETER));
    set_output_stream(job_ptr, destination.data(), (uint32_t)destination.size(),
                      static_cast<qpl_out_format>(NOT_APPLICABLE_PARAMETER));

    job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_HUFFMAN_BE | QPL_FLAG_GEN_LITERALS;
    ASSERT_EQ(QPL_STS_FLAG_CONFLICT_ERR, run_job_api(job_ptr));

    // Check that the correct error is returned when QPL_FLAG_NO_HDRS and QPL_FLAG_DYNAMIC_HUFFMAN
    // are used and the job is not a single job
    job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_NO_HDRS | QPL_FLAG_DYNAMIC_HUFFMAN;
    ASSERT_EQ(QPL_STS_FLAG_CONFLICT_ERR, run_job_api(job_ptr));
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(deflate, null_index_array) {
    qpl_status status = QPL_STS_OPERATION_ERR;

    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    std::fill(std::begin(source), std::end(source), 1);
    std::fill(std::begin(destination), std::end(destination), 1);

    job_ptr->op              = qpl_op_compress;
    job_ptr->next_in_ptr     = source.data();
    job_ptr->available_in    = static_cast<uint32_t>(source.size());
    job_ptr->next_out_ptr    = destination.data();
    job_ptr->available_out   = static_cast<uint32_t>(destination.size());
    job_ptr->mini_block_size = qpl_mblk_size_1k;
    job_ptr->flags           = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN;

    status = run_job_api(job_ptr);

    ASSERT_EQ(QPL_STS_MISSING_INDEX_TABLE_ERR, status) << "idx_array equals to NULL";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(deflate, flag_conflict) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);

    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);

    // Check that the correct error is returned when QPL_FLAG_NO_HDRS and QPL_FLAG_DYNAMIC_HUFFMAN
    // are used and the job is not a single job
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, QPL_FLAG_NO_HDRS | QPL_FLAG_DYNAMIC_HUFFMAN | QPL_FLAG_FIRST,
                             qpl_op_compress);

    ASSERT_EQ(run_job_api(job_ptr), QPL_STS_FLAG_CONFLICT_ERR);

    set_operation_properties(job_ptr, DROP_INITIAL_BYTES,
                             QPL_FLAG_HUFFMAN_BE | !QPL_FLAG_NO_HDRS | QPL_FLAG_FIRST | QPL_FLAG_LAST, qpl_op_compress);

    ASSERT_EQ(run_job_api(job_ptr), QPL_STS_FLAG_CONFLICT_ERR);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(deflate, incorrect_compression_level) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);

    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);

    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, QPL_FLAG_DYNAMIC_HUFFMAN, qpl_op_compress);

    job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST;

    job_ptr->level = (qpl_compression_levels)0xFF;

    ASSERT_EQ(run_job_api(job_ptr), QPL_STS_UNSUPPORTED_COMPRESSION_LEVEL);

    if (qpl::test::util::TestEnvironment::GetInstance().GetExecutionPath() == qpl_path_hardware) {
        job_ptr->level = qpl_high_level;

        ASSERT_EQ(run_job_api(job_ptr), QPL_STS_UNSUPPORTED_COMPRESSION_LEVEL);
    }
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(deflate, buffers_overlap) {
    check_buffer_overlap(job_ptr, qpl_op_compress, QPL_FLAG_FIRST | QPL_FLAG_LAST);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(deflate, dictionary_and_gzip_zlib_wrappers) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);

    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);

    // Emulate dictionary
    uint8_t dictionary[1];
    job_ptr->dictionary = reinterpret_cast<qpl_dictionary*>(&dictionary);

    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, QPL_FLAG_FIRST | QPL_FLAG_GZIP_MODE | QPL_FLAG_OMIT_VERIFY,
                             qpl_op_compress);

    ASSERT_EQ(run_job_api(job_ptr), QPL_STS_NOT_SUPPORTED_MODE_ERR);

    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, QPL_FLAG_FIRST | QPL_FLAG_ZLIB_MODE | QPL_FLAG_OMIT_VERIFY,
                             qpl_op_compress);

    ASSERT_EQ(run_job_api(job_ptr), QPL_STS_NOT_SUPPORTED_MODE_ERR);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(deflate, dictionary_and_verification) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);

    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);

    // Emulate dictionary
    uint8_t dictionary[1];
    job_ptr->dictionary = reinterpret_cast<qpl_dictionary*>(&dictionary);

    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, QPL_FLAG_FIRST, qpl_op_compress);

    ASSERT_EQ(run_job_api(job_ptr), QPL_STS_NOT_SUPPORTED_MODE_ERR);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(compress_huffman_only, fixed) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);

    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);

    set_operation_properties(job_ptr, DROP_INITIAL_BYTES,
                             QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_NO_HDRS | QPL_FLAG_GEN_LITERALS,
                             qpl_op_compress);

    job_ptr->huffman_table = nullptr;

    ASSERT_EQ(run_job_api(job_ptr), QPL_STS_NOT_SUPPORTED_MODE_ERR);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(deflate_canned, nullptr_huffman_table) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);

    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);

    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE,
                             qpl_op_compress);

    job_ptr->huffman_table = nullptr;

    ASSERT_EQ(run_job_api(job_ptr), QPL_STS_NULL_PTR_ERR);
}

/**
 * @brief Tests deflate operation for Canned Mode and Indexing Mode flags and expects the
 * mode not supported error.
 */
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(deflate, canned_indexing) {
    const qpl_path_t execution_path = qpl::test::util::TestEnvironment::GetInstance().GetExecutionPath();

    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};
    std::array<uint64_t, INDEX_ARRAY_SIZE>      index_array {};

    set_input_stream(job_ptr, source.data(), SOURCE_ARRAY_SIZE, INPUT_BIT_WIDTH, ELEMENTS_TO_PROCESS, INPUT_FORMAT);

    set_output_stream(job_ptr, destination.data(), DESTINATION_ARRAY_SIZE, OUTPUT_BIT_WIDTH);

    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE,
                             qpl_op_compress);

    set_indexing_parameters(job_ptr, INDEX_MBLK_SIZE, index_array.data(), INDEX_ARRAY_SIZE);

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
