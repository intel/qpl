<!--
Copyright (C) 2022 Intel Corporation 
SPDX-License-Identifier: MIT
-->

Intel® Query Processing Library (Intel® QPL) C API Manual
=========================================================

Intel Query Processing Driver Operation
=======================================

Notices and Disclaimers
=================

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

Disclaimer
==========

This document provides instructions on how to use the Intel® Query Processing Library (Intel® QPL) C API 
(also called as Job API). The library exists in a prototype form and is being actively developed.
This means that some things described here may change in the future. However, this is being
released now in the expectation that it will be more useful than nothing to
people initially trying to use the library.

Header Files
============

The application only needs to include one header file: “qpl/qpl.h”. It includes the
entire API definition.

Basic Usage
===========

The basic idea is that the application fills in a “job” data structure with the
necessary parameters to describe the job to be done. This is then passed to the
library, and when it completes, the job structure contains the results (error
codes, output parameters, etc.).

The Intel QPL does not perform any internal memory allocations. All memory
buffers that are required for the library work must be provided by an
application. As the library supports several implementation/execution paths
(qpl_path_hardware, qpl_path_software, and qpl_path_auto), the size of the required memory buffer
for job structure is not predefined and depends on the used CPU/HW and execution
path. Therefore, before submitting any job to the
library, the application must determine the size of the required memory buffer, allocate this
memory buffer, and perform initialization of this memory buffer. The algorithm of
these steps is as follows:

1.  Call the qpl_get_job_size() function:
```
    status = qpl_get_job_size(qpl_path, job_size_ptr);
```
where:
```
   status - has type qpl_status and contains execution status of the function – success or error code – see qpl.h for details
   qpl_path – has type qpl_path_t, which can be:
   	qpl_path_hardware – all hardware-supported functions are executed by the Intel® In-Memory Analytics Accelerator (Intel® IAA)
   	qpl_path_software – all supported functionality is executed by the software library
   	qpl_path_auto – the library automatically dispatches execution of the requested jobs either to the Intel IAA or to the software library depending on internal heuristics
   job_size_ptr – has type uint32_t* (a pointer to unsigned int) and stores the calculated memory buffer size returned by the function
```
2.  Allocate the required memory buffer (size in bytes
    = \*job_size_ptr). The Intel® Query Processing Library (Intel® QPL) does not allocate or free memory, therefore an application must provide required
    memory buffer for the job structure and free it after it becomes
    unnecessary. During allocation, the memory buffer should be casted to
    qpl_job\* data type, for example
```
	qpl_job_ptr = (qpl_job\*)malloc(\*job_size_ptr).
```
Check allocation results for success.

3.  Call the qpl_init_job() function:
```
    status = qpl_init_job(qpl_path, qpl_job_ptr);
```
> Once the qpl_job initialized, it does not require reinitialization and updating of output fields before reusing.

where:

```
   status - has type qpl_status and contains execution status of the function – success or error code – see qpl.h for details
   qpl_path – has type qpl_path_t, which can be:
   	qpl_path_hardware – all hardware-supported functions are executed by the Intel IAA
   	qpl_path_software – all supported functionality is executed by the software library
   	qpl_path_auto – the library automatically dispatches execution of the requested jobs either to the Intel IAA or to the software library dependending on internal heuristics
     Note: It shall have the same value as in the 1st step – the call to qpl_get_job_size().
   qpl_job_ptr – has type qpl_job (job structure) and stores the initialized job structure returned by the function.
```
An example of C-code for these first steps:
```
	uint32_t size; 
	qpl_job *qpl_job_ptr;
	qpl_status status;

	status = qpl_get_job_size(qpl_path_auto, &size);
	qpl_job_ptr = (qpl_job*)malloc(size);
	status = qpl_init_job(qpl_path_auto, qpl_job_ptr);

```

After the application initializes the job structure, it can request any library operation needed.

The simplest interface is qpl_execute_job (job). It does not return any result until the job completes. This is the “synchronous” interface. The “asynchronous” interface is available as well. In this case, the job is submitted using qpl_submit_job(job). The application can then periodically query the status of the job with qpl_check_job(job). The application may call qpl_wait_job(job) to wait until the job completes.

**Note** that the qpl_execute_job() function is essentially a combination of
qpl_submit_job() followed by qpl_wait_job().

In the context of the behavioral model (i.e. without actual hardware support), the job is processed when submitted. The qpl_wait_job() function always returns completed status. This is not the case when real hardware is
involved.

The job structure contains three types of data:

1.  Parameters defining the job to be done.

2.  Results of the job performed.

3.  Internal state.

In some cases, for example, compression and decompression, a larger overall task may be broken into a series of separate library calls. For example, an application compressing a large file might call the library repeatedly with 64kB buffers, until the end of the file is reached. These calls are related to each other as they use the same job structure. As it will be described later, certain flags indicate whether a particular job submission is the start of a new overall task or it is a continuation of the previous one. Because of this, there is not a separate “init” function; the initialization is implied by the job’s parameters.

The library auxiliary functions will be described later, as they become relevant.

A number of examples can be found in Section 18.

Basic/Common Fields
-------------------


The input buffer is described by two fields: next_in_ptr and available_in. The first points to the start of the input data, and the second gives the length of the input data. If the job completes successfully, the
next_in_ptr field should be advanced by available_in, and the available_in field should be zero.

The output buffer is described by two similar fields: next_out_ptr and available_out. Note that available_out is both an input and an output. As an input, it describes the size of the output buffer (i.e. how many bytes can be written into the buffer). As an output, it describes the remaining number of bytes unused.

**Note** that these fields are updated with each job call in a sequence. To concatenate the result of a sequence of jobs into one buffer, initialize the output buffer before the first job call.

Two output fields: total_in and total_out return the cumulative number of bytes read and written.

Enumerated field “op” defines the operation to be done. 

The “flags” field is used for different bits to define flags that affect the
operation.

Many of the operations return a CRC32 and an XOR checksum. By default, the CRC uses the polynomial 0x104c11db7 (used by DEFLATE and related tools). If the flag QPL_FLAG_CRC32C is specified, then the polynomial 0x11edc6f41 is used (used by iSCSI). The XOR checksum is just the XOR of the data (represented as 16-bit words), which is the last byte padded by 0x00 if the
total length is odd.

The hardware cannot write partial bytes. When the output stream ends with a partial byte, the byte is zero-padded to a byte boundary before being
written. The number of valid bits in the byte can be obtained from the
last_bit_offset field. On completion, this field contains the number of bits written to the last byte mod 8. That is, this field contains the number of bits written to the last byte. If all 8 bits were written, it contains the value of 0. For an operation like decompression, which generates a stream of bytes, this field should always have a value of zero. For operations like filtering, which generate a stream of bits, this value can be used to determine where the actual data ends.


Decompression
=============

Decompression of a large stream can be accomplished through linking multiple
jobs. The first job is specified with the flag QPL_FLAG_FIRST, and the last job is specified with the flag QPL_FLAG_LAST. If the stream consists of a single job, then both flags should be specified.

The decompressor expects the stream to be formatted with DEFLATE data. The GZIP standard defines an encapsulation layer around a DEFLATE stream. In particular, a GZIP stream consists of a GZIP header, a DEFLATE stream, and a GZIP trailer. The decompressor looks at a raw DEFLATE stream by default. If
the QPL_FLAG_GZIP_MODE flag is specified, then the library will skip over the
GZIP header on the FIRST job, and then start parsing the DEFLATE stream.

In general, the application may not know how much data a given compressed stream will uncompress to. To handle this, the Intel QPL allows a decompress job to complete “successfully” but with not all input processed. This is called euphemistically “output overflow” although no actual overflow occurs (The label comes from the fact that the output buffer would have overflowed if the decompressed did not stop early).

If the job completes with the status QPL_MORE_OUTPUT_NEEDED, then the output
buffer filled up before the input is completely processed (Note that output 
buffer may be considered “full” even though the bytes remaining in it are not zero). In this case, the remaining data from this job needs to be processed before a new job (if exists) is submitted. Due to the update of the
input buffer parameters, these do not need to be reset, but the output buffer
parameters need to be reset. They can be pointed to a new buffer, or the current output buffer could be processed (e.g. written to disk) and then reused.

**Note** that the available_in field could be 0, and the status could
still be QPL_MORE_OUTPUT_NEEDED. In this case, the subsequent job will be
submitted with a zero-length input buffer.

For decompression, the CRC and checksum fields are calculated on the output of the decompressor, that is, on the uncompressed data.

Normally, the hardware will only write complete qwords (8 bytes) on all, but a LAST job-\> If the flag QPL_FLAG_DECOMP_FLUSH_ALWAYS is used, then even on a non-LAST job, all the processed data will be written to the output.

In case when the application passes in a tiny input buffer, the library will normally copy the data into internal buffer, and try to combine it with other small input buffers before submitting it to the hardware. If the flag QPL_FLAG_NO_BUFFER is specified, then this option will be ignored.


Compression
===========

