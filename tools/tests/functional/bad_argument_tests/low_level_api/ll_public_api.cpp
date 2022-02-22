/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "gtest/gtest.h"
#include "tb_ll_common.hpp"
#include "../../../common/operation_test.hpp"
#include <memory>

namespace qpl::test {

constexpr uint32_t dictionary_test_size = 10u;
constexpr uint32_t dictionary_id_test   = 1u;

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_get_job_size, test) {
    qpl_status status;
    uint32_t   size;

    status = qpl_get_job_size(PATH, nullptr);

    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on job_ptr == nullptr";

    status = qpl_get_job_size(INCORRECT_PATH, &size);

    EXPECT_EQ(status, QPL_STS_PATH_ERR) << "Failed on incorrect path check";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_init, test) {
    qpl_status status;
    qpl_job    job;
    job.op = qpl_op_memcpy;

    status = qpl_init_job(PATH, nullptr);

    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on job_ptr == nullptr";

    status = qpl_init_job(INCORRECT_PATH, &job);

    EXPECT_EQ(status, QPL_STS_PATH_ERR) << "Failed on incorrect path check";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_submit, test) {
    qpl_status status;
    uint8_t    *stored_ptr;

    job_ptr->op          = qpl_op_memcpy;
    job_ptr->next_in_ptr = (uint8_t *) job_ptr;

    status = qpl_submit_job(nullptr);

    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on job_ptr == nullptr";

    // Save corrupted pointer
    stored_ptr = job_ptr->data_ptr.analytics_state_ptr;

    // Check analytics_state_ptr
    job_ptr->data_ptr.analytics_state_ptr = nullptr;
    status = qpl_submit_job(job_ptr);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on analytics_state_ptr == nullptr";

    // Restore corrupted pointer and save next
    job_ptr->data_ptr.analytics_state_ptr = stored_ptr;
    stored_ptr = job_ptr->data_ptr.compress_state_ptr;

    // Check compress_state_ptr
    job_ptr->data_ptr.compress_state_ptr = nullptr;
    status = qpl_submit_job(job_ptr);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on compress_state_ptr == nullptr";

    // Restore corrupted pointer and save next
    job_ptr->data_ptr.compress_state_ptr = stored_ptr;
    stored_ptr = job_ptr->data_ptr.decompress_state_ptr;

    // Check decompress_state_ptr
    job_ptr->data_ptr.decompress_state_ptr = nullptr;
    status = qpl_submit_job(job_ptr);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on decompress_state_ptr == nullptr";

    // Restore corrupted pointer and save next
    job_ptr->data_ptr.decompress_state_ptr = stored_ptr;
    stored_ptr = job_ptr->data_ptr.hw_state_ptr;

    job_ptr->data_ptr.hw_state_ptr = nullptr;
    status = qpl_submit_job(job_ptr);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on hw_state_ptr == nullptr";

    // Restore corrupted pointer
    job_ptr->data_ptr.hw_state_ptr = stored_ptr;

    qpl::test::random random_opcode(0, RESERVED_OPCODES_COUNT - 1, TestEnviroment::GetInstance().GetSeed());

    auto opcode_index = static_cast<uint8_t>(random_opcode);

    job_ptr->op = (qpl_operation) (reserved_op_codes[opcode_index]);
    status = qpl_submit_job(job_ptr);
    EXPECT_EQ(status, QPL_STS_OPERATION_ERR) << "Failed on opcode test";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_execute, test) {
    qpl_status status;
    qpl_job    job;
    job.op = qpl_op_memcpy;

    status = qpl_execute_job(nullptr);

    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on job_ptr == nullptr";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_wait, test) {
    qpl_status status;
    qpl_job    job;
    job.op = qpl_op_memcpy;

    status = qpl_wait_job(nullptr);

    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on job_ptr == nullptr";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_check, test) {
    qpl_status status;
    qpl_job    job;
    job.op = qpl_op_memcpy;

    status = qpl_check_job(nullptr);

    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on job_ptr == nullptr";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_finalize, test) {
    qpl_status status;
    qpl_job    job;
    job.op = qpl_op_memcpy;

    status = qpl_fini_job(nullptr);

    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on job_ptr == nullptr";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_gather_deflate_statistics, test) {
    qpl_status             status;
    uint8_t                source;
    uint32_t               source_length = 1u;
    qpl_histogram          deflate_histogram{};
    qpl_path_t             path          = qpl_path_software;
    qpl_compression_levels level         = qpl_default_level;

    status = qpl_gather_deflate_statistics(nullptr,
                                           source_length,
                                           &deflate_histogram,
                                           level,
                                           path);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);

    status = qpl_gather_deflate_statistics(&source,
                                           source_length,
                                           nullptr,
                                           level,
                                           path);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);

    /* todo implement check in function
    status = qpl_gather_deflate_statistics(&source,
                                           source_length,
                                           &literals_histogram,
                                           nullptr,
                                           INCORRECT_LEVEL,
                                           path);
    //EXPECT_EQ(status, QPL_STS_LEVEL_ERR);*/

    /* todo implement check in function
    status = qpl_gather_deflate_statistics(&source,
                                           source_length,
                                           &literals_histogram,
                                           nullptr,
                                           level,
                                           INCORRECT_PATH);
    EXPECT_EQ(status, QPL_STS_PATH_ERR);*/
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_build_compression_table_from_statistics, test) {
    qpl_status                    status;
    qpl_histogram                 deflate_histogram{};
    auto table_buffer = std::make_unique<uint8_t[]>(static_cast<uint32_t>(QPL_COMPRESSION_TABLE_SIZE));
    auto huffman_table_ptr = reinterpret_cast<qpl_compression_huffman_table *>(table_buffer.get());

    status = qpl_build_compression_table(&deflate_histogram,
                                         nullptr,
                                         QPL_DEFLATE_REPRESENTATION);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);

    status = qpl_build_compression_table(nullptr,
                                         huffman_table_ptr,
                                         QPL_DEFLATE_REPRESENTATION);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);

    /* todo implement check in function
    status = qpl_build_compression_table(&deflate_histogram,
                                         huffman_table_ptr,
                                         INCORRECT_REPRESENTATION);
    EXPECT_EQ(status != QPL_STS_OK); */
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_compression_to_decompression_table, test) {
    qpl_status                    status;
    auto decompression_table_buffer = std::make_unique<uint8_t[]>(QPL_DECOMPRESSION_TABLE_SIZE);
    auto decompression_table_ptr    = reinterpret_cast<qpl_decompression_huffman_table *>(decompression_table_buffer.get());

    auto compression_table_buffer = std::make_unique<uint8_t[]>(static_cast<uint32_t>(QPL_COMPRESSION_TABLE_SIZE));
    auto compression_table_ptr = reinterpret_cast<qpl_compression_huffman_table *>(compression_table_buffer.get());

    status = qpl_comp_to_decompression_table(nullptr,
                                             decompression_table_ptr,
                                             QPL_DEFLATE_REPRESENTATION);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);

    status = qpl_comp_to_decompression_table(compression_table_ptr,
                                             nullptr,
                                             QPL_DEFLATE_REPRESENTATION);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);

    /* todo implement check in function
    status = qpl_comp_to_decompression_table(compression_table_ptr,
                                             decompression_table_ptr,
                                             INCORRECT_REPRESENTATION);
    EXPECT_EQ(status != QPL_STS_OK); */
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_get_existing_dict_size, test) {
    size_t dictionary_size = 0;

    auto status = qpl_get_existing_dict_size(NULL, &dictionary_size);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);

    std::array<uint8_t, dictionary_test_size> dictionary_buffer{};
    auto *dictionary_ptr = reinterpret_cast<qpl_dictionary *>(dictionary_buffer.data());

    status = qpl_get_existing_dict_size(dictionary_ptr, NULL);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_build_dictionary, test) {
    std::array<uint8_t, dictionary_test_size> dictionary_raw{};

    auto status = qpl_build_dictionary(NULL,
                                       sw_compression_level::SW_NONE,
                                       hw_compression_level::HW_NONE,
                                       dictionary_raw.data(),
                                       dictionary_test_size);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);


    std::array<uint8_t, dictionary_test_size> dictionary_buffer{};
    auto *dictionary_ptr = reinterpret_cast<qpl_dictionary *>(dictionary_buffer.data());


    status = qpl_build_dictionary(dictionary_ptr,
                                  sw_compression_level::SW_NONE,
                                  hw_compression_level::HW_NONE,
                                  NULL,
                                  dictionary_test_size);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_set_dictionary_id, test) {
    auto status = qpl_set_dictionary_id(NULL, dictionary_id_test);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_get_dictionary_id, test) {
    uint32_t dictionary_id = 0;
    auto status = qpl_get_dictionary_id(NULL, &dictionary_id);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);

    std::array<uint8_t, dictionary_test_size> dictionary_buffer{};
    auto *dictionary_ptr = reinterpret_cast<qpl_dictionary *>(dictionary_buffer.data());

    status = qpl_get_dictionary_id(dictionary_ptr, NULL);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);
}
}
