# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Intel® Query Processing Library (Intel® QPL)
# Build system

# If QPL is build with -DSANITIZE_THREADS=ON, use CMake v3.23 or higher
# Before v3.23 CMake will not add -pthread when pthread is found in libc,
# and this causes undefined reference errors when QPL is build with -DSANITIZE_THREADS=ON
if (UNIX AND "${SANITIZE_THREADS}" STREQUAL "ON")
    cmake_minimum_required(VERSION 3.23 FATAL_ERROR)
else ()
    cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
endif ()

project(QPL VERSION 2.0.0 LANGUAGES C CXX
    DESCRIPTION "A library to provide high-performance query processing operations on Intel CPUs."
)

# Set default build type to release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# This specify the build version
set(QPL_SHARED_LIB_VERSION   ${PROJECT_VERSION})
# This is the indicator for the API and ABI compatibility.
set(QPL_SHARED_LIB_SOVERSION ${PROJECT_VERSION_MAJOR})

set(CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/overrides.cmake")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
set(CMAKE_EXPORT_PACKAGE_REGISTRY ON)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

message(STATUS "Intel QPL version: ${CMAKE_PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Build Options
set(QPL_USE_CLANG_SANITIZER "" CACHE STRING
    "Enables build with sanitizers. Supported values:
    ADDRESS: Enables AddressSanitizer
    LEAK: Enables LeakSanitizer
    UNDEFINED: Enables UndefinedBehaviorSanitizer
    THREAD: Enables ThreadSanitizer
    ALL_COMPATIBLE: Enables most of the currently compatible checks (Note: this option is for convenience and may change).")
option(LOG_HW_INIT "Enables HW initialization log" OFF)
option(QPL_EXPERIMENTAL_LOG_IAA "Enables logging of execution information on the accelerator to the job structure" OFF)
option(EFFICIENT_WAIT "Enables usage of efficient wait instructions" OFF)
option(LIB_FUZZING_ENGINE "Enables fuzzy testing" OFF)
option(DYNAMIC_LOADING_LIBACCEL_CONFIG "Loads the accelerator configuration library (libaccel-config) dynamically with dlopen" ON)
set(QPL_LIBRARY_TYPE "STATIC" CACHE STRING "Specifies the resulting library type")
option(QPL_USE_CLANG_TIDY "Run clang-tidy" OFF)

# Deprecated Build Options handling
option(SANITIZE_MEMORY "Enables memory sanitizing, Deprecated, use QPL_USE_CLANG_SANITIZER=ADDRESS, LEAK or ALL_COMPATIBLE instead" OFF)
option(SANITIZE_THREADS "Enables threads sanitizing, Deprecated, use QPL_USE_CLANG_SANITIZER=THREAD instead" OFF)

if (SANITIZE_MEMORY)
    message(DEPRECATION "SANITIZE_MEMORY is deprecated and will be removed in a future version. Use QPL_USE_CLANG_SANITIZER instead.")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fsanitize=leak -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fsanitize=leak -g")
endif ()

if (SANITIZE_THREADS)
    message(DEPRECATION "SANITIZE_THREADS is deprecated and will be removed in a future version. Use QPL_USE_CLANG_SANITIZER instead.")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g")
endif ()

# Print user's settings
message(STATUS "Sanitizer used for build and testing: ${QPL_USE_CLANG_SANITIZER}")
message(STATUS "Hardware initialization logging: ${LOG_HW_INIT}")
message(STATUS "IAA execution information logging to job structure: ${QPL_EXPERIMENTAL_LOG_IAA}")
message(STATUS "Efficient wait instructions: ${EFFICIENT_WAIT}")
message(STATUS "Fuzz testing build: ${LIB_FUZZING_ENGINE}")
message(STATUS "Load libaccel-config dynamically with dlopen: ${DYNAMIC_LOADING_LIBACCEL_CONFIG}")
message(STATUS "Run clang-tidy: ${QPL_USE_CLANG_TIDY}")
message(STATUS "Library build type: ${QPL_LIBRARY_TYPE}")

if (QPL_USE_CLANG_SANITIZER)
    if (WIN32)
        message(WARNING "Sanitizers are not supported on Windows")
    elseif (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(WARNING "Sanitizers are only supported with Clang")
    else ()
        set(SANITIZER_FLAGS "")
        if (QPL_USE_CLANG_SANITIZER STREQUAL "ADDRESS")
            set(SANITIZER_FLAGS "-fsanitize=address")
        elseif (QPL_USE_CLANG_SANITIZER STREQUAL "LEAK")
            set(SANITIZER_FLAGS "-fsanitize=leak")
        elseif (QPL_USE_CLANG_SANITIZER STREQUAL "UNDEFINED")
            set(SANITIZER_FLAGS "-fsanitize=integer -fsanitize=null")
        elseif (QPL_USE_CLANG_SANITIZER STREQUAL "THREAD")
            set(SANITIZER_FLAGS "-fsanitize=thread")
        elseif (QPL_USE_CLANG_SANITIZER STREQUAL "ALL_COMPATIBLE")
            set(SANITIZER_FLAGS "-fsanitize=address -fsanitize=leak -fsanitize=integer -fsanitize=null")
        endif ()
        if (SANITIZER_FLAGS)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS} -g")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS} -g")
        endif ()
    endif ()
endif ()

if (WIN32 AND "${QPL_LIBRARY_TYPE}" STREQUAL "SHARED")
    message(FATAL_ERROR "Building shared library is not supported in Windows")
endif ()

if (LIB_FUZZING_ENGINE)
    option(QPL_BUILD_EXAMPLES "Builds examples" OFF)
    option(QPL_BUILD_TESTS "Builds tests and benchmarks framework" OFF)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping -fsanitize=address,fuzzer -g -O1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping -fsanitize=address,fuzzer -g -O1")
endif ()

if (QPL_USE_CLANG_TIDY)
    find_program(DO_CLANG_TIDY NAMES clang-tidy)
    set(CMAKE_CXX_CLANG_TIDY ${DO_CLANG_TIDY})
    set(CMAKE_C_CLANG_TIDY ${DO_CLANG_TIDY})
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    message(STATUS "Using clang-tidy to run checks")
endif ()

include(cmake/CompileOptions.cmake)
include(cmake/target_settings.cmake)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

install(EXPORT ${PROJECT_NAME}Targets
        NAMESPACE QPL::
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

SET(QPL_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Build library
add_subdirectory(sources)

# Build additional components
# Set of extra options that allows to build only library, or library
# and examples excluding tests, etc.
option(QPL_BUILD_EXAMPLES "Builds examples" ON)
option(QPL_BUILD_TESTS "Builds tests and benchmarks framework" ON)

if (QPL_BUILD_TESTS)
    enable_testing()
endif ()

# Print user's settings
message(STATUS "Build with examples: ${QPL_BUILD_EXAMPLES}")
message(STATUS "Build with tests and benchmarks framework: ${QPL_BUILD_TESTS}")

if (QPL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

add_subdirectory(tools)
