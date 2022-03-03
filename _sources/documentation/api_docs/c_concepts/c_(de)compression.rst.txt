 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Compression and Decompression
#############################


Decompression
*************


Decompression of a large stream can be accomplished through linking
multiple jobs. The first job is specified with the flag ``QPL_FLAG_FIRST``,
and the last job is specified with the flag ``QPL_FLAG_LAST``. If the stream
consists of a single job, then both flags should be specified.

The decompressor expects the stream to be formatted with DEFLATE data.
The GZIP standard defines an encapsulation layer around a DEFLATE
stream. In particular, a GZIP stream consists of a GZIP header, a
DEFLATE stream, and a GZIP trailer. The decompressor looks at a raw
DEFLATE stream by default. If the ``QPL_FLAG_GZIP_MODE`` flag is specified,
then the library will skip over the GZIP header on the FIRST job, and
then start parsing the DEFLATE stream.

In general, the application may not know how much data a given
compressed stream will uncompress to. To handle this, the 
Intel® Query Processing Library (Intel® QPL)
allows a decompress job to complete “successfully” but with not all
input processed. This is called euphemistically *output overflow*
although no actual overflow occurs (The label comes from the fact that
the output buffer would have overflowed if the decompressed did not stop
early).

If the job completes with the status ``QPL_MORE_OUTPUT_NEEDED``, then the
output buffer filled up before the input is completely processed (Note
that output buffer may be considered “full” even though the bytes
remaining in it are not zero). In this case, the remaining data from
this job needs to be processed before a new job (if exists) is
submitted. Due to the update of the input buffer parameters, these do
not need to be reset, but the output buffer parameters need to be reset.
They can be pointed to a new buffer, or the current output buffer could
be processed (e.g. written to disk) and then reused.

**Note** that the ``available_in`` job field could be 0, and the status could
still be ``QPL_MORE_OUTPUT_NEEDED``. In this case, the subsequent job will
be submitted with a zero-length input buffer.

For decompression, the CRC and checksum fields are calculated on the
output of the decompressor, that is, on the uncompressed data.

Normally, the hardware will only write complete qwords (8 bytes) on all,
but a LAST job. If the flag ``QPL_FLAG_DECOMP_FLUSH_ALWAYS`` is used, then
even on a non-LAST job, all the processed data will be written to the
output.

In case when the application passes in a tiny input buffer, the library
will normally copy the data into internal buffer, and try to combine it
with other small input buffers before submitting it to the hardware. If
the flag ``QPL_FLAG_NO_BUFFER`` is specified, then this option will be
ignored.


Advanced Decompress Features
****************************

By default, the system expects a DEFLATE stream to end cleanly during
decompression; otherwise it reports an error. The library expects the
stream to end at the end of a block (i.e. with an “EOB”). That block
mast be marked as a final (i.e. the “b_final” bit is set in the block
header). When the library detects a b_final EOB, it stops processing.

The application can change this behavior by setting the job
``decomp_end_processing`` field. This holds an enumeration. The
enumeration determines the conditions for stopping the processing, as
well as the expectations for how the stream should end. The meaning of
the enumeration values is:

+-----------------------------------------+-----------+----------------+
| Value                                   | Stop on   | Expect to End  |
|                                         |           | on             |
+=========================================+===========+================+
| qpl_stop_and_check_for_bfinal_eob       | b_final   | b_final EOB    |
| (default)                               | EOB       |                |
+-----------------------------------------+-----------+----------------+
| qpl_dont_stop_or_check                  |           |                |
+-----------------------------------------+-----------+----------------+
| qpl_stop_and_check_for_any_eob          | EOB       | EOB            |
+-----------------------------------------+-----------+----------------+
| qpl_stop_on_any_eob                     | EOB       |                |
+-----------------------------------------+-----------+----------------+
| qpl_stop_on_bfinal_eob                  | b_final   |                |
|                                         | EOB       |                |
+-----------------------------------------+-----------+----------------+
| qpl_check_for_any_eob                   |           | EOB            |
+-----------------------------------------+-----------+----------------+
| qpl_check_for_bfinal_eob                |           | b_final EOB    |
+-----------------------------------------+-----------+----------------+


In particular, on a non-Last job (i.e. a job without the LAST flag set),
only the specified “stop” condition will be applied. There will be no
expectation as to the end of that job's buffer. That expectation only
applies for a LAST job. To apply the expectation also to a non-LAST job,
add (or “or”) in the value *qpl_check_on_nonlast_block*.

