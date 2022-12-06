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

#include "compressor_stress_test.hpp"
#include "qpl/qpl.h"

namespace qpl::test {

static int32_t verbose_level = 0;

namespace details {

struct data_property_t {
    uint32_t min_block_length = 100;
    uint32_t max_block_length = 16384;
};

static auto create_block(size_t length, int seed = 0) {
    std::vector<uint8_t> data_block(length);
    auto                 symbol = 0u;

    auto next_symbol = [&symbol, &seed]() -> auto {
        return 'a' + ((symbol++ + seed) % 20);
    };

    std::generate(data_block.begin(), data_block.end(), next_symbol);

    return data_block;
}

int test(uint32_t in_len) {
    // Generate input
    auto                 source = create_block(in_len, rand() % 26);
    std::vector<uint8_t> compressed(source.size() + 10u);
    std::vector<uint8_t> uncompressed(source.size());

    if (verbose_level >= 1) {
        std::cout << std::this_thread::get_id() << "source length: " << source.size() << std::endl;
    }

    qpl_status status;

    // job structure initialization
    uint32_t size = 0;
    status = qpl_get_job_size(qpl_path_hardware, &size);
    if (QPL_STS_OK != status) {
        std::cout << std::this_thread::get_id() << "qpl_get_job_size sts: " << status << std::endl;
    }

    std::unique_ptr<uint8_t[]> job_buffer;
    job_buffer = std::make_unique<uint8_t[]>(size);
    auto job   = reinterpret_cast<qpl_job *>(job_buffer.get());

    status = qpl_init_job(qpl_path_hardware, job);
    if (QPL_STS_OK != status) {
        std::cout << std::this_thread::get_id() << "qpl_init_job sts: " << status << std::endl;
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
        std::cout << std::this_thread::get_id() << "qpl_execute_job sts: " << status << std::endl;
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
        std::cout << std::this_thread::get_id() << "qpl_execute_job sts: " << status << std::endl;
    }

    uint32_t out_len = job->total_out;
    if (verbose_level >= 1 || QPL_STS_OK != status) {
        std::cout << std::this_thread::get_id() << "uncompressed length: " << out_len << std::endl;
    }

    status = qpl_fini_job(job);
    if (QPL_STS_OK != status) {
        std::cout << std::this_thread::get_id() << "qpl_fini_job sts: " << status << std::endl;
    }

    if (QPL_STS_OK != status) {
        return 1;
    } else if (out_len != in_len) {
        std::cout << std::this_thread::get_id() << "length mismatch" << std::endl;
        return 1;
    } else if (source != uncompressed) {
        std::cout << std::this_thread::get_id() << "data mismatch" << std::endl;
        return 1;
    }

    return 0;
}

int runThroughIterations(uint32_t iterations, data_property_t data_property) {
    std::cout << "Thread id: " << std::this_thread::get_id() << "\n";
    decltype(test(0u)) result{};

    for (uint32_t i = 0; i < iterations; i++) {
        auto in_len = rand() % (data_property.max_block_length - data_property.min_block_length)
                            + data_property.min_block_length;

        result = test(in_len);

        if (result != 0) {
            return result;
        }
    }
    return result;
}

}

CompressorStressTest::CompressorStressTest(int iterations, int thread_count)
        : iterations_per_thread_(iterations), thread_count_(thread_count) {
    // No action required
}

void CompressorStressTest::set_min_block_length(uint32_t length) {
    min_block_length_ = length;
}

void CompressorStressTest::set_max_block_length(uint32_t length) {
    max_block_length_ = length;
}

void CompressorStressTest::run() {
    bool                          test_passed = true;
    std::vector<std::future<int>> results;

    details::data_property_t data_property = {min_block_length_, max_block_length_};

    results.reserve(thread_count_ - 1);

    for (uint32_t i = 0; i < thread_count_ - 1; i++) {
        results.push_back(std::async(details::runThroughIterations, iterations_per_thread_, data_property));
    }

    if (details::runThroughIterations(iterations_per_thread_, data_property) != 0) {
        test_passed = false;
    }

    for (auto &result: results) {
        auto ret = result.get();
        if (ret != 0) {
            test_passed = false;
        }
    }

    std::cout << ((test_passed) ? "PASSED" : "FAILED") << std::endl;
}
}
