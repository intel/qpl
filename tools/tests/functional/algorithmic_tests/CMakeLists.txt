# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Intel® Query Processing Library (Intel® QPL)
# Build system

enable_language(CXX)

file(GLOB SOURCES low_level_api/*.cpp)

add_library(algorithmic_tests OBJECT ${SOURCES})

get_target_property(QPL_SOURCE_DIR qpl SOURCE_DIR)
set_target_properties(algorithmic_tests PROPERTIES CXX_STANDARD 17)

target_include_directories(algorithmic_tests
        PRIVATE ${QPL_SOURCE_DIR}/sources/include
        PRIVATE $<TARGET_PROPERTY:tests_common,INTERFACE_INCLUDE_DIRECTORIES>
        PRIVATE $<TARGET_PROPERTY:tool_common,INTERFACE_INCLUDE_DIRECTORIES>
        PRIVATE $<TARGET_PROPERTY:tool_generator,INTERFACE_INCLUDE_DIRECTORIES>
        PRIVATE $<TARGET_PROPERTY:qpl,INTERFACE_INCLUDE_DIRECTORIES>
        PRIVATE $<TARGET_PROPERTY:qplref,INTERFACE_INCLUDE_DIRECTORIES>)

target_compile_definitions(algorithmic_tests
        PRIVATE $<TARGET_PROPERTY:tests_common,COMPILE_DEFINITIONS>)

target_compile_options(algorithmic_tests
        PRIVATE $<TARGET_PROPERTY:tests_common,COMPILE_OPTIONS>)
