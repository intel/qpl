 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

.. _compression_styles_reference_link:


Compression Styles
##################


The library supports three types of compression styles: 

- ``dynamic_mode`` 
- ``static_mode`` 
- ``fixed_mode``

The examples of how to use them are described in the following sections.


.. _dynamic_blocks_reference_link:


Dynamic Blocks
**************


This is a default compression style that compresses data as a single
dynamic deflate block. This way gives the best compression ratio, but it
requires two passes through the hardware.

Example of usage:

.. code:: cpp

   auto compression_operation = qpl::compression_operation::builder()
           .compression_mode<qpl::compression_modes::dynamic_mode>()
           .build()


.. _fixed_block_reference_link:


Fixed Block
***********


The simplest compression style is to generate one large fixed block.

Example of usage:

.. code:: cpp

   auto compression_operation = qpl::compression_operation::builder()
           .compression_mode<qpl::compression_modes::fixed_mode>()
           .build()


.. _static_blocks_reference_link:


Static Blocks
*************


This compression style is similar to the Fixed block, but with the
possibility to specify the own Huffman table. This style gives the
application the ability to change the Huffman table in the middle of the
stream. If the stream is compressed with multiple tasks, the application
can specify a different Huffman table in the middle of the stream.


.. note::
        
    For the ``static_mode`` compression, you must
    specify the object with the Huffman table filled (see section
    :ref:`Creating Huffman Table <creating_huffman_table_reference_link>` 
    for more information).


Example of usage:

.. code:: cpp

   std::vector<uint8_t> source(1000, 5);

   auto path = qpl::execution_path::software;

   qpl::deflate_histogram deflate_histogram;

   qpl::update_deflate_statistics<path>(source.begin(), source.end(), deflate_histogram, compression_levels::default_level);

   auto deflate_huffman_table = qpl::make_deflate_table<path>(deflate_histogram);

   auto compression_operation = qpl::deflate_operation::builder()
           .compression_mode<qpl::compression_modes::static_mode>(deflate_huffman_table)
           .build()

   // ...
   // Execute and handle operation result
   //...


.. _canned_mode_reference_link:


Canned Compression
******************


Compression with canned mode is almost like static style compression,
but no deflate header is written to the output stream. So, to use it,
the valid deflate Huffman table should be set in the operation. This
approach is particularly good for compressing the series of small data,
so the compression ratio for every block is expected to be higher.

To decompress such stream, the valid inflate Huffman table should be set
in the operation. For better clarity, see canned mode example:

.. code:: cpp


   std::vector<uint8_t> source(1000, 5);

   auto path = qpl::execution_path::software;

   qpl::deflate_histogram deflate_histogram;

   qpl::update_deflate_statistics<path>(source.begin(), source.end(), deflate_histogram, compression_levels::default_level);

   auto deflate_huffman_table = qpl::make_deflate_table<path>(deflate_histogram);

   auto compression_operation = qpl::deflate_operation::builder()
           .compression_mode<qpl::compression_modes::canned_mode>(deflate_huffman_table)
           .build()

   // ...
   // Execute and handle compression operation result
   // ...

   auto inflate_huffman_table = qpl::make_inflate_table<path>(deflate_huffman_table);

   auto decompression_operation = qpl::inflate_operation::builder()
           .compression_mode<qpl::compression_modes::canned_mode>(inflate_huffman_table)
           .build()

   // ...
   // Execute and handle decompression operation result
   // ...
