 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _c_filter_and_other_operations_reference_link:

Filter and Other Operations
###########################

The filtering unit performs an operation on a series of unsigned
integers, resulting in an output stream of integers. The bit-width of
these integers can vary between 1-32 bits.

Some operations process a single data stream, which comes from the
normal input fields in the job structure. Some of them operate on two
data streams. For these operations, the second data stream is specified
using the :c:member:`qpl_job.next_src2_ptr` and
:c:member:`qpl_job.available_src2` fields. This will be referred
to as *source-2*, whereas the main input stream will be referred to as
*source-1*.

By default, the source-1 data is read directly from the specified input
buffer. If the :c:macro:`QPL_FLAG_DECOMPRESS_ENABLE` flag is given, then the data
from the input buffer is firstly decompressed, and then the decompressed
data is sent to the filter unit.

Each filtering job is independent. Filtering job does not support
processing a large stream as a series of smaller jobs, as compression
and decompression jobs do. Thus, each filtering job is considered
implicitly both a FIRST and a LAST job. That is, for a filtering
operation, it is as if the flags specified both FIRST and LAST.

If the bit-width of an array is 1, then the array will be referred to as
a bit-vector. That is, a *bit-vector* is just an array whose width
happens to be 1. If the bit-width is more than 1, it is called an
*array*.

The output can be written as a packed little-endian array or as a packed
big-endian array.

In some cases, for example, for small bit-widths, the number of input
elements is ambiguous. To account for this, the number of input elements
must be specified in :c:member:`qpl_job.num_input_elements`.
Processing will stop when these input elements are processed.

If the input ends before these input elements are processed, then the
actual number of elements processed is implementation specific, and an
error status will be returned.

In most cases, :c:member:`qpl_job.num_input_elements` reflects the number
of elements in source-1. The exception is that for the expand operation,
it reflects the number of elements in source-2.


.. toctree::
   :maxdepth: 1

   c_operations_op
   c_operations_output_modification
   c_operations_aggregates
   c_operations_parsers
   c_operations_hw_path_optimization


