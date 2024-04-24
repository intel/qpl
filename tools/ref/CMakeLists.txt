# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Build system for the library with Reference implementations.
# Includes reference implementations for CRC and filter functions, and used
# for functional testing.

cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

project(qplref C)

# Finding library sources
file(GLOB QPL_C_SRC *.c)

# Adding library target
add_library(qplref STATIC ${QPL_C_SRC})

# Setting external and internal interfaces for reference library
target_include_directories(qplref
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           PRIVATE $<TARGET_PROPERTY:qpl,INTERFACE_INCLUDE_DIRECTORIES>)

set_target_properties(qplref PROPERTIES
                      C_STANDARD 99)

target_compile_options(qplref PUBLIC
                       "$<$<CONFIG:Debug>:>"
                       "$<$<CONFIG:Release>:>"
                       PRIVATE "$<$<C_COMPILER_ID:GNU>:-Werror>"
                       PRIVATE "$<$<C_COMPILER_ID:MSVC>:/WX>")

target_compile_definitions(qplref PUBLIC REF_BAD_ARG_CHECK)
