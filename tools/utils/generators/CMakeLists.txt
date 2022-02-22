# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Intel® Query Processing Library (Intel® QPL)
# Build system

enable_language(CXX)

file(GLOB_RECURSE QPL_GEN_CXX_SRC "*.cpp")
file(GLOB_RECURSE QPL_GEN_C_SRC "*.c")

#set(QPL_GEN_ASM_SRC
#        "deflate_generator/asm_intel64/expand_l_tree.asm"
#        "deflate_generator/asm_intel64/proc_heap.asm")

add_library(tool_generator OBJECT
        ${QPL_GEN_CXX_SRC}
        ${QPL_GEN_C_SRC}
        ${QPL_GEN_ASM_SRC})

target_include_directories(tool_generator
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deflate_generator/include>
        PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/configurators>)

set_target_properties(tool_generator PROPERTIES
        CXX_STANDARD 17
        C_STANDARD 99)

target_compile_options(tool_generator PRIVATE
        "$<$<CONFIG:Debug>:>"
        "$<$<CONFIG:Release>:>")
