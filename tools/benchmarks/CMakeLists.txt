# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

add_executable(qpl_benchmarks
    src/main.cpp
    src/cases/deflate.cpp
    src/cases/inflate.cpp
)

target_link_libraries(qpl_benchmarks
    PUBLIC qpl middle_layer_lib benchmark stdc++fs)

get_target_property(GBENCH_SOURCE_DIR benchmark SOURCE_DIR)

target_include_directories(qpl_benchmarks
    PRIVATE ./include
    PRIVATE ${GBENCH_SOURCE_DIR})

target_compile_options(qpl_benchmarks PUBLIC -Wall)

install(TARGETS qpl_benchmarks RUNTIME DESTINATION bin)

