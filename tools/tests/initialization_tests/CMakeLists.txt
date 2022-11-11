# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Intel® Query Processing Library (Intel® QPL)
# Build system

enable_language(CXX)

file(GLOB SOURCES ./test_backend/*.cpp)

add_executable(init_tests ${SOURCES})

set_target_properties(init_tests PROPERTIES CXX_STANDARD 17)

target_link_libraries(init_tests
        PRIVATE tests_common
        PRIVATE tool_generator
        PRIVATE gtest
        PRIVATE qplhl #todo remove dependency
        PRIVATE middle_layer_lib)

set_target_properties(init_tests PROPERTIES CXX_STANDARD 17)

target_compile_definitions(init_tests
        PRIVATE $<TARGET_PROPERTY:tests_common,COMPILE_DEFINITIONS>)

target_compile_options(init_tests
        PRIVATE $<TARGET_PROPERTY:tests_common,COMPILE_OPTIONS>)

install(TARGETS   init_tests
                  RUNTIME)
install(DIRECTORY test_frontend
                  TYPE             BIN
                  COMPONENT        RUNTIME
                  FILE_PERMISSIONS USE_SOURCE_PERMISSIONS)

