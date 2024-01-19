 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Job Structure
#############


The Intel® Query Processing Library (Intel® QPL)
job structure :c:struct:`qpl_job` contains three types of data:

1. Parameters defining the job to be done.

2. Results of the job performed.

3. Internal state.

Note that some fields in the job structure are of type 1 and 2 at the
same time, which means that they are both inputs and outputs. This will
become clear to users in the following sections.

Let's look at some fields in the job structure that are common across operations.

The field :c:member:`qpl_job.op` defines the operation to be done and has a
value in enum :c:enum:`qpl_operation`.

The input buffer for the operation is described by two fields:
:c:member:`qpl_job.next_in_ptr` and :c:member:`qpl_job.available_in`. The
first pointer points to the start of the input data, and the second gives the
length of the input data (or equivalently, the number of bytes of the input
data). If the job completes successfully, the :c:member:`qpl_job.next_in_ptr`
field should be advanced by the value of :c:member:`qpl_job.available_in`,
and the value of :c:member:`qpl_job.available_in` field should become zero.

The output buffer for the operation is described by two similar fields:
:c:member:`qpl_job.next_out_ptr` and :c:member:`qpl_job.available_out`.
When the job completes, the :c:member:`qpl_job.next_out_ptr` field should
be advanced by the number of bytes that was written, and the value of
:c:member:`qpl_job.available_out` field should describe the remaining number
of bytes unused in the output buffer.

The :c:member:`qpl_job.flags` field defines flags that affect the operation.
Each flag is represented by one of the 32 bits. To set the bit flags, use the
bitwise OR operator (``|``), for example,
:c:macro:`QPL_FLAG_FIRST` | :c:macro:`QPL_FLAG_LAST` | :c:macro:`QPL_FLAG_DYNAMIC_HUFFMAN`.
The setting of flags will be introduced in later sections.

Two output fields :c:member:`qpl_job.total_in` and :c:member:`qpl_job.total_out`
return the cumulative number of bytes read and written.

For applicable operations that return a CRC-32 and a 16-bit XOR checksum, the
results are contained in the fields :c:member:`qpl_job.crc` and
:c:member:`qpl_job.xor_checksum`. By default, the CRC-32 uses the polynomial
``0x104c11db7``, which follows ITU-T Recommendation V.42. If the flag
:c:macro:`QPL_FLAG_CRC32C` is specified in :c:member:`qpl_job.flags`, then the
polynomial ``0x11edc6f41`` is used, which follows RFC 3720. To compute the
16-bit XOR checksum, the data is treated as 16-bit words. If the data has an
odd number of bytes, the final byte is zero-extended to 16 bits.

.. warning::

    The implementation of :c:macro:`QPL_FLAG_CRC32C` is in progress.

For an operation that generates a stream of bytes, like decompression, the
output stream always ends at a byte boundary. For an operation that generates a
stream of bits, like scan, the stream is zero-padded to a byte boundary
before being written. The :c:member:`qpl_job.last_bit_offset` field indicates
where the data actually ends: when the output stream does not end at a byte
boundary, this field contains the number of bits written to the last byte.
When the output stream ends at a byte boundary, the value of this field is 0 (not 8).
The Huffman-only compression also writes the :c:member:`qpl_job.last_bit_offset` field.
You need to set the :c:member:`qpl_job.ignore_end_bits` field in the Huffman-only
decompression. For details on the Huffman-only and the Big Endian 16 format, which is
a special format it supports, see :ref:`huffman_only_reference_link`.

Internal state that could be used and re-used for various operations and holds
multiple internal representations is stored in :c:member:`qpl_job.data_ptr`
and shouldn't be touched by the user. Memory for internal state is allocated
on the user's side. A call to :c:func:`qpl_get_job_size` provides a size estimate,
and a call to :c:func:`qpl_init_job` ensures that internal buffers are aligned
and initialized properly.

.. attention::

    The action of both :c:func:`qpl_get_job_size` and :c:func:`qpl_init_job` depends on
    the provided execution path, so re-using the same job structure
    for different execution paths is not possible.

