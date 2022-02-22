# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Intel® Query Processing Library (Intel® QPL)
# Build system

cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR)

project(qplref C)

# Finding Intel QPL reference library sources
file(GLOB QPL_C_SRC *.c)

# Adding Intel QPL reference library target
add_library(qplref STATIC ${QPL_C_SRC})

# Setting external and internal interfaces for Intel QPL reference library
target_include_directories(qplref
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           PRIVATE $<TARGET_PROPERTY:qpl,INTERFACE_INCLUDE_DIRECTORIES>
                           PRIVATE $<TARGET_PROPERTY:isal,INTERFACE_INCLUDE_DIRECTORIES>)

set_target_properties(qplref PROPERTIES
                      CXX_STANDARD 11
                      C_STANDARD 99)

target_compile_options(qplref PUBLIC
                       "$<$<CONFIG:Debug>:>"
                       "$<$<CONFIG:Release>:>"
                       PRIVATE "$<$<C_COMPILER_ID:GNU>:-Werror>"
                       PRIVATE "$<$<C_COMPILER_ID:MSVC>:/WX>")

target_compile_definitions(qplref PUBLIC REF_BAD_ARG_CHECK)
