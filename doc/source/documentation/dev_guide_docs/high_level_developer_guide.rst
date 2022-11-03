 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _developer_guide_high_level_reference_link:


Developer Guide for the High-Level C++ API
##########################################


This document provides quick introduction into usage of High Level API
for Intel® Query Processing Library (Intel® QPL). It describes general 
usage concepts.

.. attention::
   Intel QPL presently is in a prototype form. Public APIs described in this
   documentation may change at any time.


Header Files
************

Header files for Intel QPL high-level API 
are in ``include/qpl/cpp_api``. The application only needs to include one 
header file: ``qpl/qpl.hpp``. It includes the entire API definition.


.. toctree::
   :maxdepth: 4
   :hidden:

   use_cases/use_library
   use_cases/integrate_with_other_projects

   use_cases/select_execution_path
   use_cases/set_parser
   use_cases/create_huffman_table
   use_cases/use_deflate_block
   use_cases/use_execution_result
   use_cases/use_exceptions
   use_cases/use_compression_streams
   use_cases/build_custom_operation
   use_cases/chain_operations
   use_cases/optimize_hw_path


