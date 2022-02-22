<!--
Copyright (C) 2022 Intel Corporation 
SPDX-License-Identifier: MIT
-->

Intel® Query Processing Library (Intel® QPL) Status Codes
==================================================================

Notices and Disclaimers
-----------------

No license (express or implied, by estoppel or otherwise) to any intellectual
property rights is granted by this document.

Intel disclaims all express and implied warranties, including withoutЛ
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

This software and the related documents are Intel copyrighted materials,
and your use of them is governed by the express license under which they
were provided to you ("License"). Unless the License provides
otherwise, you may not use, modify, copy, publish, distribute, disclose
or transmit this software or the related documents without Intel's prior
written permission. This software and the related documents are provided
as is, with no express or implied warranties, other than those that are
expressly stated in the License.

# Status Codes

The return values are defined in ``qpl_defs.h``.

In general, they fall into three categories:

1. Initialization statuses inform about errors during thr initialization step.
2. Processing statuses inform about execution status: Done, In Progress, etc. They include several error statuses.
3. Operation statuses inform about incorrect parameters, corrupted streams, and other situations relative to operation per se.

Full list of status codes is below:

| Status  | Description                                                                                              |
|---------|----------------------------------------------------------------------------------------------------------|
|         | **Processing Statuses**                                                                                  |
|   0     | Operation completed successfully                                                                         |
|   1     | Job is still being processed                                                                             |
|   4     | A job after a LAST job was not marked as FIRST                                                           |
|   5     | Descriptor can't be submitted into filled work queue                                                     |
|         | **Operations Errors::Incorrect parameter Value**                                                         |
|   50    | Null pointer error                                                                                       |
|   51    | Non-supported value in the qplJob operation field                                                        |
|   52    | Indicates an error if the requested mode is not supported                                                |
|   53    | Indicates that the job structure does not match the operation                                            |
|   54    | Incorrect value for the qpl_path input parameter                                                         |
|   55    | Invalid combination of fields in the qpl_job structure                                                   |
|   56    | qpl_job flags field contains conflicted values                                                           |
|   57    | Incorrect size error                                                                                     |
|   58    | Buffer exceeds max size supported by library                                                             |
|   60    | Incorrect polynomial value for CRC64                                                                     |
|   70    | Set is too large for operation                                                                           |
|   71    | Non-supported value in the qplJob parser field                                                           |
|   72    | qplJob out_bit_width field contains invalid value                                                        |
|   73    | Incorrect dropBits value (param_low + param_high must be beyond 0..32)                                   |
|   74    | qpl_job bit-width field contains an invalid value for current output format                              |
|   80    | Flags specify NO_HDRS and DYNAMIC_HUFFMAN, but no Huffman table provided                                 |
|   81    | Invalid Huffman table data                                                                               |
|   82    | Indexing enabled but but Indexing table is not set                                                       |
|   83    | The style of a compression job does not match the style of the previous related job                      |
|   84    | Inflate needs dictionary to perform decompression                                                        |
|   85    | The qpl_job field for decompression manipulation is incorrect                                            |
|   86    | Invalid block size used during indexing                                                                  |
|         | **Operations Errors::Execution Fail**                                                                    |  
|   100   | Currently unused                                                                                         |
|   101   | Not enough memory for the operation                                                                      |
|   102   | Indexing buffer is too small                                                                             |
|   103   | Mini-block creation error                                                                                |
|   104   | Invalid GZIP/Zlib header                                                                                 |
|   105   | Gzip/Zlib header specifies unsupported compress method                                                   |
|   201   | Reached the end of the input stream before decoding header and header is too big to fit in input buffer  |
|   202   | Bad CL code                                                                                              |
|   203   | First code in LL tree is 16                                                                              |
|   204   | First code in D tree is 16                                                                               |
|   205   | All LL codes are specified with 0 length                                                                 |
|   206   | After parsing LL code lengths, total codes != expected value                                             |
|   207   | After parsing D code lengths, total codes != expected value                                              |
|   208   | First CL code of length N is greater than 2^N-1                                                          |
|   209   | First LL code of length N is greater than 2^N-1                                                          |
|   210   | First D code of length N is greater than 2^N-1                                                           |
|   211   | Incorrect LL code                                                                                        |
|   212   | Incorrect D code                                                                                         |
|   213   | Invalid type of deflate block                                                                            |
|   214   | Length of stored block doesn't match inverse length                                                      |
|   215   | EOB flag was set but last token was not EOB                                                              |
|   216   | Decoded Length code is 0 or greater 258                                                                  |
|   217   | Decoded Distance is 0 or greater than History Buffer                                                     |
|   218   | Distance of reference is before start of file                                                            |
|   219   | Library has input data, but is not making forward progress                                               |
|   220   | PRLE format is incorrect or is truncated                                                                 |
|   221   | Output index value is greater than max available for current output data type                            |
|   222   | Unexpected internal error condition                                                                      |
|   223   | Source 1 contained fewer than expected elements/bytes                                                    |
|   224   | Source 2 contained fewer than expected elements/bytes                                                    |
|   225   | qpl_job destination buffer has less bytes than required to process num_input_elements/bytes              |
|   226   | Distance spans mini-block boundary on indexing                                                           |
|   227   | Length spans mini-block boundary on indexing                                                             |
|   228   | Invalid block size (not multiple of mini-block size)                                                     |
|   229   | Verify logic for decompress detected incorrect output                                                    |
|   230   | Compressor tried to use an invalid huffman code                                                          |
|   231   | Bit width is out of range [1..32]                                                                        |
|   232   | The input stream ended before specified Number of input Element was seen                                 |
|   233   | Invalid value for a counter (32bit) in PrleExpand, specifically, counter < prev counter or exceeds 2^16  |
|   234   | Invalid header for the ZeroDecompress functionality                                                      |
|   235   | The number of LL codes specified in the DEFLATE header exceed 286                                        |
|   236   | The number of D codes specified in the DEFLATE header exceed 30                                          |
|         | **Initialization Errors**                                                                                |
|   501   | libaccel is not found or not compatible                                                                  |
|   502   | libaccel internal error                                                                                  |
|   503   | Supported and enabled work queues are not found                                                          |
