/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tests
 */

#ifndef QPL_TESTS_THREAD_TESTS_STRESS_COMPRESSOR_STRESS_TEST_HPP_
#define QPL_TESTS_THREAD_TESTS_STRESS_COMPRESSOR_STRESS_TEST_HPP_

namespace qpl::test {
class CompressorStressTest {
public:
    explicit CompressorStressTest(int iterations, int thread_count = 1u);

    void set_min_block_length(uint32_t length);

    void set_max_block_length(uint32_t length);

    void run();

private:
    uint32_t iterations_per_thread_ = 1u;
    uint32_t thread_count_          = 1u;
    uint32_t min_block_length_      = 100u;
    uint32_t max_block_length_      = 16384u;
};
}

#endif //QPL_TESTS_THREAD_TESTS_STRESS_COMPRESSOR_STRESS_TEST_HPP_
