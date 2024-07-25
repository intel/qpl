/*******************************************************************************
 * Copyright (C) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <array>
#include <memory>
#include <vector>

#include "iaa_features_checks.hpp"
#include "operation_test.hpp"
#include "ta_ll_common.hpp"
#include "util.hpp"

namespace qpl::test {
/**
 * @brief Test for fixed mode compression and decompression with dictionary using the same job structure
 */
QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(dictionary_deflate_inflate_job_reusage, fixed) {
    auto path = util::TestEnvironment::GetInstance().GetExecutionPath();

    // Skip this test if not on software path and dictionary compression is not supported on the hardware path
    if (path == qpl_path_hardware && !is_iaa_dictionary_compress_supported()) {
        GTEST_SKIP() << "Dictionary is not supported in this generation of accelerator";
    }

    uint32_t             num_iterations = 0U;
    sw_compression_level sw_compr_level = sw_compression_level::SW_NONE;
    hw_compression_level hw_compr_level = hw_compression_level::HW_NONE;

    if (path == qpl_path_software) {
        num_iterations = sw_levels.size();
    } else {
        num_iterations = hw_levels.size();
    }

    for (uint32_t i = 0U; i < num_iterations; i++) {
        if (path == qpl_path_software) {
            sw_compr_level = sw_levels[i];
        } else {
            hw_compr_level = hw_levels[i];
        }

        for (auto& dataset : util::TestEnvironment::GetInstance().GetAlgorithmicDataset().get_data()) {
            std::vector<uint8_t> source;
            source = dataset.second;

            for (auto dictionary_length : get_dictionary_lengths()) {
                uint32_t   size   = 0U;
                qpl_status status = qpl_get_job_size(path, &size);
                ASSERT_EQ(QPL_STS_OK, status) << "Failed to get job size";

                auto        job_buffer          = std::make_unique<uint8_t[]>(size);
                auto* const comp_decomp_job_ptr = reinterpret_cast<qpl_job*>(job_buffer.get());

                status = qpl_init_job(path, comp_decomp_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status) << "Failed to init job";

                if (dictionary_length > 4096U) { dictionary_length = static_cast<uint32_t>(source.size()); }

                std::vector<uint8_t> destination(source.size() * 2, 0U);
                std::vector<uint8_t> reference_buffer(destination.size(), 0U);

                auto dictionary_buffer_size =
                        qpl_get_dictionary_size(sw_compr_level, hw_compr_level, dictionary_length);

                auto dictionary_buffer = std::make_unique<uint8_t[]>(dictionary_buffer_size);
                auto dictionary_ptr    = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.get());

                status = qpl_build_dictionary(dictionary_ptr, sw_compr_level, hw_compr_level, source.data(),
                                              dictionary_length);
                ASSERT_EQ(QPL_STS_OK, status) << "Dictionary build failed for source: " << dataset.first;

                // Configure compression job fields
                comp_decomp_job_ptr->op            = qpl_op_compress;
                comp_decomp_job_ptr->level         = qpl_default_level;
                comp_decomp_job_ptr->next_in_ptr   = source.data();
                comp_decomp_job_ptr->available_in  = static_cast<uint32_t>(source.size());
                comp_decomp_job_ptr->next_out_ptr  = destination.data();
                comp_decomp_job_ptr->available_out = static_cast<uint32_t>(destination.size());
                comp_decomp_job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY;
                comp_decomp_job_ptr->dictionary    = dictionary_ptr;
                comp_decomp_job_ptr->huffman_table = NULL;

                status = run_job_api(comp_decomp_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status) << "Compression failed";

                const uint32_t compressed_size = comp_decomp_job_ptr->total_out;
                destination.resize(compressed_size);

                // Configure decompression job fields
                comp_decomp_job_ptr->op            = qpl_op_decompress;
                comp_decomp_job_ptr->next_in_ptr   = destination.data();
                comp_decomp_job_ptr->available_in  = compressed_size;
                comp_decomp_job_ptr->next_out_ptr  = reference_buffer.data();
                comp_decomp_job_ptr->available_out = static_cast<uint32_t>(reference_buffer.size());
                comp_decomp_job_ptr->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;
                comp_decomp_job_ptr->dictionary    = dictionary_ptr;

                status = run_job_api(comp_decomp_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status) << "Decompression failed";

                reference_buffer.resize(comp_decomp_job_ptr->total_out);

                EXPECT_TRUE(CompareVectors(reference_buffer, source));

                status = qpl_fini_job(comp_decomp_job_ptr);
                ASSERT_EQ(QPL_STS_OK, status) << "Failed to fini job";
            }
        }
    }
}
} // namespace qpl::test
