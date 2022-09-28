 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Deflate Compression
###################


Job structure Settings
**********************

We introduce the basic settings of the job structure for Deflate compression jobs.
With small changes, users can substitute the job structure setting in the
:ref:`quick_start_reference_link` example with the following ones. It is
assumed that a single job is used (see :ref:`multiple_jobs_reference_link`).


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


Static Blocks
=============

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


The Huffman table object ``huffman_table`` can be allocated by
:c:func:`qpl_deflate_huffman_table_create` and initialized with
literal/length and distance histograms through
:c:func:`qpl_huffman_table_init_with_histogram`.
Refer to the :ref:`huffman-tables-api-label` section for more information.

If the stream is compressed with multiple jobs, then in the middle of the
stream, the application can specify a different Huffman table and use the
``QPL_FLAG_START_NEW_BLOCK`` flag. This will instruct the library to end the
current block and to start a new block with the new table. The
``QPL_FLAG_START_NEW_BLOCK`` is not needed on the first job; it is implied.

If the ``QPL_FLAG_START_NEW_BLOCK`` flag is not used, then the table
pointed to in the job structure must be the same as those used by the
previous job. When the tables are changed without indication to the
library, then the resulting bit-stream will not be valid.


Canned
======

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
literal/length and distance histograms through
:c:func:`qpl_huffman_table_init_with_histogram`.
Refer to the :ref:`huffman-tables-api-label` section for more information.


.. attention::
   The current implementation does not support canned compression
   across multiple jobs, thus canned compression jobs must specify
   the flags ``QPL_FLAG_FIRST | QPL_FLAG_LAST``.


.. warning::
   The compression mode cannot be changed across related jobs. For example,
   you cannot start a sequence of jobs using dynamic compression and then halfway
   through switch to static compression.


Compression Output Overflow
***************************

If the compressed output does not fit into the output buffer, the library
attempts to copy the input data into the output stream as a series
of stored blocks. When the stored blocks fit into the output buffer,
the library writes stored blocks to the output buffer and returns the
successful ``QPL_STS_OK`` status. In case when the stored blocks do not
fit, the library returns the ``QPL_STS_MORE_OUTPUT_NEEDED`` status and
the compression fails.

Users are suggested to use an output buffer whose size is slightly
larger than the size of the input buffer so that it can accommodate the
additional headers for stored blocks. This suggestion does not apply to canned
compression (see :ref:`deflate_compression_structure_canned_reference_link`).


Structure of Compressed Data
****************************

In this section, we discuss possible structures of the compressed data.


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

On the hardware path, the data from one job will be compressed as a single
dynamic block when the data is compressible, or as multiple stored blocks
when the data is incompressible. The data from one job may be compressed
as multiple dynamic blocks on the software path.


Static Blocks
=============

The structure of compressed data using static compression is similar to
what was described in :ref:`deflate_compression_structure_fixed_reference_link`,
except that there can be multiple static blocks between stored blocks
due to the change of a Huffman table using ``QPL_FLAG_START_NEW_BLOCK``.


.. _deflate_compression_structure_canned_reference_link:

Canned
======

Canned compression only writes the body of one static block to the output
buffer. If the input data is incompressible, even when the size of the
output buffer is enough for a stored block, the library returns the
``QPL_STS_MORE_OUTPUT_NEEDED`` status.


Compression Verification
************************

By default, the library will verify the correctness of the generated
compressed bit stream. The library decompresses the resulting bit
stream, and then checks that the CRC of the decompressed data matches
the CRC of the original data. If the user does not want to pay the
additional cost for verification, this can be turned off with the
``QPL_FLAG_OMIT_VERIFY`` flag.

.. note::
    Currently verification is not performed in case of ``Huffman only BE``.

.. warning::
    Compression verification on the software path now only works with
    indexing and data of size smaller than 32 KB in other modes.
