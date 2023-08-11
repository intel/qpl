 .. ***************************************************************************
 .. * Copyright (C) 2023 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

Support of ZLIB and GZIP Formats
********************************

.. _zlib_and_gzip_compatibility_reference_link:

Compression and Decompression of ZLIB Stream
============================================

In order to be compatible with ZLIB compression or decompression,
Intel® Query Processing Library (Intel® QPL) needs to either wrap the output stream
(on a first and last job, when doing compression) and insert
the correct ZLIB header and trailer information, or unwrap
(on a first job, when doing decompression) the provided stream.

By default, Intel QPL generates a raw Deflate stream, so in order to add
or remove ZLIB header and trailer information, the flag :c:macro:`QPL_FLAG_ZLIB_MODE`
must be specified. Refer to `RFC 1950 <https://www.rfc-editor.org/info/rfc1950>`_ for more details
on ZLIB header and trailer.

.. warning::
    Intel QPL supports only 4 KB Window size for both `software_path` and `hardware_path` execution.
    If the stream was compressed with a larger Window size, an error is returned.

Compression and Decompression of GZIP Stream
============================================

Similarly, the GZIP standard defines an encapsulation layer
(consisting of a header and trailer) around a Deflate stream.
If the :c:macro:`QPL_FLAG_GZIP_MODE` flag is specified,
the library will add GZIP header and trailer information to the output stream
(when performing compression) or skip over the GZIP header on the first job
and then start parsing the Deflate stream (when performing decompression).
Refer to `RFC 1952 <https://www.rfc-editor.org/info/rfc1952>`_ for more details
on GZIP header and trailer.