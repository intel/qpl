# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

project(core_iaa C CXX)

# Find Kernels Sources
file(GLOB HW_PATH_SRC sources/aecs/*.c
                      sources/aecs/*.cpp
                      sources/driver_loader/*.c
                      sources/driver_loader/*.cpp
                      sources/descriptors/*.c
                      sources/descriptors/*.cpp
                      sources/bit_rev.c)

option(BLOCK_ON_FAULT "Use `Block on Fault` flag for descriptor" ON)

# Create library
add_library(core_iaa OBJECT ${HW_PATH_SRC})

set_property(GLOBAL APPEND PROPERTY QPL_LIB_DEPS
             $<TARGET_OBJECTS:core_iaa>)

target_include_directories(core_iaa
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sources/include>
        PRIVATE $<BUILD_INTERFACE:${QPL_PROJECT_DIR}/include>  # status.h in own_checkers.h
        PRIVATE $<BUILD_INTERFACE:${QPL_PROJECT_DIR}/sources/c_api> # own_checkers.h
        PRIVATE $<TARGET_PROPERTY:qplcore_sw_dispatcher,INTERFACE_INCLUDE_DIRECTORIES>)

set_target_properties(core_iaa PROPERTIES
        $<$<C_COMPILER_ID:MSVC>:C_STANDARD 18>
        $<$<C_COMPILER_ID:GNU>:C_STANDARD 17>
        CXX_STANDARD 17)

if (WIN32)
    modify_standard_language_flag(LANGUAGE_NAME "CXX"
            FLAG_NAME "/GR"
            NEW_FLAG_VALUE "-")
endif ()

target_compile_options(core_iaa
        PRIVATE $<$<C_COMPILER_ID:GNU>:${QPL_LINUX_TOOLCHAIN_REQUIRED_FLAGS};
                                       $<$<CONFIG:Release>:-O3;-D_FORTIFY_SOURCE=2>>
        PRIVATE $<$<C_COMPILER_ID:MSVC>:${QPL_WINDOWS_TOOLCHAIN_REQUIRED_FLAGS};
                                        $<$<CONFIG:Release>:-O2>>)

target_compile_features(core_iaa PRIVATE c_std_11)

target_compile_definitions(core_iaa PRIVATE QPL_BADARG_CHECK
        PRIVATE $<$<BOOL:${BLOCK_ON_FAULT}>: BLOCK_ON_FAULT_ENABLED>
        PRIVATE $<$<BOOL:${LOG_HW_INIT}>:LOG_HW_INIT>
        PRIVATE $<$<BOOL:${DYNAMIC_LOADING_LIBACCEL_CONFIG}>:DYNAMIC_LOADING_LIBACCEL_CONFIG>)
