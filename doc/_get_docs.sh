#!/bin/bash
# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================


# Intel® Query Processing Library (Intel® QPL) documentation generation script.

set -ex

# This file is required for local build of Doxygen for Sphinx to use docs preview in VSCode
# because of that it is targeted to src, there is no way to use preview with other location

# Full path to the directory where Intel(R) QPL include folder is located, need also to strip it from Doxygen paths
export PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
export DOXYGEN_OUTPUT_DIR=$PROJECT_DIR/build/doxygen_html
export SPHINX_OUTPUT_DIR=$PROJECT_DIR/build/html
export SPHINX_SRC_DIR=$PROJECT_DIR/source

# Run Doxygen
cd $PROJECT_DIR && cd .. && doxygen Doxyfile

# Run Sphinx 
#  -W - turn Sphinx warnings into errors
#  -keep-going - generate full documentation even if errors encountered
#  -n check for broken links in documentation
#  -b - build output format
sphinx-build -W --keep-going -b html $SPHINX_SRC_DIR $SPHINX_OUTPUT_DIR
