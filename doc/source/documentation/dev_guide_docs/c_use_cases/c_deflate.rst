 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Deflate Compression/Decompression
#################################


Compressed Data Format
**********************

The Deflate standard organizes the compressed data into a series of
"blocks". Each block is defined by one of the three types:

-  **Stored Block**: The input data is stored in an uncompressed form.
-  **Fixed Block**: The input data is compressed with a fixed Huffman table
   defined by the standard. The fixed Huffman table is usually suboptimal, but in
   this case the compressed stream doesn't need to include a set of Huffman codes.
-  **Dynamic Block**: The input data is compressed with a Huffman table that
   is optimized for that particular block. This results in a more
   efficient code, but requires two passes: one pass to analyze the
   data, and another pass to compress the data after the optimal
   Huffman table is generated.

Intel® Query Processing Library (Intel® QPL) includes the fourth type called
**Static Block**, where the input data is compressed with a user-defined Huffman
table which can differ from block to block. A static block, from the point of view
of Deflate standard, is a dynamic block. However, for static blocks, the Huffman
tables are determined ahead of time and do not depend on the input data. Note that
the compression can happen in one pass with a static block because the Huffman
Table is provided by the application. The application must make sure that the
Huffman table is appropriate for the data; otherwise, the compression ratio may
suffer.

Intel QPL provides another compression mode called **Canned**, which, like
static compression, compresses the input data with a user-defined Huffman table but
does not write the 3-bit Deflate header and Huffman table to the output stream.
This mode is useful when compressing many small buffers all with the same Huffman
table. It offers the compression speed of static compression, with a better
compression ratio (since there are no bits used for the header and Huffman table),
and a faster decompression speed (since the same Huffman table does not need to
be parsed multiple times).


.. _multiple_jobs_reference_link:

Compression/Decompression across Multiple Jobs
**********************************************

Compression and decompression of a large stream can be accomplished through a
sequence of jobs. For example, an application that was compressing a large file
might call Intel QPL repeatedly with 64 KB input buffers, until the end of the
file was reached. In this case, the first job is specified with the flag
``QPL_FLAG_FIRST`` in :c:member:`qpl_job.flags`, the middle jobs do not need
additional flags, and the last job is specified with the flag ``QPL_FLAG_LAST``.
If a single job is used to compress the stream, then both flags should be specified.
The same :c:struct:`qpl_job` object can be reused for multiple jobs. And 
:c:func:`qpl_init_job` should not be called in between jobs. (:c:func:`qpl_init_job`
should be called only after a new :c:struct:`qpl_job` object is allocated)

.. attention::
   The current implementation does not support canned compression/decompression
   across multiple jobs, thus canned compression/decompression jobs must specify
   the flags ``QPL_FLAG_FIRST | QPL_FLAG_LAST``.

Intel QPL does not save history state between jobs. This means that one compression
job will never reference data from an earlier job. This implies that submitting,
for example, 10 jobs of 1,000 bytes each will generate different output than
submitting one job with all 10,000 bytes. In general submitting very small buffers
will result in a worse compression ratio than submitting fewer large buffers.


.. toctree::
   :maxdepth: 4
   :hidden:

   Compression <c_deflate_compression>
   Decompression <c_deflate_decompression>
   Dictionary <c_deflate_dictionary>
   Indexing <c_deflate_indexing>
