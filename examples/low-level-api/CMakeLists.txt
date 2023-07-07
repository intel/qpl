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

file(GLOB files "*_example.c*")
foreach(source_file ${files})
    # for each source file, remove path and extension, and prepend "ll_"
    get_filename_component(example_name ${source_file} NAME_WE)
    set(example_name "ll_${example_name}")

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
