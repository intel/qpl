/*******************************************************************************
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#include "gtest/gtest.h"
#include "qpl_test_environment.hpp"

#include "command_line.hpp"
#include "arguments_list.hpp"

#if defined(__linux__)
#include <sys/types.h>
#include <unistd.h>

// tests_common
#include "execution_wrapper.hpp"
#endif

namespace qpl::test {

static inline void show_help() {
    std::cout << "\nQPL test system arguments:" << std::endl;
    std::cout << "  " << QPL_ARG_PATH << "=(sw|hw)" << std::endl;
    std::cout << "     " << "Set execution path for functional tests. The default is software path." << std::endl;
    std::cout << "  " << QPL_ARG_ASYNC << "=(on|off)" << std::endl;
    std::cout << "     " << "Execute tests using asynchronous mode. The default is off (synchronous mode)." << std::endl;
    std::cout << "  " << QPL_ARG_SEED << "=[NUMBER]" << std::endl;
    std::cout << "     " << "Random number seed to use for generating some testing data. The default is based on std::chrono value." << std::endl;
    std::cout << "  " << QPL_ARG_DATASET_PATH << "=[PATH]" << std::endl;
    std::cout << "     " << "Path to folder containing dataset." << std::endl;
    std::cout << "  " << QPL_ARG_TEST_CASE_ID << "=[NUMBER]" << std::endl;
    std::cout << "     " << "Set test case id for this test." << std::endl;
}

static inline auto parse_execution_path_argument(std::string &value) -> qpl_path_t {
    qpl_path_t execution_path;

    if (value == "sw") {
        execution_path = qpl_path_software;
    } else {
        if (value == "hw") {
            execution_path = qpl_path_hardware;
        } else {
            throw std::runtime_error("Invalid usage of --path argument\n");
        }
    }

    return execution_path;
}

static inline util::arguments_list_t get_testing_settings(int argc, char *argv[]) {
    util::arguments_list_t arguments_list;

    try {
        qpl::test::CommandLine cmd(argv, argc);

        auto execution_path = cmd.find<std::string>(QPL_ARG_PATH);
        if (execution_path.has_value()) {
            arguments_list.execution_path = parse_execution_path_argument(execution_path.value());
        } else {
            arguments_list.execution_path = qpl_path_software;
        }

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

/*
 * qpl_hw_compress and test_init_with_fork work together as a check for HW dispatcher initialization with multiprocessing
 * It verifies that the HW dispatcher is properly initialized in a forked child process
 * This check needs to be run before the first job submission
 */
#if defined(__linux__)
qpl_status qpl_hw_compress() {
    qpl_path_t execution_path = qpl_path_hardware;
    uint32_t job_size         = 0;

    qpl_status status = qpl_get_job_size(execution_path, &job_size);
    if (QPL_STS_OK != status) return status;

    // Allocate buffers for compression job
    auto job_buffer = std::make_unique<uint8_t[]>(job_size);
    auto job        = reinterpret_cast<qpl_job *>(job_buffer.get());

    // Initialize compression job
    status = qpl_init_job(execution_path, job);
    if (QPL_STS_OK != status) return status;

    char source_buf[256] = "The Intel® Query Processing Library (Intel® QPL) is an open-source library to provide high-performance query processing operations on Intel CPUs.";
    char dest_buf[256]   = {0};

    job->op            = qpl_op_compress;
    job->level         = qpl_default_level;
    job->next_in_ptr   = (unsigned char*)source_buf;
    job->next_out_ptr  = (unsigned char*)dest_buf;
    job->available_in  = 256;
    job->available_out = 256;
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_DYNAMIC_HUFFMAN | QPL_FLAG_LAST | QPL_FLAG_OMIT_VERIFY;

    status = qpl::test::run_job_api(job);

    qpl_fini_job(job);
    return status;
}

int test_init_with_fork() {
    qpl_status status = QPL_STS_OK;

    // create a child process with fork()
    pid_t pid;
    pid = fork();

    if (pid < 0) {
        std::cout << "Failed to fork a process. " << std::endl;
        return 1;
    } else if (pid == 0) {
        // calling qpl_compress in child process
        status = qpl_hw_compress();
        exit(0);
    } else if (pid > 0) {
        // calling qpl_compress in parent process
        status = qpl_hw_compress();

        // wait for child process to finish
        int child_status;
        int ret = waitpid(pid, &child_status, 0);

        if (ret != pid) {
            std::cout << "Failed to wait for child process to finish. " << std::endl;
            return 2;
        }
        if (!WIFEXITED(child_status)) {
            std::cout << "Child process did not terminate normally. " << std::endl;
            return 3;
        }
    }
    return 0;
}
#endif


int main(int argc, char *argv[]) {

    std::vector<std::string> arguments(argv + 1, argv + argc);
    if (std::find(begin(arguments), end(arguments), QPL_ARG_HELP) != end(arguments)) {
        qpl::test::show_help();
        return 0;
    }

    testing::InitGoogleTest(&argc, argv);

    auto arguments_list = qpl::test::get_testing_settings(argc, argv);

    using environment = qpl::test::util::TestEnvironment;

    environment::GetInstance().Initialize(arguments_list);

    int init_with_fork_status = 0;
#if defined(__linux__)
    auto execution_path = environment::GetInstance().GetExecutionPath();
    if (execution_path == qpl_path_hardware) {
        std::cout << "Running HW dispatcher initialization check with multiprocessing " << std::endl;
        init_with_fork_status = test_init_with_fork();
        EXPECT_TRUE(init_with_fork_status == 0) << "Hardware dispatcher initialization with fork() failed. ";
        std::cout << "Finished running HW dispatcher initialization check. " << std::endl;
    }
#endif

    std::cout << "Tests seed = " << environment::GetInstance().GetSeed() << std::endl;

    int status = RUN_ALL_TESTS();

    if (init_with_fork_status) std::cout << "Hardware dispatcher initialization with fork() failed. " << std::endl;
    std::cout << "Tests seed = " << environment::GetInstance().GetSeed() << std::endl;

    int final_status = status | init_with_fork_status;
    return final_status;
}
