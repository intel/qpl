# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Build system for Benchmarks Framework.
# Currently there is a dependency on middle-layer for determining device mapping to NUMA nodes.
# todo: change dependency to copy of dispatcher from utils instead.

cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

enable_language(CXX)

add_executable(qpl_benchmarks
    src/main.cpp
    src/cases/deflate.cpp
    src/cases/inflate.cpp
    src/cases/crc64.cpp
)

target_link_libraries(qpl_benchmarks
    PRIVATE qpl
    PRIVATE tool_common
    PRIVATE tool_hw_dispatcher
    PRIVATE benchmark
    "$<$<PLATFORM_ID:Linux>:stdc++fs>"
    "$<$<AND:$<PLATFORM_ID:Linux>,$<STREQUAL:${DYNAMIC_LOADING_LIBACCEL_CONFIG},ON>>:${CMAKE_DL_LIBS}>"
    "$<$<AND:$<PLATFORM_ID:Linux>,$<STREQUAL:${DYNAMIC_LOADING_LIBACCEL_CONFIG},OFF>>:accel-config>")

get_target_property(GBENCH_SOURCE_DIR benchmark SOURCE_DIR)

target_include_directories(qpl_benchmarks
    PRIVATE $<BUILD_INTERFACE:${QPL_PROJECT_DIR}/include/> $<INSTALL_INTERFACE:include>
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
    PRIVATE $<TARGET_PROPERTY:tool_common,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE $<TARGET_PROPERTY:tool_hw_dispatcher,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE ${GBENCH_SOURCE_DIR})

target_compile_options(qpl_benchmarks PUBLIC -Wall)

install(TARGETS qpl_benchmarks RUNTIME DESTINATION bin)

