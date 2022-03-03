 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


extract_operation
#################


Extract operation performs an extraction from the source using two index
boundaries (for example, extract everything from position 4 to position
123). Here is an example of how to get the extract operation:

.. code:: cpp

   #include <qplhl/operations/extract_operation.hpp>

   // ...
   constexpr const uint32_t lower_index = 4;
   constexpr const uint32_t upper_index = 123;

   auto operation = qpl::extract_operation(lower_index, upper_index);
   // ...
