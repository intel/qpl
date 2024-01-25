 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

Deflate Compression
###################

.. _compressed_data_format_reference_link:

Supported Compressed Data Formats
*********************************

The Deflate standard organizes the compressed data into a series of
"blocks". Each block is defined by one of the three types:

-  **Stored Block**: The input data is stored in an uncompressed form.
-  **Fixed Block**: The input data is compressed with a fixed Huffman table
   defined by the standard. The fixed Huffman table is usually suboptimal, but in
   this case the compressed stream doesn't need to include a set of Huffman codes.
-  **Dynamic Block**: The input data is compressed with a Huffman table that
   is optimized for that particular block. This results in a more
   efficient code, but requires two passes: one pass to analyze the
   data and gather its statistics, with which an optimal Huffman table can be generated,
   and another pass to compress the data with the optimal Huffman table.

Intel® Query Processing Library (Intel® QPL) includes a fourth type called
**Static Block**, where the input data is compressed with a user-defined Huffman
table which can differ from block to block. A static block, from the point of view
of the Deflate standard, is a dynamic block. However, for static blocks, the Huffman
tables are determined ahead of time and do not depend on the input data. Note that
the compression can happen in one pass with a static block because the Huffman
Table is provided by the application. The application is responsible for providing
a suitable Huffman table for the data; otherwise, the compression ratio may be low.

Intel QPL provides another compression mode called **Canned**, which, like
static compression, compresses the input data with a user-defined Huffman table. But
it does not write the 3-bit Deflate header and the Huffman table to the output stream.
This mode is useful when compressing many small buffers all with the same Huffman
table. It offers the compression speed of static compression, with a better
compression ratio (since it saves the bits used for the header and the Huffman table),
and a faster decompression speed (since the same Huffman table does not need to
be parsed multiple times).

Job Structure Settings
**********************

This section introduces the basic settings of the job structure for Deflate compression jobs
using different available modes.
In the code snippets below, it is assumed that a single job is used, for
details on how to divide workload into multiple jobs submissions,
refer to :ref:`multiple_jobs_reference_link`.

.. warning::
   The compression mode cannot be changed across related jobs.
   For example, it is not supported to start a sequence of jobs using **Dynamic Compression**
   and then switch to **Static Compression** halfway through the sequence.

Fixed Blocks
============

The job structure for fixed compression can be set using:

.. code-block:: c

    job->op            = qpl_op_compress;
    job->level         = qpl_default_level;   // or qpl_high_level
    job->next_in_ptr   = <input_buffer>;
    job->available_in  = <input_size>;
    job->next_out_ptr  = <output_buffer>;
    job->available_out = <output_size>;
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;
    job->huffman_table = NULL;


Dynamic Blocks
==============

The job structure for dynamic compression can be set using:

.. code-block:: c

    job->op            = qpl_op_compress;
    job->level         = qpl_default_level;   // or qpl_high_level
    job->next_in_ptr   = <input_buffer>;
    job->available_in  = <input_size>;
    job->next_out_ptr  = <output_buffer>;
    job->available_out = <output_size>;
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN;
    job->huffman_table = NULL;


Static Blocks and Canned Mode
=============================

The job structure for static compression can be set using:

.. code-block:: c

    job->op            = qpl_op_compress;
    job->level         = qpl_default_level;   // or qpl_high_level
    job->next_in_ptr   = <input_buffer>;
    job->available_in  = <input_size>;
    job->next_out_ptr  = <output_buffer>;
    job->available_out = <output_size>;
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;
    job->huffman_table = huffman_table;


The job structure for canned compression can be set using:

.. code-block:: c

    job->op            = qpl_op_compress;
    job->level         = qpl_default_level;   // or qpl_high_level
    job->next_in_ptr   = <input_buffer>;
    job->available_in  = <input_size>;
    job->next_out_ptr  = <output_buffer>;
    job->available_out = <output_size>;
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE;
    job->huffman_table = huffman_table;

The Huffman table object ``huffman_table`` can be allocated by
:c:func:`qpl_deflate_huffman_table_create` and initialized with
literal/length and distance histograms through :c:func:`qpl_huffman_table_init_with_histogram`.
Refer to the :ref:`huffman-tables-api-label` section for more information.

