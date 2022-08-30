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
qpl_scan           1                       Bit Vector
qpl_set_membership 2                       Bit Vector
qpl_extract        1                       Array or Bit Vector
qpl_select         2                       Array or Bit Vector
qpl_expand_rle     2                       Array or Bit Vector
qpl_find_unique    1                       Bit Vector
qpl_expand         2                       Array or Bit Vector
================== ======================= ===================


.. toctree::
    :maxdepth: 4

    c_qpl_scan
    c_qpl_set_membership
    c_qpl_extract
    c_qpl_select
    c_qpl_expand_rle
    c_qpl_find_unique
    c_qpl_expand

    c_qplZeroCompress
    c_qpl_crc64

    c_qpl_gather_deflate_statistics
