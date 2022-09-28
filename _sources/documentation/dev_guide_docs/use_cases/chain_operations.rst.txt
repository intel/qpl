 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _operations_chaining_reference_link:


Chaining Operations
###################


One of the main features of Intel® Query Processing Library (Intel® QPL) 
is a capability to build custom chains of operations. For example, 
if you need to perform operations ``decompress -> filter -> extract``, 
use chaining functionality instead of simple operations abstraction level. 
To do this, include the header file called ``<qpl/cpp_api/chaining/operation_chain.hpp>`` 
and then you will be able to chain Intel QPL operations like this:

.. code:: cpp

   #include <qpl/cpp_api/chaining/operation_chain.hpp>
   #include <qpl/cpp_api/operations/compression/inflate_operation.hpp>
   #include <qpl/cpp_api/operations/analytics/scan_range_operation.hpp>
   #include <qpl/cpp_api/operations/analytics/select_operation.hpp>
   // ...
   constexpr const uint32_t lower_boundary = 48;
   constexpr const uint32_t upper_boundary = 58;

   auto chain = qpl::inflate_operation() |
                qpl::scan_range_operation(lower_boundary, upper_boundary) |
                qpl::select_operation();

   // ...


An overloaded operator ``|`` for Intel QPL operations constructs
specific object ``qpl::operation_chain``. Use of an object of this class
is the same as of a simple operation.


.. code:: cpp

   // ...
   std::vector source(/*...*/);
   std::vector destination(/*...*/);

   auto result = qpl::execute(chain, source, destination);
   // ...


If you need to create an ``qpl::operation_chain`` with custom allocator
(will be used for allocation of temporary buffers and result buffer),
pass additional template parameter to ``qpl::execute()`` function like
this:


.. code:: cpp

   // ...
   auto chain = qpl::inflate_operation() |
                qpl::scan_range_operation(lower_boundary, upper_boundary) |
                qpl::select_operation();

   auto result = qpl::execute<allocator_t>(chain, source, destination);
   // ...


After you chain your operation, you will not be able to use it anywhere
else.


merge Manipulator
*****************


For some chain use cases, you can merge decompression and analytics
operations into one. Such operation is able to perform both actions. To
do this, add ``qpl::merge`` to chain as shown in the following example:

.. code:: cpp

   #include <qpl/cpp_api/chaining/operation_chain.hpp>
   #include <qpl/cpp_api/chaining/merge_manipulator.hpp>
   #include <qpl/cpp_api/operations/compression/inflate_operation.hpp>
   #include <qpl/cpp_api/operations/analytics/scan_range_operation.hpp>
   // ...
   constexpr const uint32_t lower_boundary = 48;
   constexpr const uint32_t upper_boundary = 58;
   constexpr const uint32_t number_of_decompressed_elements = 1000;

   auto chain = qpl::inflate_operation() |
                qpl::merge(number_of_decompressed_elements) |
                qpl::scan_range_operation(lower_boundary, upper_boundary);
   // ...

The ``qpl::merge`` constructor takes number of elements that should be
processed after decompression as an argument.

``qpl::merge`` should be placed:

-  After decompression operation
-  Before analytics operation that doesn’t use mask

Adding ``qpl::select_operation`` and ``qpl::set_membership_operation``
to a chain right after merged operations makes no sense since the mask
for these operations is calculated during chain execution.


Chaining Limitations
********************


Intel QPL has some limitations for operations chaining. Here is the list
of such limitations:

-  ``qpl::find_unique_operation`` is always the last operation in an
   Intel QPL chain, since there is no operation that can produce
   something useful after it.
-  All allocations are performed in the first execution call of the
   chain (should not really affect the performance).
-  Even if you specify a custom builder for any analytics operation,
   ``qpl::operation_chain`` can change input vector width of any
   operation in the chain to be able to continue the execution.

In case if you discover some other limitations or you would
like to remove one of already known limitations - please, don not hesitate
to tell us about it.
