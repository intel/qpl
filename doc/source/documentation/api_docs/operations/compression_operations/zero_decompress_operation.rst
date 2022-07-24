 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


zero_decompress_operation
#########################


Zero decompress operation represents a zero compression functionality at
either a 16-bit or a 32-bit granularity.

Here is an example:

.. code:: cpp

   #include <qpl/cpp_api/operations/compression/zero_decompress_operation.hpp>

   // ...
   auto zero_decompress_16_operation = qpl::zero_decompress_operation(qpl::zero_input_format::word_16_bit);
   auto zero_decompress_32_operation = qpl::zero_decompress_operation(qpl::zero_input_format::word_32_bit);
   // ...
