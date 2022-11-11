 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

Welcome to Intel® QPL Documentation!
####################################


The Intel® Query Processing Library (Intel® QPL) provides interfaces
for a number of commonly used algorithms. Using this library enables you to
automatically tune your application to many generations of processors without
changing your application. The Intel® QPL provides high performance implementations
of data processing functions for existing hardware accelerator, and/or software path
in case if hardware accelerator is not available. Code written within the library
automatically takes advantage of available modern CPU capabilities. This behavior
can provide tremendous development and maintenance savings. Use the Intel® QPL
to improve performance of database, enterprise data, communications, and
scientific/technical applications.


.. attention::
   Intel QPL presently is in a prototype form. Public APIs described in this
   documentation may change at any time.


.. toctree::
   :maxdepth: 4
   :hidden:
   :caption: About

   documentation/introduction_docs/introduction.rst
   documentation/introduction_docs/legal.rst

.. toctree::
   :maxdepth: 4
   :hidden:
   :caption: Get Started

   documentation/get_started_docs/system_requirements.rst
   documentation/get_started_docs/installation.rst
   documentation/get_started_docs/quick_start.rst
   documentation/get_started_docs/code_examples.rst
   documentation/get_started_docs/testing.rst

.. toctree::
   :maxdepth: 4
   :hidden:
   :caption: Developer Guide

   Low-Level C API <documentation/dev_guide_docs/low_level_developer_guide.rst>
   High-Level C++ API <documentation/dev_guide_docs/high_level_developer_guide.rst>

.. toctree::
   :maxdepth: 4
   :hidden:
   :caption: Developer Reference

   Low-Level C API <documentation/api_docs/low_level_api.rst>
   High-Level C++ API <documentation/api_docs/high_level_api.rst>

.. toctree::
   :maxdepth: 4
   :hidden:
   :caption: Contributing

   documentation/contributing_docs/issue_reporting.rst