If the stream is compressed with multiple jobs, then in the middle of the
stream, the application can specify a different Huffman table and use the
:c:macro:`QPL_FLAG_START_NEW_BLOCK` flag. This will instruct the library to end the
current block and to start a new block with the new table. The
:c:macro:`QPL_FLAG_START_NEW_BLOCK` is not needed on the first job (i.e., this flag
is implied when :c:macro:`QPL_FLAG_FIRST` is set).

If the :c:macro:`QPL_FLAG_START_NEW_BLOCK` flag is not used, then the table
pointed to in the job structure must be the same as those used by the
previous job. If the tables are changed without signaling the library with
the appropriate flags, the resulting bit-stream will not be valid.

.. attention::
   The current implementation does not support canned compression
   across multiple jobs, thus canned compression jobs must specify
   the flags :c:macro:`QPL_FLAG_FIRST` | :c:macro:`QPL_FLAG_LAST`.

Structure of Compressed Data
****************************

.. _deflate_compression_structure_fixed_reference_link:

Fixed Blocks
============

If a single job is used, compressible data will result in one large
fixed block, while incompressible data could lead to multiple
stored blocks.

If multiple jobs are used and all data are compressible, then the
result will be one large fixed block followed by a zero-length stored
block which is marked as the final block. When some of the jobs
contain incompressible data, the result will be a mixture of fixed
blocks, stored blocks, and a possible zero-length stored block
at the end. Note that there will be at most one fixed block between
stored blocks.

Dynamic Blocks
==============

On the ``hardware_path``, the data from one job will be compressed as a single
dynamic block when the data is compressible, or as multiple stored blocks
when the data is incompressible. The data from one job may be compressed
as multiple dynamic blocks on the software path.

Static Blocks
=============

The structure of compressed data using static compression is similar to
what was described in :ref:`deflate_compression_structure_fixed_reference_link`,
except that there can be multiple static blocks between stored blocks
due to the change of a Huffman table using :c:macro:`QPL_FLAG_START_NEW_BLOCK`.

.. _deflate_compression_structure_canned_reference_link:

Canned
======

Canned compression only writes the body of one static block to the output
buffer. If the input data is incompressible, even when the size of the
output buffer is enough for a stored block, the library returns the
:c:macro:`QPL_STS_MORE_OUTPUT_NEEDED` status.

Compression Output Overflow
***************************

If the compressed output does not fit into the output buffer (e.g., a bad Huffman table
is provided by the application, causing data to expand in a static compression),
the library attempts to copy the input data into the output stream as a series
of ``stored blocks``. When the ``stored blocks`` fit into the output buffer,
the library writes them to the output buffer and returns the
successful :c:macro:`QPL_STS_OK` status. If the stored blocks do not
fit, the library returns the :c:macro:`QPL_STS_MORE_OUTPUT_NEEDED` status and
the compression fails.

.. attention::
    Currently, Intel QPL does not have an API to return maximum compressed size for a given source size.
    To avoid getting :c:macro:`QPL_STS_MORE_OUTPUT_NEEDED` for incompressible source data, 
    users are suggested to use an output buffer with a size slightly larger than the size of 
    the input buffer so that it can accommodate the additional headers for stored blocks. 
    This suggestion does not apply to canned compression 
    (see :ref:`deflate_compression_structure_canned_reference_link`).

Compression Verification
************************

By default, the library will verify the correctness of the generated
compressed bit stream. The library decompresses the resulting bit
stream, and then checks that the CRC of the decompressed data matches
the CRC of the original data. If the user does not want to pay the
additional performance cost for verification, the step can be skipped
with the :c:macro:`QPL_FLAG_OMIT_VERIFY` flag.

.. attention::
    Currently verification is not performed for Huffman-only compression
    on the hardware path. For more information on Huffman-only, see
    :ref:`huffman_only_reference_link`.

.. warning::
    Currently compression verification on the software path only works with
    indexing and data of size smaller than 32 KB in other modes.
