 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Deflate Decompression
#####################


Job Structure Settings
**********************


Fixed, Dynamic, and Static Blocks
=================================

Assuming that a single job is used (see :ref:`multiple_jobs_reference_link`),
the job structure for fixed, dynamic, and static decompression all can be
set using:

.. code-block:: c

    job->op            = qpl_op_decompress;
    job->next_in_ptr   = <input_buffer>;
    job->available_in  = <input_size>;
    job->next_out_ptr  = <output_buffer>;
    job->available_out = <output_size>;
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST;


Canned
======

The job structure for canned decompression can be set using:

.. code-block:: c

    job->op            = qpl_op_compress;
    job->next_in_ptr   = <input_buffer>;
    job->available_in  = <input_size>;
    job->next_out_ptr  = <output_buffer>;
    job->available_out = <output_size>;
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE | QPL_FLAG_NO_BUFFERING | QPL_FLAG_RND_ACCESS;
    job->huffman_table = huffman_table;


The Huffman table object ``huffman_table`` can be allocated by
:c:func:`qpl_deflate_huffman_table_create` and initialized with
literal/length and distance histograms through
:c:func:`qpl_huffman_table_init_with_histogram`.
Refer to the :ref:`huffman-tables-api-label` section for more information.


.. attention::
   The current implementation does not support canned decompression
   across multiple jobs, thus canned decompression jobs must specify
   the flags ``QPL_FLAG_FIRST | QPL_FLAG_LAST``.


Decompression "Output Overflow"
*******************************

In general, the application may not know how much data a given
compressed stream will decompress to. If the output buffer filled up
before the input is completely processed, the decompression can
be resumed with a new output buffer to contain the remaining decompressed
output.

When the output buffer filled up before the input is completely processed,
the library returns the status ``QPL_MORE_OUTPUT_NEEDED``. In this case,
users can provide a new output buffer and reset :c:member:`qpl_job.next_out_ptr`
and :c:member:`qpl_job.available_out` accordingly. The input buffer
parameters :c:member:`qpl_job.next_in_ptr` and :c:member:`qpl_job.available_in`
do not need to be reset since they would have been updated.

Note that it is possible that the :c:member:`qpl_job.available_in` field
is 0, but the returned status is ``QPL_MORE_OUTPUT_NEEDED``. In this case,
the subsequent job will be submitted with a zero-length input buffer.


.. note::
    The situation is called euphemistically "output overflow"
    although no actual overflow occurs. It comes from the fact that
    the output buffer would have been overflowed if the decompressor did
    not stop early.


Checksums
*********

Decompression jobs return CRC-32 and 16-bit XOR checksums of the decompressed
data in :c:member:`qpl_job.crc` and :c:member:`qpl_job.xor_checksum`.
By default, the CRC-32 uses the polynomial ``0x104c11db7``, which follows ITU-T
Recommendation V.42. If the flag ``QPL_FLAG_CRC32C`` is specified in
:c:member:`qpl_job.flags`, then the polynomial ``0x11edc6f41`` is used,
which follows RFC 3720. To compute the 16-bit XOR checksum, the data is
treated as 16-bit words. If the data has an odd number of bytes,
the final byte is zero-extended to 16 bits.


Gzip Stream Decompression
*************************

The Gzip standard defines an encapsulation layer around a Deflate
stream. In particular, a Gzip stream consists of a Gzip header, a
Deflate stream, and a Gzip trailer. The decompressor assumes a raw
Deflate stream by default. If the ``QPL_FLAG_GZIP_MODE`` flag is specified,
then the library will skip over the Gzip header on the first job, and
then start parsing the Deflate stream.







