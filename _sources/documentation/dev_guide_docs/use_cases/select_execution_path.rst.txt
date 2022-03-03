 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Selecting Execution Path
########################


To select the execution path, specify a suitable template parameter
for execution. By default, this is Software. This feature is available 
for both operations and chains. See the following examples:

.. code:: cpp

   // ...

   std::vector<uint8_t> source(/*...*/);
   std::vector<uint8_t> destination(/*...*/);

   auto chain = qpl::scan_range_operation(48, 58) |
                qpl::select_operation();

   auto result = qpl::execute<qpl::hardware>(chain, source, destination);

   // ...

.. code:: cpp

   // ...

   std::vector<uint8_t> source("..."); 
   std::vector<uint8_t> destination("...");

   auto operation = qpl::deflate_operation();

   auto result = qpl::execute<qpl::hardware>(operation, source, destination);

   // ...

Two models are currently available:

-  ``qpl::hardware`` Execution on hardware
-  ``qpl::software`` Execution on the software library
