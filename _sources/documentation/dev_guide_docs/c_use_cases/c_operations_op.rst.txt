 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

.. _c_operations_reference_link:

Operations
##########


.. _c_operations_table_reference_link:


================== ======================= ===================
Operation          Number of Input Streams Output Stream Type
================== ======================= ===================
Scan               1                       Bit Vector
Set membership     2                       Bit Vector
Extract            1                       Array or Bit Vector
Select             2                       Array or Bit Vector
RLE burst          2                       Array or Bit Vector
Find unique        1                       Bit Vector
Expand             2                       Array or Bit Vector
================== ======================= ===================


.. toctree::
   :maxdepth: 4
   :hidden:

   c_operations_op_scan
   c_operations_op_set_membership
   c_operations_op_extract
   c_operations_op_select
   c_operations_op_rle_burst
   c_operations_op_find_unique
   c_operations_op_expand
   c_operations_op_zero_compress
   c_operations_op_crc64

