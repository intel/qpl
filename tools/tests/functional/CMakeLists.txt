# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Intel® Query Processing Library (Intel® QPL)
# Build system

enable_language(CXX)

add_subdirectory(unit_tests)
add_subdirectory(algorithmic_tests)
add_subdirectory(bad_argument_tests)
add_subdirectory(negative_tests)

add_executable(tests main.cpp
               $<TARGET_OBJECTS:thread_tests>
               $<TARGET_OBJECTS:algorithmic_tests>
               $<TARGET_OBJECTS:bad_argument_tests>
               $<TARGET_OBJECTS:negative_tests>
               $<TARGET_OBJECTS:unit_tests>)

set_target_properties(tests PROPERTIES CXX_STANDARD 17)

get_property(LIB_DEPS GLOBAL PROPERTY QPL_LIB_DEPS)

target_link_libraries(tests
        PRIVATE ${LIB_DEPS}
        PRIVATE tests_common
        PRIVATE tool_generator
        PRIVATE qplref
        PRIVATE qpl
        "$<$<PLATFORM_ID:Linux>:stdc++fs>"
        "$<$<AND:$<PLATFORM_ID:Linux>,$<STREQUAL:${DYNAMIC_LOADING_LIBACCEL_CONFIG},ON>>:${CMAKE_DL_LIBS}>"
        "$<$<AND:$<PLATFORM_ID:Linux>,$<STREQUAL:${DYNAMIC_LOADING_LIBACCEL_CONFIG},OFF>>:accel-config>")

set_target_properties(tests PROPERTIES CXX_STANDARD 17)

target_compile_definitions(tests
        PRIVATE $<TARGET_PROPERTY:tests_common,COMPILE_DEFINITIONS>)

target_compile_options(tests
        PRIVATE $<TARGET_PROPERTY:tests_common,COMPILE_OPTIONS>)

# Install rules
install(TARGETS tests RUNTIME DESTINATION bin)
