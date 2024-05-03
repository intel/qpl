# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Build system for core-iaa layer, that includes:
# - accel-config loading mechanism (driver_loader/)
# - logic for filling structures (AECS, descriptor) required for offloading operations to accelerator (aecs/, descriptors/)

cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR)

project(core_iaa C)

# Find Kernels Sources
file(GLOB HW_PATH_SRC sources/aecs/*.c
                      sources/driver_loader/*.c
                      sources/descriptors/*.c
                      sources/*.c)

# Create library
add_library(core_iaa OBJECT ${HW_PATH_SRC})

set_property(GLOBAL APPEND PROPERTY QPL_LIB_DEPS
             $<TARGET_OBJECTS:core_iaa>)

target_include_directories(core_iaa
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/sources/include>
        PRIVATE $<TARGET_PROPERTY:qpl_c_api,INTERFACE_INCLUDE_DIRECTORIES> # for own_checkers.h
        PRIVATE $<TARGET_PROPERTY:qplcore_sw_dispatcher,INTERFACE_INCLUDE_DIRECTORIES>)

set_target_properties(core_iaa PROPERTIES
        $<$<C_COMPILER_ID:MSVC>:C_STANDARD 18>
        $<$<C_COMPILER_ID:GNU,Clang>:C_STANDARD 17>
        C_VISIBILITY_PRESET hidden)

target_compile_options(core_iaa
        PRIVATE $<$<C_COMPILER_ID:GNU,Clang>:${QPL_LINUX_TOOLCHAIN_REQUIRED_FLAGS};
                                       $<$<CONFIG:Release>:-O3;-U_FORTIFY_SOURCE;-D_FORTIFY_SOURCE=2>>
        PRIVATE $<$<C_COMPILER_ID:MSVC>:${QPL_WINDOWS_TOOLCHAIN_REQUIRED_FLAGS};
                                        $<$<CONFIG:Release>:-O2>>)

target_compile_features(core_iaa PRIVATE c_std_11)

target_compile_definitions(core_iaa PRIVATE QPL_BADARG_CHECK
        PRIVATE $<$<BOOL:${LOG_HW_INIT}>:LOG_HW_INIT>
        PRIVATE $<$<BOOL:${DYNAMIC_LOADING_LIBACCEL_CONFIG}>:DYNAMIC_LOADING_LIBACCEL_CONFIG>)
