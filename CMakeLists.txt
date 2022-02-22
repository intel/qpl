# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Intel® Query Processing Library (Intel® QPL)
# Build system

cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(Qpl VERSION 0.1.20 LANGUAGES C CXX)

set(CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/overrides.cmake")

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
set(CMAKE_EXPORT_PACKAGE_REGISTRY ON)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

option(SANITIZE_MEMORY "Enables memory sanitizing" OFF)
option(SANITIZE_THREADS "Enables threads sanitizing" OFF)
option(LOG_HW_INIT "Enables HW initialization log" OFF)
option(EFFICIENT_WAIT "Enables usage of efficient wait instructions" OFF)
option(LIB_FUZZING_ENGINE "Enables fuzzy testing" OFF)

if (SANITIZE_MEMORY)
    message(STATUS "Memory sanitizing build is enabled")

    if (WIN32)
        message(FATAL_ERROR "Memory sanitizing is not supported in Windows")
    else ()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fsanitize=leak -g")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak -g")
    endif ()
endif ()

if (SANITIZE_THREADS)
    message(STATUS "Threads sanitizing build is enabled")

    if (WIN32)
        message(FATAL_ERROR "Threads sanitizing is not supported in Windows")
    else ()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread -g")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g")
    endif ()
endif ()

if (LIB_FUZZING_ENGINE)
    message(STATUS "Fuzzing build is enabled")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
endif ()

include(cmake/tests.cmake)
include(cmake/CompileOptions.cmake)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

message(STATUS "Hardware Path Status: ON")
message(STATUS "Intel QPL version: ${CMAKE_PROJECT_VERSION}")
message(STATUS "Hardware initialization logging: ${LOG_HW_INIT}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

install(EXPORT ${PROJECT_NAME}Targets
        NAMESPACE Qpl::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        "include(\${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}Targets.cmake)\n")

write_basic_package_version_file(
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# Disable building google-benchmarks tests
SET(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
SET(QPL_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Build components
add_subdirectory(sources)
add_subdirectory(examples)
add_subdirectory(tools)
