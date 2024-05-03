# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Build system for core-sw layer, that includes:
# - host optimizations for different platforms (src/)
# - implementation of run-time dispatcher between those (dispatcher/)

enable_language(C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

get_list_of_supported_optimizations(PLATFORMS_LIST)

#
# Create set of libraries
# corresponding to supported platforms
#
foreach(PLATFORM_ID IN LISTS PLATFORMS_LIST)

     # Find Core Sources
     file(GLOB SOURCES
          src/checksums/*.c
          src/filtering/*.c
          src/other/*.c
          src/compression/*.c)

     file(GLOB DATA_SOURCES
          src/data/*.c)

     # Create library
     add_library(qplcore_${PLATFORM_ID} OBJECT ${SOURCES})

     set_property(GLOBAL APPEND PROPERTY QPL_LIB_DEPS
                 $<TARGET_OBJECTS:qplcore_${PLATFORM_ID}>)

     target_include_directories(qplcore_${PLATFORM_ID}
                                PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>
                                PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/compression/include>
                                PRIVATE $<TARGET_PROPERTY:isal,INTERFACE_INCLUDE_DIRECTORIES>)

     set_target_properties(qplcore_${PLATFORM_ID} PROPERTIES
                           $<$<C_COMPILER_ID:MSVC>:C_STANDARD 18>
                           $<$<C_COMPILER_ID:GNU,Clang>:C_STANDARD 17>
                           C_VISIBILITY_PRESET hidden)

     if (WIN32)
     target_compile_options(qplcore_${PLATFORM_ID}
                            PRIVATE "$<$<CONFIG:Debug>:>"
                            PRIVATE ${QPL_WINDOWS_TOOLCHAIN_REQUIRED_FLAGS}
                            PRIVATE "$<$<CONFIG:Release>:-O2>")
     else ()
     target_compile_options(qplcore_${PLATFORM_ID}
                            PRIVATE ${QPL_LINUX_TOOLCHAIN_REQUIRED_FLAGS}
                            PRIVATE "$<$<CONFIG:Debug>:>"
                            PRIVATE "$<$<CONFIG:Release>:-O3;-U_FORTIFY_SOURCE;-D_FORTIFY_SOURCE=2>")
     endif ()

     # Set specific compiler options and/or definitions based on a platform
     if (${PLATFORM_ID} MATCHES "avx512")
          target_compile_definitions(qplcore_${PLATFORM_ID} PRIVATE PLATFORM=2)

          if (WIN32)
          target_compile_options(qplcore_${PLATFORM_ID}
                                 PRIVATE /arch:AVX512)
          else ()
          target_compile_options(qplcore_${PLATFORM_ID}
                                 PRIVATE -march=skylake-avx512)
          endif ()
     else() # Create default px library
          target_compile_definitions(qplcore_${PLATFORM_ID} PRIVATE PLATFORM=0)
     endif()

     target_link_libraries(qplcore_${PLATFORM_ID} isal)
endforeach()

#
# Create dispatcher between platforms and auto-generated wrappers
#
file(GLOB SW_DISPATCHER_SOURCES
     dispatcher/*.cpp)

add_library(qplcore_sw_dispatcher OBJECT
            ${SW_DISPATCHER_SOURCES})

set_property(GLOBAL APPEND PROPERTY QPL_LIB_DEPS
             $<TARGET_OBJECTS:qplcore_sw_dispatcher>)

target_include_directories(qplcore_sw_dispatcher
                          PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/dispatcher>)

# Generate kernel wrappers
generate_unpack_kernel_arrays(${CMAKE_CURRENT_BINARY_DIR} "${PLATFORMS_LIST}")

foreach(PLATFORM_ID IN LISTS PLATFORMS_LIST)
     file(GLOB GENERATED_${PLATFORM_ID}_TABLES_SRC ${CMAKE_CURRENT_BINARY_DIR}/generated/${PLATFORM_ID}_*.cpp)

     target_sources(qplcore_sw_dispatcher PRIVATE ${GENERATED_${PLATFORM_ID}_TABLES_SRC})

     # Set specific compiler options and/or definitions based on a platform
     if (${PLATFORM_ID} MATCHES "avx512")
          set_source_files_properties(${GENERATED_${PLATFORM_ID}_TABLES_SRC} PROPERTIES COMPILE_DEFINITIONS PLATFORM=2)
     else()
          set_source_files_properties(${GENERATED_${PLATFORM_ID}_TABLES_SRC} PROPERTIES COMPILE_DEFINITIONS PLATFORM=0)
     endif()

     target_include_directories(qplcore_sw_dispatcher
                                PUBLIC $<TARGET_PROPERTY:qplcore_${PLATFORM_ID},INTERFACE_INCLUDE_DIRECTORIES>)
endforeach()

set_target_properties(qplcore_sw_dispatcher PROPERTIES
                      CXX_STANDARD 17
                      C_VISIBILITY_PRESET hidden
                      CXX_VISIBILITY_PRESET hidden)

# w/a for build compatibility with ISAL codebase
target_compile_definitions(qplcore_sw_dispatcher PUBLIC -DQPL_LIB)

target_compile_options(qplcore_sw_dispatcher
        PRIVATE $<$<C_COMPILER_ID:GNU,Clang>:${QPL_LINUX_TOOLCHAIN_REQUIRED_FLAGS};
                                             $<$<CONFIG:Release>:-O3;-U_FORTIFY_SOURCE;-D_FORTIFY_SOURCE=2>>
        PRIVATE $<$<C_COMPILER_ID:GNU>:${QPL_LINUX_TOOLCHAIN_DYNAMIC_LIBRARY_FLAGS}>
        PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:${QPL_LINUX_TOOLCHAIN_CPP_EMBEDDED_FLAGS}>
        PRIVATE $<$<C_COMPILER_ID:MSVC>:${QPL_WINDOWS_TOOLCHAIN_REQUIRED_FLAGS};
                                        ${QPL_WINDOWS_TOOLCHAIN_DYNAMIC_LIBRARY_FLAGS}
                                        $<$<CONFIG:Release>:-O2>>
        PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:${QPL_WINDOWS_TOOLCHAIN_CPP_EMBEDDED_FLAGS}>)

if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_link_options(qplcore_sw_dispatcher PRIVATE ${QPL_LINUX_TOOLCHAIN_DYNAMIC_LIBRARY_FLAGS})
endif ()
