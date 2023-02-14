 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

.. _extract_operation_reference_link:

Extract
#######

The extract operation (or :c:member:`qpl_operation.qpl_op_extract`)
outputs input elements whose indices (starting at 0) fall within the inclusive
range defined by :c:member:`qpl_job.param_low` and :c:member:`qpl_job.param_high`.
So if the bit width of the output is the same as the bit width of the
input, then the number of output elements should be ``(param_high - param_low + 1)``.

