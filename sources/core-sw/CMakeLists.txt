# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

project(qpl_core)
enable_language(C)

#
# Create avx512 library
#
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Find Core Sources
file(GLOB SOURCES
     src/checksums/*.c
     src/filtering/*.c
     src/other/*.c
     src/compression/*.c)

file(GLOB DATA_SOURCES
     src/data/*.c)

# Create library
add_library(qplcore_avx512 OBJECT ${SOURCES})

target_compile_definitions(qplcore_avx512 PRIVATE PLATFORM=2)

target_include_directories(qplcore_avx512
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/compression/include>
                           PRIVATE $<TARGET_PROPERTY:isal,INTERFACE_INCLUDE_DIRECTORIES>)

set_target_properties(qplcore_avx512 PROPERTIES
        $<$<C_COMPILER_ID:MSVC>:C_STANDARD 18>
        $<$<C_COMPILER_ID:GNU>:C_STANDARD 17>)

target_link_libraries(qplcore_avx512 ${CMAKE_DL_LIBS} isal)

if (WIN32)
    target_compile_options(qplcore_avx512
                           PRIVATE ${QPL_WINDOWS_TOOLCHAIN_REQUIRED_FLAGS}
                           PRIVATE /arch:AVX512
                           PRIVATE "$<$<CONFIG:Debug>:>"
                           PRIVATE "$<$<CONFIG:Release>:-O2>")
else ()
    target_compile_options(qplcore_avx512
                           PRIVATE ${QPL_LINUX_TOOLCHAIN_REQUIRED_FLAGS}
                           PRIVATE -march=skylake-avx512
                           PRIVATE "$<$<CONFIG:Debug>:>"
                           PRIVATE "$<$<CONFIG:Release>:-O3;-D_FORTIFY_SOURCE=2>")
endif ()

target_compile_definitions(qplcore_avx512 PUBLIC QPL_BADARG_CHECK)

#
# Create px library
#
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Create library
add_library(qplcore_px OBJECT ${SOURCES} ${DATA_SOURCES})

target_compile_definitions(qplcore_px PRIVATE PLATFORM=0)

target_include_directories(qplcore_px
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/compression/include>
                           PRIVATE $<TARGET_PROPERTY:isal,INTERFACE_INCLUDE_DIRECTORIES>)

set_target_properties(qplcore_px PROPERTIES
        $<$<C_COMPILER_ID:MSVC>:C_STANDARD 18>
        $<$<C_COMPILER_ID:GNU>:C_STANDARD 17>)

target_link_libraries(qplcore_px isal ${CMAKE_DL_LIBS})

if (WIN32)
    target_compile_options(qplcore_px
                           PRIVATE "$<$<CONFIG:Debug>:>"
                           PRIVATE ${QPL_WINDOWS_TOOLCHAIN_REQUIRED_FLAGS}
                           PRIVATE "$<$<CONFIG:Release>:-O2>")
else ()
    target_compile_options(qplcore_px
                           PRIVATE ${QPL_LINUX_TOOLCHAIN_REQUIRED_FLAGS}
                           PRIVATE "$<$<CONFIG:Debug>:>"
                           PRIVATE "$<$<CONFIG:Release>:-O3;-D_FORTIFY_SOURCE=2>")
endif ()

target_compile_definitions(qplcore_px PUBLIC QPL_BADARG_CHECK)

