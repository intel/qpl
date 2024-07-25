/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <memory>

#include "gtest/gtest.h"
#include "tb_ll_common.hpp"

// tests_common
#include "operation_test.hpp"

namespace qpl::test {

constexpr uint32_t dictionary_test_size = 10U;
constexpr uint32_t dictionary_id_test   = 1U;

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_get_job_size, test) {
    uint32_t size = 0U;

    qpl_status status = qpl_get_job_size(PATH, nullptr);

    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on job_ptr == nullptr";

    status = qpl_get_job_size(INCORRECT_PATH, &size);

    EXPECT_EQ(status, QPL_STS_PATH_ERR) << "Failed on incorrect path check";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_init, test) {
    qpl_status status = qpl_init_job(PATH, nullptr);

    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on job_ptr == nullptr";

    job_ptr->op = qpl_op_crc64;
    status      = qpl_init_job(INCORRECT_PATH, job_ptr);

    EXPECT_EQ(status, QPL_STS_PATH_ERR) << "Failed on incorrect path check";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_submit, test) {
    uint8_t* stored_ptr = nullptr;

    job_ptr->op          = qpl_op_crc64;
    job_ptr->next_in_ptr = (uint8_t*)job_ptr;

    qpl_status status = qpl_submit_job(nullptr);

    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on job_ptr == nullptr";

    // Save corrupted pointer
    stored_ptr = job_ptr->data_ptr.analytics_state_ptr;

    // Check analytics_state_ptr
    job_ptr->data_ptr.analytics_state_ptr = nullptr;
    status                                = qpl_submit_job(job_ptr);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on analytics_state_ptr == nullptr";

    // Restore corrupted pointer and save next
    job_ptr->data_ptr.analytics_state_ptr = stored_ptr;
    stored_ptr                            = job_ptr->data_ptr.compress_state_ptr;

    // Check compress_state_ptr
    job_ptr->data_ptr.compress_state_ptr = nullptr;
    status                               = qpl_submit_job(job_ptr);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on compress_state_ptr == nullptr";

    // Restore corrupted pointer and save next
    job_ptr->data_ptr.compress_state_ptr = stored_ptr;
    stored_ptr                           = job_ptr->data_ptr.decompress_state_ptr;

    // Check decompress_state_ptr
    job_ptr->data_ptr.decompress_state_ptr = nullptr;
    status                                 = qpl_submit_job(job_ptr);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on decompress_state_ptr == nullptr";

    // Restore corrupted pointer and save next
    job_ptr->data_ptr.decompress_state_ptr = stored_ptr;
    stored_ptr                             = job_ptr->data_ptr.hw_state_ptr;

    job_ptr->data_ptr.hw_state_ptr = nullptr;
    status                         = qpl_submit_job(job_ptr);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR) << "Failed on hw_state_ptr == nullptr";

    // Restore corrupted pointer
    job_ptr->data_ptr.hw_state_ptr = stored_ptr;

    qpl::test::random random_opcode(0, RESERVED_OPCODES_COUNT - 1, TestEnviroment::GetInstance().GetSeed());

    auto opcode_index = static_cast<uint8_t>(random_opcode);

    job_ptr->op = (qpl_operation)(reserved_op_codes[opcode_index]);
    status      = qpl_submit_job(job_ptr);
    EXPECT_EQ(status, QPL_STS_OPERATION_ERR) << "Failed on opcode test";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_execute, test) {
    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_execute_job(nullptr)) << "Failed on job_ptr == nullptr";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_wait, test) {
    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_wait_job(nullptr)) << "Failed on job_ptr == nullptr";

    if (qpl_path_hardware == job_ptr->data_ptr.path) {
        // re-init the job, and wait for unsubmitted job
        qpl_status status = QPL_STS_OK;
        status            = qpl_init_job(qpl_path_hardware, job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        job_ptr->op          = qpl_op_crc64;
        job_ptr->next_in_ptr = (uint8_t*)job_ptr;
        EXPECT_EQ(QPL_STS_JOB_NOT_SUBMITTED, qpl_wait_job(job_ptr)) << "Failed on waiting unsubmitted job";
    }
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_check, test) {
    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_check_job(nullptr)) << "Failed on job_ptr == nullptr";

    if (qpl_path_hardware == job_ptr->data_ptr.path) {
        // re-init the job, and check for unsubmitted job
        qpl_status status = QPL_STS_OK;
        status            = qpl_init_job(qpl_path_hardware, job_ptr);
        ASSERT_EQ(QPL_STS_OK, status);

        job_ptr->op          = qpl_op_crc64;
        job_ptr->next_in_ptr = (uint8_t*)job_ptr;
        EXPECT_EQ(QPL_STS_JOB_NOT_SUBMITTED, qpl_check_job(job_ptr)) << "Failed on checking unsubmitted job";
    }
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_finalize, test) {
    EXPECT_EQ(QPL_STS_NULL_PTR_ERR, qpl_fini_job(nullptr)) << "Failed on job_ptr == nullptr";
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_gather_deflate_statistics, test) {
    uint8_t                      source        = 0U;
    const uint32_t               source_length = 1U;
    qpl_histogram                deflate_histogram {};
    const qpl_path_t             path  = qpl_path_software;
    const qpl_compression_levels level = qpl_default_level;

    qpl_status status = qpl_gather_deflate_statistics(nullptr, source_length, &deflate_histogram, level, path);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);

    status = qpl_gather_deflate_statistics(&source, source_length, nullptr, level, path);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);

    status = qpl_gather_deflate_statistics(&source, source_length, &deflate_histogram, INCORRECT_LEVEL, path);
    EXPECT_EQ(status, QPL_STS_UNSUPPORTED_COMPRESSION_LEVEL);

    status = qpl_gather_deflate_statistics(&source, source_length, &deflate_histogram, level, INCORRECT_PATH);
    EXPECT_EQ(status, QPL_STS_PATH_ERR);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_get_existing_dict_size, test) {
    size_t dictionary_size = 0U;

    auto status = qpl_get_existing_dict_size(NULL, &dictionary_size);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);

    std::array<uint8_t, dictionary_test_size> dictionary_buffer {};
    auto* dictionary_ptr = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.data());

    status = qpl_get_existing_dict_size(dictionary_ptr, NULL);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_build_dictionary, test) {
    std::array<uint8_t, dictionary_test_size> dictionary_raw {};

    auto status = qpl_build_dictionary(NULL, sw_compression_level::SW_NONE, hw_compression_level::HW_NONE,
                                       dictionary_raw.data(), dictionary_test_size);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);

    std::array<uint8_t, dictionary_test_size> dictionary_buffer {};
    auto* dictionary_ptr = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.data());

    status = qpl_build_dictionary(dictionary_ptr, sw_compression_level::SW_NONE, hw_compression_level::HW_NONE, NULL,
                                  dictionary_test_size);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_set_dictionary_id, test) {
    auto status = qpl_set_dictionary_id(NULL, dictionary_id_test);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);
}

QPL_LOW_LEVEL_API_BAD_ARGUMENT_TEST(qpl_get_dictionary_id, test) {
    uint32_t dictionary_id = 0U;
    auto     status        = qpl_get_dictionary_id(NULL, &dictionary_id);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);

    std::array<uint8_t, dictionary_test_size> dictionary_buffer {};
    auto* dictionary_ptr = reinterpret_cast<qpl_dictionary*>(dictionary_buffer.data());

    status = qpl_get_dictionary_id(dictionary_ptr, NULL);
    EXPECT_EQ(status, QPL_STS_NULL_PTR_ERR);
}
} // namespace qpl::test
