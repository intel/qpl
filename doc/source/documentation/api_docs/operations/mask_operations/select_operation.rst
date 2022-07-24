 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


select_operation
################


Select operation works with bit-mask as an addition to usual input and
output buffers. This bit-mask should be specified while creating the
operation, for example:

.. code:: cpp

   #include <qpl/cpp_api/operations/analytics/select_operation.hpp>

   // ...
   const uint8_t *const mask = custom_method_to_build_mask();
   const uint32_t mask_byte_size = custom_get_mask_size();
    
   auto operation = qpl::select_operation(mask, mask_byte_size);
   // ...
