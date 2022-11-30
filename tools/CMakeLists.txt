# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Intel® Query Processing Library (Intel® QPL)
# Build system

project(tests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

if (LIB_FUZZING_ENGINE)
    add_subdirectory(tests/fuzzing)
else ()
    add_subdirectory(ref)
    add_subdirectory(utils)
    add_subdirectory(third-party/google-test EXCLUDE_FROM_ALL)
    add_subdirectory(tests)
    if (UNIX)
        # Disable building google-benchmarks tests
        SET(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
        add_subdirectory(third-party/benchmark EXCLUDE_FROM_ALL)
        add_subdirectory(benchmarks)
    endif ()
endif ()

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/configs
                  TYPE             BIN
                  COMPONENT        RUNTIME)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/scripts
                  TYPE             BIN
                  COMPONENT        RUNTIME
                  FILE_PERMISSIONS USE_SOURCE_PERMISSIONS)
