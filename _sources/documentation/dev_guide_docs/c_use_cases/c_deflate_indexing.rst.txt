 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Deflate with Indexing
#####################

Indexing is used to generate a fully-compliant Deflate stream. This
stream can be decompressed in its entirety with any compliant Deflate
decompressor, with "random access" in the middle of the stream.

Key to using this feature is the concept of "mini-blocks". These are
fixed-size regions of the uncompressed data that can be decompressed
independently. The size of the mini-blocks can be set at powers of 2
from 512 through 32768. This is also called the "index-size".

A compressed Deflate stream is composed of a series of Deflate blocks.
The indexing feature requires that all blocks except for the last one to
be a multiple of the mini-block size. For example, one could have 1 KB
mini-blocks and 32 KB blocks, or 2 KB mini-blocks and put the entire input
into a single Deflate block.

The significance of the mini-blocks is the following:

-  no references from a given mini-block to any previous one
-  no matches that span mini-block boundaries

This means that each mini-block can be decompressed independently of any
other mini-block.

The size of the mini-block becomes the granularity at which "random
access" can be done. To get one particular uncompressed byte, one needs
to decompress the entire mini-block containing that byte (One could
decompress that mini-block up to and including the needed data, but then
one would lose the CRC check). So the smaller the mini-block, the less
work is needed to access any particular byte. On the other hand, the
smaller the mini-blocks become, the worse the compression ratio becomes.
So an application needs to make a reasonable trade-off between having
the mini-blocks be too small or too large.

.. warning::
   The implementation of indexing is in progress.


.. toctree::
   :maxdepth: 4
   :hidden:

   Format of Indices <c_deflate_indexing_format>
   Creating and Using Indices <c_deflate_indexing_usage>
