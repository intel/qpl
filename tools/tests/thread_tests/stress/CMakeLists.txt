# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Intel® Query Processing Library (Intel® QPL)
# Build system

enable_language(CXX)

file(GLOB SOURCES ./*.cpp)

add_executable(stress_thread_tests ${SOURCES})

target_include_directories(stress_thread_tests
        PRIVATE $<TARGET_PROPERTY:core_iaa,INTERFACE_INCLUDE_DIRECTORIES>) #todo remove dependency

target_link_libraries(stress_thread_tests
        PRIVATE tests_common
        PRIVATE tool_generator
        PRIVATE qplhl qpl
        "$<$<PLATFORM_ID:Linux>:dl;pthread;stdc++fs>")

set_target_properties(stress_thread_tests PROPERTIES CXX_STANDARD 17)

target_compile_definitions(stress_thread_tests
        PRIVATE $<TARGET_PROPERTY:tests_common,COMPILE_DEFINITIONS>)

target_compile_options(stress_thread_tests
        PRIVATE $<TARGET_PROPERTY:tests_common,COMPILE_OPTIONS>)
