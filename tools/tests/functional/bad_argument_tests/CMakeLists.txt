# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Intel® Query Processing Library (Intel® QPL)
# Build system

enable_language(CXX)

file(GLOB SOURCES low_level_api/*.cpp)

add_library(bad_argument_tests OBJECT ${SOURCES})

set_target_properties(bad_argument_tests PROPERTIES CXX_STANDARD 17)

target_include_directories(bad_argument_tests
        PRIVATE $<TARGET_PROPERTY:tests_common,INTERFACE_INCLUDE_DIRECTORIES>
        PRIVATE $<TARGET_PROPERTY:tool_common,INTERFACE_INCLUDE_DIRECTORIES>
        PRIVATE $<TARGET_PROPERTY:tool_generator,INTERFACE_INCLUDE_DIRECTORIES>
        PRIVATE $<TARGET_PROPERTY:qpl,INTERFACE_INCLUDE_DIRECTORIES>)

target_compile_definitions(bad_argument_tests
        PRIVATE $<TARGET_PROPERTY:tests_common,COMPILE_DEFINITIONS>)

target_compile_options(bad_argument_tests
        PRIVATE $<TARGET_PROPERTY:tests_common,COMPILE_OPTIONS>)
