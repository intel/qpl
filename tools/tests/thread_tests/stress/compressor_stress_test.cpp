/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tests
 */

#include "qpl/cpp_api/operations/compression/deflate_operation.hpp"
#include "qpl/cpp_api/operations/compression/inflate_operation.hpp"
#include "iostream"
#include "compressor_stress_test.hpp"

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

int test(uint32_t input_length) {
    // Generate input

    auto                 source = create_block(input_length, rand() % 26);
    std::vector<uint8_t> compressed(source.size() + 10u);
    std::vector<uint8_t> uncompressed(source.size());

    if (verbose_level >= 1) {
        std::cout << std::this_thread::get_id() << " Source length: " << source.size() << "\n";
    }


    auto deflate_operation = qpl::deflate_operation();
    auto result            = qpl::execute<qpl::hardware>(deflate_operation, source, compressed);

    uint32_t output_len  = 0;
    uint32_t status_code = 0;
    result.handle([&output_len](uint32_t value) -> void {
        output_len = value;
    }, [&status_code](uint32_t status) -> void {
        status_code = status;
    });
    if (verbose_level >= 1 || status_code != 0) {
        std::cout << std::this_thread::get_id() << " Compress status: " << status_code << "\n";
        std::cout << std::this_thread::get_id() << " Compressed length: " << output_len << "\n";
    }

    if (status_code != 0) {
        return 1;
    }

    compressed.resize(output_len);

    auto inflate_operation = qpl::inflate_operation();
    result = qpl::execute<qpl::hardware>(inflate_operation, compressed, uncompressed);

    status_code = 0;
    result.handle([&output_len](uint32_t value) -> void {
        output_len = value;
    }, [&status_code](uint32_t status) -> void {
        status_code = status;
    });

    if (verbose_level >= 1 || status_code != 0) {
        std::cout << std::this_thread::get_id() << " Uncompress status: " << status_code << "\n";
        std::cout << std::this_thread::get_id() << " Uncompressed length: " << output_len << "\n";
    }

    if (status_code != 0) {
        return 1;
    } else if (output_len != input_length) {
        std::cout << std::this_thread::get_id() << " Length mismatch\n";
        return 1;
    } else if (source != uncompressed) {
        std::cout << std::this_thread::get_id() << " Data mismatch\n";
        return 1;
    }

    return 0;
}

int runThroughIterations(uint32_t iterations, data_property_t data_property) {
    std::cout << "Thread id: " << std::this_thread::get_id() << "\n";
    decltype(test(0u)) result{};

    for (uint32_t i = 0; i < iterations; i++) {
        auto input_length = rand() % (data_property.max_block_length - data_property.min_block_length)
                            + data_property.min_block_length;

        result = test(input_length);

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
