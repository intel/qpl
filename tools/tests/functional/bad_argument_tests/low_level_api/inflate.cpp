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

#include <algorithm>
#include <array>

#include "qpl/qpl.h"

#include "gtest/gtest.h"
#include "tb_ll_common.hpp"

// tests_common
#include "dispatcher_checks.hpp"
#include "operation_test.hpp"

// tool_common
#include "util.hpp"

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

/**
 * @brief A basic @ref qpl_op_decompress operation based on accelerator configuration
 */
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(inflate, max_transfer_size) {
    QPL_SKIP_TEST_FOR(qpl_path_software);
    QPL_SKIP_TEST_FOR(qpl_path_auto);

    const uint64_t max_transfer_size = get_max_transfer_size();

    // need to allocate one more byte to exceed the transfer size
    auto source      = std::unique_ptr<uint8_t[]>(new uint8_t[max_transfer_size + 1]);
    auto destination = std::unique_ptr<uint8_t[]>(new uint8_t[max_transfer_size + 1]);

    // Preset
    set_operation_properties(job_ptr, NOT_APPLICABLE_PARAMETER, QPL_FLAG_FIRST | QPL_FLAG_LAST, qpl_op_decompress);
    set_input_stream(job_ptr, source.get(), SOURCE_ARRAY_SIZE, NOT_APPLICABLE_PARAMETER, NOT_APPLICABLE_PARAMETER,
                     static_cast<qpl_parser>(NOT_APPLICABLE_PARAMETER));

    set_output_stream(job_ptr, destination.get(), DESTINATION_ARRAY_SIZE,
                      static_cast<qpl_out_format>(NOT_APPLICABLE_PARAMETER));

    job_ptr->available_in = static_cast<uint32_t>(max_transfer_size + 1);
    ASSERT_EQ(run_job_api(job_ptr), QPL_STS_TRANSFER_SIZE_INVALID);
    job_ptr->available_in = SOURCE_ARRAY_SIZE;

    job_ptr->available_out = static_cast<uint32_t>(max_transfer_size + 1);
    ASSERT_EQ(run_job_api(job_ptr), QPL_STS_TRANSFER_SIZE_INVALID);
    job_ptr->available_out = DESTINATION_ARRAY_SIZE;
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

// Complete gzip member (RFC 1952) holding GZIP_PAYLOAD_SIZE bytes of (i * 7 + 3) in one stored deflate block
constexpr uint32_t GZIP_HEADER_SIZE  = 10U;
constexpr uint32_t GZIP_TRAILER_SIZE = 8U;
constexpr uint32_t GZIP_PAYLOAD_SIZE = 32U;

constexpr std::array<uint8_t, 55U> gzip_member = {
        // Header: ID1, ID2, CM = deflate, FLG = 0, MTIME = 0, XFL = 0, OS = unknown
        0x1FU, 0x8BU, 0x08U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0xFFU,
        // Stored block: BFINAL = 1, BTYPE = 00, LEN = 32, NLEN = ~32
        0x01U, 0x20U, 0x00U, 0xDFU, 0xFFU,
        // Payload
        0x03U, 0x0AU, 0x11U, 0x18U, 0x1FU, 0x26U, 0x2DU, 0x34U, 0x3BU, 0x42U, 0x49U, 0x50U, 0x57U, 0x5EU, 0x65U, 0x6CU,
        0x73U, 0x7AU, 0x81U, 0x88U, 0x8FU, 0x96U, 0x9DU, 0xA4U, 0xABU, 0xB2U, 0xB9U, 0xC0U, 0xC7U, 0xCEU, 0xD5U, 0xDCU,
        // Trailer: CRC32 = 0xA10E8695, ISIZE = 32
        0x95U, 0x86U, 0x0EU, 0xA1U, 0x20U, 0x00U, 0x00U, 0x00U};

static void set_gzip_decompress_job(qpl_job* job_ptr, uint8_t* source_ptr, uint32_t source_size,
                                    uint8_t* destination_ptr, uint32_t destination_size) {
    job_ptr->op            = qpl_op_decompress;
    job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_GZIP_MODE;
    job_ptr->next_in_ptr   = source_ptr;
    job_ptr->available_in  = source_size;
    job_ptr->next_out_ptr  = destination_ptr;
    job_ptr->available_out = destination_size;
}

/**
 * @brief Test @ref qpl_op_decompress operation in gzip mode with a final job too short for the gzip header
 */
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(inflate, gzip_truncated_header) {
    std::array<uint8_t, gzip_member.size()>     source = gzip_member;
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    for (uint32_t source_size = 1U; source_size < GZIP_HEADER_SIZE; source_size++) {
        ASSERT_EQ(QPL_STS_OK, qpl_init_job(GetExecutionPath(), job_ptr));
        set_gzip_decompress_job(job_ptr, source.data(), source_size, destination.data(), DESTINATION_ARRAY_SIZE);

        EXPECT_EQ(QPL_STS_MORE_INPUT_NEEDED, run_job_api(job_ptr)) << "source_size = " << source_size;
        EXPECT_EQ(0U, job_ptr->total_out) << "source_size = " << source_size;
    }
}

/**
 * @brief Test @ref qpl_op_decompress operation in gzip mode with a final job too short for the gzip trailer
 *
 * A complete gzip member is placed right after the bytes the job may read. Any read past
 * available_in decodes that member and reports output, so no output means no over-read.
 */
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(inflate, gzip_truncated_trailer) {
    std::array<uint8_t, GZIP_HEADER_SIZE + GZIP_TRAILER_SIZE + gzip_member.size()> source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE>                                    destination {};

    // Sanity check: the complete member decompresses on this path
    ASSERT_EQ(QPL_STS_OK, qpl_init_job(GetExecutionPath(), job_ptr));
    std::copy(gzip_member.begin(), gzip_member.end(), source.begin());
    set_gzip_decompress_job(job_ptr, source.data(), static_cast<uint32_t>(gzip_member.size()), destination.data(),
                            DESTINATION_ARRAY_SIZE);

    ASSERT_EQ(QPL_STS_OK, run_job_api(job_ptr));
    ASSERT_EQ(GZIP_PAYLOAD_SIZE, job_ptr->total_out);
    ASSERT_EQ(0xA10E8695U, job_ptr->crc);

    // Header followed by 0 to 7 body bytes, then a complete member outside of available_in
    for (uint32_t body_size = 0U; body_size < GZIP_TRAILER_SIZE; body_size++) {
        const uint32_t source_size = GZIP_HEADER_SIZE + body_size;

        source.fill(0U);
        std::copy(gzip_member.begin(), gzip_member.begin() + source_size, source.begin());
        std::copy(gzip_member.begin(), gzip_member.end(), source.begin() + source_size);
        destination.fill(0U);

        ASSERT_EQ(QPL_STS_OK, qpl_init_job(GetExecutionPath(), job_ptr));
        set_gzip_decompress_job(job_ptr, source.data(), source_size, destination.data(), DESTINATION_ARRAY_SIZE);

        const qpl_status status = run_job_api(job_ptr);

        // Asynchronous hardware submission does not strip the trailer, so the
        // accelerator itself reports the deflate stream as truncated
        if (GetExecutionPath() == qpl_path_hardware && TestEnviroment::GetInstance().IsAsynchronousApiTesting()) {
            EXPECT_TRUE(QPL_STS_MORE_INPUT_NEEDED == status || QPL_STS_BAD_EOF_ERR == status)
                    << "body_size = " << body_size << ", status = " << status;
        } else {
            EXPECT_EQ(QPL_STS_MORE_INPUT_NEEDED, status) << "body_size = " << body_size;
        }
        EXPECT_EQ(0U, job_ptr->total_out) << "body_size = " << body_size;
        EXPECT_TRUE(std::all_of(destination.begin(), destination.end(), [](uint8_t value) { return value == 0U; }))
                << "body_size = " << body_size;
    }
}

} // namespace qpl::test
