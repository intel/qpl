/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 *  Intel® Query Processing Library (Intel® QPL)
 *  Tests
 */

#include "command_line.hpp"
#include "compressor_stress_test.hpp"
#include "iostream"

namespace qpl::test {

struct general_test_settings {
    int      verbose_level = 0u;
    uint32_t seed          = 0u;
};

struct tread_test_settings : public general_test_settings {
    int iterations       = 1;
    int num_threads      = 1;
    int min_block_length = 100u;
    int max_block_length = 16384u;
};

tread_test_settings get_testing_settings(int argc, char *argv[]) {
    tread_test_settings settings;

    try {
        qpl::test::CommandLine cmd(argv, argc);
        auto                   arg = cmd.find<int>("--iterations");
        settings.iterations = arg.value_or(1u);

        arg = cmd.find<int>("--treads");
        settings.num_threads = arg.value_or(1u);

        arg = cmd.find<int>("--verbose_level");
        settings.verbose_level = arg.value_or(0u);

        arg = cmd.find<int>("--min_block_length");
        settings.min_block_length = arg.value_or(100);

        arg = cmd.find<int>("--max_block_length");
        settings.max_block_length = arg.value_or(16384);
    } catch (std::runtime_error &err) {
        std::cout << err.what() << "\n";
        exit(1);
    }

    return settings;
}

}

int main(int argc, char *argv[]) {
    auto settings = qpl::test::get_testing_settings(argc, argv);

    qpl::test::CompressorStressTest test(settings.iterations, settings.num_threads);
    test.set_min_block_length(settings.min_block_length);
    test.set_max_block_length(settings.max_block_length);

    test.run();

    return 0;
}
