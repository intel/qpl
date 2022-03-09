 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


qplZeroCompress
###############


The library implements 4 zero-compress operations. The library can
compress and decompress at either a 16-bit or a 32-bit granularity.

.. note:: 
    
    This compression has nothing to do with DEFLATE. This is a
    very light-weight compression scheme that can be useful in certain
    circumstances.


In these schemes, the data (both compressed and uncompressed) is made of
a series of “words”, where the word length is either 16-bits or 32-bits
depending on the operation. This means that the input buffer size must
be a multiple of the appropriate word size, and that the generated
output will also be such a multiple.

During compression, the words are processed N at a time, where N=32 for
the 32-bit case, and N=64 for the 16-bit case. It means that the
uncompressed data is always processed 128 bytes at a time, regardless of
the word-length. This will be called a block.

The compressed data consists of N tag bits. A 0-bit means that the
corresponding word has a value of 0, and a 1-bit means that the
corresponding word has a value other than 0. This is followed by the
non-zero words within that block.

So for the 32-bit wide case, there are 32 tag bits forming one tag word,
and in the 16-bit case, there are 64 tag bits, forming four tag words.

In general, the last block will not be full; it does not have to contain
128 bytes in it (although the size still needs to be a multiple of the
word size). To represent this, the “missing” words will be encoded in
the header as 1-bits (assuming they are non-zero), but no data will be
written to the output. When the decompressor sees that a word should be
present, but is missing (due to reaching the end of the input stream),
it stops outputting words.

These functions do compute the CRC and XOR checksums. The only flag that
is valid to use with them is ``QPL_FLAG_CRC32C``, used to select the CRC.
These jobs are implicitly both FIRST and LAST job, so they cannot be
linked.
