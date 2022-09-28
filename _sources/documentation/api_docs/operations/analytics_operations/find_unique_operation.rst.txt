 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


find_unique_operation
#####################


Find unique operation builds a set that can be used for the
``qpl::set_membership_operation`` operation.

.. code:: cpp

   #include <qpl/cpp_api/operations/analytics/find_unique_operation.hpp>

   // ... 
   auto find_unique_operation = qpl::find_unique_operation();
   // ...
