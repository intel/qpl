 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

scan_range_operation
####################


Scan range operation is similar to the ``scan_operation`` but it
works with two boundaries (lower and upper):

.. code:: cpp

   #include <qplhl/operations/scan_range_operation.hpp>

   // ...
   constexpr const uint32_t lower_boundary = 48;
   constexpr const uint32_t upper_boundary = 58;

   auto in_range_operation     = qpl::scan_range_operation(lower_boundary, upper_boundary);
   auto out_of_range_operation = qpl::scan_range_operation::builder(lower_boundary, upper_boundary)
           .is_inclusive(false)
           .build();
   // ...

Family of ``scan`` operations can become significantly faster on
hardware path under some conditions described in the 
:ref:`Hardware Path Optimizations <hw_path_optimization_reference_link>` section.
