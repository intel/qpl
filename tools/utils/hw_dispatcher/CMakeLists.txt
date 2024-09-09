# ==========================================================================
# Copyright (C) 2024 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Build system for hardware dispatcher used in testing, that includes
# accel-config loading mechanism as well as device initialization routines.

cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR)

enable_language(C CXX)

file(GLOB TOOLS_HW_DISPATCH_SRC *.c *.cpp)

add_library(tool_hw_dispatcher OBJECT ${TOOLS_HW_DISPATCH_SRC})

target_include_directories(tool_hw_dispatcher
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        PRIVATE $<TARGET_PROPERTY:tool_common,INTERFACE_INCLUDE_DIRECTORIES>
        PRIVATE $<TARGET_PROPERTY:qpl,INTERFACE_INCLUDE_DIRECTORIES>)

set_target_properties(tool_hw_dispatcher PROPERTIES
        CXX_STANDARD 17
        C_STANDARD 99)

target_compile_definitions(tool_hw_dispatcher
        PUBLIC $<$<BOOL:${DYNAMIC_LOADING_LIBACCEL_CONFIG}>:DYNAMIC_LOADING_LIBACCEL_CONFIG>)
