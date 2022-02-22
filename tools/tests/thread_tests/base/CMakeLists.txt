# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Intel® Query Processing Library (Intel® QPL)
# Build system

enable_language(CXX)

file(GLOB SOURCES *.cpp)

add_library(thread_tests OBJECT ${SOURCES})

target_include_directories(thread_tests
        PRIVATE $<TARGET_PROPERTY:core_iaa,INTERFACE_INCLUDE_DIRECTORIES>) #todo remove dependency

target_link_libraries(thread_tests
        PRIVATE tests_common
        PRIVATE tool_generator
        PRIVATE qplhl
        PRIVATE qpl)

set_target_properties(thread_tests PROPERTIES CXX_STANDARD 17)

target_compile_definitions(thread_tests
        PRIVATE $<TARGET_PROPERTY:tests_common,COMPILE_DEFINITIONS>)

target_compile_options(thread_tests
        PRIVATE $<TARGET_PROPERTY:tests_common,COMPILE_OPTIONS>)
