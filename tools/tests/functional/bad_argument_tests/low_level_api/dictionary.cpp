/*******************************************************************************
 * Copyright (C) 2023 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 * Intel® Query Processing Library (Intel® QPL)
 * Tests
 * @brief Bad argument tests for @ref qpl_op_compress and @ref qpl_op_decompress
 *        operation using @ref qpl_dictionary
 */

#include "qpl/qpl.h"

#include "gtest/gtest.h"
#include "tb_ll_common.hpp"

// tests_common
#include "operation_test.hpp"

// tool_common
#include "iaa_features_checks.hpp"

namespace qpl::test {
/**
 * @brief Bad argument test for dictionary compression with invalid level
 */
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(deflate_with_dictionary, level_none) {
    if (util::TestEnvironment::GetInstance().GetExecutionPath() == qpl_path_software) {
        GTEST_SKIP() << "Software dictionary level currently depends on the compression level set in qpl_job";
    }

    if (util::TestEnvironment::GetInstance().GetExecutionPath() == qpl_path_auto) {
        GTEST_SKIP() << "Skip because this test is not designed for qpl_path_auto";
    }

    if (util::TestEnvironment::GetInstance().GetExecutionPath() == qpl_path_hardware &&
        !is_iaa_dictionary_compress_supported()) {
        GTEST_SKIP() << "Dictionary is not supported in this generation of accelerator";
    }

    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    source.fill(0);
    destination.fill(0);

    // Preset
    set_input_stream(job_ptr, source.data(), (uint32_t)source.size(), NOT_APPLICABLE_PARAMETER,
                     NOT_APPLICABLE_PARAMETER, static_cast<qpl_parser>(NOT_APPLICABLE_PARAMETER));
    set_output_stream(job_ptr, destination.data(), (uint32_t)destination.size(),
                      static_cast<qpl_out_format>(NOT_APPLICABLE_PARAMETER));

    // Emulate dictionary
    const uint32_t             dictionary_length = source.size();
    const sw_compression_level sw_compr_level    = sw_compression_level::SW_NONE;
    const hw_compression_level hw_compr_level    = hw_compression_level::HW_NONE;

    auto dictionary_buffer_size = qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

    auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
    auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

    auto status =
            qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(), dictionary_length);
    ASSERT_EQ(QPL_STS_OK, status);

    job_ptr->dictionary = dictionary_ptr;

    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY,
                             qpl_op_compress);

    ASSERT_EQ(QPL_STS_NOT_SUPPORTED_MODE_ERR, run_job_api(job_ptr));
}

/**
 * @brief Bad argument test for dictionary compression with multi-chunk on HW path
 */
QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(deflate_with_dictionary, hw_multi_chunk) {
    if (util::TestEnvironment::GetInstance().GetExecutionPath() == qpl_path_software) {
        GTEST_SKIP() << "Software dictionary does not have limitation with multi-chunk";
    }

    if (util::TestEnvironment::GetInstance().GetExecutionPath() == qpl_path_auto) {
        GTEST_SKIP() << "Skip because this test is not designed for qpl_path_auto";
    }

    if (util::TestEnvironment::GetInstance().GetExecutionPath() == qpl_path_hardware &&
        !is_iaa_dictionary_compress_supported()) {
        GTEST_SKIP() << "Dictionary is not supported in this generation of accelerator";
    }

    std::array<uint8_t, SOURCE_ARRAY_SIZE>      source {};
    std::array<uint8_t, DESTINATION_ARRAY_SIZE> destination {};

    source.fill(0);
    destination.fill(0);

    // Preset
    set_input_stream(job_ptr, source.data(), (uint32_t)source.size(), NOT_APPLICABLE_PARAMETER,
                     NOT_APPLICABLE_PARAMETER, static_cast<qpl_parser>(NOT_APPLICABLE_PARAMETER));
    set_output_stream(job_ptr, destination.data(), (uint32_t)destination.size(),
                      static_cast<qpl_out_format>(NOT_APPLICABLE_PARAMETER));

    // Emulate dictionary
    const uint32_t             dictionary_length = source.size();
    const sw_compression_level sw_compr_level    = sw_compression_level::SW_NONE;
    const hw_compression_level hw_compr_level    = hw_compression_level::HW_LEVEL_1;

    auto dictionary_buffer_size = qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

    auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
    auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

    auto status =
            qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(), dictionary_length);
    ASSERT_EQ(QPL_STS_OK, status);

    job_ptr->dictionary = dictionary_ptr;

    // Submit the first job and expect error because hw path dictionary only supports single chunk
    set_operation_properties(job_ptr, DROP_INITIAL_BYTES, QPL_FLAG_FIRST | QPL_FLAG_OMIT_VERIFY, qpl_op_compress);

    ASSERT_EQ(QPL_STS_NOT_SUPPORTED_MODE_ERR, run_job_api(job_ptr));
}

} // namespace qpl::test
