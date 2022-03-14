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

#include "gtest/gtest.h"
#include "qpl/qpl.h"
#include "tb_ll_common.hpp"
#include "../../../common/operation_test.hpp"

#include <array>

namespace qpl::test {

/**
 * @brief A basic @ref qpl_op_decompress operation bad argument test
 */
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(inflate, base) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE> source{};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination{};

    source.fill(0);
    destination.fill(0);

    // Preset
    job_ptr->op = qpl_op_decompress;
    set_input_stream(job_ptr,
                     source.data(),
                     (uint32_t) source.size(),
                     NOT_APPLICABLE_PARAMETER,
                     NOT_APPLICABLE_PARAMETER,
                     static_cast<qpl_parser>(NOT_APPLICABLE_PARAMETER));

    set_output_stream(job_ptr, destination.data(),
                      (uint32_t) destination.size(),
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
    std::array<uint8_t, SOURCE_ARRAY_SIZE> source{};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination{};

    source.fill(0);
    destination.fill(0);

    // Preset
    job_ptr->op = qpl_op_decompress;
    set_input_stream(job_ptr,
                     source.data(),
                     (uint32_t) source.size(),
                     NOT_APPLICABLE_PARAMETER,
                     NOT_APPLICABLE_PARAMETER,
                     static_cast<qpl_parser>(NOT_APPLICABLE_PARAMETER));
    set_output_stream(job_ptr, destination.data(),
                      (uint32_t) destination.size(),
                      static_cast<qpl_out_format>(NOT_APPLICABLE_PARAMETER));

    job_ptr->ignore_end_bits       = 8u;
    job_ptr->ignore_start_bits     = 0u;
    job_ptr->decomp_end_processing = qpl_stop_and_check_for_bfinal_eob;

    ASSERT_EQ(QPL_STS_INVALID_PARAM_ERR, run_job_api(job_ptr)) << "ignore_end_bits more than 7 bits";

    job_ptr->ignore_end_bits       = 0u;
    job_ptr->ignore_start_bits     = 8u;
    job_ptr->decomp_end_processing = qpl_stop_and_check_for_bfinal_eob;

    ASSERT_EQ(QPL_STS_INVALID_PARAM_ERR, run_job_api(job_ptr)) << "ignore_start_bits more than 7 bits";

    job_ptr->ignore_end_bits       = 0u;
    job_ptr->ignore_start_bits     = 0u;
    job_ptr->decomp_end_processing = 9u;

    ASSERT_EQ(QPL_STS_INVALID_DECOMP_END_PROC_ERR, run_job_api(job_ptr)) << "Incorrect DecompressEndProcessing ";

    job_ptr->ignore_end_bits       = 0u;
    job_ptr->ignore_start_bits     = 0u;
    job_ptr->decomp_end_processing = OWN_RESERVED_INFLATE_MANIPULATOR;

    ASSERT_EQ(QPL_STS_INVALID_DECOMP_END_PROC_ERR, run_job_api(job_ptr)) << "Incorrect DecompressEndProcessing ";
}

/**
 * @brief Test @ref qpl_op_decompress operation behaviour in case if incompatible flags were set
 */
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(inflate, flags_confilct) {
    std::array<uint8_t, SOURCE_ARRAY_SIZE> source{};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination{};

    source.fill(0);
    destination.fill(0);

    // Preset
    job_ptr->op = qpl_op_decompress;
    set_input_stream(job_ptr,
                     source.data(),
                     (uint32_t) source.size(),
                     NOT_APPLICABLE_PARAMETER,
                     NOT_APPLICABLE_PARAMETER,
                     static_cast<qpl_parser>(NOT_APPLICABLE_PARAMETER));
    set_output_stream(job_ptr, destination.data(),
                      (uint32_t) destination.size(),
                      static_cast<qpl_out_format>(NOT_APPLICABLE_PARAMETER));

    job_ptr->flags = QPL_FLAG_RND_ACCESS | QPL_FLAG_NO_HDRS;

    ASSERT_EQ(QPL_STS_FLAG_CONFLICT_ERR, run_job_api(job_ptr))
                                << "Don't found flag conflict: user try decompress huffman_only in random mode";

    job_ptr->flags = QPL_FLAG_GZIP_MODE | QPL_FLAG_NO_HDRS;

    ASSERT_EQ(QPL_STS_FLAG_CONFLICT_ERR, run_job_api(job_ptr))
                                << "Don't found flag conflict: user try decompress huffman_only in with gzip header";

    job_ptr->flags = QPL_FLAG_ZLIB_MODE | QPL_FLAG_NO_HDRS;

    if (qpl_path_hardware == job_ptr->data_ptr.path) {
        ASSERT_EQ(run_job_api(job_ptr), QPL_STS_NOT_SUPPORTED_MODE_ERR);
    } else {
        ASSERT_EQ(QPL_STS_FLAG_CONFLICT_ERR, run_job_api(job_ptr))
                                    << "Don't found flag conflict: user try decompress huffman_only in with zlib header";
    }


    job_ptr->flags = QPL_FLAG_ZLIB_MODE | QPL_FLAG_GZIP_MODE;

    if (qpl_path_hardware == job_ptr->data_ptr.path) {
        ASSERT_EQ(run_job_api(job_ptr), QPL_STS_NOT_SUPPORTED_MODE_ERR);
    } else {
        ASSERT_EQ(QPL_STS_FLAG_CONFLICT_ERR, run_job_api(job_ptr))
                                    << "Don't found flag conflict: user try decompress with zlib and gzip header both";
    }

}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(inflate, buffers_overlap) {
    check_buffer_overlap(job_ptr, qpl_op_decompress, OPERATION_FLAGS);
}
}
