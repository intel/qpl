<!--
Copyright (C) 2022 Intel Corporation 
SPDX-License-Identifier: MIT
-->

Intel® Query Processing Library (Intel® QPL) Reference Manual
===============================================================================


Notices and Disclaimers
-----------------------

No license (express or implied, by estoppel or otherwise) to any intellectual
property rights is granted by this document.

Intel disclaims all express and implied warranties, including without
limitation, the implied warranties of merchantability, fitness for a particular
purpose, and non-infringement, as well as any warranty arising from course of
performance, course of dealing, or usage in trade. This document contains
information on products, services and/or processes in development. All
information provided here is subject to change without notice. Contact your
Intel representative to obtain the latest forecast, schedule, specifications and
roadmaps. The products and services described may contain defects or errors
which may cause deviations from published specifications. Current characterized
errata are available on request. Intel, the Intel logo, Intel Atom, Intel Core
and Xeon are trademarks of Intel Corporation in the U.S. and/or other countries.
\*Other names and brands may be claimed as the property of others. Microsoft,
Windows, and the Windows logo are trademarks, or registered trademarks of
Microsoft Corporation in the United States and/or other countries. Java is a
registered trademark of Oracle and/or its affiliates.

© Intel Corporation.

This software and the related documents are Intel copyrighted materials, and
your use of them is governed by the express license under which they were
provided to you ("License"). Unless the License provides otherwise, you may
not use, modify, copy, publish, distribute, disclose or transmit this software
or the related documents without Intel's prior written permission. This software
and the related documents are provided as is, with no express or implied
warranties, other than those that are expressly stated in the License.


Introduction
------------

This document describes architecture of the Intel® In-Memory Analytics Accelerator (Intel® IAA) software library. Intel IAA is a hardware accelerator that provides compression and decompression of very high throughput combined with analytic primitive functions. The primitive functions are commonly used for data filtering during analytic query processing.

Intel IAA primarily targets:

- Big data applications and in-memory analytic databases.
- Application-transparent usages such as memory page compression.
- Memory move and data integrity (e.g. CRC64) operations.

Intel IAA supports lightweight compression schemes such as zero-compression and heavier formats such as Huffman encoding and deflating. For the deflating format, Intel IAA supports indexing of the compressed stream for efficient random access.

Use the Intel® Query Processing Library (Intel® QPL) to improve
performance of database, enterprise data, communications, and
scientific/technical applications. Intel QPL is a common interface for a
number of commonly used algorithms. Using this library enables you to
automatically tune your application to many generations of processors without
changes in your application. The Intel QPL provides high performance
implementations of data processing functions for existing hardware accelerator,
and/or software path in case if hardware accelerator is not available. Code written
with the library automatically takes advantage of available modern CPU
capabilities. This can provide tremendous development and maintenance savings.
The goal of the Intel QPL is to provide application programming
interface (API) with:

- C and C++ interfaces and data structures to enhance usability and portability.
- Faster time to market.
- Scalability with Intel IAA hardware.



Overview
--------

Intel® Query Processing Library (Intel® QPL) consists of two main functional blocks: compression and analytics. The analytics part contains two sub-blocks: Decompress and Filter. These functions
are tied together, so that each analytics operation can perform decompress-only,
filter-only, or decompress-and-filter processing, as illustrated in Figure 3-1.
Alternatively, you can compress the input.

```
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
  ###                   
```
Figure 3-1: Intel® Query Processing Library (Intel® QPL) pipeline

With the library, you can store columnar databases in a compressed form, decreasing memory
footprint. In addition to increased effective memory capacity, this also reduces
memory bandwidth by executing the filter function used for database queries “on
the fly”, avoiding use of memory bandwidth for uncompressed raw data transfer.

Intel QPL supports decompression compatible with the deflate compression standard
described in RFC 1951. The uncompressed data may be written directly to memory
or passed to the input of the filter function. Decompression is supported for
deflate streams where the size of the history buffer is no more than 4 KB.

The library also supports DEFLATE compression, along with the calculation of arbitrary
CRCs and two varieties of zero compression/decompression.

The SQL filter function block takes one or two input streams, a primary input,
and an optional secondary input. The primary input may be read from memory or
received from the decompression block. The second input, if used, is always read
from memory. The data streams logically contain an array of unsigned values, but
they may be formatted in any of several ways, e.g. as a packed array. If the
bit-width of the values is 1, the stream will be referenced as a “bit-vector”,
otherwise, it will be referenced as an “array”.

The output of the filter function may be either an array or a bit vector,
depending on the function.

In addition to generating output data, Intel QPL computes a 32-bit CRC of the
uncompressed data (either the result of decompression, or the direct input to the
filter function), the XOR checksum of this data, and several “aggregates” of the
output data. The CRC, XOR checksum, and aggregates are written to the completion
record.



Getting Started
---------------

### Requirements and Build

Information about how to build Intel® Query Processing Library (Intel® QPL) is available in <README.md> file.

### Specific Build Options

To achieve the best experience depending on the user's needs, the library has different buil options:

- `-DSANITIZE_MEMORY=[ON|OFF]`    - Enables memory sanitizing (OFF by default)
- `-DSANITIZE_THREADS=[ON|OFF]`   - Enables threads sanitizing (OFF by default)
- `-DLOG_HW_INIT=[ON|OFF]`        - Enables HW initialization log (OFF by default)
- `-DEFFICIENT_WAIT=[ON|OFF]`     - Enables usage of efficient wait instructions (OFF by default)
- `-DLIB_FUZZING_ENGINE=[ON|OFF]` - Enables fuzzy testing (OFF by default)
- `-DBLOCK_ON_FAULT=[ON|OFF]`     - Enables Page Fault Processing on the accelerator side (ON by default) 

