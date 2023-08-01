# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Examples could be built and used independently of the library,
# following next steps:
# 0. Resolve all prerequisites
# 1. Change qpl to QPL::qpl in target_link_libraries
# 2. Point to existing installation via -DCMAKE_PREFIX_PATH
# 3. Add find_package(QPL CONFIG REQUIRED) at the beginning of this makefile

file(GLOB cpp_files "*_example.cpp")
foreach(source_file ${cpp_files})
    # for each source file, remove path and extension, and prepend "ll_"
    get_filename_component(example_name ${source_file} NAME_WE)
    set(example_name "ll_cpp_${example_name}")

    # create executable
    add_executable(${example_name} ${source_file})
    if (NOT ${source_file} MATCHES "with_data")
        target_link_libraries(${example_name} PRIVATE qpl)
    else()
        target_link_libraries(${example_name} PRIVATE qpl $<$<C_COMPILER_ID:GNU,Clang>:stdc++fs>)
    endif()
    set_target_properties(${example_name} PROPERTIES CXX_STANDARD 17)

    if(WIN32)
        target_compile_options(${example_name} PRIVATE "$<$<CONFIG:Release>:-O2>" /WX)
    else()
        target_compile_options(${example_name} PRIVATE "$<$<CONFIG:Release>:-O3>" -Werror)
    endif()
endforeach()

file(GLOB c_files "*_example.c")
foreach(c_source_file ${c_files})
    # for each source file, remove path and extension, and prepend "ll_"
    get_filename_component(example_name ${c_source_file} NAME_WE)
    set(example_name "ll_c_${example_name}")

    # create executable
    add_executable(${example_name} ${c_source_file})
    target_link_libraries(${example_name} PRIVATE qpl)
    if(NOT WIN32)
        # Using from C applications require C++ runtime library installed on the system, and adding -lstdc++
        target_link_libraries(${example_name} PRIVATE -lstdc++)
    endif()
   
    set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
    set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")
    set_target_properties(${example_name} PROPERTIES LINKER_LANGUAGE C)

    if(WIN32)
        target_compile_options(${example_name} PRIVATE "$<$<CONFIG:Release>:-O2>" /WX)
    else()
        target_compile_options(${example_name} PRIVATE "$<$<CONFIG:Release>:-O3>")
    endif()
endforeach()

