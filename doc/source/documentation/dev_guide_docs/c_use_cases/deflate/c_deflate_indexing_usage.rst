 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Creating and Using Indices
##########################

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


Creating Indices
****************


To generate indices, the application needs to allocate an array large
enough to hold all of the index values. The application then defines the
mini-block size using the :c:member:`qpl_job.mini_block_size` field,
points to the index table memory with the :c:member:`qpl_job.idx_array`
field, and specifies the size of the table with the
:c:member:`qpl_job.idx_max_size` field.

The index generation is done as part of the verification pass, so the
application cannot specify the :c:macro:`QPL_FLAG_OMIT_VERIFY` flag.


Intel® Query Processing Library (Intel® QPL) implements two
slightly different approaches:

-  with multiple blocks of fixed size,
-  with a single block.

To allow for CRC checking, the entire mini-block needs to be
decompressed. If the desired data straddles two mini-blocks, then the
application separately decompresses each of the mini-blocks, and then
extracts the data from both.

To use this feature, specify the ``ignore_start_bits`` flag and the
``ignore_end_bits`` flag. Both flags should contain values from 0 through 7.
They indicate the number of bits at the start and end of the first byte
to be skipped/ignored. This allows the application to start and end
decompression on a bit rather than a byte boundary.

In general, for each job one would get a START bit-offset and an END
bit-offset from the index array. More particularly, the START bit is the
first bit to be processed. The END bit is the last bit to be processed
plus one. So the total number of bits to be processed is just (END -
START).

To convert these to job structure parameters, one needs to calculate:

-  the offsets of the byte containing the START bit,
-  the number of bytes to process (i.e. the number of bytes starting
   with the START byte to include the byte containing the last bit to be
   processed (END - 1)),
-  the number of bits to ignore at the start of the first byte,
-  the number of bits to ignore at the end of the last byte.

One way to do this is:

::

               job->next_in_ptr = comp_buffer + START/8;
               job->ignore_start_bits = START & 7;
               job->available_in = ((END+7)/8) - (START/8);
               job->ignore_end_bits = 7 & (0 - END);


**Note** that ((END+7)/8) - (START/8) is not the same as ((END+7-START)/8).


Multi-Block Usage
=================


For multi-block case, there is a “block_size”, where all blocks but the
last are exactly that size, and the last block is no larger than that
size. The block_size is a multiple of the mini-block size.

During the compression, each block of input data is submitted as a
single job. Each job is flagged with either :c:macro:`QPL_FLAG_DYNAMIC_HUFFMAN` or
:c:macro:`QPL_FLAG_START_NEW_BLOCK`.

Using the indices requires up to two jobs. The first parses the block
header, and the second decompresses the specified mini-block. To
decompress multiple mini-blocks within a given block, the header only
needs to be parsed once. For example, the application can maintain a
“block cache”. Then to decode a given mini-block (if its header is in
the cache), a single job decompresses the mini-block. If its header is
not in the cache, then some entry in the cache is replaced with the
decompressed data from the new header, and the second job decompresses
the mini-block.

The application needs a derived value: *mb_per_b*, which is the number of
mini-blocks per block. This is the ratio of the two sizes.

To access the mini-block containing byte “B”, compute which mini-block
needs to be decompressed first. This is “M”:

::

   M = B / mini-block_size

Then compute which block and mini-block within that block contains “B”:

::

   blk_num = M / mb_per_b
   mblk_num = M % mb_per_b


**Note** that if the *mb_per_b* is a power of two, then this can be done
as a SHIFT and an AND rather than using actual divides.


Decompressing the Block Header
------------------------------


The index of the start of the block header is
``(blk_num \* (mb_per_b + 2))``. The index of the end is one greater
than this.

The job to parse the header is a decompress job with :c:macro:`QPL_FLAG_FIRST` and
:c:macro:`QPL_FLAG_RND_ACCESS`. The START bit offset and the END bit offset are
the low 32-bits of the two entries identified earlier. Those offsets are
converted to job parameters as previously described.


Decompressing the Mini-block
----------------------------


The index of the start of the block header is
``(1 + blk_num \* (mb_per_b + 2) + mb_num)``, and the index of the end
is one greater. That is, the start of the mini-block is found by adding
the mini-block number within the block to the end of the block header.

For example, assuming the user wants to decompress mini-block 6. In this
case, there are 4 mb_per_b. So the blk_num is 1, and the mb_num is 2.
The header can be found between indices 6 and 7, and the mini-block
between indices 9 and 10.

The job uses the flag :c:macro:`QPL_FLAG_RND_ACCESS`.

The initial CRC (i.e. the value written to job crc before the job is
executed) is the CRC value (the high 32-bits of the index array) from
the start of the mini-block. In the previous example, that is from index 1.
After the job completes, the returned CRC value (in job crc) should
match the CRC value from the end of the mini-block, in this example 10.
If it does not match, then the mini-block is not decompressed properly.

.. note::

    The state of decompression with mini-blocks is not saved from job
    to job. Namely the :c:member:`qpl_job.total_out` will not store the
    cumulative number of bytes written, but only that job's particular
    mini-block's number of bytes written.

Single Block Usage
==================


This scenario is similar to the Multi-block case, except that there is
only one block, and the *mb_per_b* is not used.

In this case, the header is always between indices 0 and 1, and the
start of the mini-block is index M+1. So using the example earlier, if
the user wants to decompress mini-block 6, the start of that mini-block
would be index 7.
