 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _building_deflate_block:


Building Deflate Block
######################


To build a ``deflate_block``, use the function
``build_deflate_block()``. The function takes configured
``deflate_operation``, input iterators, and mini-block size as input.
The function outputs the ``deflate_block`` with the compressed stream.
See the following example:

.. code:: cpp

   #include <operations/compression/deflate_operation.hpp>

   // ...

   std::vector<uint8_t> source(/*...*/);

   auto result = build_deflate_block(qpl::deflate_operation,
                                   source.begin(),
                                   source.end(),
                                   qpl::mini_block_size_1k);

   // ...

..

   Note that the deflate_block does not support CANNED compression mode.


Random Access to bytes
**********************


To access one particular uncompressed byte, call
``deflate_block::operator[]`` with the index of an element in the
uncompressed stream. When accessing one particular uncompressed byte
only the mini-block containing that byte is decompressed and saved to
the internal buffer. All subsequent requests that do not go beyond this
mini-block work with the buffer and do not call a decompression until a
new block is decompressed.

**Note** that ``deflate_block`` is a read-only object and a subscript
operator returns only the copy of an element.

.. code:: cpp

   // ...

   auto deflate_block = qpl::build_deflate_block(/* ... */);

   for (size_t i = startIndex; i < endIndex; i++)
   {
       std::cout << deflate_block[i];
   }

   // ...


Advanced Options for the Deflate Block
**************************************


**Execution Path**

``execution_path`` for the ``deflate_block`` can be specified by the
appropriate template parameter of the ``qpl::build_deflate_block``. This
path is used for building the ``deflate_block`` and for future
``deflate_block::operator[]`` requests.

.. code:: cpp

   // ...

   auto deflate_block = qpl::build_deflate_block<qpl::HARDWARE>(/* ... */);

   // ...


**allocator_t**

You can provide a custom allocator for the ``deflate_block`` using the
``template<class> allocator_t`` template parameter.

.. code:: cpp

   // ...

   auto deflate_block = qpl::build_deflate_block<qpl::software, std::allocator>(/* ... */);

   // ...


Deflate Block Usage Examples
****************************


.. code:: cpp

   #include <operations/compression/deflate_operation.hpp>
   #include <results/deflate_block.hpp>

   // ...

   std::vector<uint8_t> source(/*...*/);
   std::vector<uint8_t> subset;

   auto result = qpl::build_deflate_block(qpl::deflate_operation,
                                          source.begin(),
                                          source.end(),
                                          qpl::MINI_BLOCK_SIZE_4k);

   std::cout << "Size of the compressed stream: " << result.compressedSize() << "\n";

   for (size_t i = startIndex; i < result.size(); i++)
   {
       subset.push_back(result[i]);
   }

   // ...
