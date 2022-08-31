/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "operation_test.hpp"
#include "compression_huffman_table.hpp"
#include "tn_common.hpp"
#include "random_generator.h"

namespace qpl::test {

static qpl_status perform_dynamic_compression(qpl_job *job_ptr,
                                              uint8_t *source_ptr,
                                              uint32_t source_length,
                                              uint8_t *destination_ptr,
                                              int32_t destination_length,
                                              uint32_t order_flag) {
    job_ptr->next_in_ptr  = source_ptr;
    job_ptr->available_in = source_length;

    job_ptr->next_out_ptr  = destination_ptr;
    job_ptr->available_out = destination_length;

    job_ptr->flags         = order_flag | QPL_FLAG_DYNAMIC_HUFFMAN | QPL_FLAG_OMIT_VERIFY;
    job_ptr->huffman_table = nullptr;

    return run_job_api(job_ptr);
}

static qpl_status perform_static_compression(qpl_job *job_ptr,
                                             uint8_t *source_ptr,
                                             uint32_t source_length,
                                             uint8_t *destination_ptr,
                                             int32_t destination_length,
                                             uint32_t order_flag) {
    qpl_histogram deflate_histogram{};
    auto          path = util::TestEnvironment::GetInstance().GetExecutionPath();

    qpl_huffman_table_t huffman_table;

    auto status = QPL_STS_OK;

    status = qpl_deflate_huffman_table_create(compression_table_type, path, {malloc, free}, &huffman_table);
    if (status != QPL_STS_OK) {
        std::cout << "Huffman table create error";

        return status;
    }

    status = qpl_gather_deflate_statistics(source_ptr,
                                           source_length,
                                           &deflate_histogram,
                                           qpl_default_level,
                                           path);
    if (status != QPL_STS_OK) {
        std::cout << "Statistics gathering failed";

        return status;
    }

    status = qpl_huffman_table_init_with_histogram(huffman_table, &deflate_histogram);
    if (status != QPL_STS_OK) {
        std::cout << "Table build failed";

        return status;
    }

    job_ptr->huffman_table = huffman_table;
    job_ptr->next_in_ptr   = source_ptr;
    job_ptr->available_in  = source_length;

    job_ptr->next_out_ptr  = destination_ptr;
    job_ptr->available_out = destination_length;
    job_ptr->flags         = order_flag | QPL_FLAG_OMIT_VERIFY;

    status = run_job_api(job_ptr);
    if (status != QPL_STS_OK) {
        std::cout << "Job failed";

        return status;
    }


    status = qpl_huffman_table_destroy(huffman_table);
    if (status != QPL_STS_OK) {
        std::cout << "Table destruction failed";

        return status;
    }

    return status;
}

static qpl_status perform_fixed_compression(qpl_job *job_ptr,
                                            uint8_t *source_ptr,
                                            uint32_t source_length,
                                            uint8_t *destination_ptr,
                                            int32_t destination_length,
                                            uint32_t order_flag) {
    job_ptr->next_in_ptr  = source_ptr;
    job_ptr->available_in = source_length;

    job_ptr->next_out_ptr  = destination_ptr;
    job_ptr->available_out = destination_length;
    job_ptr->flags         = order_flag | QPL_FLAG_OMIT_VERIFY;
    job_ptr->level         = qpl_default_level;

    job_ptr->huffman_table = nullptr;

    return run_job_api(job_ptr);
}

QPL_LOW_LEVEL_API_NEGATIVE_TEST_F(deflate, JobFixture, try_to_compress_different_styles) {
    const uint32_t    maximum_length = 4096;
    qpl::test::random random_style(0, 2u, GetSeed());
    qpl::test::random random_bit_of_pie(2u, 8u, GetSeed());
    qpl::test::random random_element_generator(0, 1, GetSeed());

    source.resize(maximum_length);
    destination.resize(source.size() * 2);

    for (auto &elem: source) {
        elem = (uint8_t) random_element_generator;
    }

    job_ptr->op    = qpl_op_compress;
    job_ptr->level = qpl_default_level;

    using compression_function_ptr = qpl_status (*)(qpl_job *job_ptr,
                                                    uint8_t *source_ptr,
                                                    uint32_t source_length,
                                                    uint8_t *destination_ptr,
                                                    int32_t destination_length,
                                                    uint32_t order_flag);

    constexpr compression_function_ptr compress_with_style_table[] = {
            perform_dynamic_compression,
            perform_static_compression,
            perform_fixed_compression
    };

    auto     current_style        = (uint32_t) random_style;
    auto     bytes_remain         = (uint32_t) source.size();
    uint32_t current_block_length = bytes_remain / (uint32_t) random_bit_of_pie;

    auto first_run_status = compress_with_style_table[current_style](job_ptr,
                                                                     source.data(),
                                                                     current_block_length,
                                                                     destination.data(),
                                                                     (uint32_t) destination.size(),
                                                                     QPL_FLAG_FIRST);

    ASSERT_EQ(QPL_STS_OK, first_run_status);

    bytes_remain -= current_block_length;
    uint32_t previous_style = current_style;

    while (previous_style == (current_style = (uint32_t) random_style)) {
        // Switching styles until get a new one
    }

    auto second_run_status = compress_with_style_table[current_style](job_ptr,
                                                                      job_ptr->next_in_ptr,
                                                                      current_block_length,
                                                                      job_ptr->next_out_ptr,
                                                                      job_ptr->available_out,
                                                                      QPL_FLAG_LAST);

    ASSERT_EQ(QPL_STS_INVALID_COMPRESS_STYLE_ERR, second_run_status)
                                << "First style: " << previous_style
                                << ",second style: " << current_style;
}
}