> **Important!**: 
> Right value of `BLOCK_ON_FAULT` option has strong effect on `hw-path` usage experience. 
> Read more in the [Accelerator Configuration](#accelerator-configuration) section

### Install Package Structure

After installation of the library, you should see the following directory structure inside your `<install_dir>`:
```
  ├── bin
  ├── include  
  │   └── qpl  
  |       ├── c_api
  │       └── cpp_api    
  │           ├── chaining  
  │           ├── common
  │           ├── operations                
  │           ├── results    
  │           └── util    
  └── lib64
      └── cmake 
```

Library Presetting
------------------

The library requires presetting of hardware configuration before usage. The library doesn't perform hardware setup independently. System administrator is responsible for correctness of accelerator configuration. If configuration is invalid or doesn't exist, the library returns an appropriate status code in case if hardware execution path used.

**Note: Intel QPL doesn't support all hardware possibilities. Library limitations are described in the [Library Limitations](#library-limitations) section.**

### Linux

#### System Requirements

- OS kernel has an appropriate version that supports Intel IAA devices.
- libaccel-config.so is placed in /usr/lib64/. The library can be found on `https://github.com/intel/idxd-config`
- BIOS has enabled VT-d option.
- System administrator privileges for `HW-path`. Requirement exists due to work with MMIO regions.

#### Accelerator Configuration

You can configure the accelerator with [accel-config](https://github.com/intel/idxd-config).

For detailed instructions on configuration, refer to:
- [accel-config ReadMe](https://github.com/intel/idxd-config/blob/master/README.md)
- [accel-config Wiki](https://github.com/intel/idxd-config)

> **Important!**
> Intel QPL supports libaccel-config starting from version 3.2. Refer to 
> [accel-config releases](https://github.com/intel/idxd-config/releases/tag/) for latest version

> **Important!**
> By default, Intel QPL uses `Block On Fault` feature required to handle page faults on the Intel IAA side. The `block on fault` flag must be set with `accel-config` for each device. 
> Performance can be increased if an application performs `pre-faulting` by own. In this case, the `block on fault` flag is not required and `Block On Fault` feature must be disabled using CMake build option `-DBLOCK_ON_FAULT`.

```shell
# Disabling `block on fault`
cmake -DCMAKE_BUILD_TYPE=Release -DBLOCK_ON_FAULT=OFF <path_to_cmake_folder> 
```


Library Architecture
--------------------

Library Possibilities
---------------------
In this section is high level overview of the Intel QPL possibilities provided.

### Execution Paths

The library supports several implementation/execution paths that help to achieve the optimal system resources utilization:

- `Hardware Path` - all hardware-supported functions are executed by the Intel IAA.
- `Software Path` - all supported functionality is executed by the software library.
- `Auto Path`     - Intel QPL automatically dispatches execution of the requested operations either to the Intel IAA or
  to the software library depending on internal heuristics (`Load Balancing` feature).

### NUMA Support

The library is NUMA aware and respects the NUMA node ID of the calling thread. If a user needs to use a device from a specific node, it can be done in two ways:

- Pin thread that performs submissions to the specific NUMA, the library will use devices only from this node.
- Set NUMA ID parameter of the job to the specific node ID, then devices will be selected only from this node.

Load balancer of the library doesn't cross a detected or specified NUMA boundary. Balancing workloads between different nodes is the responsibility of a user.


### Operations

#### Compression Operations

Intel QPL supports compression/decompression in different formats. These formats additionally have their own modes.

- Deflate compression/decompression (history size is limited by 4Kb)
  - Default RFC1951 Deflate format with different compression levels.
  - Deflate with stream indexation for random access to compressed data
  - Deflate without headers writing. So-called `Canned Mode`.
- Huffman only compression/decompression
  - Huffman only Big-Endian mode
  - Huffman only Little-Endian mode
- Zero Compression


#### Filtering Operations

Intel QPL supports different filtering functions:

- Scan
- Expand
- Extract
- Select  
- Find Unique
- Set Membership
- RLE Burst


#### Merged Operations

All `Filtering operations` can be performed with preliminary input source decompression by a single step.

#### Other Operations

- Memory Copy
- CRC64

Library Limitations
---------------------

- Library doesn't work with Dedicated WQs for the accelerator, but uses shared ones only. 
- Library hasn't API for the hardware path configuration.
- Library hasn't API for `Load Balancing` feature customization.
- Library doesn't support Hardware execution path on Windows OS.
- Library is not developed for kernel mode usage. It is user level driver library.

Library APIs
------------

Library has several interfaces that provide access to Intel QPL possibilities through different languages and paradigms:
- `Public C API`
- `Public C++ API`

### C API (Job API)

Represents a state based interface. The base idea is to allocate a single state and configure one with different ways 
to perform necessary operation. Such API doesn't allocate memory internally.

More details: [C API Manual](./QPL_C_API_MANUAL.md)

### C++ API (High Level API)

Represents high level operations API written with C++. API provides compile time optimizations and less operation preparation latency.
It allocates memory internally with std::allocator, but support custom user allocators that correspond to С++ Standard.

More details: [C++ API Manual](./QPL_HIGH_LEVEL_API_MANUAL.md)