For example, given a value of *qpl_stop_and_check_for_any_eob*, the
processing stops when EOB on any job appears. The library returns error
if the LAST job does not end with an EOB. In contrast, the values
(*qpl_stop_and_check_for_any_eob* + *qpl_check_for_bfinal_eob*) allow the
library to stop on an EOB on any job. In this case, an error is returned
on any job that is not ended with EOB.

For example, in some non-standard use cases, there may be multiple
blocks, so the processing should not stop on an EOB, and the stream
might be expected to end with an EOB, but that block is not expected to
have the ``b_final`` flag set. In this case, the user can specify
*qpl_check_for_any_eob*. Then the user gets an error if the stream does
not end with an EOB, but the processing will not stop until it reaches
the end of the bit stream.

The ``ignore_start_bits`` and ``ignore_start_bytes`` fields can be used 
to start and stop decompression on a bit granularity rather than a byte granularity.


Compression
***********


At a high level, compression is very similar to decompression. The
application specifies the input buffer and the output buffer, submits
one or more jobs, marks the FIRST job and the LAST job, etc. Compression
has more possible variations, and ways to deal with *output overflow* 
(for more information on *output overflow*, refer to the decompression part).

The DEFLATE standard organizes the compressed data into a series of
“blocks”. Each block is defined by a type, with the three types being:

-  *Stored Block*: the input data is stored in an uncompressed form.
-  *Fixed Block*: the input data is compressed with a fixed Huffman table
   defined by the standard. This is usually a suboptimal code, but in
   this case the block header is very small, as it doesn't need to
   define a set of Huffman codes.
-  *Dynamic Block*: the input data is compressed with a Huffman table that
   is optimized for that particular block. This results in a more
   efficient code, but requires two passes: one pass to analyze the
   data, and another pass to do the compression after the optimal
   Huffman table is generated.

The library extends this with what it calls a *Static Block*. From the
DEFLATE point of view, this is a Dynamic Block, but from the library
point of view, the Huffman Table is provided by the application, so that
the compression can happen in one pass. Application must make sure that
the table is appropriate for the data; otherwise, the compression ratio
may suffer.

The main reason to start a new block in the middle of the stream is to
change the type of block generated, or to start using a different
Huffman table.

The Intel QPL does not save history state between jobs. The compressed
form of the data from one job will never reference data from an earlier
job. For example, when 10 jobs of 1,000 bytes are submitted, each will
generate different output. In this case, one job with all 10,000 bytes
can be submitted. In general, submitting very small buffers will result
in a worse compression ratio than submitting fewer large buffers.


Compression and Output Overflow
*******************************

Compression job cannot be partially completed. Each compression job will
complete successfully with all the input processed, or it will fail with
an error and conceptually no output generated.

To complete the compression in any case, the library can specify minimum
size for the output buffer. The size of the output buffer must be bigger
than the input buffer. This actual value can be obtained from the
auxiliary function ``min_compress_buffer_size(input_size)``. If the output
buffer is at least this size, then the input will always be processed
through to the output, barring some error condition. If the output
buffer is smaller than the value specified, there is a chance that the
output will not fit. The status ``QPL_LARGER_OUTPUT_NEEDED`` will be
returned as a result.

If the compressed output does not fit into the output buffer specified,
the library attempts to copy the input data into the stream as a series
of Stored Blocks. When the Stored Blocks fit, the library returns a
successful status. In case when the Stored Blocks do not fit, the
library returns the ``QPL_LARGER_OUTPUT_NEEDED`` status. With the output
buffer is at least as large as ``min_compress_buffer_size(input_size)``
specifies, then the Stored Blocks will always fit, and the
``QPL_LARGER_OUTPUT_NEEDED`` status is not returned.


Compression and Decompression with Canned Mode 
**********************************************


Compression with canned mode is like static style compression, but no
deflate header will be written to the output stream. To use it:

-  specify the pointer to the Huffman table. It cannot be *NULL* and must
   point to a valid Huffman table.
-  specify the ``QPL_FLAG_CANNED_MODE`` flag.

**Note** that the FIRST job and the LAST job logic remains the same.

This approach is particularly good for compressing the series of small
data, so the compression ratio for every block is expected to be higher.

To decompress such stream, pointer to the decompression table should be
valid and the following combinations of the flags should be set:

``QPL_FLAG_NO_BUFFERING | QPL_FLAG_RND_ACCESS | QPL_FLAG_CANNED_MODE``

For better clarity, see canned mode examples.


Dictionary Compression
**********************


Intel QPL supports compression with preset dictionary by implementing
several auxiliary functions that work with ``qpl_dictionary``. For
more information, see the :ref:`Compressing with Preset Dictionary 
<c_building_dictionary_reference_link>` chapter.

