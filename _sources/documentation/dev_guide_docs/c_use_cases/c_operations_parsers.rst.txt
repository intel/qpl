 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Parser
######


The "parser" is responsible for converting a series of bytes into a
series of unsigned integers. The "parser" is defined by an enumeration
:c:enum:`qpl_parser` in the field :c:member:`qpl_job.parser`.

The default value :c:member:`qpl_parser.qpl_p_le_packed_array` views the input buffer as a
little-endian packed array of N-bit integers, where N is given by
src1_bit_width. For example, if N=3, then the first element will be bits
2:0 in the first byte, the second element will be bits 5:3, etc.

If the parser is :c:member:`qpl_parser.qpl_p_be_packed_array`, the buffer is viewed as a
big-endian packed array. For example, with N=3, the first element will be bits
7:5 of the first byte, the second element will be bits 4:2, etc.

If the parser is specified as :c:member:`qpl_parser.qpl_p_parquet_rle`, it is viewed as being
in Parquet RLE format. In this case, the bit width is given in the data
stream, so :c:member:`qpl_job.src1_bit_width` must be set to 0.

Source-2 can only be "parsed" as a packed-array. The default parser
views the source-2 data a little-endian packed array. If the
:c:macro:`QPL_FLAG_SRC2_BE` flag is specified, then it is viewed as a big-endian
packed array.


Parquet RLE Format
******************

The input is in the Parquet RLE format. The first byte of the data
stream gives the bit width. This is followed by the encoded data. The
bit-width cannot exceed 32-bits.

The format is:

::

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
      obtained bits are connected sequentially - therefore the 1st bit of the
      second byte must go to the 7th position of the resulting unsigned integer.


.. attention::
    The standard varint can consist of 5 encoded bytes.
    In the Intel® Query Processing Library (Intel® QPL), it is limited by 4.
