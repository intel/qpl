 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

.. _analytics_output_modifications_reference_link:

Output Modification
###################

The output data can be modified from its nominal form into a modified
form. The nature of the modification depends on the nominal output (a
bit-vector, or an array); i.e. whether the bit-width of the output is 1
or more than 1.

The output modification is specified through an enumeration in the field
:c:member:`qpl_job.out_bit_width`. It can have logical values: 0, 8, 16, or 32. A value of 0
corresponds to no output modification. The other three values invoke
output modification.


.. _analytics_output_modifications_nominal_bit_vector_reference_link:

Output Modification for Nominal Bit Vector Output
*************************************************

In case of nominal bit vector output, when the bit vector is expected to
be sparse, it may be more efficient to represent the bit vector as a
list of indices of those bits that are 1 rather than as a dense bit
vector. In the worst case, if the bit vector is expected to have only a
single 1-bit, it would be better to get the index of that 1-bit instead
of the large array that is almost all 0.

In this case, if output modification is invoked, then the output is a
list of the indices of the 1-bits, where the indices are written as an
8-, 16-, or 32-bit value. If an actual index to be written is larger (in
bit-width) than the specified size, the operation terminates with an
error.


Output Modification for Nominal Array Output
********************************************

Here, the nominal output format is a packed array of integers. The
output modification is to extend the bit-width of the output to the
specified size, effectively “unpacking” them. This may make it easier
for software to process the results.

For example, if the output is nominally 11-bits wide, it could be
written as 16-bit integers. The specified size must be at least as wide
as the nominal sizes.

.. warning::
    User must not truncate an 11-bit wide value to 8-bits.

