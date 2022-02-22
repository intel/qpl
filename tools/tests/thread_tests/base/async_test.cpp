/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include <vector>
#include <string>

#include "gtest/gtest.h"

#include "tt_common.hpp"
#include "qpl/cpp_api/operations/compression/deflate_operation.hpp"
#include "qpl/cpp_api/operations/compression/inflate_operation.hpp"
#include "qpl/cpp_api/chaining/operation_chain.hpp"

#include "source_provider.hpp"
#include "util.hpp"
#include "check_result.hpp"

namespace qpl::test {

QPL_HIGH_LEVEL_API_ALGORITHMIC_TEST(asynchronous_execution, default_callable) {
    constexpr uint32_t source_size      = 111261u;
    constexpr uint32_t number_of_copies = 7u;

    auto &dataset = util::TestEnvironment::GetInstance().GetAlgorithmicDataset();

    std::vector<std::vector<uint8_t>> source(number_of_copies);
    std::vector<uint32_t>             result_size;

    std::vector<qpl::execution_result<uint32_t, qpl::execution_mode::sync>> result_event;

    for (int i = 0; i < number_of_copies; i++) {
        source[i] = dataset.get_data().begin()->second;
    }

    std::vector<std::vector<uint8_t>> destination(number_of_copies, std::vector<uint8_t>(source[0].size()));

    auto chain = qpl::deflate_operation() |
                 qpl::inflate_operation();

    auto path = util::TestEnvironment::GetInstance().GetExecutionPath();
    result_event.reserve(number_of_copies);
    if (qpl_path_hardware == path) {
        for (int i = 0; i < number_of_copies; i++) {
            result_event.push_back(qpl::submit<qpl::hardware>(chain, source[i], destination[i]));
        }
    } else {
        for (int i = 0; i < number_of_copies; i++) {
            result_event.push_back(qpl::submit<qpl::software>(chain, source[i], destination[i]));
        }
    }

    for (int i = 0; i < number_of_copies; i++) {
        uint32_t res_size = 0;
        result_event[i].handle([&res_size](uint32_t value) -> void {
                                   res_size = value;
                               },
                               [](uint32_t status) -> void {
                                   throw std::runtime_error("Error: Status code - " + std::to_string(status));
                               });

        result_size.push_back(res_size);
    }

    for (size_t i = 0; i < result_size.size(); i++) {
        EXPECT_EQ(source[i].size(), result_size[i]);
        EXPECT_TRUE(CompareVectors(source[i], destination[i]));
    }
}

QPL_LOW_LEVEL_API_ALGORITHMIC_TEST(asynchronous_execution, copy) {
    constexpr uint32_t length       = 8000u;
    uint32_t           num_threads  = 8u;
    auto               sources      = std::vector<std::vector<uint8_t>>(num_threads, std::vector<uint8_t>(length, 1u));
    auto               destinations = std::vector<std::vector<uint8_t>>(num_threads, std::vector<uint8_t>(length, 0u));
    auto               threads      = std::vector<std::thread>(num_threads);
    auto               qpl_job_ptrs = std::vector<qpl_job *>(num_threads);

    uint32_t   job_size = 0u;
    qpl_path_t qpl_path = util::TestEnvironment::GetInstance().GetExecutionPath();
    qpl_status status   = QPL_STS_OK;

    status = qpl_get_job_size(qpl_path, &job_size);
    EXPECT_EQ(status, QPL_STS_OK);

    auto job_buffers = std::vector<std::vector<uint8_t>>(num_threads, std::vector<uint8_t>(job_size, 0u));

    // Initialization
    for (uint32_t i = 0u; i < num_threads; ++i) {
        qpl_job_ptrs[i] = reinterpret_cast<qpl_job *>(job_buffers[i].data());
    }

    for (auto i = 0u; i < num_threads; ++i) {
        threads[i] = std::thread([&qpl_job_ptrs, &sources, &destinations, qpl_path, length](uint32_t i) {
            auto t_status = qpl_init_job(qpl_path, qpl_job_ptrs[i]);
            EXPECT_EQ(t_status, QPL_STS_OK);

            qpl_job_ptrs[i]->op            = qpl_op_memcpy;
            qpl_job_ptrs[i]->next_in_ptr   = sources[i].data();
            qpl_job_ptrs[i]->available_in  = length;
            qpl_job_ptrs[i]->next_out_ptr  = destinations[i].data();
            qpl_job_ptrs[i]->available_out = length;

            t_status = qpl_submit_job(qpl_job_ptrs[i]);
            EXPECT_EQ(t_status, QPL_STS_OK);
        }, i);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    for (auto i = 0u; i < num_threads; ++i) {
        qpl_wait_job(qpl_job_ptrs[i]);
    }

    for (size_t i = 0; i < num_threads; i++) {
        EXPECT_TRUE(CompareVectors(sources[i], destinations[i]));
    }
}
} // namespace qpl::test
