 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

Code Examples
#############


.. _code_examples_c_reference_link:

Low-Level C API Examples
************************


This section contains a number of simple C API examples, illustrating how you
can use the Intel® Query Processing Library (Intel® QPL). 


.. _code_examples_c_compression_reference_link:

Compression
===========


.. literalinclude:: ../../../../examples/low-level-api/compression_example.cpp
    :language: cpp


Canned Mode
===========


.. literalinclude:: ../../../../examples/low-level-api/canned_mode_example.cpp
    :language: cpp


Canned Mode with Data
=====================


.. literalinclude:: ../../../../examples/low-level-api/canned_mode_with_data_example.cpp
    :language: cpp


Serialization
=============


.. literalinclude:: ../../../../examples/low-level-api/serialization_example.cpp
    :language: cpp


CRC64
=====


.. literalinclude:: ../../../../examples/low-level-api/crc64_example.cpp
    :language: cpp


Expand
======


.. literalinclude:: ../../../../examples/low-level-api/expand_example.cpp
    :language: cpp


Extract
=======


.. literalinclude:: ../../../../examples/low-level-api/extract_example.cpp
    :language: cpp


RLE Burst
=========


.. literalinclude:: ../../../../examples/low-level-api/rle_burst_example.cpp
    :language: cpp


Scan
====


.. literalinclude:: ../../../../examples/low-level-api/scan_example.cpp
    :language: cpp


Scan for Unique Value
=====================


.. literalinclude:: ../../../../examples/low-level-api/scan_for_unique_value_example.cpp
    :language: cpp


Scan Range
==========


.. literalinclude:: ../../../../examples/low-level-api/scan_range_example.cpp
    :language: cpp


Select
======


.. literalinclude:: ../../../../examples/low-level-api/select_example.cpp
    :language: cpp


Zero Compression
================


.. literalinclude:: ../../../../examples/low-level-api/zero_compression_example.cpp
    :language: cpp


High-Level C++ API Examples
***************************


This section contains a number of C++ API examples, illustrating how you
can use the Intel QPL library.


Simple Operations
=================


Compression
-----------


.. literalinclude:: ../../../../examples/high-level-api/simple-operations/compression_example.cpp
    :language: cpp


Compression Stream
------------------


.. literalinclude:: ../../../../examples/high-level-api/simple-operations/compression_stream_example.cpp
    :language: cpp


CRC
---


.. literalinclude:: ../../../../examples/high-level-api/simple-operations/crc_example.cpp
    :language: cpp


Deflate Block
-------------


.. literalinclude:: ../../../../examples/high-level-api/simple-operations/deflate_block_example.cpp
    :language: cpp


Expand
------


.. literalinclude:: ../../../../examples/high-level-api/simple-operations/expand_example.cpp
    :language: cpp


Extract
-------


.. literalinclude:: ../../../../examples/high-level-api/simple-operations/extract_example.cpp
    :language: cpp

RLE Burst
---------


.. literalinclude:: ../../../../examples/high-level-api/simple-operations/rle_burst_example.cpp
    :language: cpp


Scan
----


.. literalinclude:: ../../../../examples/high-level-api/simple-operations/scan_example.cpp
    :language: cpp


Scan Range
----------


.. literalinclude:: ../../../../examples/high-level-api/simple-operations/scan_range_example.cpp
    :language: cpp


Scan with NUMA Device ID
------------------------


.. literalinclude:: ../../../../examples/high-level-api/simple-operations/scan_with_numa_device_id_example.cpp
    :language: cpp


Select
------


.. literalinclude:: ../../../../examples/high-level-api/simple-operations/select_example.cpp
    :language: cpp


Zero Compression
----------------


.. literalinclude:: ../../../../examples/high-level-api/simple-operations/zero_compression_example.cpp
    :language: cpp


Operations Chaining
===================


Decompression with Analytics
----------------------------


.. literalinclude:: ../../../../examples/high-level-api/operation-chains/decompression_merged_with_analytics_example.cpp
    :language: cpp 


Decompression with Scan and Select
----------------------------------


.. literalinclude:: ../../../../examples/high-level-api/operation-chains/decompression_scan_select_example.cpp
    :language: cpp 



Other Examples
==============


Scan with Custom Allocator
--------------------------


.. literalinclude:: ../../../../examples/high-level-api/utils/scan_with_custom_allocator_example.cpp
    :language: cpp


.. literalinclude:: ../../../../examples/high-level-api/utils/stack_allocator.hpp
    :language: cpp
