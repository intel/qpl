 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

.. _scan_operation_reference_link:

Scan
####

The scan operation (or :c:member:`qpl_operation.qpl_op_scan`)
outputs a bit-vector with the 1-bits correspond to
input elements that satisfy a numerical relationship. I.e. it can search
for elements that are EQ, NE, GT (etc.) a specified value, or for those
values that fall within an inclusive range.

The range is specified with the job fields:
:c:member:`qpl_job.param_low` and :c:member:`qpl_job.param_high`.

The operation will look for those values X that satisfy the following
conditions:

===============================================  ================================
Operation                                        Condition
===============================================  ================================
:c:member:`qpl_operation.qpl_op_scan_eq`         X = param_low
:c:member:`qpl_operation.qpl_op_scan_ne`         X ≠ param_low
:c:member:`qpl_operation.qpl_op_scan_lt`         X < param_low
:c:member:`qpl_operation.qpl_op_scan_le`         X ≤ param_low
:c:member:`qpl_operation.qpl_op_scan_gt`         X > param_low
:c:member:`qpl_operation.qpl_op_scan_ge`         X ≥ param_low
:c:member:`qpl_operation.qpl_op_scan_range`      param_low ≤ X AND X ≤ param_high
:c:member:`qpl_operation.qpl_op_scan_not_range`  X < param_low OR X > param_high
===============================================  ================================

The number of output bits (i.e. the number of output elements)
is the same as the number of input elements.