At a high level, compression is very similar to decompression. The application specifies the input buffer and the output buffer, submits one or more jobs, marks the FIRST job and the LAST job, etc. Compression has more possible variations, and ways to deal with “output overflow”.

The DEFLATE standard organizes the compressed data into a series of “blocks”.
Each block is defined by a type, with the three types being:

-   Stored Block: the input data is stored in an uncompressed form

-   Fixed Block: the input data is compressed with a fixed Huffman table defined by the standard. This is usually a suboptimal code, but in this case the block header is very small, as it doesn’t need to define a set of Huffman codes.

-   Dynamic Block: the input data is compressed with a Huffman table that is
    optimized for that particular block. This results in a more efficient code, but requires two passes: one pass to analyze the data, and another pass to do the compression after the optimal Huffman table is generated.

The library extends this with what it calls a “Static Block”. From the DEFLATE point of view, this is a Dynamic Block, but from the library point of view, the Huffman Table is provided by the application, so that the compression can happen in one pass. Application must make sure that the table
is appropriate for the data; otherwise, the compression ratio may suffer.

The main reason to start a new block in the middle of the stream is to change the type of block generated, or to start using a different Huffman table.

The Intel QPL does not save history state between jobs. The compressed form of the data from one job will never reference data from an earlier job. For example, when 10 jobs of 1,000 bytes are submitted, each will generate different output. In this case, one job with all 10,000 bytes can be submitted. In general, submitting very small buffers will result in a worse
compression ratio than submitting fewer large buffers.


Compression and “Output Overflow”
---------------------------------

Compression job cannot be partially completed. Each compression job will complete successfully with all the input processed, or it will fail with an error and conceptually no output generated. 

To complete the compression in any case, the library can specify minimum size for the output buffer. The size of the output buffer must be bigger than the input buffer. This actual value can be obtained from the auxiliary function min_compress_buffer_size(input_size). If the output buffer is at least this size, then the input will always be processed through to the output, barring some error condition. If the output buffer is smaller than the value specified, there is a chance that the output will not fit. The status QPL_LARGER_OUTPUT_NEEDED will be returned as a result.

If the compressed output does not fit into the output buffer specified, the library attempts to copy the input data into the stream as a series of Stored Blocks. When the Stored Blocks fit, the library returns a successful status. In case when the Stored Blocks do not fit, the library returns the QPL_LARGER_OUTPUT_NEEDED status. With the output buffer is at least as large as min_compress_buffer_size(input_size) specifies, then the Stored Blocks will always fit, and the QPL_LARGER_OUTPUT_NEEDED status is not returned.


Compression Styles
------------------

The library has many ways to compress the input data. The most common approaches are described in this section.

### Fixed Block

The simplest approach is to generate one large Fixed Block. To achieve this, do NOT use the QPL_FLAG_DYNAMIC_HUFFMAN flag, and set the pointer to the Huffman table (chufftab) to NULL.

For compressible data, this will result in one large fixed block. If multiple 
jobs are used (i.e. the first job is not marked as FIRST and LAST), then the result will be one large fixed block followed by a zero-length stored block. When multiple jobs are used, and some of them contain incompressible data, then the result will be a mixture of fixed blocks and stored blocks.

### Static Blocks

The Static Blocks case is similar to the Fixed Block case. Do NOT use the
QPL_FLAG_DYNAMIC_HUFFMAN flag. The pointer to the Huffman table (chufftab)
must point to a valid Huffman table. This will result in nominally
one large block, except for potential stored blocks as described in the previous case.

This approach, however, gives the application the ability to change the Huffman
Tables in the middle of the stream. If the stream is compressed with multiple
jobs, then in the middle of the stream, the application can specify a different
Huffman table and use the QPL_FLAG_START_NEW_BLOCK flag. This will instruct the
library to end the current block and to start a new block with the new table.
The QPL_FLAG_START_NEW_BLOCK is not needed on the first job; it is implied.

If the QPL_FLAG_START_NEW_BLOCK flag is not used, then the tables pointed to in
the job structure must be the same as those used by the previous job. When the
tables are changed without indication to the library, then the resulting
bit-stream will not be valid.

The qpl_build_compression_table() auxilary function takes as an input a histogram of the Literal-Length (LL) tokens and the distance (D) tokens, and fills in a compression Huffman table structure (see the related section for more information).


### Dynamic Blocks

If the jobs are submitted with the QPL_FLAG_DYNAMIC_HUFFMAN flag, then the data
from that job will be compressed as a single dynamic DEFLATE block (when buffer overflow occurs, STORED block is stored instead of DYNAMIC one). This gives the best compression ratio, but requires two passes through the hardware.


### Mixing Styles

The style cannot be changed in the middle of a stream. That is, if the first job in
a sequence of jobs has a particular style, then the remaining jobs in that
sequence must use the same style.

Verification
------------

By default, the library will verify the correctness of the generated compressed bit stream. The library decompresses the resulting bit stream, and then checks that the CRC of the uncompressed data matches the CRC of the original data. The verification step adds one more pass through the hardware. That is, for a fixed stream or static stream, each job results in two passes through the hardware (one for compression, one for verification). For a dynamic stream, it results in three passes.

If the user doesn’t want to pay the cost for verification, this can be turned off with the QPL_FLAG_OMIT_VERIFY flag.

**Note**: Currently verification is not performed in case of `Huffman only BE`.

Filtering / Analytics
=====================

The filtering unit performs an operation on a series of unsigned integers,
resulting in an output stream of integers. The bit-width of these integers can
vary between 1–32 bits.

Some operations process a single data stream, which comes from the
normal input fields in the job structure. Some of them operate on two data
streams. For these operations, the second data stream is specified using the
next_src2_ptr and available_src2 fields. This will be referred to as source-2, whereas
the main input stream will be referred to as source-1.

By default, the source-1 data is read directly from the specified input buffer.
If the QPL_FLAG_DECOMPRESS_ENABLE flag is given, then the data from the input
buffer is firstly decompressed, and then the decompressed data is sent to the
filter unit.

Each filtering job is independent. Filtering job does not support processing a large
stream as a series of smaller jobs, as compression and decompression jobs do. Thus, each filtering job is considered implicitly both a FIRST and a LAST job. That is, for a filtering operation, it is as if the flags specified both FIRST and LAST.

If the bit-width of an array is 1, then the array will be referred to as a
bit-vector. That is, a “bit-vector” is just an array whose width happens to be 1. If the bit-width is more than 1, it is called an “array”.

The output can be written as a packed little-endian array or as a packed big-endian array.


num_input_elements
----------------

In some cases, for example, for small bit-widths, the number of input elements is
ambiguous. To account for this, the number of input elements must be specified in num_input_elements. Processing will stop when these input elements are processed.

If the input ends before these input elements are processed, then the actual
number of elements processed is implementation specific, and an error status
will be returned.

In most cases, num_input_elements reflects the number of elements in source-1. For the expand operation, it reflects the number of elements in source-2.


Parsers
-------

The “parser” is responsible for converting a series of bytes into a series of
unsigned integers. The "parser" is defined by an enumeration in the job’s “parser” field.

The default value (qpl_p_le_packed_array) views the input buffer as a
little-endian packed array of N-bit integers, where N is given by src1_bit_width.
For example, if N=3, then the first element will be bits 2:0 in the first byte,
the second element will be bits 5:3, etc.

If the parser is qpl_p_be_packed_array, the buffer is viewed as a big-endian
packed array, e.g. with N=3, the first element will be bits 7:5 of the first
byte, the second element will be bits 4:2, etc.

If the parser is specified as qpl_p_parquet_rle, it is viewed as being in
Parquet RLE format. In this case, the bit width is given in the data stream, so
src1_bit_width should be set to 0.

Source-2 can only be “parsed” as a packed-array. The default parser views the
source-2 data a little-endian packed array. If the QPL_FLAG_SRC2_BE flag is
specified, then it is viewed as a big-endian packed array.

### Parquet RLE Format

The input is in the Parquet RLE format. The first byte of the data stream gives
the bit width. This is followed by the encoded data. The bit-width cannot exceed
32-bits.

The format is:
```
   parquet-rle: <bit-width> <encoded-data>
   bit-width := bit-width of data stored as one byte
   encoded-data := <run>*
   run := <bit-packed-run> | <rle-run>
   bit-packed-run := <bit-packed-header> <bit-packed-values> 
   bit-packed-header := varint-encode(<bit-pack-count> << 1 | 1)
      // we always bit-pack a multiple of 8 values at a time, so we only store the number of values / 8 
   bit-pack-count := (number of values in this run) / 8 
   bit-packed-values := data stored as a packed array of bit-width values
   rle-run := <rle-header> <repeated-value> 
   rle-header := varint-encode( (number of times repeated) << 1) 
   repeated-value := value that is repeated, using a fixed-width of round-up-to-next-byte(bit-width)

   varint := if((byte & 0x80) > 0) than the first bits are (byte & 0x7F), read 
   next byte until number of read bytes = 4, or ((byte & 0x80) == 0). Then all 
   obtained bits are connected sequentially – therefore the 1st bit of the 
   second byte must go to the 7th position of the resulting unsigned integer. 
      Note: the standard varint can consist of 5 encoded bytes, in the Intel® Query Processing Library (Intel® QPL) it 
   is limited by 4.

```

