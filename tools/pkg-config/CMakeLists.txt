# ==========================================================================
# Copyright (C) 2024 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

# Intel® Query Processing Library (Intel® QPL) pkg-config file generation.

if (UNIX AND DYNAMIC_LOADING_LIBACCEL_CONFIG AND "${QPL_LIBRARY_TYPE}" STREQUAL "SHARED")
    configure_file(qpl.pc.in qpl.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qpl.pc
              DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif ()
