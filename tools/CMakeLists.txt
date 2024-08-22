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
endif()

if (QPL_BUILD_TESTS)
    add_subdirectory(ref)
    add_subdirectory(utils)
    set(TEMP_CMAKE_CXX_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY})
    set(CMAKE_CXX_CLANG_TIDY "")
    add_subdirectory(third-party/google-test EXCLUDE_FROM_ALL)
    set(CMAKE_CXX_CLANG_TIDY ${TEMP_CMAKE_CXX_CLANG_TIDY})
    add_subdirectory(tests)
    if (UNIX)
        # Disable building google-benchmarks tests
        SET(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
        set(TEMP_CMAKE_CXX_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY})
        set(CMAKE_CXX_CLANG_TIDY "")
        add_subdirectory(third-party/benchmark EXCLUDE_FROM_ALL)
        set(CMAKE_CXX_CLANG_TIDY ${TEMP_CMAKE_CXX_CLANG_TIDY})
        add_subdirectory(benchmarks)
    endif ()
endif ()

add_subdirectory(pkg-config)

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/configs
                  DESTINATION share/${CMAKE_PROJECT_NAME}
                  COMPONENT        RUNTIME)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/scripts
                  DESTINATION share/${CMAKE_PROJECT_NAME}
                  COMPONENT        RUNTIME
                  FILE_PERMISSIONS USE_SOURCE_PERMISSIONS)
