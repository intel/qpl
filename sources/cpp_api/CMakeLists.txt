# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Intel® Query Processing Library (Intel® QPL)
# Build system

project(high-level-api CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# We need pthreads library in order to compile Intel QPL high-level API tests
find_package(Threads)

# Main library
file(GLOB SOURCES
     chaining/*.cpp
     common/*.cpp
     operations/*.cpp
     operations/analytics/*.cpp
     operations/compression/*.cpp
     operations/other/*.cpp
     results/*.cpp
     util/*.cpp)

add_library(qplhl ${SOURCES})

target_include_directories(qplhl
                           PUBLIC $<BUILD_INTERFACE:${QPL_PROJECT_DIR}/include/>
                           PRIVATE $<TARGET_PROPERTY:middle_layer_lib,INTERFACE_INCLUDE_DIRECTORIES>)

set_target_properties(qplhl PROPERTIES CXX_STANDARD 17)

target_link_libraries(qplhl PRIVATE qpl)

target_compile_options(qplhl
        PRIVATE $<$<C_COMPILER_ID:GNU>:${QPL_LINUX_TOOLCHAIN_REQUIRED_FLAGS};
                                       ${QPL_LINUX_TOOLCHAIN_DYNAMIC_LIBRARY_FLAGS};
                                       $<$<CONFIG:Release>:-O3;-D_FORTIFY_SOURCE=2>>
        PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-fno-rtti>
        PRIVATE $<$<C_COMPILER_ID:MSVC>:${QPL_WINDOWS_TOOLCHAIN_REQUIRED_FLAGS};
                                        ${QPL_WINDOWS_TOOLCHAIN_DYNAMIC_LIBRARY_FLAGS}
                                        $<$<CONFIG:Release>:-O2>>)

install(TARGETS qplhl
    EXPORT QplTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY ${QPL_PROJECT_DIR}/include/qpl/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qpl
        FILES_MATCHING PATTERN "*.hpp" PATTERN "*.cxx"
        PATTERN "qpl_util.hpp" EXCLUDE)
