# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Intel® Query Processing Library (Intel® QPL)
# Build system

enable_language(CXX)

file(GLOB SOURCES *.cpp)

add_library(tests_common STATIC ${SOURCES})
set_target_properties(tests_common PROPERTIES CXX_STANDARD 17)

target_include_directories(tests_common
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        PUBLIC $<TARGET_PROPERTY:tool_common,INTERFACE_INCLUDE_DIRECTORIES>
        PUBLIC $<TARGET_PROPERTY:tool_generator,INTERFACE_INCLUDE_DIRECTORIES>
        PRIVATE $<TARGET_PROPERTY:qpl,INTERFACE_INCLUDE_DIRECTORIES>)

target_link_libraries(tests_common
        PUBLIC gtest
        PUBLIC tool_common)

target_compile_definitions(tests_common PRIVATE $<TARGET_PROPERTY:tool_common,COMPILE_DEFINITIONS>)
target_compile_options(tests_common PRIVATE $<TARGET_PROPERTY:tool_common,COMPILE_OPTIONS>)
