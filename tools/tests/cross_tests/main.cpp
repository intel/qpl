/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"

#include "command_line.hpp"
#include "arguments_list.hpp"

namespace qpl::test {

static inline void show_help() {
    std::cout << "\nQPL test system arguments:" << std::endl;
    std::cout << "  " << QPL_ARG_SEED << "=(value)" << std::endl;
    std::cout << "     " << "Set seed value for tests" << std::endl;
    std::cout << "  " << QPL_ARG_DATASET_PATH << "=(path)" << std::endl;
    std::cout << "     " << "Specify path to folder containing dataset" << std::endl;
    std::cout << "  " << QPL_ARG_TEST_CASE_ID << "=(value)" << std::endl;
    std::cout << "     " << "Specify test case id for this test" << std::endl;
}

static inline util::arguments_list_t get_testing_settings(int argc, char *argv[]) {
    util::arguments_list_t arguments_list;

    try {
        qpl::test::CommandLine cmd(argv, argc);

        auto dataset_path = cmd.find<std::string>(QPL_ARG_DATASET_PATH);
        arguments_list.path_to_dataset = dataset_path.value_or("");

        auto is_async_testing = cmd.find<bool>(QPL_ARG_ASYNC);
        arguments_list.is_async_testing = is_async_testing.value_or(false);

        auto seed = cmd.find<uint32_t>(QPL_ARG_SEED);
        arguments_list.seed = seed.value_or(std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());

        auto case_id = cmd.find<uint32_t>(QPL_ARG_TEST_CASE_ID);
        if (case_id.has_value()) {
            arguments_list.test_case_id         = case_id.value();
            arguments_list.is_test_case_id_used = true;
        } else {
            arguments_list.is_test_case_id_used = false;
        }
    } catch (std::runtime_error &err) {
        std::cout << err.what() << "\n";
        exit(1);
    }

    return arguments_list;
}

}

int main(int argc, char *argv[]) { //NOLINT(bugprone-exception-escape)
    testing::InitGoogleTest(&argc, argv);

    auto arguments_list = qpl::test::get_testing_settings(argc, argv);

    using environment = qpl::test::util::TestEnvironment;

    environment::GetInstance().Initialize(arguments_list);

    std::cout << "Tests seed = " << environment::GetInstance().GetSeed() << std::endl;

    const int status = RUN_ALL_TESTS();

    std::cout << "Tests seed = " << environment::GetInstance().GetSeed() << std::endl;

    return status;
}
