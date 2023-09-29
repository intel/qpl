 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Aggregates
##########

The filter unit also computes various summaries or aggregates of the
data, which are returned in fields in the job structure. The meaning of
these values varies slightly depending on whether the output is
nominally a bit vector or an array:

+-------------------------------------------+----------------------------+--------------------+
| Field                                     | Bit Vector Meaning         | Array Meaning      |
+===========================================+============================+====================+
| :c:member:`qpl_job.first_index_min_value` | Index of first 1-bit in    | Min value over     |
|                                           | output                     | output             |
+-------------------------------------------+----------------------------+--------------------+
| :c:member:`qpl_job.last_index_max_value`  | Index of last 1-bit in     | Max value over     |
|                                           | output                     | output             |
+-------------------------------------------+----------------------------+--------------------+
| :c:member:`qpl_job.sum_value`             | Number of 1-bits in output | Sum of output      |
|                                           |                            | values             |
+-------------------------------------------+----------------------------+--------------------+

Considering a bit vector output, e.g., doing a scan operation, these values
can tell the software how sparse the result is, as well as where one
should start and end walking through the results to find all the 1's.

.. note::

    The meaning of the aggregates is not changed by the output
    modification (see :ref:`analytics_output_modifications_reference_link`).
    If the output is nominally a bit vector, and due to the
    output modification, the output actually contains 32-bit indices, the
    aggregates still reflect the bit vector values.
