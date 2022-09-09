# ==========================================================================
# Copyright (C) 2022 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==========================================================================


# Intel® Query Processing Library (Intel® QPL) Documentation Configuration file.


# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.

import os
import sys
import docutils
# sys.path.insert(0, os.path.abspath('.'))

# -- Project information -----------------------------------------------------

project = 'Intel® QPL'
copyright = '2022, Intel'
author = 'Intel'
release = 'v0.1.21'

# -- General configuration ---------------------------------------------------

extensions = [
    'breathe'
]
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []

# -- Breathe configuration -------------------------------------------------

DOXYGEN_OUTPUT = os.path.join(os.path.abspath(os.path.join(os.path.curdir, os.pardir)), 'build', 'doxygen_xml')

breathe_projects = {
    "Intel(R) Query Processing Library": DOXYGEN_OUTPUT,
    }
breathe_default_project = "Intel(R) Query Processing Library"

# Tell sphinx what the primary language being documented is.
# primary_domain = 'cpp'

# Tell sphinx what the pygments highlight language should be.
# highlight_language = 'cpp'

breathe_domain_by_extension = {
    "h" : "c",
}

# -- Options for HTML output -------------------------------------------------

html_theme = "sphinx_book_theme"

html_theme_options = {
    'repository_url': 'https://github.com/intel/qpl',
    'path_to_docs': 'doc/source',
    'use_issues_button': True,
    'use_edit_page_button': True,
    'repository_branch': 'develop',
    'extra_footer': '<p align="right"><a href="https://www.intel.com/content/www/us/en/privacy/intel-cookie-notice.html">Cookies</a></p>'
}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
# html_static_path = ['_static']