Operations
----------

| Operation          | Number of Input Streams | Output Stream Type  |
|--------------------|-------------------------|---------------------|
| qpl_scan           | 1                       | Bit Vector          |
| qpl_set_membership | 2                       | Bit Vector          |
| qpl_extract        | 1                       | Array or Bit Vector |
| qpl_select         | 2                       | Array or Bit Vector |
| qpl_expand_rle     | 2                       | Array or Bit Vector |
| qpl_find_unique    | 1                       | Bit Vector          |
| qpl_expand         | 2                       | Array or Bit Vector |


### qpl_scan

The scan operation outputs a bit-vector with the 1-bits correspond to input
elements that satisfy a numerical relationship. I.e. it can search for elements
that are EQ, NE, GT (etc.) a specified value, or for those values that fall
within an inclusive range.

The range is specified with the job fields: param_low and param_high. The others
use param_low.

The operations will look for those values X that satisfy the following
conditions:

| **Operation**         | **Condition**                   |
|-----------------------|---------------------------------|
| qpl_op_scan_eq        | X = param_low                    |
| qpl_op_scan_ne        | X ≠ param_low                    |
| qpl_op_scan_lt        | X \< param_low                   |
| qpl_op_scan_le        | X ≤ param_low                    |
| qpl_op_scan_gt        | X \> param_low                   |
| qpl_op_scan_ge        | X ≥ param_low                    |
| qpl_op_scan_range     | param_low ≤ X AND X ≤ param_high  |
| qpl_op_scan_not_range | X \< param_low OR X \> param_high |

The number of output bits (i.e. the number of output elements) is the same as
the number of input elements.

### qpl_set_membership

The qpl_set_membership operation can be viewed as a generalization of the scan
operation. Here, source-2 contains a bit-vector containing 2N bits, where N is the bit-width of
source-1 minus the number of dropped low and/or high bits. 
The bits define a set. The output is a bit-vector whose 1-bits identify which input elements are members of that set.

The number of output bits (i.e. the number of output elements) is the same as
the number of source-1 elements.

Note maximum N for operation is limited by library backend.
TBD: Create qpl_get_limits function.


### qpl_extract

The extract operation outputs input elements whose indices (starting at 0)
fall within the inclusive range defined by param_low and param_high. So the bit
width of the output is the same as the bit width of the input, and the number of
output elements should be (param_high - param_low + 1).


### qpl_select

The select operation can be considered as a generalization of extract operation. Here,
source-2 is a bit-vector that must have at least as many elements as
source-1. Those source-1 items that correspond to 1-bits in source-2 will be the output.


### qpl_expand_rle

The RLE Burst operation (qpl_expand_rle) replicates each element in source-2 by the number of times based on the corresponding element in source-1.

The bit width of source-1 must be 8, 16, or 32. The behavior of the operation
depends on the bit width of source-1.

The source-2 input is a packed array of unsigned integers of any width between 1-32 bits. If the bit width of source-1 is 8 or 16 bits, then source-2 should have
the same number of elements as source-1. When the bit width of source-1 is 32
bits, then source-2 should have one less element than source-1.

By default, the output bit width is the same as the source-2 bit width. The
output may be modified by zero-extending each output value to 8, 16, or 32 bits.

If the bit width of source-1 is 8 or 16, each element specifies the number of
times to replicate the corresponding element in source-2. When the value is 0, the
corresponding element is dropped.

If the bit width of source-1 is 32, each element of source-1 specifies the
cumulative number of elements in the output to that point. Thus, the repetition
count for each element is the difference between that element and the next. In
this case, the first element should always be 0. The number of times each bit is
replicated is limited to the range 0–65,535 inclusive. For example, with the
first element equal to 0 and the second element equal to 3, the first element of
source-2 is replicated three times in the output. As another example, the two
primary inputs shown in the following table are equivalent:

| Secondary Input (1 bit) | Primary input   |                             | Output         |
|-------------------------|-----------------|-----------------------------|----------------|
|                         | Length (8 bits) | Cumulative Length (32 bits) |                |
| 1                       | 2               | 0                           | 11             |
| 0                       | 4               | 2                           | 110000         |
| 1                       | 3               | 6                           | 110000111      |
| 0                       | 3               | 9                           | 110000111000   |
| 1                       | 2               | 12                          | 11000011100011 |
|                         |                 | 14                          |                |

**Note** that in case of 32-bit/cumulative counts, the number of elements field
should be set to the number of source-1 elements, which is one greater than the
number of source-2 elements. In the previous example, if the bit width of
source-1 was less than 32, the number of elements field would be set to 5.
If the bit width was 32, it would be set to 6.


### qpl_find_unique

This is in some sense the inverse of qpl_set_membership. Conceptually, it starts
with a bit-vector of size 2N, where N is the bit-width of source-1 minus the number 
of dropped low and/or high bits. 
The output bit vector starts with all zero values. As it reads the elements from source-1, the
hardware sets those bits corresponding to the values from source-1. At the end,
the 1-bits in the bit vector specify all of the values seen from source-1. This
bit vector is the output.

**Note** that maximum N for operation is limited by library backend.


### qpl_expand

qpl_expand is in some sense the inverse of qpl_select. This also reads a bit
vector from source-2.

In this operation, the bit-width of the output is the same as the bit width of
source-1, but the number of output elements is equal to the number of input
elements on source-2. So for this operation, the job field “num_input_elements”
actually contains the number of elements in source-2 rather than source-1.

Each 0-bit from source-2 writes a zero to the output. Each 1-bit writes the next
entry from source-1.

The qpl_expand is the opposite of Select. If you take the output of
expand and perform a select operation on it (with the same bit vector as
source-2), then you get back the same data as the original source-1.


Output Modification
-------------------

The output data can be modified from its nominal form into a modified form. The
nature of the modification depends on the nominal output (a bit-vector, or
an array); i.e. whether the bit-width of the output is 1 or more than 1.

The output modification is specified through an enumeration in the field
out_bit_width. It can have logical values: 0, 8, 16, or 32. A value of 0
corresponds to no output modification. The other three values invoke output
modification.


### Output Modification for Nominal Bit Vector Output

In case of nominal bit vector output, when the bit vector is expected to be sparse, it
may be more efficient to represent the bit vector as a list of indices of those
bits that are 1 rather than as a dense bit vector. In the worst case, if the bit
vector is expected to have only a single 1-bit, it would be better to get the
index of that 1-bit instead of the large array that is almost all 0.

In this case, if output modification is invoked, then the output is a list of
the indices of the 1-bits, where the indices are written as an 8-, 16-, or
32-bit value. If an actual index to be written is larger (in bit-width) than the
specified size, the operation terminates with an error.


### Output Modification for Nominal Array Output

Here, the nominal output format is a packed array of integers. The output
modification is to extend the bit-width of the output to the specified size,
effectively “unpacking” them. This may make it easier for software to process
the results.

For example, if the output is nominally 11-bits wide, it could be written as
16-bit integers. The specified size must be at least as wide as the nominal
sizes. You may **not** truncate an 11-bit wide value to 8-bits.


Aggregates
----------

The filter unit also computes various summaries or aggregates of the data, which
are returned in fields in the job structure. The meaning of these values varies
slightly depending on whether the output is nominally a bit vector or an array:

| Field                 | Bit Vector Meaning             | Array Meaning         |
|-----------------------|--------------------------------|-----------------------|
| first_index_min_value | Index of first 1-bit in output | Min value over output |
| last_index_max_value  | Index of last 1-bit in output  | Max value over output |
| sum_value             | Number of 1-bits in output     | Sum of output values  |

Considering a bit vector output (doing a scan operation), these values
can tell the software how sparse the result is, as well as where one should
start/end walking through the results to find all the 1’s.

**Note** that the meaning of the aggregates is not changed by the output
modification. If the output is nominally a bit vector, and due to the
output modification, the output actually contains 32-bit indices, the aggregates
still reflect the bit vector values.


Advanced Filter Features
------------------------

The library implements a number of “advanced” features that augment or modify the normal
behavior of filter operations.


### Omit Checksums / Aggregates Calculation

If the flag `QPL_FLAG_OMIT_CHECKSUMS` is specified, then CRC/XOR checksums are not calculated
during filtering on software path. `QPL_FLAG_OMIT_AGGREGATES` has the same effect but omits
aggregates calculation. These flags do not directly affect the performance of the hardware path by itself but can allow the library to do some optimizations. See the `Multi-descriptor processing` section for more information.

### Invert_output

Filter operation with the flag QPL_FLAG_INV_OUT specified will result in inverse output in case of a bit-vector-like output. Omitting this flag will not produce inverse output.

