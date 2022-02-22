# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Variables
set(QPL_DEFAULT_TEST_PARAMETERS -B -u -G -o -r -p ../tools/testdata)

enable_testing()

add_test(NAME deflate_tests
         COMMAND tests ${QPL_DEFAULT_TEST_PARAMETERS} -f qpl_deflate)
add_test(NAME inflate_tests
         COMMAND tests ${QPL_DEFAULT_TEST_PARAMETERS} -f qpl_inflate)
