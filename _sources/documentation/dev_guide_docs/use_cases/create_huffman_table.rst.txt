 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _creating_huffman_table_reference_link:


Creating Huffman Table
######################


Creating a Huffman table consists of two steps: 

- Collecting deflate statistics 
- Building a Huffman table using statistics from the previous step


Collecting Deflate Statistics
*****************************


Deflate statistics (or deflate histogram) represents metadata used for
compression and decompression. The function
``update_deflate_statistics`` processes given input data and updates the
deflate histogram:

.. code:: cpp

   #include <qplhl/operations/deflate_operation.hpp>
   // ...

   std::vector<uint8_t> source(1000, 5);

   qpl::deflate_histogram deflate_histogram; // Create empty deflate histogram

   compression_levels level = compression_levels::default_level; // Set compression level for algorithm, that collects statistics

   qpl::update_deflate_statistics<qpl::execution_path::software>(source.begin(), source.end(), deflate_histogram, level);

As a result, code example earlier will process ``source`` and store
statistics to ``deflate_histogram``.


Building Huffman Table
**********************


Two representations for Huffman Table are available: one for the use
with the ``deflate_operation``, and one for the use with the
``inflate_operation``. The inflate Huffman table is only needed for
certain advanced use cases, for example, canned mode compression.


Deflate Huffman table
=====================
                     

To create a deflate Huffman table, use the following function:

.. code:: cpp

   // Use the deflate histogram from the previous section
   auto deflate_huffman_table = qpl::make_deflate_table<qpl::execution_path::software>(histogram);
   auto deflate_huffman_table = qpl::make_deflate_table<qpl::execution_path::hardware>(histogram);
   auto deflate_huffman_table = qpl::make_deflate_table<qpl::execution_path::auto_detect>(histogram);


Inflate Huffman table
=====================
                     

To create a inflate Huffman table, use the following function:

.. code:: cpp

   // Use a deflate huffman table
   auto inflate_huffman_table = qpl::make_inflate_table<qpl::execution_path::software>(deflate_huffman_table);
   auto inflate_huffman_table = qpl::make_inflate_table<qpl::execution_path::hardware>(deflate_huffman_table);
   auto inflate_huffman_table = qpl::make_inflate_table<qpl::execution_path::auto_detect>(deflate_huffman_table);
