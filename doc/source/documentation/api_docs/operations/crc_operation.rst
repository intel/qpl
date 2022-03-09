 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _crc_operation_reference_link:


crc_operation
#############


This operation computes an arbitrary CRC up to 64-bytes in size.

For this operation, ``bit_order`` supports only
``little_endian_packed_array`` and ``big_endian_packed_array``
parsers.

Example:

.. code:: cpp

   #include <qplhl/operations/other/crc_operation.hpp>

   // ...
   constexpr const uint64_t poly = 0x04C11DB700000000;

   auto crc_operation = qpl::crc_operation::builder(poly)
           .bit_order<qpl::parsers::little_endian_packed_array>()
           .is_inverse(false)
           .build();
   // ...
