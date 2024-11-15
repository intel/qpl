 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Advanced Filter Features
########################

The library implements a number of “advanced” features that augment or
modify the normal behavior of filter operations.


Omit Checksums and Aggregates Calculation
=========================================


If the flag :c:macro:`QPL_FLAG_OMIT_CHECKSUMS` is specified, then CRC/XOR
checksums are not calculated during filtering on software path.
:c:macro:`QPL_FLAG_OMIT_AGGREGATES` has the same effect but omits aggregates
calculation. These flags do not directly affect the performance of the
hardware path by itself but can allow the library to do some
optimizations. See the :ref:`hw_path_optimizations_reference_link`
section for more information.


Invert Output
=============


Filter operation with the flag :c:macro:`QPL_FLAG_INV_OUT` specified will result in
inverse output in case of a bit-vector-like output. Omitting this flag
will not produce inverse output.

For example, doing a ``SCAN_LT`` with ``INV_OUT`` is the same as doing a
``SCAN_GE``. Similarly, ``SCAN_NOT_RANGE`` with ``INV_OUT`` is the same as
``SCAN_RANGE``.


Initial Output Index
====================


When using output modification with a bit-vector output, the indices of
the 1-bits are written to the output. By default, the index of the first
bit is 0. By setting the :c:member:`qpl_job.initial_output_index` field
to a non-zero value, the index of the first bit can be set.

This could be used, for example, if a column of data was broken into
groups of 1,000,000. The first such group could be processed with an
initial index of 0, the second group with an initial index of 1,000,000,
the third group with an initial index of 2,000,000, etc.

.. attention::

    This field only affects the indices written for the
    modification of the bit vector output. It does not affect the indices
    used for the :c:member:`qpl_operation.qpl_op_extract` operation.
    Those always start at 0 for the first element.

.. attention::

    This field does not affect operations that invoke
    ``src2`` (see the table :ref:`Operations <c_operations_table_reference_link>`
    for operations with number of input streams equal to 2). This statement means
    that ``initial_output_index`` field works for ``qpl_op_scan_*`` operation only.


Drop Initial Bytes
==================


In some cases, the filter input data may be compressed, and the
uncompressed data might contain some sort of header before the columnar
data begins. Assuming that this header is an integral number of bytes
(i.e. that the columnar data begins on a byte boundary of the
uncompressed data), the :c:member:`qpl_job.drop_initial_bytes` field can be used to have the
filter unit skip over these bytes.


.. attention::

    The :c:member:`qpl_job.drop_initial_bytes` feature is supported only by
    1-source filter operations. Maximal :c:member:`qpl_job.drop_initial_bytes`
    feature value is 65,535 bytes.


This field could in theory be used if the input data is not compressed.
In this case, it is better to advance the input buffer pointer to the
start of the columnar data, and adjust the size accordingly.



Force Array Output Modification
===============================

By default, when the output bit width of a filter operation is 1, the output
is returned as a bit vector. However, if the flag
:c:macro:`QPL_FLAG_FORCE_ARRAY_OUTPUT` is specified, the output is returned
as an array of a bit width defined by the :c:member:`qpl_job.output_bit_width`
field. Use this feature to receive the output as an array of a larger bit width
when the output is expected to have a bit width of 1 bit.

.. attention::

    This feature is only supported on the ``Hardware Path`` and Intel® In-Memory Analytics Accelerator 2.0 and later.