For example, doing a SCAN_LT with INV_OUT is the same as doing a SCAN_GE.
Similarly, SCAN_NOT_RANGE with INV_OUT is the same as SCAN_RANGE.

### initial_output_index

When using output modification with a bit-vector output, the indices of the
1-bits are written to the output. By default, the index of the first bit is 0.
By setting the initial_output_index field to a non-zero value, the index of the
first bit can be set.

This could be used, for example, if a column of data was broken into groups of
1,000,000. The first such group could be processed with an initial index of 0,
the second group with an initial index of 1,000,000, the third group with an
initial index of 2,000,000, etc.

**Note #1**: This field only affects the indices written for the modification of the bit vector output. It does not affect the indices used for the extract operation.
Those always start at 0 for the first element.
**Note #2**: This field does not affect operations that invoke src2 -
see the table "Operations" for operations with number of input streams equal to 2. 
This statement means that initialOutputIndex field works for qpl_scan and qpl_find_unique operations only.


### param_low & param_high for Dropping Low and High bits

The size of the set used in qpl_set_membership and qpl_find_unique is limited. If
the bit-width of the input is too large (i.e. the set would be too large for the
hardware to handle), in some cases the application can use a smaller set by
using only some bits from each element. Effectively this “lumps together”
several input values. Bits can be dropped from the most-significant end by
param_high and from the least-significant end by param_low.

The bit-width of the input needs to be greater than the sum of these two fields.
Then the size of the set is 2N, where N=(inputBitWidth – param_high –
param_low).

When this feature is used, each element has the appropriate number of bits
dropped from each end, and the remaining bits are used to index into the set.


### drop_initial_bytes

In some cases, the filter input data may be compressed, and the uncompressed
data might contain some sort of header before the columnar data begins. Assuming
that this header is an integral number of bytes (i.e. that the columnar data
begins on a byte boundary of the uncompressed data), the drop_initial_bytes field
can be used to have the filter unit skip over these bytes.

>Note 1: The `drop_initial_bytes` feature is supported only by 1-source filter operations
>Note 2: Maximal `drop_initial_bytes` feature value is 65,535 bytes

This field could in theory be used if the input data is not compressed. In this case,
 it is better to advance the input buffer pointer to the
start of the columnar data, and adjust the size accordingly.


Other Operations
================


The Intel® Query Processing Library (Intel® QPL) supports several operations other than compress, decompress, and filter. These are described below.


qplZeroCompress
---------------

The library implements 4 zero-compress operations. The library can compress and decompress at either
a 16-bit or a 32-bit granularity. **Note** that this compression has nothing to do with DEFLATE. This is a very light-weight compression scheme that can be useful in certain circumstances.

In these schemes, the data (both compressed and uncompressed) is made of a series of “words”, where the word length is either 16-bits or 32-bits depending on the operation. This means that the input buffer size must be a multiple of the appropriate word size, and that the generated output will also be such a multiple.

During compression, the words are processed N at a time, where N=32 for the 32-bit
case, and N=64 for the 16-bit case. It means that the uncompressed data is always processed 128 bytes at a time, regardless of the word-length. This will be called a block.

The compressed data consists of N tag bits. A 0-bit means that the
corresponding word has a value of 0, and a 1-bit means that the corresponding
word has a value other than 0. This is followed by the non-zero words within
that block.

So for the 32-bit wide case, there are 32 tag bits forming one tag word, and in
the 16-bit case, there are 64 tag bits, forming four tag words.

In general, the last block will not be full; it doesn't have to contain 128 bytes in
it (although the size still needs to be a multiple of the word size). To
represent this, the “missing” words will be encoded in the header as 1-bits (assuming they are non-zero), but no data will be written to the output. When the decompressor sees that a word should be present, but is missing (due to reaching the end of the input stream), it stops outputting words.

These functions do compute the CRC and XOR checksums. The only flag that is
valid to use with them is QPL_FLAG_CRC32C, used to select the CRC. These jobs
are implicitly and FIRST and LAST job, so they cannot be linked.


qpl_crc64
--------

This operation computes an arbitrary CRC up to 64-bytes in size.

The polynomial is expressed in the normal or “not bit-reversed” form in the
job’s crc64_poly field. The most-significant bit of the polynomial (which is
always 1) is not expressed, and the next most-significant bit is written to
bit-63 of the field. That is, the polynomial is written to the high end of the
field. If the polynomial is for a 64-bit CRC, then bit-0 would also be set. If
the CRC is smaller, then the least-significant bit set will be at a higher bit
position.

For example, the CRC32 used by DEFLATE is defined by the 33-bit polynomial
104c11db7. To generate this CRC with the qpl_crc64 operation, one would set the
crc64_poly field to: 04c11db700000000. Since the least-significant set bit is
bit-32, then the size of the resulting CRC would be 64-32 or 32.

By default, the data is viewed as Little Endian. If the QPL_FLAG_CRC64_BE flag
is used, the data is viewed as Big Endian.

When the data is LE, bit-0 of each byte is the least significant. Additionally,
bit-0 of byte-0 is the least significant bit of the output. If the polynomial
is smaller than 64-bits, the CRC is found in the high end of the crc64 field.

When the data is BE, bit-0 of each byte is the most significant. Bit-7 of byte-7
of the result is then the least significant bit. If the polynomial is smaller
than 64-bits, the CRC is found in the low end of the crc64 field.

This operation may not be linked, so the initial CRC is logically always 0, but
many CRC invert (in a bitwise sense) the initial CRC and the final CRC. To
achieve this, the application should use the QPL_FLAG_CRC64_INV flag. **Note** that
the number of bits to be inverted is implied by the size of the polynomial (i.e.
determined by where the least-significant set bit is found).

Here is some well-known CRC’s, with corresponding settings and place to find the result:

| **CRC**       | **crc64_poly**   | **Bit Order**     | **Invert CRC**     | **crc64 (Output)**   |
|---------------|------------------|-------------------|--------------------|----------------------|
| CRC32 (gzip)  | 04C11DB700000000 | QPL_FLAG_CRC64_BE | QPL_FLAG_CRC64_INV | 00000000**XXXXXXXX** |
| CRC32 (wimax) | 04C11DB700000000 |                   | QPL_FLAG_CRC64_INV | **XXXXXXXX**00000000 |
| T10DIF        | 8BB7000000000000 |                   |                    | **XXXX**000000000000 |
| CRC-16-CCITT  | 1021000000000000 | QPL_FLAG_CRC64_BE | QPL_FLAG_CRC64_INV | 000000000000**XXXX** |


qpl_memcpy
---------

The qpl_memcpy operation performs a simple memory copy. The application must define the input buffer and the output buffer. The output buffer must be at least as large as the input buffer, and the two buffers cannot overlap.


Indexing
========

Indexing is used to generate a fully-compliant DEFLATE stream. This stream can be decompressed in its entirety with any compliant DEFLATE decompressor, with “random access” in the middle of the stream.

Key to using this feature is the concept of “mini-blocks”. These are fixed-size
regions of the uncompressed data that can be decompressed independently. The
size of the mini-blocks can be set at powers of 2 from 512 through 32768. This is
also called the “index-size”.

A compressed DEFLATE stream is composed of a series of DEFLATE blocks. The
indexing feature requires that all blocks except for the last one to be a multiple
of the mini-block size. For example, one could have 1kB mini-blocks and 32kB blocks, 
or 2kB mini-blocks and put the entire input into a single DEFLATE block.

The significance of the mini-blocks is the following:

- no references from a given mini-block to any previous one
- no matches that span mini-block boundaries 

This means that each mini-block can be decompressed independently of any other mini-block.

The size of the mini-block becomes the granularity at which “random access” can be
done. To get one particular uncompressed byte, one needs to decompress the
entire mini-block containing that byte (One could decompress that mini-block up to and 
including the needed data, but then one would lose the CRC check). So the smaller the mini-block,
the less work is needed to access any particular byte. On the other hand, the smaller the
mini-blocks become, the worse the compression ratio becomes. So an application needs
to make a reasonable trade-off between having the mini-blocks be too small or too
large.


Creating Indices
----------------

To generate indices, the application needs to allocate an array large enough to
hold all of the index values. The application then defines the mini-block size using
the mini_block_size field, points to the index table memory with the idx_array field,
and specifies the size of the table with the idx_max_size field.

The index generation is done as part of the verification pass, so the
application cannot specify the QPL_FLAG_OMIT_VERIFY flag.


Format of Indices
-----------------

The index array consists of 64-bit records, where each record contains two 32-bit fields:

| 63:32 | CRC        |
|-------|------------|
| 31:0  | Bit Offset |

To use indices for access and understand where the different indices for each block point to, use the following description:

-   The start of the header

-   The start of the first mini-block

-   The starts of the next mini-blocks

-   The start of the End Of Block token

Additionally, there is one extra entry at the end pointing to the end of the
DEFLATE stream.

So in general, if there are B blocks and M mini-blocks, the total number of
index entries generated will be (M+2B+1).

The library supports two common usage models: many-identical-blocks and single-block.

