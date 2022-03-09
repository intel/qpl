 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Low-level C API Developer Guide 
###############################


This document provides instructions on how to use the Intel® Query
Processing Library (Intel® QPL) C API (also called as Job API).

**Disclaimer**

The library exists in a prototype form and is being actively developed. This
means that some things described here may change in the future. However,
this is being released now in the expectation that it will be more
useful than nothing to people initially trying to use the library.


Header Files
************

The application only needs to include one header file: ``qpl/qpl.h``. It
includes the entire API definition.


.. toctree::
   :maxdepth: 4

   c_use_cases/c_use_library

   c_use_cases/c_optimize_hw_path
   c_use_cases/c_compress_dictionary
   c_use_cases/c_use_huffman_tables
   c_use_cases/c_use_indices
