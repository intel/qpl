/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <string>
#include <vector>

#include "qpl/qpl.h"

#include "gtest/gtest.h"
#include "source_provider.hpp"
#include "ta_ll_common.hpp"
#include "util.hpp"

/* This test validates the library's asynchronous behavior
   It generates a set number of jobs to submit (currently 7),
   then submits them all in a batch. It then waits on each job
   after all submits are finished, then verifies the correctness of each
   job's output. */

namespace qpl::test {

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(async_multiple_jobs_submit, default_compression_decompression) {
    constexpr uint32_t source_size      = 111261U;
    constexpr uint32_t number_of_copies = 7U;

    auto& dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();
    auto  path    = util::TestEnvironment::GetInstance().GetExecutionPath();

    std::vector<std::vector<uint8_t>> source(number_of_copies);
    for (int i = 0; i < number_of_copies; i++) {
        source[i] = dataset.get_data().begin()->second;
    }

    std::vector<std::vector<uint8_t>> destination(number_of_copies, std::vector<uint8_t>(source[0].size() * 2));
    std::vector<std::vector<uint8_t>> uncompressed(source);

    qpl_status status = QPL_STS_OK;
    uint32_t   size   = 0;

    status = qpl_get_job_size(path, &size);
    ASSERT_EQ(QPL_STS_OK, status);

    std::vector<std::unique_ptr<uint8_t[]>> job_buffer(number_of_copies);
    std::vector<qpl_job*>                   job(number_of_copies);

    for (int i = 0; i < number_of_copies; i++) {
        job_buffer[i] = std::make_unique<uint8_t[]>(size);
        job[i]        = reinterpret_cast<qpl_job*>(job_buffer[i].get());

        status = qpl_init_job(path, job[i]);
        ASSERT_EQ(QPL_STS_OK, status);

        // compression
        job[i]->op            = qpl_op_compress;
        job[i]->level         = qpl_default_level;
        job[i]->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN | QPL_FLAG_OMIT_VERIFY;
        job[i]->next_in_ptr   = source[i].data();
        job[i]->available_in  = source[i].size();
        job[i]->next_out_ptr  = destination[i].data();
        job[i]->available_out = static_cast<uint32_t>(destination[i].size());
        status                = qpl_submit_job(job[i]);
        ASSERT_EQ(QPL_STS_OK, status);
    }

    for (int i = 0; i < number_of_copies; i++) {
        status = qpl_wait_job(job[i]);
        ASSERT_EQ(QPL_STS_OK, status);
    }

    for (int i = 0; i < number_of_copies; i++) {
        destination[i].resize(job[i]->total_out);

        // decompression
        job[i]->op            = qpl_op_decompress;
        job[i]->next_in_ptr   = destination[i].data();
        job[i]->available_in  = destination[i].size();
        job[i]->next_out_ptr  = uncompressed[i].data();
        job[i]->available_out = static_cast<uint32_t>(uncompressed[i].size());
        job[i]->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;
        status                = qpl_submit_job(job[i]);
        ASSERT_EQ(QPL_STS_OK, status);
    }

    for (int i = 0; i < number_of_copies; i++) {
        status = qpl_wait_job(job[i]);
        ASSERT_EQ(QPL_STS_OK, status);

        uncompressed[i].resize(job[i]->total_out);

        status = qpl_fini_job(job[i]);
        ASSERT_EQ(QPL_STS_OK, status);
    }

    for (size_t i = 0; i < number_of_copies; i++) {
        EXPECT_EQ(source[i].size(), uncompressed[i].size());
        EXPECT_TRUE(CompareVectors(source[i], uncompressed[i]));
    }
}
} // namespace qpl::test
