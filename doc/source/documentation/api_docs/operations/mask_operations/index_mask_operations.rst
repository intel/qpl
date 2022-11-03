 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Operations with Mask
####################


This section describes the Intel® Query Processing Library (Intel® QPL) 
high-level API operations with mask.

For all operations with the mask, you can leave the mask value as
``nullptr`` and the mask byte size equal to zero, and calculate the mask
during chaining. For more details, see the :ref:`Operations
chaining <operations_chaining_reference_link>` section.


Supported Methods
*****************


Common methods for operation with mask:

- ``mask(const uint8_t *mask, size_t maskByteLength)``
- ``output_vector_width(uint32_t value)``
- ``input_vector_width(uint32_t value)``
- ``parser(size_t number_of_input_elements)``


.. list-table::
   :header-rows: 1

   * - Operation
   * - ``select_operation``
   * - ``expand_operation``


Methods Description
*******************


+---------------------------------+------------------------------------+
| **Method**                      | **Description**                    |
+=================================+====================================+
| ``mask(const uint8_t            | Sets mask value and mask byte size |
| *mask, size_t maskByteLength)`` |                                    |
+---------------------------------+------------------------------------+
| ``output                        | Sets bit width of one element in   |
| _vector_width(uint32_t value)`` | output buffer                      |
+---------------------------------+------------------------------------+
| ``input                         | Sets bit width of one element in   |
| _vector_width(uint32_t value)`` | input buffer                       |
+---------------------------------+------------------------------------+
| ``                              | Sets operation compare filter      |
| comparator(Comparators value)`` |                                    |
+---------------------------------+------------------------------------+
| ``boundary(uint32_t value)``    | Sets the boundary for the          |
|                                 | analytics                          |
+---------------------------------+------------------------------------+
| ``is_inclusive(bool value)``    | Sets “in-range” or “not-in-range”  |
|                                 | filter operation                   |
+---------------------------------+------------------------------------+
| ``l                             | Sets lower boundary for scan       |
| ower_boundary(uint32_t value)`` | operations                         |
+---------------------------------+------------------------------------+
| ``u                             | Sets upper boundary for scan       |
| pper_boundary(uint32_t value)`` | operations                         |
+---------------------------------+------------------------------------+
| ``lower_index(uint32_t value)`` | Sets lower index for extract       |
|                                 | operations                         |
+---------------------------------+------------------------------------+
| ``upper_index(uint32_t value)`` | Sets upper index for extract       |
|                                 | operations                         |
+---------------------------------+------------------------------------+


.. toctree::
    :maxdepth: 4

    select_operation
    expand_operation
