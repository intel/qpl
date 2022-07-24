 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


set_membership_operation
########################


Set membership operation leaves in destination buffer just an element
from the given set. Set membership operation is another one analytics
operation that works with bit-mask as an addition to usual input and
output buffers.


.. code:: cpp

   #include <qpl/cpp_api/operations/analytics/set_membership_operation.hpp>

   // ...
   const uint8_t *const mask = custom_method_to_build_mask();
   const uint32_t mask_byte_size = custom_get_mask_size();
    
   auto operation = qpl::set_membership_operation(mask, mask_byte_size);
   // ...

``set_membership_operation`` can become significantly faster on hardware
path under some conditions described in the :ref:`Hardware Path Optimizations 
<hw_path_optimization_reference_link>`.
