/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tests
 */

#include <algorithm>
#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <mutex>
#include <string>
#include <fstream>
#include <iostream>
#include <utility>

#include "gtest/gtest.h"
#include "qpl/qpl.h"
#include "check_result.hpp"
#include "../tt_common.hpp"

namespace qpl::test {

static void trim(std::string &str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char val) { return !std::isspace(val); }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char val) { return !std::isspace(val); }).base(), str.end());
}

static uint32_t get_num_cores() {

    static bool is_setup{false};
    static std::mutex guard;

    uint32_t cpu_sockets             = 1U;
    uint32_t cpu_physical_per_socket = 1U;
    uint32_t cpu_physical_cores      = 1U;

    guard.lock();
    if(!is_setup)
    {
#if defined( __linux__ )
        
        std::ifstream info_file("/proc/cpuinfo");
        if(!info_file.is_open()){
            guard.unlock();
            throw std::runtime_error("Failed to open /proc/cpuinfo");
        }

        std::string line;
        while (std::getline(info_file, line))
        {
            if (line.empty())
                continue;
            auto del_index = line.find(':');
            if(del_index == std::string::npos)
                continue;

            auto key = line.substr(0, del_index);
            auto val = line.substr(del_index+1);
            
            trim(key);
            trim(val);

            if(key == "physical id")
                cpu_sockets = std::max(cpu_sockets, (std::uint32_t)atoi(val.c_str())+1);
            else if(key == "cpu cores")
                cpu_physical_per_socket = std::max(cpu_physical_per_socket, (std::uint32_t)atoi(val.c_str()));
        }
        cpu_physical_cores = cpu_physical_per_socket*cpu_sockets;

        // Print num cores for debugging purposes
        printf("Physical Cores:   %d\n", cpu_physical_cores);
#endif
        is_setup = true;
    }
    guard.unlock();

    return cpu_physical_cores;
}

int compress_test() {
    // Generate input
    auto &dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();
    auto source = dataset.get_data().begin()->second;

    std::vector<uint8_t> compressed(source.size() * 2);
    std::vector<uint8_t> uncompressed(source.size());

    qpl_status status;

    // job structure initialization
    auto path = util::TestEnvironment::GetInstance().GetExecutionPath();
    uint32_t size = 0;
    status = qpl_get_job_size(path, &size);
    if (QPL_STS_OK != status) {
        return status;
    }

    std::unique_ptr<uint8_t[]> job_buffer;
    job_buffer = std::make_unique<uint8_t[]>(size);
    auto job   = reinterpret_cast<qpl_job *>(job_buffer.get());

    status = qpl_init_job(path, job);
    if (QPL_STS_OK != status) {
        return status;
    }

    // perform compression
    job->op            = qpl_op_compress;
    job->level         = qpl_default_level;
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN | QPL_FLAG_OMIT_VERIFY;
    job->next_in_ptr   = source.data();
    job->next_out_ptr  = compressed.data();
    job->available_in  = source.size();
    job->available_out = static_cast<uint32_t>(compressed.size());

    status = qpl_execute_job(job);
    if (QPL_STS_OK != status) {
        return status;
    }

    const uint32_t compressed_size = job->total_out;
    compressed.resize(compressed_size);

    job->op            = qpl_op_decompress;
    job->next_in_ptr   = compressed.data();
    job->next_out_ptr  = uncompressed.data();
    job->available_in  = compressed_size;
    job->available_out = static_cast<uint32_t>(uncompressed.size());
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;

    status = qpl_execute_job(job);
    if (QPL_STS_OK != status) {
        return status;
    }

    uint32_t out_len = job->total_out;

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

    uint32_t num_threads = get_num_cores();

    ASSERT_TRUE(num_threads > 0);

    results.reserve(num_threads);

    for (uint32_t i = 0; i < num_threads; i++) {
        results.push_back(std::async(std::launch::async, compress_test));
    }

    std::cout << "Number of threads spawned: " << num_threads << std::endl;
    for (uint32_t i = 0; i < results.size(); i++) {
        auto ret = results[i].get();
        if (ret > QPL_STS_OK && ret != QPL_STS_QUEUES_ARE_BUSY_ERR) { // QPL_STS_QUEUES_ARE_BUSY_ERR is expected when running with many cores
            test_passed = false;
            std::cout << "Thread " << i << " returned with error code " << ret << std::endl;
        } else if (ret == -1) {
            test_passed = false;
            std::cout << "Thread " << i << " compression and decompression resulted in length mismatch" << std::endl;
        } else if (ret == -2) {
            test_passed = false;
            std::cout << "Thread " << i << " compression and decompression resulted in data mismatch" << std::endl;
        }
    }

    ASSERT_EQ(test_passed, true);
}
} // namespace qpl::test
