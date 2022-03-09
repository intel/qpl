 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


inflate_operation
#################


Inflate operation represents a decompression functionality that uses
the inflate decompression algorithm. Currently,
``qpl::inflate_operation`` is not capable of dynamic detection of GZIP
mode, so GZIP mode should be explicitly set in factory method (GZIP
mode is turned off by default).

Here is an example of how to get the inflate operation:

.. code:: cpp

   #include <qplhl/operations/inflate_operation.hpp>

   // ...
   auto inflate_operation      = qpl::inflate_operation();
   auto inflate_gzip_operation = qpl::inflate_operation::builder()
           .gzip_mode(true)
           .build();
   // ...

To use the inflate operation in the canned mode, set the mode clearly
and inflate Huffman table, like in the example:

.. code:: cpp

   #include <qplhl/operations/inflate_operation.hpp>

   // ...
   auto inflate_operation = qpl::inflate_operation::builder()
           .compression_mode<qpl::compression_modes::canned_mode>(inflate_table)
           .build();
   // ...
