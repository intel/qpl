 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Expand
######


The ``qpl_expand`` operation is in some sense the inverse of the ``qpl_select``
operation. This also reads a bit vector from source-2.

In this operation, the bit-width of the output is the same as the bit
width of source-1, but the number of output elements is equal to the
number of input elements on source-2. So for this operation, the job
field “num_input_elements” actually contains the number of elements in
source-2 rather than source-1.

Each 0-bit from source-2 writes a zero to the output. Each 1-bit writes
the next entry from source-1.

The ``qpl_expand`` is the opposite of the ``qpl_select``. If you take the
output of expand and perform a select operation on it (with the same bit vector as
source-2), then you get back the same data as the original source-1.
