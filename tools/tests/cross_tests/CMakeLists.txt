# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Intel® Query Processing Library (Intel® QPL)
# Build system

enable_language(CXX)

file(GLOB SOURCES *.cpp)

add_executable(cross_tests ${SOURCES})

get_target_property(QPL_SOURCE_DIR qpl SOURCE_DIR)

get_property(LIB_DEPS GLOBAL PROPERTY QPL_LIB_DEPS)

target_link_libraries(cross_tests
        PRIVATE ${LIB_DEPS}
        PRIVATE tests_common
        PRIVATE tool_generator
        PRIVATE gtest
        PRIVATE qpl
        "$<$<PLATFORM_ID:Linux>:stdc++fs>"
        "$<$<AND:$<PLATFORM_ID:Linux>,$<STREQUAL:${DYNAMIC_LOADING_LIBACCEL_CONFIG},ON>>:${CMAKE_DL_LIBS}>"
        "$<$<AND:$<PLATFORM_ID:Linux>,$<STREQUAL:${DYNAMIC_LOADING_LIBACCEL_CONFIG},OFF>>:accel-config>")

set_target_properties(cross_tests PROPERTIES CXX_STANDARD 17)

target_compile_definitions(cross_tests PRIVATE $<TARGET_PROPERTY:tests_common,COMPILE_DEFINITIONS>)
target_compile_options(cross_tests PRIVATE $<TARGET_PROPERTY:tests_common,COMPILE_OPTIONS>)

install(TARGETS cross_tests RUNTIME DESTINATION bin)
