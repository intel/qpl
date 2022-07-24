 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


copy_operation
##############


The copy operation performs a simple memory copy. The application
defines the input buffer and the output buffer. The output buffer must
be at least as large as the input buffer, and the two buffers cannot
overlap.

Example:

.. code:: cpp

   #include <qpl/cpp_api/operations/other/copy_operation.hpp>

   // ...
   auto copy_operation = qpl::copy_operation();
   // ...
