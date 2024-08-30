/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tests
 */

#include <future>
#include <iostream>
#include <thread>
#include <vector>

#include "qpl/qpl.h"

#include "gtest/gtest.h"
#include "tt_common.hpp"

// utils
#include "system_info.hpp"

namespace qpl::test {

// With many threads spawned, some are expected to get QPL_STS_QUEUES_ARE_BUSY_ERR
// We set a predetermined wait time and max resubmit count for threads to wait and
// resubmit their job. This will not make every single thread finish their job;
// though it will allow more threads to complete work that can be tested.

constexpr const uint32_t max_resubmit_cnt = 10;

static uint32_t get_num_cores() {
    return get_sys_info().cpu_physical_cores;
}

int compress_test() {
    // Generate input
    auto& dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();
    auto  source  = dataset.get_data().begin()->second;

    std::vector<uint8_t> compressed(source.size() * 2);
    std::vector<uint8_t> uncompressed(source.size());

    // job structure initialization
    auto       path   = util::TestEnvironment::GetInstance().GetExecutionPath();
    uint32_t   size   = 0;
    qpl_status status = qpl_get_job_size(path, &size);
    if (QPL_STS_OK != status) { return status; }

    std::unique_ptr<uint8_t[]> job_buffer;
    job_buffer = std::make_unique<uint8_t[]>(size);
    auto job   = reinterpret_cast<qpl_job*>(job_buffer.get());

    status = qpl_init_job(path, job);
    if (QPL_STS_OK != status) { return status; }

    // perform compression
    job->op            = qpl_op_compress;
    job->level         = qpl_default_level;
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN | QPL_FLAG_OMIT_VERIFY;
    job->next_in_ptr   = source.data();
    job->next_out_ptr  = compressed.data();
    job->available_in  = source.size();
    job->available_out = static_cast<uint32_t>(compressed.size());

    // If queues are busy, wait then resubmit before moving on
    uint32_t resubmit_cnt = 0;

    const bool is_sync = !util::TestEnvironment::GetInstance().IsAsynchronousApiTesting();
    if (is_sync) {
        status = qpl_execute_job(job);

        while (status == QPL_STS_QUEUES_ARE_BUSY_ERR && resubmit_cnt < max_resubmit_cnt) {
            status = qpl_execute_job(job);
            resubmit_cnt++;
        }
        if (QPL_STS_OK != status) { return status; }
    } else {
        // Async API
        status = qpl_submit_job(job);

        while (status == QPL_STS_QUEUES_ARE_BUSY_ERR && resubmit_cnt < max_resubmit_cnt) {
            status = qpl_submit_job(job);
            resubmit_cnt++;
        }
        if (QPL_STS_OK != status) { return status; }

        status       = qpl_wait_job(job);
        resubmit_cnt = 0;

        while (status == QPL_STS_QUEUES_ARE_BUSY_ERR && resubmit_cnt < max_resubmit_cnt) {
            status = qpl_wait_job(job);
            resubmit_cnt++;
        }

        if (QPL_STS_OK != status) { return status; }
    }

    const uint32_t compressed_size = job->total_out;
    compressed.resize(compressed_size);

    job->op            = qpl_op_decompress;
    job->next_in_ptr   = compressed.data();
    job->next_out_ptr  = uncompressed.data();
    job->available_in  = compressed_size;
    job->available_out = static_cast<uint32_t>(uncompressed.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;

    resubmit_cnt = 0;

    if (is_sync) {
        status = qpl_execute_job(job);

        while (status == QPL_STS_QUEUES_ARE_BUSY_ERR && resubmit_cnt < max_resubmit_cnt) {
            status = qpl_execute_job(job);
            resubmit_cnt++;
        }
        if (QPL_STS_OK != status) { return status; }
    } else {
        status = qpl_submit_job(job);

        while (status == QPL_STS_QUEUES_ARE_BUSY_ERR && resubmit_cnt < max_resubmit_cnt) {
            status = qpl_submit_job(job);
            resubmit_cnt++;
        }
        if (QPL_STS_OK != status) { return status; }

        status       = qpl_wait_job(job);
        resubmit_cnt = 0;

        while (status == QPL_STS_QUEUES_ARE_BUSY_ERR && resubmit_cnt < max_resubmit_cnt) {
            status = qpl_wait_job(job);
            resubmit_cnt++;
        }
        if (QPL_STS_OK != status) { return status; }
    }

    const uint32_t out_len = job->total_out;

    status = qpl_fini_job(job);

    if (QPL_STS_OK != status) {
        return status;
    } else if (out_len != source.size()) {
        return -1;
    } else if (!CompareVectors(source, uncompressed)) {
        return -2;
    }

    return 0;
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(thread_stress_test, default_compression_decompression) {
    bool                          test_passed = true;
    std::vector<std::future<int>> results;

    const uint32_t num_threads = get_num_cores();

    ASSERT_TRUE(num_threads > 0);

    results.reserve(num_threads);

    for (uint32_t i = 0; i < num_threads; i++) {
        results.push_back(std::async(std::launch::async, compress_test));
    }

    std::cout << "Number of threads spawned: " << num_threads << '\n';

    uint32_t num_threads_with_queues_busy = 0;

    for (uint32_t i = 0; i < results.size(); i++) {
        auto ret = results[i].get();

        if (ret == QPL_STS_QUEUES_ARE_BUSY_ERR) { num_threads_with_queues_busy++; }

        if (ret > QPL_STS_OK &&
            ret != QPL_STS_QUEUES_ARE_BUSY_ERR) { // QPL_STS_QUEUES_ARE_BUSY_ERR is expected when running with many cores
            test_passed = false;
            std::cout << "Thread " << i << " returned with error code " << ret << '\n';
        } else if (ret == -1) {
            test_passed = false;
            std::cout << "Thread " << i << " compression and decompression resulted in length mismatch\n";
        } else if (ret == -2) {
            test_passed = false;
            std::cout << "Thread " << i << " compression and decompression resulted in data mismatch\n";
        }
    }
    std::cout << "Number of threads that get QPL_STS_QUEUES_ARE_BUSY_ERR even after resubmission: "
              << num_threads_with_queues_busy << '\n';

    ASSERT_EQ(test_passed, true);
}
} // namespace qpl::test
