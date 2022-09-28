
 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Huffman-Only Compression/Decompression
######################################

The Intel® Query Processing Library (Intel® QPL) supports Huffman-only
compression and decompression.


.. warning::
   The implementation of Huffman-only compression/decompression is in progress.


To use the hardware for non-DEFLATE usages, three advanced flags can be
specified:

+--------------+--------------------+------------------------------------+
| HUFFMAN_BE   | compress or        | Huffman tokens are in Big Endian   |
|              | decompress         | format                             |
+--------------+--------------------+------------------------------------+
| NO_HDRS      | decompress         | Parse only Huffman tokens          |
+--------------+--------------------+------------------------------------+
|              | compress           | Write no headers or EOBs           |
+--------------+--------------------+------------------------------------+
| GEN_LITERALS | compress-only      | Generate only literals             |
|              |                    |                                    |
+--------------+--------------------+------------------------------------+

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
:c:func:`qpl_huffman_table_init_with_triplets()`,
:c:func:`qpl_huffman_table_init_with_histogram` or
:c:func:`qpl_huffman_table_init_with_other()`.
Refer to the :ref:`huffman-tables-api-label`
section for more information.

If ``NO_HDRS` and ``GEN_LITERALS`` flags are used for decompression,
then the pointer to the decompression Huffman table must be non-null.
Either it must point to a reserved memory area where the table be created
in the case ``DYNAMIC_HUFFMAN``, or to already created table otherwise.

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

If ``GEN_LITERALS`` flag is used for compression, then the pointer to the
compression Huffman table must be non-null. Either it must point to a reserved
memory area where the table be created in the case ``DYNAMIC_HUFFMAN``, or to
already created table otherwise.


Big Endian 16 Format
====================


Normal DEFLATE streams are little-endian (LE). Tokens are written
starting at bit-0 of each byte and extending from bit-7 of byte-0 to
bit-0 of byte-1.

For example, if there are four 5-bit tokens, then would be written to
the first 3 bytes as:

+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+--------+
| 3 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | Bytes  |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+--------+
| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | Bits   |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+--------+
| … | … | 5 | 5 | 5 | 5 | 5 | 4 | 4 | 4 | 4 | 4 | 3 | 3 | 3 | 3 | 3 | 2 | 2 | 2 | 2 | 2 | 1 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | Tokens |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+--------+


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

+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+--------+
| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | Words  |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+--------+
| 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | Bytes  |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+--------+
| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | Bits   |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+--------+
| 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 2 | 2 | 2 | 2 | 2 | 3 | 3 | 3 | 3 | 3 | 4 | 4 | 4 | 4 | 4 | 5 | 5 | 5 | 5 | 5 | … | … | Tokens |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+--------+

Here, the non-bit-reversed form of the Huffman Tokens needs to be used.

When the data (while being read/written to the user's buffer) is
bit-reversed within every 16-bit word, after the bit reversal, it looks
like:

+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+--------+
| 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | Words  |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+--------+
| 2 | 2 | 2 | 2 | 2 | 2 | 2 | 2 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 | Bytes  |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+--------+
| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | Bits   |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+--------+
| … | … | 5 | 5 | 5 | 5 | 5 | 4 | 4 | 4 | 4 | 4 | 3 | 3 | 3 | 3 | 3 | 2 | 2 | 2 | 2 | 2 | 1 | 1 | 1 | 1 | 1 | 0 | 0 | 0 | 0 | 0 | Tokens |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+--------+


**Note** that in the figure earlier, “Words”, “Bytes”, and “Bits” refer
to the original value before the bit reversal.

After the bit-reversal, the tokens appear as if the input stream is
encoded in LE format. To process BE16 data, all we need to bit-reverse
each 16-bit word as we read it or write it, and otherwise pretend that
it is LE data. Note that as we pretending that the data is LE, we need
to use the bit-reversed form of the Huffman Codes as well.
