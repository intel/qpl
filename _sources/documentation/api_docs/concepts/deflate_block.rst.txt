 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Deflate Block
#############


The Deflate block is a representation of a DEFLATE stream, which allows
the library for “random access” in the middle of the stream. The main
idea is to split uncompressed data for fixed-size mini-blocks that can
be decompressed independently. The size of the mini-blocks is a power of
2 from 512 through 32768.

With this approach, when accessing one particular uncompressed byte,
only the mini-block containing that byte is decompressed.

.. note::
    
    The smaller the mini-block is, the less work is needed to
    access any particular byte. On the other hand, the smaller the
    mini-blocks become, the worse the compression ratio becomes. So an
    application needs to make a reasonable trade-off between having the
    mini-blocks be too small or too large. To evaluate compression ratio,
    use the ``deflate_block::compressedSize()`` method.

For more information on deflate block usage, 
refer to the :ref:`Building Deflate Block <building_deflate_block>` chapter.
