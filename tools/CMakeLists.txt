# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Intel® Query Processing Library (Intel® QPL)
# Build system

project(tests LANGUAGES CXX)

if (LIB_FUZZING_ENGINE)
    add_subdirectory(tests/fuzzing)
else ()
    add_subdirectory(ref)
    add_subdirectory(utils)
    add_subdirectory(third-party/google-test EXCLUDE_FROM_ALL)
    add_subdirectory(tests)
endif ()

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/configs
                  TYPE             BIN
                  COMPONENT        RUNTIME)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/scripts
                  TYPE             BIN
                  COMPONENT        RUNTIME)

