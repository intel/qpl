 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Huffman Tables
##############


Two formats can be applied for Huffman Tables (one for the compressor
and one for the decompressor) in Intel® Query Processing Library (Intel® QPL). 
The decompression Huffman Table is needed for certain advanced use cases. Normally, 
the user never needs to deal with it.

These structures are not public. The user cannot access or modify its
fields. Consider the following snippet:

.. code-block:: c

   // Allocate buffer for compression table
   uint8_t *comp_table_buffer_ptr = (uint8_t *)malloc(QPL_COMPRESSION_TABLE_SIZE);
   // Initialize buffer with zeros
   memset(comp_table_buffer_ptr, 0u, QPL_COMPRESSION_TABLE_SIZE);
   // Get pointer to the compression table
   qpl_compression_huffman_table *comp_table_ptr = (qpl_compression_huffman_table *)comp_table_buffer_ptr;

   // ...
   // Now, you can use compression table with Intel QPL, for example set it to qpl_job:

   qpl_job_ptr->compression_huffman_table = comp_table_ptr;
   // ...


The usage for decompression table is the same, except for pointer type
and buffer size constant (``qpl_decompression_huffman_table``, and
respectively ``QPL_DECOMPRESSION_TABLE_SIZE``).


Creating Compression Huffman Tables from Deflate Histogram
**********************************************************


The ``qpl_build_compression_table_from_statistics`` function is used to
build the compression huffman table from deflate tokens histograms.

.. code-block:: c

   status = qpl_build_compression_table_from_statistics();


Built table is guaranteed to be complete (in terms of that every deflate
token is assigned a value).


Huffman Triplets
****************

An array of the ``qpl_huffman_triplet`` objects is considered as a
“generic format” Huffman table. See doxygen code annotations for more
information about this structure.


Creating Compression and Decompression Huffman Tables from Huffman Triplets
***************************************************************************


To build Huffman tables from array of ``qpl_huffman_triplet``, use
functions:

-  ``qpl_triplets_to_compression_table()``
-  ``qpl_triplets_to_decompression_table()``


Converting Compression Table into Decompression Table
*****************************************************


Another way to build a decompression table is to use the function
``qpl_comp_to_decompression_table()``, which is capable of converting
compression table into decompression.
