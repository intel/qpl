# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Build system for c_api layer, that includes:
# - entry points for job API, e.g., initialization, destroy, serialization, etc.
# - entry points for Huffman Table-related API
# - some legacy logic for hardware_path (todo: refactor and remove)
# - perform_<foo> functions that are the layer between API and middle-layer

cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR)

enable_language(C CXX)

file(GLOB QPL_C_API_SRC
        compression_operations/*.c
        compression_operations/*.cpp
        filter_operations/*.cpp
        legacy_hw_path/*.c
        legacy_hw_path/*.cpp
        other_operations/*.cpp
        serialization/*.cpp
        *.cpp)

add_library(qpl_c_api OBJECT ${QPL_C_API_SRC})

target_include_directories(qpl_c_api
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        PUBLIC $<BUILD_INTERFACE:${QPL_PROJECT_DIR}/include/> $<INSTALL_INTERFACE:include>
        PRIVATE $<TARGET_PROPERTY:middle_layer_lib,INTERFACE_INCLUDE_DIRECTORIES>)

set_target_properties(qpl_c_api PROPERTIES
        $<$<C_COMPILER_ID:MSVC>:C_STANDARD 18>
        $<$<C_COMPILER_ID:GNU,Clang>:C_STANDARD 17>
        CXX_STANDARD 17
        C_VISIBILITY_PRESET hidden
        CXX_VISIBILITY_PRESET hidden)

target_compile_options(qpl_c_api
        PRIVATE $<$<C_COMPILER_ID:GNU,Clang>:${QPL_LINUX_TOOLCHAIN_REQUIRED_FLAGS};
                                             $<$<CONFIG:Release>:-O3;-U_FORTIFY_SOURCE;-D_FORTIFY_SOURCE=2>>
        PRIVATE $<$<C_COMPILER_ID:GNU>:${QPL_LINUX_TOOLCHAIN_DYNAMIC_LIBRARY_FLAGS}>
        PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:${QPL_LINUX_TOOLCHAIN_CPP_EMBEDDED_FLAGS}>
        PRIVATE $<$<C_COMPILER_ID:MSVC>:${QPL_WINDOWS_TOOLCHAIN_REQUIRED_FLAGS};
                                        ${QPL_WINDOWS_TOOLCHAIN_DYNAMIC_LIBRARY_FLAGS}
                                        $<$<CONFIG:Release>:-O2>>
        PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:${QPL_WINDOWS_TOOLCHAIN_CPP_EMBEDDED_FLAGS}>)

# GCC allows for linking options to be passed as compile option, where Clang does not
# Passing these linking options here
if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_link_options(qpl_c_api PRIVATE ${QPL_LINUX_TOOLCHAIN_DYNAMIC_LIBRARY_FLAGS})
endif ()

target_compile_definitions(qpl_c_api
        PUBLIC -DQPL_BADARG_CHECK # own_checkers.h
        PUBLIC -DQPL_LIB          # needed for middle_layer_lib
        PUBLIC $<$<BOOL:${DYNAMIC_LOADING_LIBACCEL_CONFIG}>:DYNAMIC_LOADING_LIBACCEL_CONFIG> # needed for middle_layer_lib
        PUBLIC $<$<BOOL:${LOG_HW_INIT}>:LOG_HW_INIT>                                         # needed for middle_layer_lib
        PUBLIC $<$<BOOL:${QPL_LOG_IAA_TIME}>:QPL_LOG_IAA_TIME>                               # needed for middle_layer_lib
        PUBLIC $<$<C_COMPILER_ID:MSVC>:_ENABLE_EXTENDED_ALIGNED_STORAGE>)

set_property(GLOBAL APPEND PROPERTY QPL_LIB_DEPS
             $<TARGET_OBJECTS:qpl_c_api>)
