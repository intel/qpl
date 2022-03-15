 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _c_job_structure_reference_link:


Job Structure
#############


The Intel® Query Processing Library (Intel® QPL) 
job structure contains three types of data:

1. Parameters defining the job to be done.

2. Results of the job performed.

3. Internal state.


Parameters
**********


.. doxygenstruct:: qpl_job
    :project: Intel QPL
    :members:


Basic/Common Fields
*******************

The input buffer is described by two fields: ``next_in_ptr`` and
``available_in``. The first points to the start of the input data, and the
second gives the length of the input data. If the job completes
successfully, the ``next_in_ptr`` field should be advanced by ``available_in``,
and the ``available_in`` field should be zero.

The output buffer is described by two similar fields: ``next_out_ptr`` and
``available_out``. Note that ``available_out`` is both an input and an output.
As an input, it describes the size of the output buffer (i.e. how many
bytes can be written into the buffer). As an output, it describes the
remaining number of bytes unused.

.. note::
    
    The above fields are updated with each job call in a sequence.
    To concatenate the result of a sequence of jobs into one buffer,
    initialize the output buffer before the first job call.

Two output fields: ``total_in`` and ``total_out`` return the cumulative number
of bytes read and written.

Enumerated field ``op`` defines the operation to be done.

The ``flags`` field is used for different bits to define flags that affect
the operation.

Many of the operations return a CRC32 and an XOR checksum. By default,
the CRC uses the polynomial *0x104c11db7* (used by DEFLATE and related
tools). If the flag ``QPL_FLAG_CRC32C`` is specified, then the polynomial
*0x11edc6f41* is used (used by iSCSI). The XOR checksum is just the XOR of
the data (represented as 16-bit words), which is the last byte padded by
*0x00* if the total length is odd.

The hardware cannot write partial bytes. When the output stream ends
with a partial byte, the byte is zero-padded to a byte boundary before
being written. The number of valid bits in the byte can be obtained from
the ``last_bit_offset`` field. On completion, this field contains the number
of bits written to the last byte mod 8. That is, this field contains the
number of bits written to the last byte. If all 8 bits were written, it
contains the value of 0. For an operation like decompression, which
generates a stream of bytes, this field should always have a value of
zero. For operations like filtering, which generate a stream of bits,
this value can be used to determine where the actual data ends.
