 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

.. _expand_operation_reference_link:

Expand
######

The expand operation (or :c:member:`qpl_operation.qpl_op_expand`) is
in some sense the inverse of the :c:member:`qpl_operation.qpl_op_select`
operation (see :ref:`select_operation_reference_link` for more information).
This also reads a bit vector from ``source-2``.
If you take the output of expand and perform a select operation on it
(with the same bit vector as ``source-2``),
then you get back the same data as the original ``source-1``.

.. note::

  The output of expand may not be in a format readable by the select operation
  (for example, in the case mentioned in :ref:`analytics_output_modifications_nominal_bit_vector_reference_link`)

In this operation, the bit-width of the output is the same as the bit
width of ``source-1`` (unless explicitly changed by :c:member:`qpl_job.out_bit_width`)
but the number of output elements is equal to the number of input elements on
``source-2``. So for this operation, the job field :c:member:`qpl_job.num_input_elements`
actually contains the number of elements in ``source-2`` rather than ``source-1``.

Each 0-bit from ``source-2`` writes a zero to the output. Each 1-bit writes
the next entry from ``source-1``.
