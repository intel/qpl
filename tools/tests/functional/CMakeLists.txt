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

qpl_set_common_target_settings(tests)
set_target_properties(tests PROPERTIES
                      OUTPUT_NAME qpl-func-tests)

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

target_compile_definitions(tests
        PRIVATE $<TARGET_PROPERTY:tests_common,COMPILE_DEFINITIONS>)

target_compile_options(tests
        PRIVATE $<TARGET_PROPERTY:tests_common,COMPILE_OPTIONS>)

install(TARGETS tests
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Test groups that run under target: make test

# Smoke tests are a group of small number of tests that can run in a few seconds on any HW.

add_test(NAME test-unit-smoke-sw_path
	COMMAND $<TARGET_FILE:tests> --dataset=${QPL_PROJECT_DIR}/tools/testdata
	--gtest_filter=ta_c_api_async_multiple_jobs_submit.*:ta_c_api_integrity_control.*:ta_c_api_crc64.*
	--path=sw)

# Short tests are a group of a few tests for each unit that can be run in a short time on any HW.

add_test(NAME test-unit-short-sw_path
	COMMAND $<TARGET_FILE:tests> --dataset=${QPL_PROJECT_DIR}/tools/testdata
	--gtest_filter=ta_c_api_deflate.*:ta_c_api_inflate.*:ta_c_api_scan.range_only:ta_c_api_extract.initial_output_index:ta_unit_api_qplc_extract_16u.base:ta_unit_api_qplc_select_16u.base
	--path=sw)
