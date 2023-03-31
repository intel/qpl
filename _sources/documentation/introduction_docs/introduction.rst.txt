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
without changing your application. Intel QPL provides high
performance implementations of data processing functions for existing
hardware accelerator, and/or software path if the hardware
accelerator is not available. Code written with the library
automatically takes advantage of available modern CPU capabilities. This
can provide tremendous development and maintenance savings. The goal of
Intel QPL is to provide application programming interface (API)
with:

-  C and C++ compatible interfaces and data structures to enhance usability and portability
-  Faster time to market
-  Scalability with Intel® In-Memory Analytics Accelerator (Intel® IAA) hardware


Library Architecture Overview
*****************************


The diagram below includes both the architecture of Intel QPL and the external components that the library
interacts with. The components in the diagram are numbered from 0 to 8, each with a short description.

.. figure:: ../_images/qpl_arch_diagram.png
  :scale: 25%

  Architecture Diagram of Intel QPL

0. Users should use the accelerator utility tool (``accel-config``), which cooperates with Intel® Data Accelerator
   Driver (``idxd``), to pre-configure Intel IAA hardware.
1. C Job API is compatible with C and C++.
2. Contains the sequences of steps, including optimized function calls or accelerator operations, needed to
   handle specific query processing cases, and returns appropriate status back to users.
3. The CPU dispatcher detects what instruction sets are available in CPU.
4. The accelerator dispatcher detects available capabilities in Intel IAA hardware and records available devices and workqueues.
5. Set of optimized kernels for CPU that can be used if Intel IAA hardware is not available on the platform.
6. Set of optimized kernels for CPU, which are adopted from Intel® Intelligent Storage Acceleration Library (Intel® ISA-L).
   Intel QPL keeps its own copy of relevant source code from Intel ISA-L.
7. Set of low-level descriptors and service functions for interaction with Intel IAA hardware
   (using Accelerator Interfacing Architecture ISA).
8. The accelerator utility library (``libaccel-config``), which is linked to Intel QPL, provides APIs for communicating
   with Intel IAA hardware.
9. The Intel® Data Accelerator Driver (``idxd``) is a kernel driver that manages Intel IAA devices.


Library Functionality Overview
******************************


Intel® Query Processing Library (Intel® QPL) consists of two main
functional blocks: analytics and compression.

The analytics part contains two sub-blocks: Decompress and Filter.
These functions are tied together, so that each analytics operation
can perform decompress-only, filter-only, or decompress-and-filter
processing, as illustrated in the figure below.

Alternatively, you can compress the input with the compression part.

::


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

                       Intel® Query Processing Library (Intel® QPL) pipeline


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
calculation of arbitrary CRCs.

The SQL filter function block takes one or two input streams, a primary
input, and an optional secondary input. The primary input may be read
from memory or received from the decompression block. The second input,
if used, is always read from memory. The data streams logically contain
an array of unsigned values, but they may be formatted in any of several
ways, e.g., as a packed array. If the bit-width of the values is 1, the
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


The Intel QPL library uses Intel IAA hardware that provides
compression and decompression of very high throughput combined
with analytic primitive functions. The primitive functions are
commonly used for data filtering during analytic query processing.

Intel IAA primarily targets:

-  Big data applications and in-memory analytic databases.
-  Application-transparent usages such as memory page compression.
-  Data integrity operations, e.g., CRC-64.

Intel IAA supports Huffman encoding and
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

- ``Hardware Path`` - all hardware-supported functions are executed by Intel IAA.
- ``Software Path`` - all supported functionalities are executed by the software library in the CPU.
- ``Auto Path`` - Intel QPL automatically dispatches execution of the
  requested operations either to Intel IAA or to the software
  library depending on internal heuristics (``Load Balancing`` feature).

.. warning::
   The implementation of ``Auto Path`` is in progress.

.. _library_numa_support_reference_link:

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
boundary. Users are responsible for balancing workloads between different nodes.

.. _library_limitations_reference_link:

Library Limitations
*******************

- Library does not work with Dedicated Work Queues on the accelerator, but uses Shared Work Queues only.
- Library does not have APIs for the hardware path configuration.
- Library does not have APIs for ``Load Balancing`` feature customization.
- Library does not support hardware path on Windows OS.
- Library is not developed for kernel mode usage. It is user level driver library.
- Library uses the ``Block On Fault`` feature required to handle page faults on the Intel® IAA side. The
  ``block_on_fault`` attribute must be set with the ``accel-config`` for each
  work queue. Performance of Hardware Path applications can be increased if the application performs
  its own ``pre-faulting``. In this case, the ``Block On Fault`` feature must be disabled with the ``accel-config``
  by setting the ``block_on_fault`` attribute to ``0``. Refer to :ref:`accelerator_configuration_reference_link` for more details.

Library APIs
************

Intel QPL provides Low-Level C API, that represents a state-based interface.
The base idea is to allocate a single state and configure one with different ways
to perform necessary operation. All memory allocations are happening on user side
or via user-provided allocators.
See :ref:`developer_guide_low_level_reference_link` for more details.

