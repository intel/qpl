 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


zero_compress_operation
#######################


Zero compress operation represents a zero compression functionality at
either a 16-bit or a 32-bit granularity.

This compression has nothing to do with ``deflate``.
This is a very light-weight compression scheme that can be
useful in certain circumstances.

Here is an example:

.. code:: cpp

   #include <qplhl/operations/compression/zero_compress_operation.hpp>

   // ...
   auto zero_compress_16_operation   = qpl::zero_compress_operation(qpl::zero_input_format::word_16_bit);
   auto zero_compress_32_operation   = qpl::zero_compress_operation(qpl::zero_input_format::word_32_bit);
   // ...
