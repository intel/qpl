 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Deflate Decompression
#####################

.. _decompress_settings_for_deflate_reference_link:

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
    job->flags         = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_CANNED_MODE;
    job->huffman_table = huffman_table;


The Huffman table object ``huffman_table`` can be allocated by
:c:func:`qpl_deflate_huffman_table_create` and initialized with
literal/length and distance histograms through
:c:func:`qpl_huffman_table_init_with_histogram`.
Refer to the :ref:`huffman-tables-api-label` section for more information.


.. attention::
   The current implementation does not support canned decompression
   across multiple jobs, thus canned decompression jobs must specify
   the flags :c:macro:`QPL_FLAG_FIRST` | :c:macro:`QPL_FLAG_LAST`.


Getting :c:macro:`QPL_STS_MORE_OUTPUT_NEEDED` after Decompression
*****************************************************************

In general, the application may not know how much data a given compressed stream will decompress to.
If the output buffer fills up before the input is completely processed,
the library returns the status :c:macro:`QPL_STS_MORE_OUTPUT_NEEDED`, which means that the decompressor stopped early
to avoid overflowing the output buffer.
In that case, only partial data is written to the output, and internal state is now stored in the job structure.
This allows for the decompression operation to resume from the same place where it stopped.

The job structure would be updated as in the result of successful execution, meaning, for example, that
the size of the partial data written to the output is available via :c:member:`qpl_job.total_out`,
and input buffer parameters :c:member:`qpl_job.next_in_ptr` and :c:member:`qpl_job.available_in`
are updated by the library to reflect how many input bytes were processed successfully.

.. note::
    It is possible to have :c:member:`qpl_job.available_in` returned as `0` and get the
    :c:macro:`QPL_STS_MORE_OUTPUT_NEEDED` error.
    This case means that all the input bytes were processed,
    but the output buffer was smaller than required, and resubmission will happen with the
    input buffer of zero length.

In order to submit job for continuation:
  - The user must not reset input :c:member:`qpl_job.next_in_ptr` and :c:member:`qpl_job.available_in` parameters,
    as they were updated by the library and already reflect the correct offset.
  - If the job was marked previously with :c:macro:`QPL_FLAG_FIRST`, this flag must be unset.
  - The user must provide an output buffer for continuation and reset :c:member:`qpl_job.next_out_ptr`
    to its value and :c:member:`qpl_job.available_out` to its size.
    The user could choose, for instance, to allocate a bigger output buffer or to reuse the same memory.
    In the latter case, previously decompressed data must be stored somewhere else,
    as it would be overwritten otherwise.

.. attention::
    If returned values in :c:member:`qpl_job.available_in` and :c:member:`qpl_job.available_out` fields
    are the same when error :c:macro:`QPL_STS_MORE_OUTPUT_NEEDED` occurred,
    this means that the library was not able to make any forward progress.
    In this case, it is most probably that the output buffer size is too small,
    and the user must allocate a bigger chunk of memory for an output.

Checksums
*********

Decompression jobs return CRC-32 and 16-bit XOR checksums of the decompressed
data in :c:member:`qpl_job.crc` and :c:member:`qpl_job.xor_checksum`.
By default, the CRC-32 uses the polynomial ``0x104c11db7``, which follows ITU-T
Recommendation V.42. If the flag :c:macro:`QPL_FLAG_CRC32C` is specified in
:c:member:`qpl_job.flags`, then the polynomial ``0x11edc6f41`` is used,
which follows RFC 3720. To compute the 16-bit XOR checksum, the data is
treated as 16-bit words. If the data has an odd number of bytes,
the final byte is zero-extended to 16 bits.

.. warning::
    The implementation of :c:macro:`QPL_FLAG_CRC32C` is in progress.

