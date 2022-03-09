 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


qpl_expand_rle
##############

The RLE Burst operation (``qpl_expand_rle``) replicates each element in
source-2 by the number of times based on the corresponding element in
source-1.

The bit width of source-1 must be 8, 16, or 32. The behavior of the
operation depends on the bit width of source-1.

The source-2 input is a packed array of unsigned integers of any width
between 1-32 bits. If the bit width of source-1 is 8 or 16 bits, then
source-2 should have the same number of elements as source-1. When the
bit width of source-1 is 32 bits, then source-2 should have one less
element than source-1.

By default, the output bit width is the same as the source-2 bit width.
The output may be modified by zero-extending each output value to 8, 16,
or 32 bits.

If the bit width of source-1 is 8 or 16, each element specifies the
number of times to replicate the corresponding element in source-2. When
the value is 0, the corresponding element is dropped.

If the bit width of source-1 is 32, each element of source-1 specifies
the cumulative number of elements in the output to that point. Thus, the
repetition count for each element is the difference between that element
and the next. In this case, the first element should always be 0. The
number of times each bit is replicated is limited to the range 0-65,535
inclusive. For example, with the first element equal to 0 and the second
element equal to 3, the first element of source-2 is replicated three
times in the output. As another example, the two primary inputs shown in
the following table are equivalent:

+-------------------+-------------+-----------------------+------------+
| Secondary Input   | Primary     |                       | Output     |
| (1 bit)           | input       |                       |            |
+===================+=============+=======================+============+
|                   | Length (8   | Cumulative Length (32 |            |
|                   | bits)       | bits)                 |            |
+-------------------+-------------+-----------------------+------------+
| 1                 | 2           | 0                     | 11         |
+-------------------+-------------+-----------------------+------------+
| 0                 | 4           | 2                     | 110000     |
+-------------------+-------------+-----------------------+------------+
| 1                 | 3           | 6                     | 110000111  |
+-------------------+-------------+-----------------------+------------+
| 0                 | 3           | 9                     | 11         |
|                   |             |                       | 0000111000 |
+-------------------+-------------+-----------------------+------------+
| 1                 | 2           | 12                    | 1100       |
|                   |             |                       | 0011100011 |
+-------------------+-------------+-----------------------+------------+
|                   |             | 14                    |            |
+-------------------+-------------+-----------------------+------------+

**Note** that in case of 32-bit/cumulative counts, the number of
elements field should be set to the number of source-1 elements, which
is one greater than the number of source-2 elements. In the previous
example, if the bit width of source-1 was less than 32, the number of
elements field would be set to 5. If the bit width was 32, it would be
set to 6.
