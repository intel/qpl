 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

Analytics Operations
####################


This section describes the Intel® Query Processing Library (Intel® QPL) 
high-level API analytics operations.


Supported Methods
*****************


Common methods for analytics operations:

- ``output_vector_width(uint32_t value)``
- ``input_vector_width(uint32_t value)``
- ``parser(size_t number_of_input_elements)``


.. list-table::
   :header-rows: 1

   * - Operation
     - Specific methods
   * - ``scan_operation``
     - ``comparator(Comparators value)`` \
       ``boundary(uint32_t value)`` \
       ``is_inclusive(bool value)``
   * - ``scan_range_operation``
     - ``lower_boundary(uint32_t value)`` \ 
       ``upper_boundary(uint32_t value)`` \
       ``is_inclusive(bool value)``
   * - ``extract_operation``
     - ``lower_index(uint32_t value)`` \ 
       ``upper_index(uint32_t value)``
   * - ``find_unique_operation``
     - ``number_low_order_bits_to_ignore(uint32_t value)`` \
       ``number_high_order_bits_to_ignore(uint32_t value)`` 


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
| ``number_low_order_b            | Sets number low order bits to      |
| its_to_ignore(uint32_t value)`` | ignore                             |
+---------------------------------+------------------------------------+
| ``number_high_order_b           | Sets number high order bits to     |
| its_to_ignore(uint32_t value)`` | ignore                             |
+---------------------------------+------------------------------------+


.. toctree::
    :maxdepth: 4

    scan_operation
    scan_range_operation
    extract_operation
    find_unique_operation
