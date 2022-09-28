 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Advanced Filter Features
########################

The library implements a number of “advanced” features that augment or
modify the normal behavior of filter operations.


Omit Checksums / Aggregates Calculation
=======================================


If the flag ``QPL_FLAG_OMIT_CHECKSUMS`` is specified, then CRC/XOR
checksums are not calculated during filtering on software path.
``QPL_FLAG_OMIT_AGGREGATES`` has the same effect but omits aggregates
calculation. These flags do not directly affect the performance of the
hardware path by itself but can allow the library to do some
optimizations. See the :ref:`hw_path_optimizations_reference_link`
section for more information.


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


