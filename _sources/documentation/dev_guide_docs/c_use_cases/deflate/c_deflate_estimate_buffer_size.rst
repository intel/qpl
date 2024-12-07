 .. ***************************************************************************
 .. * Copyright (C) 2024 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

.. _deflate_estimation_reference_link:

Estimating the Size of the Deflate Compression Buffer
#####################################################

In Intel® Query Processing Library (Intel® QPL), the user is required to
allocate an output buffer for the compression algorithm. The Intel® QPL
Compression Buffer Size Estimation API provides a reliable method for users to
estimate the size of the output buffer required for compression operations.

Compression Buffer Size Estimation API :c:func:`qpl_get_safe_deflate_compression_buffer_size`
returns the size of the output buffer required for deflate compression.
The API returns ``0`` if the source size exceeds the maximum supported size.

.. note::
    For multi-chunk compression, the user must call the API for each chunk of data separately.
    Refer to the corresponding multi-chunk examples for more details.

Limitations
***********

- The Intel® QPL Compression Buffer Size Estimation API only supports
  deflate compression and does not support Huffman-Only mode compression.

- Due to the limitations imposed by the Intel® In-Memory Analytics Accelerator (Intel® IAA) hardware
  , the maximum bit size supported by the Intel® QPL compression is 32 bits. To account for the
  additional compression overhead, the maximum input size supported by the
  Intel® QPL and this API is ``2^32 - 35`` bytes.

- The function does not include the additional bytes required for GZIP/ZLIB
  compatibility (i.e., when using the :c:macro:`QPL_FLAG_GZIP_MODE` or :c:macro:`QPL_FLAG_ZLIB_MODE`
  flags). Users must manually account for these additional bytes.
  For more details, see :ref:`additional overhead <deflate_estimation_additional_overhead>`.

Usage Example
*************

C++ Code Example
----------------

.. code-block:: cpp

    #include <qpl.h>
    #include <iostream>

    uint32_t source_size = 1024U;
    uint32_t compression_size;

    compression_size = qpl_get_safe_deflate_compression_buffer_size(source_size);
    if (compression_size == 0) {
        std::cout << "Invalid source size. Source size exceeds the maximum supported size.\n";
        return -1;
    }

    std::vector<uint8_t> destination(compression_size, 0);

    std::cout << "Source size: " << source_size << ", compression size: " << compression_size << ".\n";

C Code Example
--------------

.. code-block:: c

    #include <qpl.h>
    #include <stdio.h>

    uint32_t source_size = 1024U;
    uint32_t compression_size;

    compression_size = qpl_get_safe_deflate_compression_buffer_size(source_size);
    if (compression_size == 0) {
        printf("Invalid source size. Source size exceeds the maximum supported size.\n");
        return -1;
    }

    uint8_t *destination = (uint8_t *)malloc(compression_size);
    if (destination == NULL) {
        printf("Memory allocation failed.\n");
        return -1;
    }

    printf("Source size: %u, compression size: %u.\n", source_size, compression_size);

.. _deflate_estimation_additional_overhead:

Additional Overhead
*******************

The estimation provided by :c:func:`qpl_get_safe_deflate_compression_buffer_size` does not include
the additional bytes required for GZIP/ZLIB headers and trailers when using the :c:macro:`QPL_FLAG_GZIP_MODE` or :c:macro:`QPL_FLAG_ZLIB_MODE` flags.

Users must manually account for these additional bytes when dealing with compression jobs
that are flagged with either the :c:macro:`QPL_FLAG_FIRST` or :c:macro:`QPL_FLAG_LAST` flags.

Jobs marked with the :c:macro:`QPL_FLAG_FIRST` flag require additional bytes for headers.
Similarly, jobs marked with the :c:macro:`QPL_FLAG_LAST` flag require additional bytes for trailers.
Jobs that do not use either flag do not require additional bytes.

Below is a breakdown of the additional bytes required when using GZIP and ZLIB formats:

**GZIP compatible stream**

- ``10`` bytes for the GZIP header
- ``8`` bytes for the GZIP trailer

**ZLIB compatible stream**

- ``2`` bytes for the ZLIB header
- ``4`` bytes for the ZLIB trailer







