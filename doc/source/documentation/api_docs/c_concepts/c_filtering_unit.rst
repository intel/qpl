 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Filtering Unit
##############


The filtering unit performs an operation on a series of unsigned
integers, resulting in an output stream of integers. The bit-width of
these integers can vary between 1-32 bits.

Some operations process a single data stream, which comes from the
normal input fields in the job structure. Some of them operate on two
data streams. For these operations, the second data stream is specified
using the ``next_src2_ptr`` and ``available_src2`` fields. This will be referred
to as *source-2*, whereas the main input stream will be referred to as
*source-1*.

By default, the source-1 data is read directly from the specified input
buffer. If the ``QPL_FLAG_DECOMPRESS_ENABLE`` flag is given, then the data
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

num_input_elements
******************

In some cases, for example, for small bit-widths, the number of input
elements is ambiguous. To account for this, the number of input elements
must be specified in ``num_input_elements``. Processing will stop when these
input elements are processed.

If the input ends before these input elements are processed, then the
actual number of elements processed is implementation specific, and an
error status will be returned.

In most cases, ``num_input_elements`` reflects the number of elements in
source-1. For the ``qpl_expand`` operation, it reflects the number of elements
in source-2.


Aggregates
**********


The filter unit also computes various summaries or aggregates of the
data, which are returned in fields in the job structure. The meaning of
these values varies slightly depending on whether the output is
nominally a bit vector or an array:


+--------------------+----------------------------+--------------------+
| Field              | Bit Vector Meaning         | Array Meaning      |
+====================+============================+====================+
| fir                | Index of first 1-bit in    | Min value over     |
| st_index_min_value | output                     | output             |
+--------------------+----------------------------+--------------------+
| la                 | Index of last 1-bit in     | Max value over     |
| st_index_max_value | output                     | output             |
+--------------------+----------------------------+--------------------+
| sum_value          | Number of 1-bits in output | Sum of output      |
|                    |                            | values             |
+--------------------+----------------------------+--------------------+


Considering a bit vector output (doing the ``qpl_scan`` operation), these values
can tell the software how sparse the result is, as well as where one
should start/end walking through the results to find all the 1's.


.. note::
    
    The meaning of the aggregates is not changed by the output
    modification. If the output is nominally a bit vector, and due to the
    output modification, the output actually contains 32-bit indices, the
    aggregates still reflect the bit vector values.


Advanced Filter Features
************************


The library implements a number of “advanced” features that augment or
modify the normal behavior of filter operations.


Omit Checksums / Aggregates Calculation
=======================================


If the flag ``QPL_FLAG_OMIT_CHECKSUMS`` is specified, then CRC/XOR
checksums are not calculated during filtering on software path.
``QPL_FLAG_OMIT_AGGREGATES`` has the same effect but omits aggregates
calculation. These flags do not directly affect the performance of the
hardware path by itself but can allow the library to do some
optimizations. See the :ref:`Hardware Path Optimizations 
<c_hw_path_optimization_reference_link>` section for more
information.


Invert_output
=============


Filter operation with the flag ``QPL_FLAG_INV_OUT`` specified will result in
inverse output in case of a bit-vector-like output. Omitting this flag
will not produce inverse output.

For example, doing a ``SCAN_LT`` with ``INV_OUT`` is the same as doing a
``SCAN_GE``. Similarly, ``SCAN_NOT_RANGE`` with ``INV_OUT`` is the same as
``SCAN_RANGE``.


initial_output_index
====================


When using output modification with a bit-vector output, the indices of
the 1-bits are written to the output. By default, the index of the first
bit is 0. By setting the ``initial_output_index`` field to a non-zero value,
the index of the first bit can be set.

This could be used, for example, if a column of data was broken into
groups of 1,000,000. The first such group could be processed with an
initial index of 0, the second group with an initial index of 1,000,000,
the third group with an initial index of 2,000,000, etc.

**Note**: This field only affects the indices written for the
modification of the bit vector output. It does not affect the indices
used for the ``qpl_extract`` operation. Those always start at 0 for the first
element. 

**Note**: This field does not affect operations that invoke
``src2`` - see the table :ref:`Operations <c_operations_table_reference_link>` 
for operations with number of input streams equal to 2. This statement means 
that ``initialOutputIndex`` field works for ``qpl_scan`` and ``qpl_find_unique`` 
operations only.


param_low & param_high for Dropping Low and High bits
=====================================================


The size of the set used in ``qpl_set_membership`` and ``qpl_find_unique`` is
limited. If the bit-width of the input is too large (i.e. the set would
be too large for the hardware to handle), in some cases the application
can use a smaller set by using only some bits from each element.
Effectively this “lumps together” several input values. Bits can be
dropped from the most-significant end by ``param_high`` and from the
least-significant end by ``param_low``.

The bit-width of the input needs to be greater than the sum of these two
fields. Then the size of the set is 2N, where ``N=(inputBitWidth -
param_high - param_low)``.

When this feature is used, each element has the appropriate number of
bits dropped from each end, and the remaining bits are used to index
into the set.


drop_initial_bytes
==================


In some cases, the filter input data may be compressed, and the
uncompressed data might contain some sort of header before the columnar
data begins. Assuming that this header is an integral number of bytes
(i.e. that the columnar data begins on a byte boundary of the
uncompressed data), the ``drop_initial_bytes`` field can be used to have the
filter unit skip over these bytes.


.. note:: 
    
    The ``drop_initial_bytes`` feature is supported only by
    1-source filter operations. Maximal ``drop_initial_bytes``
    feature value is 65,535 bytes.


This field could in theory be used if the input data is not compressed.
In this case, it is better to advance the input buffer pointer to the
start of the columnar data, and adjust the size accordingly.
