 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _introduction_reference_link:

Introduction
############


The Intel® Query Processing Library (Intel® QPL) can be used to improve
performance of database, enterprise data, communications, and
scientific/technical applications. Intel QPL provides interfaces for a
number of commonly used algorithms. Using this library enables you to
automatically tune your application to many generations of processors
without changing your application. The Intel QPL provides high
performance implementations of data processing functions for existing
hardware accelerator, and/or software path in case if hardware
accelerator is not available. Code written with the library
automatically takes advantage of available modern CPU capabilities. This
can provide tremendous development and maintenance savings. The goal of
the Intel QPL is to provide application programming interface (API)
with:

-  C and C++ interfaces and data structures to enhance usability and portability
-  Faster time to market
-  Scalability with Intel® In-Memory Analytics Accelerator (Intel® IAA) hardware


.. attention::
   Intel QPL presently is in a prototype form. Public APIs described in this
   documentation may change at any time.


Library Overview
****************


Intel® Query Processing Library (Intel® QPL) consists of two main
functional blocks: compression and analytics. The analytics part
contains two sub-blocks: Decompress and Filter. These functions are tied
together, so that each analytics operation can perform decompress-only,
filter-only, or decompress-and-filter processing, as illustrated in
the following figure. Alternatively, you can compress the input.

::

     ###
                          Decompress               SQL Filter   Decompress
                             Bypass                   Bypass       Output
                      /------------------\   |\   /--------------------\
                      | +--------------+ |   | \  |  +-------------+   |     |\
           Source1    | |   DEFLATE    | \-->|  |-+->| SQL Filter  |   \---->| \
           -----------+-| Decompressor |---->| /     |  Functions  |-------->|  |------------>
                      | +--------------+     |/      +-------------+         | /   Analytics
                      | Decompress   |                      |                |/    Engine Output
           Source2    | Config/State |                      |           SQL Filter
           -----------(--------------(----------------------/             Output
                      |  Compress    |             Filter Optional
                      | Config/State |             Second Input
                      |              |
                      | +--------------+
                      | |   DEFLATE    |
                      +-| Compressor   |----------------------------------------------------->
                        +--------------+

     ###                  Intel® Query Processing Library (Intel® QPL) pipeline


With the library, you can store columnar databases in a compressed form,
decreasing memory footprint. In addition to increased effective memory
capacity, this also reduces memory bandwidth by executing the filter
function used for database queries “on the fly”, avoiding use of memory
bandwidth for uncompressed raw data transfer.

Intel QPL supports decompression compatible with the Deflate compression
standard described in RFC 1951. The uncompressed data may be written
directly to memory or passed to the input of the filter function.
Decompression is supported for Deflate streams where the size of the
history buffer is no more than 4 KB.

The library also supports Deflate compression, along with the
calculation of arbitrary CRCs and two varieties of zero
compression/decompression.

The SQL filter function block takes one or two input streams, a primary
input, and an optional secondary input. The primary input may be read
from memory or received from the decompression block. The second input,
if used, is always read from memory. The data streams logically contain
an array of unsigned values, but they may be formatted in any of several
ways, e.g. as a packed array. If the bit-width of the values is 1, the
stream will be referenced as a “bit-vector”, otherwise, it will be
referenced as an “array”.

The output of the filter function may be either an array or a bit
vector, depending on the function.

In addition to generating output data, Intel QPL computes a 32-bit CRC
of the uncompressed data (either the result of decompression, or the
direct input to the filter function), the XOR checksum of this data, and
several “aggregates” of the output data. The CRC, XOR checksum, and
aggregates are written to the completion record.


Intel® In-Memory Analytics Accelerator (Intel® IAA)
===================================================


The Intel QPL library uses Intel IAA hardware accelerator that
provides compression and decompression of very high throughput combined
with analytic primitive functions. The primitive functions are commonly
used for data filtering during analytic query processing.

Intel IAA primarily targets:

-  Big data applications and in-memory analytic databases.
-  Application-transparent usages such as memory page compression.
-  Data integrity operations, e.g., CRC-64.

Intel IAA supports lightweight compression schemes such as
zero-compression and heavier formats such as Huffman encoding and
Deflate. For the Deflate format, Intel IAA supports indexing of the
compressed stream for efficient random access.


Library Features
****************


Operations
==========


Intel QPL supports:

- Deflate compression/decompression with the history size limited to 4 KB
- Huffman-only compression/decompression
- Filter operations

.. warning::
   The implementation of Huffman-only compression/decompression is in progress.


Execution Paths
===============


Intel QPL supports several execution paths that help to achieve the optimal
system resources utilization:

- ``Hardware Path`` - all hardware-supported functions are executed by the Intel IAA.
- ``Software Path`` - all supported functionality is executed by the software library.
- ``Auto Path`` - Intel QPL automatically dispatches execution of the
  requested operations either to the Intel IAA or to the software
  library depending on internal heuristics (``Load Balancing`` feature).

.. warning::
   The implementation of ``Auto Path`` is in progress.


NUMA Support
============


Intel QPL is NUMA aware and respects the NUMA node ID of the calling
thread. If a user needs to use a device from a specific node, it can be
done in two ways:

-  Pin thread that performs submissions to the specific NUMA, the
   library will use devices only from this node.
-  Set NUMA ID parameter of the job to the specific node ID, then
   devices will be selected only from this node.

Load balancer of the library does not cross a detected or specified NUMA
boundary. Balancing workloads between different nodes is the
responsibility of a user.


.. _library_limitations_reference_link:

Library Limitations
*******************

- By default, the library does not work with Dedicated Work Queues (DWQ) on the accelerator, but uses Shared Work Queues only.
  DWQ can be enabled by building the library with the build option ``-DDWQ_SUPPORT=ON``
  (see :ref:`building_library_build_options_reference_link` for details and limitations).
- Library does not have APIs for the hardware path configuration.
- Library does not have APIs for ``Load Balancing`` feature customization.
- Library does not support hardware path on Windows OS.
- Library is not developed for kernel mode usage. It is user level driver library.


.. _library_apis_reference_link:

Library APIs
************


Intel QPL provides two classes of APIs:


**Low-Level C API**

Represents a state based interface. The base idea is to allocate a
single state and configure one with different ways to perform necessary
operation. Such API does not allocate memory internally.

See :ref:`developer_guide_low_level_reference_link` for more details.


**High-Level C++ API**

Represents high level operations API written with C++. API provides
compile time optimizations and less operation preparation latency. It
allocates memory internally with ``std::allocator``, but supports custom user
allocators that correspond to C++ Standard.

See :ref:`developer_guide_high_level_reference_link` for more details.
