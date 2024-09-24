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
This document serves as a guide for developers to understand and integrate the
API into their applications.

.. warning::
    The Intel® QPL Compression Buffer Size Estimation API only supports
    deflate compression and does not support Huffman-Only mode compression.

The Intel® QPL Compression Buffer Size Estimation API returns the size of the
output buffer required for deflate compression. The API returns 0 if the source
size exceeds the maximum supported size.

.. warning::
    Please note that the maximum input size supported by the Intel® QPL and
    this API is 2^32 - 35 bytes due to the limitations imposed by the
    Intel® In-Memory Analytics Accelerator (Intel® IAA) hardware. Refer to the
    `Limitations`_ section for more information.

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

.. _deflate_estimation_limitations:

Limitations
***********

- Due to the limitations imposed by the Intel® IAA hardware, the maximum bit
  size supported by the Intel® QPL compression is 32 bits. To account for the
  additional compression overhead, the maximum input size supported by the
  Intel® QPL and this API is 2^32 - 35 bytes.

- The function does not account for additional bytes needed for gzip/zlib
  headers and trailers. This API only estimates the overhead of the Intel® QPL
  Intel® IAA deflate operations. The user must account for the additional
  bytes needed for gzip/zlib headers and trailers.

.. _deflate_estimation_additional_overhead:

Additional Overhead
*******************

The Intel® QPL Compression Buffer Size Estimation API does not include the
additional bytes required for gzip/zlib headers and trailers. Users must
manually account for these additional bytes when dealing with compression jobs
that are flagged as either `QPL_FLAG_FIRST` or `QPL_FLAG_LAST`.

For jobs marked with the `QPL_FLAG_FIRST` flag, which indicates the beginning
of a compression sequence, the following additional bytes for headers must be
considered:

**GZIP**

- 10 bytes for the gzip header

**ZLIB**

- 2 bytes for the zlib header

For jobs marked with the `QPL_FLAG_LAST` flag, which indicates the end of a
compression sequence, the following additional bytes for trailers must be
included:

**GZIP**

- 8 bytes for the gzip trailer

**ZLIB**

- 4 bytes for the zlib trailer

It is important to note that jobs without the `QPL_FLAG_FIRST` or
`QPL_FLAG_LAST` flags do not require these additional bytes for headers or
trailers, as they are considered part of a continuous compression stream
without the need for marking the beginning or end.