In both cases, the size of the compression input needs to be a multiple of the
mini-block size, unless it is a LAST job.


### Many Identical Blocks

The library has no restriction for different blocks to be the same size or to be a power of 2 in size, still these settings are recommended for more convenience. For example, one could have 1kB mini-blocks and 32kB blocks.

In this case, each block corresponds to one compression operation, where the input size (except for the last one) is the block size. Each such job can use dynamic Huffman generation, or static Huffman tables. With static Huffman tables, each job can specify the QPL_FLAG_START_NEW_BLOCK flag to cause each job to create a block.

The output buffer (the buffer to receive the compressed data) should be sized
large enough to hold the input data as a DEFLATE stored block, i.e. slightly
larger than the uncompressed size. If the data is incompressible and the
compression operation results in data expansion, such that the compressed data
does not fit in the output buffer, the software library attempt to encode
that block as a stored block. If the output buffer does not have size large enough for
this, the compression fails.

If the input data is encoded as a stored block, it must be encoded as a
single stored block. When it becomes multiple stored blocks, the application
is not able to determine where the header indices are located.

This means that either the output buffer needs to be large enough to receive the
compressed data (even if the “compressed” data is expanded), or the size of the
input (i.e. the block size) must be **less than** 64kB.

If the compressed block is replaced by a stored block, the indexing is done in
the same manner as if it is a compressed block: the first index points to the
beginning of the stored-block header, the second to the start of the
(uncompressed) data, the third points to the data that is one “mini-block size” further,
etc. After the last mini-block, there are two pointers that both point to the
start of the next DEFLATE header. The first of these points to a
zero-length EOB, and the second to the block header itself.

To achieve such behavior, there must be exactly one stored block. Since the
largest size of a stored block is (64k-1), if the block-size if 64kB or larger,
one would need more than one stored-block, and the indexing scheme would be jumbled.

In this operation mode, if a stored-block is required (assuming indexing is
requested) and the input size is at least 64kB, then the compression operation
will fail with an error.


### Single Block Usage

The case happens when the entire input is represented as a single DEFLATE
block, even though multiple compress operations may have been done. This is
invoked by not using dynamic Huffman codes, and QPL_FLAG_START_NEW_BLOCK flag.

If the output does not fit into output buffer (requiring a stored
block), then the compression will fail, due to a disruption in the block
structure. The exception to this is if the compression is a single job (i.e.
both a FIRST job and LAST job). In that case, the entire file is represented as a
single stored block, so the compression succeeds.


### Computing the Size of the Index Array

The expected number of index values can be obtained from:

``uint32_t num_index_values(uint32_t buf_size, uint32_t idx_size, uint32_t blk_size);``

where the arguments are the size of the uncompressed data, the size of the mini-block, and the size of the block. It assumes all the blocks to have the same (uncompressed) size (as specified) or this value is 0. If the value is 0, all of the input is compressed into a single block (whose size is the same as buf_size).

One index value is stored per mini-block, two values are stored per block, and one extra index value. In the first example (multi-block usage), 3 blocks and 11 mini-blocks present, so the total number of indices is 3\*2+11+1 = 18. In the second example (single-block usage), 1 block and 11 mini-blocks present, so the total number of indices is 1\*2+11+1 = 14.


Using the Index Array
---------------------

This section will walk you through the steps in creating and using the indices.
The library implements two slightly different approaches: 

- with multiple blocks of fixed size, 
- with a single block.

To allow for CRC checking, the entire mini-block needs to be decompressed. If the desired data straddles two mini-blocks, then the application separately decompresses each of the mini-blocks, and then extracts the data from both.

To use this feature, specify the ignore_start_bits flag and the ignore_end_bits flag. Both flags should contain values from 0 through 7. They indicate the number of bits at the start and end of the first byte to be skipped/ignored. This allows the application to start and end decompression on a bit rather than a byte boundary.

In general, for each job one would get a START bit-offset and an END bit-offset from the index array. More particularly, the START bit is the first bit to be processed. The END bit is the last bit to be
processed plus one. So the total number of bits to be processed is just (END – START).

To convert these to job structure parameters, one needs to calculate:

- the offsets of the byte containing the START bit, 
- the number of bytes to process (i.e. the number of bytes starting with the START byte to include the byte containing the last bit to be processed (END – 1)), 
- the number of bits to ignore at the start of the first byte, 
- the number of bits to ignore at the end of the last byte.

One way to do this is:
```
            job->next_in_ptr = comp_buffer + START/8;
            job->ignore_start_bits = START & 7;
            job->available_in = ((END+7)/8) - (START/8);
            job->ignore_end_bits = 7 & (0 - END);
```

**Note** that ((END+7)/8) - (START/8) is not the same as ((END+7-START)/8).


### Multi-Block Usage

For multi-block case, there is a “block_size”, where all blocks but the last are exactly
that size, and the last block is no larger than that size. The block_size is a
multiple of the mini-block size.

During the compression, each block of input data is submitted as a single job.
Each job is flagged with either QPL_FLAG_DYNAMIC_HUFFMAN or
QPL_FLAG_START_NEW_BLOCK.

Using the indices requires up to two jobs. The first parses the block header, and the second decompresses the specified mini-block. To decompress multiple mini-blocks within a given block, the header only needs to be parsed once. For example, the application can maintain a “block cache”. Then to decode a given mini-block (if its header is in the cache), a single job decompresses the mini-block. If its header is not in the cache, then some entry in the cache is replaced with the decompressed data from the new header, and the second job decompresses the mini-block.

The application needs a derived value: mb_per_b, which is the number of
mini-blocks per block. This is the ratio of the two sizes.

To access the mini-block containing byte “B”, compute which mini-block needs to be decompressed first. This is “M”:

```
M = B / mini-block_size
```

Then compute which block and mini-block within that block contains “B”:

```
blk_num = M / mb_per_b  
mblk_num = M % mb_per_b
```

**Note** that if the mb_per_b is a power of two, then this can be done as a SHIFT and an AND rather than using actual divides.


#### Decompressing the Block Header

The index of the start of the block header is ``(blk_num \* (mb_per_b + 2))``. The index of the end is one greater than this.

The job to parse the header is a decompress job with QPL_FLAG_FIRST and QPL_FLAG_NO_BUFFERING. The START bit offset and the END bit offset are the low 32-bits of the two entries identified earlier. Those offsets are converted to job parameters as previously described.


#### Decompressing the Mini-block

The index of the start of the block header is ``(1 + blk_num \* (mb_per_b + 2) + mb_num)``, and the index of the end is one greater. That is, the start of the mini-block is found by adding the mini-block number within the block to the end of the block header.

For example, assuming the user wants to decompress mini-block 6. In this case, there are 4 mb_per_b. So the blk_num is 1, and the mb_num is 2. The header can be found between indices 6 and 7, and the mini-block between indices 9 and 10.


The job uses the flags QPL_FLAG_NO_BUFFERING and QPL_FLAG_RND_ACCESS.

The initial CRC (i.e. the value written to job crc before the job is executed)
is the CRC value (the high 32-bits of the index array) from the start of the
mini-block. In the previous example, that is from index 9. After the job
completes, the returned CRC value (in job crc) should match the CRC value from
the end of the mini-block, in this example 10. If it does not match, then the
mini-block is not decompressed properly.


### Single Block Usage 

This scenario is similar to the Multi-block case, except that there is only one block, and the mb_per_b is not used.

In this case, the header is always between indices 0 and 1, and the start of the mini-block is index M+1. So using the example earlier, if the user wants to decompress mini-block 6, the start of that mini-block would be index 7.


Non-DEFLATE Usages
==================

To use the hardware for non-DEFLATE usages, three advanced flags can be specified:

| HUFFMAN_BE   | compress or decompress | Huffman tokens are in Big Endian Format |
|--------------|------------------------|-----------------------------------------|
| NO_HDRS      | decompress             | Parse only Huffman Tokens               |
|              | compress               | Write no headers or EOBs                |
| GEN_LITERALS | compress-only          | Generate only Literals                  |

The GEN_LITERALS can be used with any compression job. If HUFFMAN_BE is
specified, then NO_HDRS must be specified as well. The only allowed combination
of these are: none, NO_HDRS, NO_HDRS+HUFFMAN_BE.

The HUFFMAN_BE flag tells the hardware that the Huffman tokens are in a 16-bit big-endian format. When HUFFMAN_BE is specified, then NO_HDRS must be specified as well, although the NO_HDRS flag itself can be used without HUFFMAN_BE.

When the NO_HDRS flag is used for decompress jobs, it should be used for all jobs, although it is needed for the FIRST job and the LAST job (i.e. the jobs with the FIRST and LAST flags set).

When a decompress job has NO_HDRS and FIRST set, it instructs the driver to
configure the hardware to expect the bit-stream to start with a Huffman token
rather than with a block header. It also requires a decompress Huffman
table to be provided, which is used to configure the hardware appropriately.

When a decompress job has NO_HDRS and LAST set, it instructs the driver to tell the hardware not to expect the stream to end with an EOB token.

