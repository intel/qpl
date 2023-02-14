 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

.. _select_operation_reference_link:

Select
######

The select operation (or :c:member:`qpl_operation.qpl_op_select`) can be considered
as a generalization of the :c:member:`qpl_operation.qpl_op_extract` operation
(see :ref:`extract_operation_reference_link` for more information).
Here, source-2 is a bit-vector that must have
at least as many elements as source-1. Those source-1 items that correspond
to 1-bits in source-2 will be the output.
