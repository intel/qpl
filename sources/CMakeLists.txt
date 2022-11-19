# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================

cmake_minimum_required(VERSION 3.12.0 FATAL_ERROR)

# Core Layer
add_subdirectory(isal)
add_subdirectory(core-sw)
add_subdirectory(core-iaa)

# Business logic layer
add_subdirectory(middle-layer)

# User API layer
add_subdirectory(c_api)
