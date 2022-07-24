 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


expand_operation
################


Expand operation is another analytics operation that works with bit-mask
as an addition to usual input and output buffers. Since there is no way
to build this bit-mask for the expand operation, you should pass this
mask into the factory method, for example:


.. code:: cpp

   #include <qpl/cpp_api/operations/analytics/expand_operation.hpp>

   // ...
   const uint8_t *const mask = custom_method_to_build_mask();
   const uint32_t maskByteSize = custom_get_mask_size();

   auto operation = qpl::expand_operation(mask, mask_byte_size);
   // ...

Output vector width for ``qpl::expand_operation`` is the same as input
vector bit width. Therefore, you can specify the input vector width as
the last parameter for constructor, for example:

.. code:: cpp

   #include <qpl/cpp_api/operations/analytics/expand_operation.hpp>

   // ...
   const uint8_t *const mask = custom_method_to_build_mask();
   const uint32_t mask_byte_size = custom_get_mask_size();

   auto operation = qpl::expand_operation::builder(mask, mask_byte_size)
           .input_vector_width(3)
           .build();
   // ...
