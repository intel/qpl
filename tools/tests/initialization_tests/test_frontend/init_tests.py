# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Validates initialization part on correctness
# Test includes:
#   - Algorithmic tests
#   - Negative tests
#       - HW_ACCELERATOR_LIBACCEL_NOT_FOUND
#       - HW_ACCELERATOR_LIBACCEL_ERROR
#       - HW_ACCELERATOR_WORK_QUEUES_NOT_AVAILABLE

import argparse
import os
import subprocess

tests_references = [# Bad arguments
                    (False, "", [], "tb_init"),
                    
                    # HW_ACCELERATOR_STATUS_OK
                    # Need to parse libaccel json to test all possible variations. Currenty only one case tested
                    (True, "setup_devices.sh",      ["all", "all"],     "try_init"),
                    
                    # HW_ACCELERATOR_LIBACCEL_NOT_FOUND
                    # Removes libaccel from search path
                    # Cases:
                    #   Library not found
                    #   Function in library not found - should be treated same as library not found. Backward compatibility is libaccel responsibility
                    (True, "corrupt_devices.sh",    ["no_libaccel"],    "libaccel_not_found"),
                    
                    # HW_ACCELERATOR_WORK_QUEUES_NOT_AVAILABLE
                    # Cases:
                    #   No supported devices
                    #   Wrong device version 
                    #   Disabled devices
                    #   Disabled WQs
                    #   Non shared WQs
                    #   Cannot map device
                    #   Device id fail
                    (True, "corrupt_devices.sh",    ["no_devices"],     "work_queues_not_available"),   # Disabled devices
                    (True, "corrupt_devices.sh",    ["no_wqs"],         "work_queues_not_available"),   # Disabled WQs
                    (True, "corrupt_devices.sh",    ["no_access"],      "work_queues_not_available"),   # Cannot map device
                    
                    # HW_ACCELERATOR_LIBACCEL_ERROR
                    # Need to fault-inject libaccel: accfg_new
                    # Cases:
                    #   Driver context error
                    (True, "corrupt_devices.sh",    ["fault_accfg_new"], "instance_not_found")
]

def run_cmd(cmd, args=[''], is_root=False):
    cmd = ['sudo', cmd] + args if is_root else [cmd] + args
    p = subprocess.Popen(cmd)
    return p.wait()


def run_test(test_path, test_name, is_root=False):
    return run_cmd(os.path.join(test_path, "init_tests"), ["--path=hw", "--gtest_filter=*" + test_name + "*"], is_root)


if __name__ == "__main__":
    print("Test initialization...")

    parser = argparse.ArgumentParser(description='Tests initialization on correctness.')
    parser.add_argument('--test_path', '-t', default="./",
                        help='path to tests')
    parser.add_argument('--script_path', '-s', default="./scripts",
                        help='path to scripts')

    args = parser.parse_args()

    script_path = args.script_path
    test_path = args.test_path
    failed_tests = 0

    run_cmd(cmd=os.path.join(script_path, "setup_devices.sh"), args=["prolog"], is_root=True)

    print("Run tests...")
    for as_root, script, args, test in tests_references:
        if script:
            run_cmd(cmd=os.path.join(script_path, script),
                    args=args,
                    is_root=as_root)

        result = run_test(test_path=test_path,
                          test_name=test)

        failed_tests += 1 if result else 0

    run_cmd(cmd=os.path.join(script_path, "setup_devices.sh"), args=["epilog"], is_root=True)

    print(" ".join(["Failed Tests:",
                    str(failed_tests),
                    "of",
                    str(len(tests_references))]))

    exit(failed_tests)