The decompress Huffman table can be constructed by the application, or it can be constructed by the auxiliary functions
qpl_triplets_to_decompression_table() or qpl_comp_to_decompression_table().

When the NO_HDRS flag is used for compress jobs, it instructs the driver not to write any block header or trailer (i.e. EOB tokens) to the stream.

If NO_HDRS is used with DYNAMIC_HUFFMAN, then the entire file must be contained
in the single block. This means that both FIRST AND LAST must be specified. Also
in this case the user must include a compress Huffman table structure. This
structure is to be overwritten with the generated Huffman Table.

The GEN_LITERALS flag is only for compress jobs. This instructs the hardware to
generate only literal tokens and no match tokens. Currently, the decompressor,
when using the NO_HDRS flag, can only parse literal tokens. So the compressor,
when using NO_HDRS, must use the GEN_LITERALS flag, otherwise the result would
not be decompressed with the Intel QPL.


Big Endian 16 Format
--------------------

Normal DEFLATE streams are little-endian (LE). Tokens are written starting at bit-0 of each byte and extending from bit-7 of byte-0 to bit-0 of byte-1. 

For example, if there are four 5-bit tokens, then would be written to the first 3 bytes as:

| 3 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | Bytes  |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|--------|
| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | Bits   |
| … | … | 5 | 5 | 5 | 5 | 5 | 4 | 4 | 4 | 4 | 4 | 3 | 3 | 3 | 3 | 3 | 2 | 2 | 2 | 2 | 2 | 1 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | Tokens |

The Huffman codes used as an example in Section 11.1 can be used either in
non-bit-reversed, or bit-reversed form:

| Non-bit-reversed | Bit-reversed |
|------------------|--------------|
| 000              | 000          |
| 001              | 100          |
| 0100             | 0010         |
| 0101             | 1010         |
| 0110             | 0110         |
| 0111             | 1110         |
| 1000             | 0001         |
| 1001             | 1001         |
| 10100            | 00101        |

The difference is: 

- the non-bit-reversed forms need to be parsed starting at the high-order bit, 
- the bit-reversed forms need to be parsed starting at the low-order bit.

Normal (LE) DEFLATE streams use the Bit-reversed form, as the tokens are parsed starting at bit-0.

In the Big-Endian-16 format, the tokens are written in each 16-bit Word, starting at the high-order bit:

| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | Words  |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|--------|
| 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | Bytes  |
| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | Bits   |
| 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 2 | 2 | 2 | 2 | 2 | 3 | 3 | 3 | 3 | 3 | 4 | 4 | 4 | 4 | 4 | 5 | 5 | 5 | 5 | 5 | … | … | Tokens |

Here, the non-bit-reversed form of the Huffman Tokens needs to be used.

When the data (while being read/written to the user’s buffer) is bit-reversed within every 16-bit word, after the bit reversal, it looks like:

| 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | Words  |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|--------|
| 2 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | Bytes  |
| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | Bits   |
| … | … | 5 | 5 | 5 | 5 | 5 | 4 | 4 | 4 | 4 | 4 | 3 | 3 | 3 | 3 | 3 | 2 | 2 | 2 | 2 | 2 | 1 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | Tokens |

**Note** that in the figure earlier, “Words”, “Bytes”, and “Bits” refer to the
original value before the bit reversal.

After the bit-reversal, the tokens appear as if the input stream is encoded in LE format. To process BE16 data, all we need to bit-reverse each 16-bit word as we read it or write it, and otherwise pretend that it is LE data. Note that as we pretending that the data is LE, we need to use the bit-reversed form of the Huffman Codes as well.


Canned Mode Compression and Decompression
-----------------------------------------

Compression with canned mode is like static style compression, but no
deflate header will be written to the output stream. To use it: 

- specify the pointer to the Huffman table. It cannot be NULL and must point to a valid Huffman table. 
- specify the `QPL_FLAG_CANNED_MODE` flag. 

**Note** that the FIRST job and the LAST job logic remains the same. 

This approach is particularly good for compressing the series of small data, so the compression ratio for every block is expected
to be higher.

To decompress such stream, pointer to the decompression table should be valid
and the following combinations of the flags should be set:

``QPL_FLAG_NO_BUFFERING | QPL_FLAG_RND_ACCESS | QPL_FLAG_CANNED_MODE``

For better clarity, see canned mode examples.


Dictionary Compression
-----------------------------------------

Intel QPL supports compression with preset dictionary by implementing
several auxiliary functions that work with `qpl_dictionary`.


### Building the Dictionary

First, you need to allocate the buffer for `qpl_dictionary`.
The size of that buffer can be obtained by the `qpl_get_dictionary_size(...)` function.

```c
qpl_get_dictionary_size(sw_compression_level, hw_compression_level, size_t)
```

This function accepts 3 arguments: software and hardware compression levels, and the size (in bytes) of a raw dictionary. 

To do the compression only on software path, set the `hw_compression_level` parameter to `HW_NONE`. In this case, the buffer size needed for `qpl_dictionary` will be less.

After allocating the buffer, `qpl_build_dictionary(...)` function should be used to fill the `qpl_dictionary`.

```c
qpl_build_dictionary(qpl_dictionary *dict_ptr,
                     sw_compression_level sw_level,
                     hw_compression_level hw_level,
                     const uint8_t *raw_dict_ptr,
                     size_t raw_dict_size))
```

This function accepts pointer to allocated dictionary, software and hardware
compression levels, pointer to array containing dictionary to use, and its length.
*Note*: if dictionary length is larger than 4,096 bytes, then only the last 4,096 bytes
will be used.

Several auxiliary functions can be used to work with dictionary:

- `qpl_get_dictionary_id(..)` and `qpl_set_dictionary_id(...)` sets and gets dictionary ID of `qpl_dictionary` (can be used in case of zlib header).
- `qpl_get_existing_dict_size(...)` is used to get the size (in bytes) of the `qpl_dictionary` structure built.


### Compressing with Dictionary

Compression with preset dictionary can be done for dynamic, fixed, static,
and canned compression modes. In order to do the compression, the `qpl_job->dictionary`
field should point to the built dictionary:

```c
// ...
job_ptr->op = qpl_op_compress;
job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC'
job_ptr->dictionary = dictionary_ptr;

qpl_status = qpl_execute_job(job_ptr);
```

The dictionary cannot be set in the middle of the compression stream. The job should be marked as FIRST.

To decompress the stream previously compressed with the dictionary, the same dictionary should be specified for the first decompression job:

```c
// ...
job_ptr->op = qpl_op_decompress;
job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST'
job_ptr->dictionary = dictionary_ptr;

qpl_status = qpl_execute_job(job_ptr);
```


Advanced Decompress Features
============================

By default, the system expects a DEFLATE stream to end cleanly during decompression; otherwise it reports an error. The library expects the stream to end at the end of a block (i.e. with an “EOB”). That block mast be marked as a final (i.e. the “b_final” bit is set in the block header). When the library detects a b_final EOB, it stops processing.

The application can change this behavior by setting the job ``decomp_end_processing`` field. This holds an enumeration. The enumeration determines the conditions for stopping the processing, as well as the expectations for how the stream should end. The meaning of the enumeration values is:

| Value                                       | Stop on     | Expect to End on |
|---------------------------------------------|-------------|------------------|
| qpl_stop_and_check_for_bfinal_eob (default) | b_final EOB | b_final EOB      |
| qpl_dont_stop_or_check                      |             |                  |
| qpl_stop_and_check_for_any_eob              | EOB         | EOB              |
| qpl_stop_on_any_eob                         | EOB         |                  |
| qpl_stop_on_bfinal_eob                      | b_final EOB |                  |
| qpl_check_for_any_eob                       |             | EOB              |
| qpl_check_for_bfinal_eob                    |             | b_final EOB      |

In particular, on a non-Last job (i.e. a job without the LAST flag set), only
the specified “stop” condition will be applied. There will be no expectation as
to the end of that job’s buffer. That expectation only applies for a LAST job.
To apply the expectation also to a non-LAST job, add (or “or”) in the value
qpl_check_on_nonlast_block.

For example, given a value of qpl_stop_and_check_for_any_eob, the processing stops when EOB on any job appears. The library returns error if the LAST job doesn’t end with an EOB. In contrast, the values (qpl_stop_and_check_for_any_eob + qpl_check_for_bfinal_eob) allow the library to stop on an EOB on any job. In this case, an error is returned on any job that is not ended with EOB.

For example, in some non-standard use cases, there may be multiple blocks, so the processing should not stop on an EOB, and the stream might be expected to end with an EOB, but that block is not expected to have the b_final flag set. In this case, the user can specify qpl_check_for_any_eob. Then the user gets an error if the stream does not end with an EOB, but the processing will not stop until it reaches the end of the bit stream.

As mentioned under “Indexing”, the ignore_start_bits and ignore_start_bytes
fields can be used to start and stop decompression on a bit granularity rather
than a byte granularity.


Gathering Deflate Statistics
============================

