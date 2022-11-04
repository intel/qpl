/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "qpl/cpp_api/operations/operation.hpp"
#include "qpl/cpp_api/util/status_handler.hpp"
#include "qpl/cpp_api/util/qpl_util.hpp"
#include "../t_common.hpp"

namespace qpl::test {

QPL_UNIT_API_ALGORITHMIC_TEST(hl_util, expected_exceptions) {
    // Operation process exceptions
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_BEING_PROCESSED), operation_process_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_VERIFY_ERR), operation_process_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_INVALID_RLE_COUNT), operation_process_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_INDEX_GENERATION_ERR), operation_process_exception);

    // Memory underflow exceptions
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_MORE_OUTPUT_NEEDED), memory_underflow_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_INDEX_ARRAY_TOO_SMALL), memory_underflow_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_MORE_INPUT_NEEDED), memory_underflow_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_NO_MEM_ERR), memory_underflow_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_SIZE_ERR), memory_underflow_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_SRC_IS_SHORT_ERR), memory_underflow_exception);

    // Short destination exceptions
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_DST_IS_SHORT_ERR), short_destination_exception);

    // Invalid data exceptions
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_INVALID_DEFLATE_DATA_ERR), invalid_data_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_ARCHIVE_HEADER_ERR), invalid_data_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_INVALID_BLOCK_SIZE_ERR), invalid_data_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_INVALID_HUFFMAN_TABLE_ERR), invalid_data_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_SRC2_IS_SHORT_ERR), invalid_data_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_DIST_SPANS_MINI_BLOCKS), invalid_data_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_LEN_SPANS_MINI_BLOCKS), invalid_data_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_VERIF_INVALID_BLOCK_SIZE), invalid_data_exception);

    // Invalid arguments exceptions
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_INVALID_PARAM_ERR), invalid_argument_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_MISSING_INDEX_TABLE_ERR), invalid_argument_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_NULL_PTR_ERR), invalid_argument_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_DROP_BITS_OVERFLOW_ERR), invalid_argument_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_CRC64_BAD_POLYNOM), invalid_argument_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_PRLE_FORMAT_ERR), invalid_argument_exception);

    // Memory overflow exceptions
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_BUFFER_TOO_LARGE_ERR), memory_overflow_exception);
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_OUTPUT_OVERFLOW_ERR), memory_overflow_exception);

    // Internal error, common exception
    EXPECT_THROW(qpl::util::handle_status(QPL_STS_LIBRARY_INTERNAL_ERR), qpl::exception);

    EXPECT_THROW({
        try {
            qpl::util::handle_status(UINT32_MAX);
        } catch(const operation_process_exception& e) {
            ASSERT_STREQ(e.what(), "Unexpected error code returned while processing an operation");
            throw;
        }
    }, operation_process_exception);
}

QPL_UNIT_API_ALGORITHMIC_TEST(hl_util, integer_to_qpl_output_format) {
    auto qpl_output = qpl::util::integer_to_qpl_output_format(1u);
    EXPECT_EQ(qpl_ow_nom, qpl_output);

    qpl_output = qpl::util::integer_to_qpl_output_format(8u);
    EXPECT_EQ(qpl_ow_8, qpl_output);

    qpl_output = qpl::util::integer_to_qpl_output_format(16u);
    EXPECT_EQ(qpl_ow_16, qpl_output);

    qpl_output = qpl::util::integer_to_qpl_output_format(32u);
    EXPECT_EQ(qpl_ow_32, qpl_output);

    // Check default branch
    qpl_output = qpl::util::integer_to_qpl_output_format(100u);
    EXPECT_EQ(qpl_ow_nom, qpl_output);
}

QPL_UNIT_API_ALGORITHMIC_TEST(hl_util, add_decompression_flags_to_job) {
    qpl_job job{};

    bool use_gzip_flag = false;
    qpl::util::add_decompression_flags_to_job(use_gzip_flag, &job);

    EXPECT_FALSE(job.flags & QPL_FLAG_GZIP_MODE);

    use_gzip_flag = true;
    qpl::util::add_decompression_flags_to_job(use_gzip_flag, &job);

    EXPECT_TRUE(job.flags & QPL_FLAG_GZIP_MODE);
}


}
