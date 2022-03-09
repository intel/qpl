 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _c_compression_streams_reference_link:


Compression Streams
###################


Indexing
********


For information on creating and using indices, refer to the
:ref:`Creating and Using Indices <c_creating_and_using_indices_reference_link>` chapter.

Indexing is used to generate a fully-compliant DEFLATE stream. This
stream can be decompressed in its entirety with any compliant DEFLATE
decompressor, with “random access” in the middle of the stream.

Key to using this feature is the concept of “mini-blocks”. These are
fixed-size regions of the uncompressed data that can be decompressed
independently. The size of the mini-blocks can be set at powers of 2
from 512 through 32768. This is also called the “index-size”.

A compressed DEFLATE stream is composed of a series of DEFLATE blocks.
The indexing feature requires that all blocks except for the last one to
be a multiple of the mini-block size. For example, one could have 1kB
mini-blocks and 32kB blocks, or 2kB mini-blocks and put the entire input
into a single DEFLATE block.

The significance of the mini-blocks is the following:

-  no references from a given mini-block to any previous one
-  no matches that span mini-block boundaries

This means that each mini-block can be decompressed independently of any
other mini-block.

The size of the mini-block becomes the granularity at which “random
access” can be done. To get one particular uncompressed byte, one needs
to decompress the entire mini-block containing that byte (One could
decompress that mini-block up to and including the needed data, but then
one would lose the CRC check). So the smaller the mini-block, the less
work is needed to access any particular byte. On the other hand, the
smaller the mini-blocks become, the worse the compression ratio becomes.
So an application needs to make a reasonable trade-off between having
the mini-blocks be too small or too large.


Creating Indices
****************


To generate indices, the application needs to allocate an array large
enough to hold all of the index values. The application then defines the
mini-block size using the mini_block_size field, points to the index
table memory with the idx_array field, and specifies the size of the
table with the idx_max_size field.

The index generation is done as part of the verification pass, so the
application cannot specify the ``QPL_FLAG_OMIT_VERIFY`` flag.


Format of Indices
*****************


The index array consists of 64-bit records, where each record contains
two 32-bit fields:

===== ==========
63:32 CRC
===== ==========
31:0  Bit Offset
===== ==========


To use indices for access and understand where the different indices for
each block point to, use the following description:


-  The start of the header

-  The start of the first mini-block

-  The starts of the next mini-blocks

-  The start of the End Of Block token



Additionally, there is one extra entry at the end pointing to the end of
the DEFLATE stream.

So in general, if there are B blocks and M mini-blocks, the total number
of index entries generated will be (M+2B+1).

The library supports two common usage models: many-identical-blocks and
single-block.

In both cases, the size of the compression input needs to be a multiple
of the mini-block size, unless it is a LAST job.


Many Identical Blocks
=====================


The library has no restriction for different blocks to be the same size
or to be a power of 2 in size, still these settings are recommended for
more convenience. For example, one could have 1kB mini-blocks and 32kB
blocks.

In this case, each block corresponds to one compression operation, where
the input size (except for the last one) is the block size. Each such
job can use dynamic Huffman generation, or static Huffman tables. With
static Huffman tables, each job can specify the ``QPL_FLAG_START_NEW_BLOCK``
flag to cause each job to create a block.

The output buffer (the buffer to receive the compressed data) should be
sized large enough to hold the input data as a DEFLATE stored block,
i.e. slightly larger than the uncompressed size. If the data is
incompressible and the compression operation results in data expansion,
such that the compressed data does not fit in the output buffer, the
software library attempt to encode that block as a stored block. If the
output buffer does not have size large enough for this, the compression
fails.

If the input data is encoded as a stored block, it must be encoded as a
single stored block. When it becomes multiple stored blocks, the
application is not able to determine where the header indices are
located.

This means that either the output buffer needs to be large enough to
receive the compressed data (even if the “compressed” data is expanded),
or the size of the input (i.e. the block size) must be **less than**
64kB.

