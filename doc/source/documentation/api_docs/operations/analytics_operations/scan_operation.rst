 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


scan_operation
##############


Scan operation represents a simple filtering function. Scan operation is
responsible for extracting elements that satisfy the given condition
(for example: less, greater, equal, not equal, etc.). Here is an example
of how to get the scan operation:

.. code:: cpp

   #include <qplhl/operations/scan_operation.hpp>

   // ...
   constexpr const uint32_t boundary = 7;

   auto scan_less_operation           = qpl::scan_operation(qpl::less, boundary);
   auto scan_less_equals_operation    = qpl::scan_operation::builder(qpl::less, boundary)
           .is_inclusive(true)
           .build();
   auto scan_greater_operation        = qpl::scan_operation(qpl::greater, boundary);
   auto scan_greater_equals_operation = qpl::scan_operation::builder(qpl::greater, boundary)
           .is_inclusive(true)
           .build();
   auto scan_equals_operation         = qpl::scan_operation(qpl::equals, boundary);
   auto scan_not_equals_operation     = qpl::scan_operation(qpl::not_equals, boundary);
   // ...
