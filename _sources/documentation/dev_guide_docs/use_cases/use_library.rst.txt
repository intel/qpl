 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Library Usage Scenarios
#######################


Currently, Intel® Query Processing Library (Intel® QPL) 
high-level API contains limited functionality. See the 
detailed description of all :ref:`included
operations <operations_reference_link>`.

The library supports several ways to create an operation:

-  :ref:`Basic usage <basic_operation_creation_reference_link>` (using default parameters)
-  :ref:`Advanced usage <advance_operation_creation_reference_link>` (advanced operation configuration)

To perform the operation, call the ``qpl::execute()`` method by
passing the operation, input, and output streams to the parameters.
The input data can be transferred in two ways, depending on the input
data type:


.. code:: cpp

   // Iterators as input data
   qpl::execute(deflate_operation, source.begin(), source.end(), destination.begin(), destination.end());

   // Containers as input data
   qpl::execute(deflate_operation, source, destination);


Method ``execute()`` returns class object ``execution_result``
containing status code and the number of elements produced in output
buffer. For more details visit :ref:`Execution_result overview 
<execution_result_overview_reference_link>`. Here is an example:

.. code:: cpp

   auto result = qpl::execute(deflate_operation, source, destination);

   result.handle([](uint32_t value) -> void
                 {
                     std::cout << "Result size: " << value;
                 }, [](uint32_t status) -> void
                 {
                     throw std::runtime_error("Error: Status code - " + std::to_string(status));
                 });


``qpl::execute()`` method also has a template parameter, which
specifies the allocator (default value ``std::allocator<uint8_t>``)
for the requested operation. If necessary, you can specify a custom 
allocator, for example:

.. code:: cpp

   auto result = qpl::execute<user::custom_allocator>(some_operation, input_container, output_container);


.. _basic_operation_creation_reference_link:


Basic Usage
***********


.. code:: cpp

   auto operation = qpl::some_operation(some_required_parameter);


.. _advance_operation_creation_reference_link:


Advanced Usage
**************


**Note:** This method is not applicable to operations such as
``compression_operation``

.. code:: cpp

   auto operation = qpl::some_operation::builder()
           .some_parameter(value)
           .build()


A list of available methods and their description can be found in
chapters for corresponding groups of operations in :ref:`Operations 
<operations_reference_link>`.


Usage Example
*************

The following example demonstrates how to use the deflate operation:

.. code:: cpp

   #include <qpl/cpp_api/operations/compression/deflate_operation.hpp>

   int main()
   {
       std::vector<uint8_t> source(1000, 5);
       std::vector<uint8_t> destination(500, 4);

       auto deflate_operation = qpl::deflate_operation();
       auto result            = qpl::execute(deflate_operation, source, destination);

       uint32_t result_size = 0;
       result.handle([&result_size](uint32_t value) -> void
                     {
                         result_size = value;
                     }, [](uint32_t status) -> void
                     {
                         throw std::runtime_error("Error: Status code - " + std::to_string(status));
                     });

       return 0;
   }
