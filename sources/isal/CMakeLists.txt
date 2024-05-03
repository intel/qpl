# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)
project(isal CXX ASM)

enable_language(ASM_NASM)

set(ISAL_C_SRC igzip/adler32_base.c
    igzip/huff_codes.c
    igzip/hufftables_c.c
    igzip/igzip.c
    igzip/igzip_base.c
    igzip/flatten_ll.c
    igzip/encode_df.c
    igzip/igzip_icf_base.c
    igzip/igzip_inflate.c
    igzip/igzip_icf_body.c
    crc/crc_base.c
    crc/crc64_base.c
    )

set(ISAL_ASM_SRC igzip/igzip_body.asm
    igzip/igzip_gen_icf_map_lh1_04.asm
    igzip/igzip_gen_icf_map_lh1_06.asm
    igzip/igzip_decode_block_stateless_04.asm
    igzip/igzip_finish.asm
    igzip/encode_df_04.asm
    igzip/encode_df_06.asm
    igzip/igzip_decode_block_stateless_01.asm
    igzip/proc_heap.asm
    igzip/igzip_icf_body_h1_gr_bt.asm
    igzip/igzip_icf_finish.asm
    igzip/igzip_inflate_multibinary.asm
    igzip/igzip_update_histogram_01.asm
    igzip/igzip_update_histogram_04.asm
    igzip/rfc1951_lookup.asm
    igzip/adler32_sse.asm
    igzip/adler32_avx2_4.asm
    igzip/igzip_deflate_hash.asm
    igzip/igzip_set_long_icf_fg_04.asm
    igzip/igzip_set_long_icf_fg_06.asm
    igzip/igzip_multibinary.asm
    crc/crc_multibinary.asm
    crc/crc32_gzip_refl_by8.asm
    crc/crc32_gzip_refl_by8_02.asm
    crc/crc32_gzip_refl_by16_10.asm
    crc/crc32_ieee_01.asm
    crc/crc32_ieee_02.asm
    crc/crc32_ieee_by4.asm
    crc/crc32_ieee_by16_10.asm
    crc/crc32_iscsi_00.asm
    crc/crc32_iscsi_01.asm
    crc/crc32_iscsi_by16_10.asm
    )

set(DISABLED_WARNINGS_FLAGS /wd4244 /wd4101 /wd4267 /wd4018 /wd4146 /wd4334 /wd4308)
set(FEATURE_FLAGS -DHAVE_AS_KNOWS_AVX512 -DAS_FEATURE_LEVEL=10)

# Adding ISA-L library target
add_library(isal OBJECT ${ISAL_C_SRC})
add_library(isal_asm OBJECT ${ISAL_ASM_SRC})

set_property(GLOBAL APPEND PROPERTY QPL_LIB_DEPS
             $<TARGET_OBJECTS:isal>)

set_property(GLOBAL APPEND PROPERTY QPL_LIB_DEPS
             $<TARGET_OBJECTS:isal_asm>)

# Setting external and internal interfaces for ISA-L library
target_include_directories(isal
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           PUBLIC igzip)

set_target_properties(isal PROPERTIES
                      CXX_STANDARD 11
                      C_STANDARD 99
                      CXX_VISIBILITY_PRESET hidden
                      C_VISIBILITY_PRESET hidden)

target_compile_options(isal PRIVATE
                       "$<$<C_COMPILER_ID:GNU,Clang>:${QPL_LINUX_TOOLCHAIN_REQUIRED_FLAGS}>"
                       "$<$<C_COMPILER_ID:MSVC>:/WX;${DISABLED_WARNINGS_FLAGS};${QPL_WINDOWS_TOOLCHAIN_REQUIRED_FLAGS}>"
                       "$<$<CONFIG:Debug>:>"
                       "$<$<CONFIG:Release>:>")

target_compile_options(isal_asm PUBLIC "-I${CMAKE_CURRENT_SOURCE_DIR}/include/"
                       PUBLIC "-I${CMAKE_CURRENT_SOURCE_DIR}/igzip/"
                       PUBLIC "-I${CMAKE_CURRENT_SOURCE_DIR}/crc/"
                       PUBLIC "-DHAVE_AS_KNOWS_AVX512"
                       PUBLIC "-DAS_FEATURE_LEVEL=10"
                       PUBLIC "-DQPL_LIB"
                       PUBLIC "$<$<STREQUAL:${QPL_LIBRARY_TYPE},SHARED>:-DQPL_HIDE_ASM_SYMBOLS>")

target_compile_definitions(isal PUBLIC
                           QPL_LIB
                           NDEBUG)