If the compressed block is replaced by a stored block, the indexing is
done in the same manner as if it is a compressed block: the first index
points to the beginning of the stored-block header, the second to the
start of the (uncompressed) data, the third points to the data that is
one “mini-block size” further, etc. After the last mini-block, there are
two pointers that both point to the start of the next DEFLATE header.
The first of these points to a zero-length EOB, and the second to the
block header itself.

To achieve such behavior, there must be exactly one stored block. Since
the largest size of a stored block is (64k-1), if the block-size if 64kB
or larger, one would need more than one stored-block, and the indexing
scheme would be jumbled.

In this operation mode, if a stored-block is required (assuming indexing
is requested) and the input size is at least 64kB, then the compression
operation will fail with an error.


Single Block
============


The case happens when the entire input is represented as a single
DEFLATE block, even though multiple compress operations may have been
done. This is invoked by not using dynamic Huffman codes, and
``QPL_FLAG_START_NEW_BLOCK`` flag.

If the output does not fit into output buffer (requiring a stored
block), then the compression will fail, due to a disruption in the block
structure. The exception to this is if the compression is a single job
(i.e. both a FIRST job and LAST job). In that case, the entire file is
represented as a single stored block, so the compression succeeds.


Computing the Size of the Index Array
*************************************


The expected number of index values can be obtained from:

``uint32_t num_index_values(uint32_t buf_size, uint32_t idx_size, uint32_t blk_size);``

where the arguments are the size of the uncompressed data, the size of
the mini-block, and the size of the block. It assumes all the blocks to
have the same (uncompressed) size (as specified) or this value is 0. If
the value is 0, all of the input is compressed into a single block
(whose size is the same as buf_size).

One index value is stored per mini-block, two values are stored per
block, and one extra index value. In the first example (multi-block
usage), 3 blocks and 11 mini-blocks present, so the total number of
indices is 3*2+11+1 = 18. In the second example (single-block usage), 1
block and 11 mini-blocks present, so the total number of indices is
1*2+11+1 = 14.


Non-DEFLATE Usages
******************

To use the hardware for non-DEFLATE usages, three advanced flags can be
specified:

+-----------+--------------------+------------------------------------+
| H         | compress or        | Huffman tokens are in Big Endian   |
| UFFMAN_BE | decompress         | Format                             |
+===========+====================+====================================+
| NO_HDRS   | decompress         | Parse only Huffman Tokens          |
+-----------+--------------------+------------------------------------+
|           | compress           | Write no headers or EOBs           |
+-----------+--------------------+------------------------------------+
| GEN       | compress-only      | Generate only Literals             |
| _LITERALS |                    |                                    |
+-----------+--------------------+------------------------------------+

The ``GEN_LITERALS`` can be used with any compression job. If ``HUFFMAN_BE`` is
specified, then ``NO_HDRS`` must be specified as well. The only allowed
combination of these are: ``none``, ``NO_HDRS``, ``NO_HDRS+HUFFMAN_BE``.

The ``HUFFMAN_BE`` flag tells the hardware that the Huffman tokens are in a
16-bit big-endian format. When ``HUFFMAN_BE`` is specified, then ``NO_HDRS``
must be specified as well, although the ``NO_HDRS`` flag itself can be used
without ``HUFFMAN_BE``.

When the ``NO_HDRS`` flag is used for decompress jobs, it should be used for
all jobs, although it is needed for the FIRST job and the LAST job
(i.e. the jobs with the FIRST and LAST flags set).

When a decompress job has ``NO_HDRS`` and FIRST set, it instructs the driver
to configure the hardware to expect the bit-stream to start with a
Huffman token rather than with a block header. It also requires a
decompress Huffman table to be provided, which is used to configure the
hardware appropriately.

When a decompress job has ``NO_HDRS`` and LAST set, it instructs the driver
to tell the hardware not to expect the stream to end with an EOB token.

