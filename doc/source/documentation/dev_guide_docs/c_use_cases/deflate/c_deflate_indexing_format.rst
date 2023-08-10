 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Format of Indices
#################


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
static Huffman tables, each job can specify the :c:macro:`QPL_FLAG_START_NEW_BLOCK`
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
receive the compressed data (even if the "compressed" data is expanded),
or the size of the input (i.e. the block size) must be **less than**
64kB.

If the compressed block is replaced by a stored block, the indexing is
done in the same manner as if it is a compressed block: the first index
points to the beginning of the stored-block header, the second to the
start of the (uncompressed) data, the third points to the data that is
one "mini-block size" further, etc. After the last mini-block, there are
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
:c:macro:`QPL_FLAG_START_NEW_BLOCK` flag.

If the output does not fit into output buffer (requiring a stored
block), then the compression will fail, due to a disruption in the block
structure. The exception to this is if the compression is a single job
(i.e. both a FIRST job and LAST job). In that case, the entire file is
represented as a single stored block, so the compression succeeds.

