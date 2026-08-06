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

    # Build third-party google-test library required for tests
    # Build it without clang-tidy checks
    set(TEMP_CMAKE_CXX_CLANG_TIDY ${CMAKE_CXX_CLANG_TIDY})
    set(CMAKE_CXX_CLANG_TIDY "")
    # Save the main project's current shared/static preference
    set(TEMP_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
    # Force google-test library to build as STATIC
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
    # Disable google-test's internal tests to speed up the build
    set(gtest_build_tests OFF CACHE BOOL "Build gtest tests" FORCE)
    # Add the subdirectory creating the 'gtest' and 'gtest_main' targets
    add_subdirectory(third-party/google-test EXCLUDE_FROM_ALL)
    # Restore the original shared/static preference for the rest of the project
    set(BUILD_SHARED_LIBS ${TEMP_BUILD_SHARED_LIBS} CACHE BOOL "Build shared libraries" FORCE)
    # Restore the original clang-tidy preference
    set(CMAKE_CXX_CLANG_TIDY ${TEMP_CMAKE_CXX_CLANG_TIDY})

    add_subdirectory(tests)
    install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/testdata
                      DESTINATION share/${CMAKE_PROJECT_NAME}
                      COMPONENT RUNTIME)
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
