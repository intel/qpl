 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


CRC-64
######

This operation computes an arbitrary CRC up to 64-bytes in size.

The polynomial is expressed in the normal or “not bit-reversed” form in
the job's :c:member:`qpl_job.crc64_poly` field. The most-significant bit of the polynomial
(which is always 1) is not expressed, and the next most-significant bit
is written to bit-63 of the field. That is, the polynomial is written to
the high end of the field. If the polynomial is for a 64-bit CRC, then
bit-0 would also be set. If the CRC is smaller, then the
least-significant bit set will be at a higher bit position.

For example, the CRC32 used by DEFLATE is defined by the 33-bit
polynomial 104c11db7.
To generate this CRC with the :c:member:`qpl_operation.qpl_op_crc64` operation,
one would set the :c:member:`qpl_job.crc64_poly` field to: ``04c11db700000000``. Since the
least-significant set bit is bit-32, then the size of the resulting CRC
would be 64-32 or 32.

By default, the data is viewed as Little Endian. If the
:c:macro:`QPL_FLAG_CRC64_BE` flag is used, the data is viewed as Big Endian.

When the data is LE, bit-0 of each byte is the least significant.
Additionally, bit-0 of byte-0 is the least significant bit of the
output. If the polynomial is smaller than 64-bits, the CRC is found in
the high end of the :c:member:`qpl_job.crc64` field.

When the data is BE, bit-0 of each byte is the most significant. Bit-7
of byte-7 of the result is then the least significant bit. If the
polynomial is smaller than 64-bits, the CRC is found in the low end of
the :c:member:`qpl_job.crc64` field.

This operation may not be linked, so the initial CRC is logically always
0, but many CRC invert (in a bitwise sense) the initial CRC and the
final CRC. To achieve this, the application should use the
:c:macro:`QPL_FLAG_CRC64_INV` flag.

.. note::
    The number of bits to be inverted is implied by the size of the polynomial
    (i.e. determined by where the least-significant set bit is found).

Here is some well-known CRC's, with corresponding settings and place to
find the result:

+----------+------------+------------------------------+-------------------------------+----------------+
| CRC      | crc64_poly | Bit Order                    | Invert CRC                    | crc64 (Output) |
+==========+============+==============================+===============================+================+
| CRC32    | 04C11D     | :c:macro:`QPL_FLAG_CRC64_BE` | :c:macro:`QPL_FLAG_CRC64_INV` | 00000000\      |
| (gzip)   | B700000000 |                              |                               |  **XXXXXXXX**  |
+----------+------------+------------------------------+-------------------------------+----------------+
| CRC32    | 04C11D     |                              | :c:macro:`QPL_FLAG_CRC64_INV` | **XXXXXXX      |
| (wimax)  | B700000000 |                              |                               | X**\ 00000000  |
+----------+------------+------------------------------+-------------------------------+----------------+
| T10DIF   | 8BB700     |                              |                               | **XXXX**\      |
|          | 0000000000 |                              |                               |  000000000000  |
+----------+------------+------------------------------+-------------------------------+----------------+
| CRC-     | 102100     | :c:macro:`QPL_FLAG_CRC64_BE` | :c:macro:`QPL_FLAG_CRC64_INV` | 000000000      |
| 16-CCITT | 0000000000 |                              |                               | 000\ **XXXX**  |
+----------+------------+------------------------------+-------------------------------+----------------+