The decompress Huffman table can be constructed by the application, or
it can be constructed by the auxiliary functions
``qpl_triplets_to_decompression_table()`` or
``qpl_comp_to_decompression_table()``.

When the ``NO_HDRS`` flag is used for compress jobs, it instructs the driver
not to write any block header or trailer (i.e. EOB tokens) to the
stream.

If ``NO_HDRS`` is used with ``DYNAMIC_HUFFMAN``, then the entire file must be
contained in the single block. This means that both FIRST and LAST must
be specified. Also in this case the user must include a compress Huffman
table structure. This structure is to be overwritten with the generated
Huffman Table.

The ``GEN_LITERALS`` flag is only for compress jobs. This instructs the
hardware to generate only literal tokens and no match tokens. Currently,
the decompressor, when using the ``NO_HDRS`` flag, can only parse literal
tokens. So the compressor, when using ``NO_HDRS``, must use the ``GEN_LITERALS``
flag, otherwise the result would not be decompressed with the Intel QPL.


Big Endian 16 Format
====================


Normal DEFLATE streams are little-endian (LE). Tokens are written
starting at bit-0 of each byte and extending from bit-7 of byte-0 to
bit-0 of byte-1.

For example, if there are four 5-bit tokens, then would be written to
the first 3 bytes as:

+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+----+
| 3 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | B  |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | yt |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | es |
+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+====+
| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | Bi |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | ts |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+----+
| … | … | 5 | 5 | 5 | 5 | 5 | 4 | 4 | 4 | 4 | 4 | 3 | 3 | 3 | 3 | 3 | 2 | 2 | 2 | 2 | 2 | 1 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | To |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | ke |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | ns |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+----+


The Huffman codes can be used either
in non-bit-reversed, or bit-reversed form:


================ ============
Non-bit-reversed Bit-reversed
================ ============
000              000
001              100
0100             0010
0101             1010
0110             0110
0111             1110
1000             0001
1001             1001
10100            00101
================ ============


The difference is:

-  the non-bit-reversed forms need to be parsed starting at the high-order bit,
-  the bit-reversed forms need to be parsed starting at the low-order bit.

Normal (LE) DEFLATE streams use the Bit-reversed form, as the tokens are
parsed starting at bit-0.

In the Big-Endian-16 format, the tokens are written in each 16-bit Word,
starting at the high-order bit:

+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+----+
| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | W  |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | or |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | ds |
+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+====+
| 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | B  |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | yt |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | es |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+----+
| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | Bi |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | ts |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+----+
| 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 2 | 2 | 2 | 2 | 2 | 3 | 3 | 3 | 3 | 3 | 4 | 4 | 4 | 4 | 4 | 5 | 5 | 5 | 5 | 5 | … | … | To |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | ke |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | ns |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+----+

Here, the non-bit-reversed form of the Huffman Tokens needs to be used.

When the data (while being read/written to the user's buffer) is
bit-reversed within every 16-bit word, after the bit reversal, it looks
like:

+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+----+
| 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | W  |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | or |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | ds |
+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+===+====+
| 2 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | B  |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | yt |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | es |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+----+
| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | Bi |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | ts |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+----+
| … | … | 5 | 5 | 5 | 5 | 5 | 4 | 4 | 4 | 4 | 4 | 3 | 3 | 3 | 3 | 3 | 2 | 2 | 2 | 2 | 2 | 1 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | To |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | ke |
|   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | ns |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+----+


**Note** that in the figure earlier, “Words”, “Bytes”, and “Bits” refer
to the original value before the bit reversal.

After the bit-reversal, the tokens appear as if the input stream is
encoded in LE format. To process BE16 data, all we need to bit-reverse
each 16-bit word as we read it or write it, and otherwise pretend that
it is LE data. Note that as we pretending that the data is LE, we need
to use the bit-reversed form of the Huffman Codes as well.
