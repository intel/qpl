 .. ***************************************************************************
 .. * Copyright (C) 2023 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

Deflate Compression and Decompression
#####################################

The Intel® Query Processing Library (Intel® QPL) supports
DEFLATE (Deflate) lossless data compression file format
as specified in `RFC 1951 <https://www.rfc-editor.org/info/rfc1951>`_.

.. attention::

   In Intel QPL, compression is always done using a history buffer size of 4 KB.
   Decompression is supported for Deflate streams where the size of the
   history buffer is no more than 4 KB, otherwise :c:macro:`QPL_STS_BAD_DIST_ERR` code is
   returned.

For more details on various supported Deflate flavors, refer to
:ref:`the Deflate Compression <compressed_data_format_reference_link>` section.
For instructions on how the Deflate stream could be decompressed,
use :ref:`Deflate Decompression <decompress_settings_for_deflate_reference_link>`.

Additionally Intel QPL supports :ref:`compressing with a Dictionary <compressing_with_dictionary_reference_link>`,
:ref:`using Deflate with Indexing <deflate_with_indexing_reference_link>`,
and dividing :ref:`compression and decompression into multiple jobs <multiple_jobs_reference_link>`.

Intel QPL Deflate could be made compatible with
ZLIB Format (as specified in `RFC 1950 <https://www.rfc-editor.org/info/rfc1950>`_)
or GZIP (as specified in `RFC 1952 <https://www.rfc-editor.org/info/rfc1952>`_).
See the :ref:`Support of ZLIB and GZIP Formats <zlib_and_gzip_compatibility_reference_link>` section for more details.

Intel QPL provides an API to estimate the size of the output buffer required for
compression operations. For more details, refer to
:ref:`Estimating the Size of the Deflate Compression Buffer <deflate_estimation_reference_link>`.

.. toctree::
   :maxdepth: 1

   c_deflate_compression
   c_deflate_decompression
   c_deflate_multiple_jobs
   c_deflate_dictionary
   c_deflate_indexing
   c_deflate_zlib_gzip
   c_deflate_estimate_buffer_size