 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Compression Styles
##################


The Intel® Query Processing Library (Intel® QPL)
has many ways to compress the input data. The most common
approaches are described in this section.


Fixed Block
***********


The simplest approach is to generate one large Fixed Block. To achieve
this, do NOT use the ``QPL_FLAG_DYNAMIC_HUFFMAN`` flag, and set the pointer
to the Huffman table to *NULL*.

For compressible data, this will result in one large fixed block. If
multiple jobs are used (i.e. the first job is not marked as FIRST and
LAST), then the result will be one large fixed block followed by a
zero-length stored block. When multiple jobs are used, and some of them
contain incompressible data, then the result will be a mixture of fixed
blocks and stored blocks.


Static Blocks
*************


The Static Blocks case is similar to the Fixed Block case. Do NOT use
the ``QPL_FLAG_DYNAMIC_HUFFMAN`` flag. The pointer to the Huffman table
must point to a valid Huffman table. This will result in
nominally one large block, except for potential stored blocks as
described in the previous case.

This approach, however, gives the application the ability to change the
Huffman Tables in the middle of the stream. If the stream is compressed
with multiple jobs, then in the middle of the stream, the application
can specify a different Huffman table and use the
``QPL_FLAG_START_NEW_BLOCK`` flag. This will instruct the library to end the
current block and to start a new block with the new table. The
``QPL_FLAG_START_NEW_BLOCK`` is not needed on the first job; it is implied.

If the ``QPL_FLAG_START_NEW_BLOCK`` flag is not used, then the tables
pointed to in the job structure must be the same as those used by the
previous job. When the tables are changed without indication to the
library, then the resulting bit-stream will not be valid.

The :c:func:`qpl_huffman_table_init_with_histogram()` auxiliary function takes as an input a
histogram of the Literal-Length (LL) tokens and the distance (D) tokens,
and fills in a compression Huffman table structure.
Refer to the :ref:`huffman-tables-api-label` section for more information.


Dynamic Blocks
**************


If the jobs are submitted with the ``QPL_FLAG_DYNAMIC_HUFFMAN`` flag, then
the data from that job will be compressed as a single dynamic DEFLATE
block (when buffer overflow occurs, STORED block is stored instead of
DYNAMIC one). This gives the best compression ratio, but requires two
passes through the hardware.


Mixing Styles
*************


The style cannot be changed in the middle of a stream. That is, if the
first job in a sequence of jobs has a particular style, then the
remaining jobs in that sequence must use the same style.


Verification
************


By default, the library will verify the correctness of the generated
compressed bit stream. The library decompresses the resulting bit
stream, and then checks that the CRC of the uncompressed data matches
the CRC of the original data. The verification step adds one more pass
through the hardware. That is, for a fixed stream or static stream, each
job results in two passes through the hardware (one for compression, one
for verification). For a dynamic stream, it results in three passes.

If the user does not want to pay the cost for verification, this can be
turned off with the ``QPL_FLAG_OMIT_VERIFY`` flag.

.. note::
    Currently verification is not performed in case of ``Huffman only BE``.