The qpl_gather_deflate_statistics() collects the deflate statistics (a literal/length tokens histogram and a distance tokens histogram) for a given vector:

```
status = qpl_gather_deflate_statistics(source_ptr, source_length, histogram_ptr, compression_level, execution_path)
```

where:

```
source_ptr                   - pointer to source, for which deflate statistics is collected
source_length                - length of the given vector
histogram_ptr                - pointer to histgoram to be updated
compression_level            - compression level of the deflate algorithm (it can be either qpl_default_level, or qpl_high_level)
execution_path               - execution path parameter (**note:** for now this variable is just a stub, statistics gathering is performed by software only,
                               this will be fixed in the following release)
```

This function can be used multiple times for processing one large source vector with the
sequence of smaller buffers, i.e. it updates given histograms with the new statistics.
In this case, further compression ratio may suffer.


Huffman Tables
==============

Two formats can be applied for Huffman Tables (one for the compressor and one for the decompressor). 
The decompression Huffman Table is needed for certain advanced use cases. Normally, the user never needs to deal with
it. See Section 9 for cases that would require use of decompress Huffman tables.

These structures are not public. The user cannot access or modify its fields. Consider the following snippet:

```
// Allocate buffer for compression table
uint8_t *comp_table_buffer_ptr = (uint8_t *)malloc(QPL_COMPRESSION_TABLE_SIZE);
// Initialize buffer with zeros
memset(comp_table_buffer_ptr, 0u, QPL_COMPRESSION_TABLE_SIZE);
// Get pointer to the compression table
qpl_compression_huffman_table *comp_table_ptr = (qpl_compression_huffman_table *)comp_table_buffer_ptr;

// ...
// Now, you can use compression table with Intel QPL, for example set it to qpl_job:

qpl_job_ptr->compression_huffman_table = comp_table_ptr;
// ...
```

The usage for decompression table is the same, except for pointer type and buffer
size constant (`qpl_decompression_huffman_table`, and respectively `QPL_DECOMPRESSION_TABLE_SIZE`).


Creating Compression Huffman Tables from Deflate Histogram
----------------------------------------------------------

The ``qpl_build_compression_table_from_statistics`` function is used to build the compression huffman table from
deflate tokens histograms.

```
status = qpl_build_compression_table_from_statistics();
```

Built table is guaranteed to be complete (in terms of that every deflate token is assigned a value).


Huffman Triplets
----------------

An array of the ``qpl_huffman_triplet`` objects is considered as a "generic format" Huffman table. See doxygen code annotations for
more information about this structure.


Creating Compression and Decompression Huffman Tables from Huffman Triplets
---------------------------------------------------------------------------

To build Huffman tables from array of `qpl_huffman_triplet`, use functions:

-   `qpl_triplets_to_compression_table()`
-   `qpl_triplets_to_decompression_table()`


Converting Compression Table into Decompression Table
-----------------------------------------------------

Another way to build a decompression table is to use the function `qpl_comp_to_decompression_table()`, which is
capable of converting compression table into decompression.


Advanced Hardware Path Optimizations
====================================

The following section contains information about optimizations for hardware path supported by Intel QPL.


Multi-Descriptor Processing
---------------------------

The library can split `Set membership` and `Scan` operations onto several sub-tasks and
perform them in paralleled mode. In other words, if the following conditions are met:

* This is `Set membership` or `Scan` (`Scan range`) operation
* Nominal bit array output, i.e. no output modifications
* Input data size is at least 32kB large
* Input is in Little- or Big-Endian format (no encoded input support)
* Aggregates and checksums are disabled via appropriate flags
* No more than 1 work queue per 1 device configured
* Operation should be executed with synchronous interface `qpl_execute_job(...)`

**Note**: the last limitation is temporary due to library refactoring.

Then the operation is auto-paralleled on the library level during execution on hardware path. This is intended to
receive better performance when processing large streams.


Job Structure Description
=========================

| Field                        | Type   | Description                                                                                                                        |
|------------------------------|--------|------------------------------------------------------------------------------------------------------------------------------------|
| next_in_ptr                  | IN/OUT | Pointer to next input byte for main input stream                                                                                   |
| available_in                 | IN/OUT | Number of valid bytes in main input stream                                                                                         |
| total_in                     | OUT    | Total number of bytes read from main input stream                                                                                  |
| next_out_ptr                 | IN/OUT | Pointer to next output byte for output stream                                                                                      |
| available_out                | IN/OUT | Size of output stream                                                                                                              |
| total_out                    | OUT    | Total number of bytes written to output stream                                                                                     |
| op                           | IN     | Operation to be done                                                                                                               |
| flags                        | IN     | Flags indicating various operation modes                                                                                           |
| crc                          | IN/OUT | CRC of uncompressed data/CRC seed                                                                                                  |
| xor_checksum                 | OUT    | XOR checksum of uncompressed data                                                                                                  |
| last_bit_offset              | OUT    | Number of output bits written modulo 8.                                                                                            |
| compression_huffman_table    | IN     | Pointer to Compress Huffman Table                                                                                                  |
| decompression_huffman_table  | IN     | Pointer to Decompress Huffman Table                                                                                                |
| mini_block_size              | IN     | Enumeration for indexing mini-block size                                                                                           |
| idx_array                    | IN     | Pointer to table to write the index table into                                                                                     |
| idx_max_size                 | IN     | Size of index table (in terms of qword entries)                                                                                    |
| idx_num_written              | OUT    | Number of index entries actually written.                                                                                          |
| decomp_end_processing        | IN     | Enumeration determining on what conditions the processing should stop, and what the expectations are for the state when stopped.   |
| ignore_start_bits            | IN     | Number of bits at the start of the first byte to be ignored by the decompressor.                                                   |
| ignore_end_bits              | IN     | Number of bits at the end of the last byte to be ignored by the decompressor.                                                      |
| crc64_poly                   | IN     | Polynomial used for the crc64 operation.                                                                                           |
| crc64                        | IN/OUT | Initial and final CRC value for the crc64 operation.                                                                               |
| next_src2_ptr                | IN/OUT | Pointer to source-2 data. Updated value is returned.                                                                               |
| available_src2               | IN/OUT | Number of valid bytes of source-2 data.                                                                                            |
| src1_bit_width               | IN     | Source-1 bit width for Analytics. Valid values are 1–32.                                                                           |
| src2_bit_width               | IN     | Source-2 bit width for Analytics. Valid values are 1–32.                                                                           |
| num_input_elements           | IN     | Number of input elements for Analytics.                                                                                            |
| out_bit_width                | IN     | Output bit width enumeration. Valid values are “nominal”, 8-, 16-, or 32-bits.                                                     |
| param_low                    | IN     | Low parameter for operations extract or scan, or a number of low-order bits to ignore for set_membership or find_unique.           |
| param_high                   | IN     | High parameter for operations extract or scan, or a number of high-order bits to ignore for set_membership or find_unique.         |
| drop_initial_bytes           | IN     | Number of initial bytes to be dropped at the start of the Analytics portion of the pipeline.                                       |
| initial_output_index         | IN     | The “index” of initial output element from Analytics. This affects modified bit-vector output and the bit-vector aggregate values. |
| parser                       | IN     | Enumeration of what parser to use to parse Analytics source-1 data.                                                                |
| first_index_min_value        | OUT    | Output aggregate value.                                                                                                            |
| last_index_max_value         | OUT    | Output aggregate value.                                                                                                            |
| sum_value                    | OUT    | Output aggregate value.                                                                                                            |
| numa_id                      | IN     | Parameter specifying hardware NUMA node where operation should be done. Set by default to -1 for auto detecting.                   |


Examples
========

This section contains a number of simple examples, illustrating how you can
use the Intel QPL. Note that these examples are intended to be illustrative and
functional, but they are not intended to be examples of a complete
implementation. In particular, their handling of error conditions is rather
primitive.

These examples use two common definitions:

```
const uint32_t BITS_PER_BYTE = 8;
#define BITS_TO_BYTES(x) ((x) + BITS_PER_BYTE - 1) / BITS_PER_BYTE;
```

The examples are:

| Analytics: Simple Scan                     | Scan a uint32 vector for a particular value                                                      |
|--------------------------------------------|--------------------------------------------------------------------------------------------------|
| Analytics: Scan for Unique Value           | Scan a packed array for a unique value                                                           |
| Analytics: Key-Value Lookup                | Scan a key array for a range; return matching elements of value array.                           |
| Decompress: Single Buffer                  | Decompress a single input buffer into a single output buffer                                     |
| Decompress: Files using Fixed-size Buffers | Decompress one file into another, using fixed-size buffers, and handling QPL_MORE_OUTPUT_NEEDED. |


Analytics: Simple Scan
----------------------

This example routine scans an array of uint32 for a particular value, fills
in an array with the indices of the matching elements, and returns the number of
indices/elements found.

```
// Scan, and return a list of indices of matching values
// returns number of values found, or 0 if output overflow or error
uint32_t QPLScanVec(uint32_t *inVec,  // Input vector
                  uint32_t nelem,   // number of input elements
                  uint32_t value,   // value to search for
                  uint32_t *outVec, // pointer to output vector
                  uint32_t numout)  // max number of output elements
{
    qpl_job *job;
    QplStaus status;
    uint32_t  size, ret;

    status = qpl_get_job_size( qpl_path_auto, &size );
    job = (qpl_job*)malloc( size );
    status = qpl_init_job( qpl_path_auto, job);
    job->next_in_ptr = (uint8_t*)inVec;
    job->available_in = nelem * sizeof(inVec[0]);
    job->next_out_ptr = (uint8_t*)outVec;
    job->available_out = numout * sizeof(outVec[0]);
    job->op = qpl_op_scan_eq;
    job->src1_bit_width = sizeof(inVec[0]) * BITS_PER_BYTE;
    job->num_input_elements = nelem;
    job->out_bit_width = qpl_ow_32; // unpack output to dwords
    job->param_low = value;

    status = qpl_execute_job(job);
    ret = job->total_out / sizeof(outVec[0])
    free( job );
    if (status != QPL_OK)
        return 0;

    return ret;
}
```


Analytics: Scan for Unique Value
--------------------------------

This example routine searches a packed array (with a specified bit-width) for a unique value. If this value is found, it returns the index. It also returns indications if the value is not found or if it is found multiple times.

```
// Look for a particular value in a list of unique values.
// Return the index of the matching value or
// -1 if it wasn't found, or
// -2 if it was found more than once, or
// -3 if there was an error.
// Input array is packed with some bit width.
int QPLScanPVecUniq(uint8_t *inVec,  // Input packed vector
                    uint32_t nelem,   // number of input elements
                    uint32_t width,   // bit width of input elements
                    uint32_t value)   // value to search for
{
    qpl_job *job;
    qpl_status status;
    uint32_t outVec;
    uint32_t  size, ret;

    status = qpl_get_job_size( qpl_path_auto, &size );
    job = (qpl_job*)malloc( size );
    status = qpl_init_job( qpl_path_auto, job);
    job->next_in_ptr = inVec;
    job->available_in = BITS_TO_BYTES(nelem * width);
    job->next_out_ptr = (uint8_t*)&outVec;
    job->available_out = sizeof(outVec);
    job->op = qpl_op_scan_eq;
    job->src1_bit_width = width;
    job->num_input_elements = nelem;
    job->out_bit_width = qpl_ow_32; // unpack output to dwords
    job->param_low = value;

    status = qpl_execute_job(job);
    ret = job->total_out;
    free( job );
    if (status != QPL_OK) {
        if (status == 225)
            return -2;
        else
            return -3;
    }

    if ( ret != 4)
        return -1;
    return (int)outVec;
}
```


Analytics: Key-Value Lookup
---------------------------

This example illustrates using two different queries to answer a more
complicated question. In this case, the routine will look in a “key” packed
array for entries falling within a specified range. It will then return an
unpacked (i.e. uint32) array of elements from “value” packed array, which
corresponds to those selected keys. It also uses a temporary bit-vector array for
intermediate results. It assumes that all the arrays have the same length (in
terms of number of elements). **Note** that the key/value arrays do not need to
have the same bit-width.

```
// Look up keys in a range of key, 
// return corresponding value elements
int QPLLookup(uint8_t *keys,        // key array, packed uints
              uint32_t keyWidth,    // bit width of key array
              uint8_t *values,      // value array, packed uints
              uint32_t valWidth,    // bit width of value array
              uint32_t *results,    // result array, uint32
              uint32_t nelem,       // num of elements in each array
              uint32_t rangeLow,    // low end of key range
              uint32_t rangeHigh,   // high end of key range
              uint8_t *tmp)         // temp array nelem bits long
{
    qpl_job *job;
    qpl_status status;
    uint32_t  size, ret;

    status = qpl_get_job_size( qpl_path_auto, &size );
    job = (qpl_job*)malloc( size );
    status = qpl_init_job( qpl_path_auto, job);
    job->next_in_ptr = keys;
    job->available_in = BITS_TO_BYTES(nelem * keyWidth);
    job->next_out_ptr = tmp;
    job->available_out = BITS_TO_BYTES(nelem * 1);
    job->op = qpl_op_scan_range;
    job->src1_bit_width = keyWidth;
    job->num_input_elements = nelem;
    job->out_bit_width = qpl_ow_nom; // no output mod
    job->param_low = rangeLow;
    job->param_high = rangeHigh;

    status = qpl_execute_job(job);
    if (status != QPL_OK)
    {
        free( job );
        return -1;
    }
    job->next_in_ptr = values;
    job->available_in = BITS_TO_BYTES(nelem * valWidth);
    job->next_out_ptr = (uint8_t*)results;
    job->available_out = BITS_TO_BYTES(nelem * sizeof(results[0]));
    job->next_src2_ptr = tmp;
    job->available_src2 = BITS_TO_BYTES(nelem * 1);
    job->op = qpl_op_select;
    job->src1_bit_width = valWidth;
    job->src2_bit_width = 1;
    job->num_input_elements = nelem;
    job->out_bit_width = qpl_ow_32; // 32-bit output width

    status = qpl_execute_job(job);
    ret = job->total_out / 4;
    free( job );
    if (status != QPL_OK)
        return -1;

    return ret;
}
```

Decompress: Single Buffer
-------------------------

This example takes a single buffer, compressed as a GZIP file, and tries to
decompress it into a single buffer. It then checks that the CRC computed by Intel QPL
matches that in the GZIP trailer.

```
// Decompress a single gzip buffer in a single job
// returns the size of the decompressed buffer, or a negative value
// if there is an error
int QPLDecompBuffer(uint8_t *inBuf,    // ptr to input (compressed) buffer
                    uint32_t inSize,   // size of input buffer
                    uint8_t *out_buf,   // pointer to output (uncompressed) buffer
                    uint32_t outSize)  // size of output buffer
{
    qpl_job *job;
    qpl_status status;
    uint32_t bufferCRC, bufferSize;
    uint32_t  size, ret;

    status = qpl_get_job_size( qpl_path_auto, &size );
    job = (qpl_job*)malloc( size );
    status = qpl_init_job( qpl_path_auto, job);
    job->next_in_ptr = inBuf;
    job->available_in = inSize;
    job->next_out_ptr = out_buf;
    job->available_out = outSize;
    job->op = qpl_op_decompress;
    job->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_GZIP_MODE;

    status = qpl_execute_job(&job);
    ret = job->total_out;
    free( job );
    if (status != QPL_OK)
        return -(int)status;

    // get gzip CRC
    bufferCRC = *(uint32_t*)(inBuf + inSize - 8);
    if (bufferCRC != job->crc)
        return -1000;
    bufferSize = *(uint32_t*)(inBuf + inSize - 4);
    if (bufferSize != ret)
        return -1001;
    return ret;
}
```


Decompress: Files Using Fixed-Size Buffers
------------------------------------------

This example illustrates decompressing a gzip file into another file, using
fixed-size buffers. In particular, the output buffers are the same size as the
input buffers, so it is expected that for each input buffer read, the
decompressor is called a number of times before all the input
data is processed (i.e. it will have to gracefully handle the
QPL_MORE_OUTPUT_NEEDED return status).

```
// decompress inFile into outFile using fixed buffers
// returns 0 on success
int QPLDecompFiles(char *inFile, char *outFile)
{
    qpl_job job;
    qpl_status status;
    const uint32_t BSIZE = 32*1024;
    uint8_t inBuf[BSIZE], out_buf[BSIZE];
    FILE *fpin, *fpout;
    uint32_t  size;

    fpin = fopen(inFile,"rb");
    if (fpin == NULL)
        return 1;
    fpout = fopen(outFile,"wb");
    if (fpout == NULL) {
        fclose(fpin);
        return 2;
    }
 
    status = qpl_get_job_size( qpl_path_auto, &size );
    job = (qpl_job*)malloc( size );
    status = qpl_init_job( qpl_path_auto, job);

    job->flags = QPL_FLAG_FIRST | QPL_FLAG_GZIP_MODE;
    job->op = qpl_op_decompress;

    while (!feof(fpin)) {
        size = (uint32_t) fread(inBuf, 1, BSIZE, fpin);
        if (size == 0)
            break;
        if (feof(fpin))
            job->flags |= QPL_FLAG_LAST;
        job->next_in_ptr = inBuf;
        job->available_in = size;
        while (1) {
            job->next_out_ptr = out_buf;
            job->available_out = BSIZE;
            status = qpl_execute_job(&job);
            fwrite(out_buf, 1, BSIZE - job->available_out, fpout);
            job->flags &= ~QPL_FLAG_FIRST;
            if (status != QPL_MORE_OUTPUT_NEEDED)
                break;
        }
        if (status != QPL_OK) {
            fclose(fpin);
            fclose(fpout);
            free( job );
            return 3;
        }
    } // end if (!feof(fpin))
    fclose(fpin);
    fclose(fpout);
    free( job );
    return 0;
}
```
