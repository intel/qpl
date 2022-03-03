 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


deflate_operation
#################


Deflate operation represents a compression functionality that uses the
deflate compression algorithm. Currently, ``qpl::deflate_operation``
supports the following compression modes:

-  :ref:`Dynamic blocks <dynamic_blocks_reference_link>`
-  :ref:`Fixed block <fixed_block_reference_link>`
-  :ref:`Static blocks <static_blocks_reference_link>`
-  :ref:`Canned <canned_mode_reference_link>`

It also supports two additional extensions like GZIP mode and high
compression level (zlib level 9). Here is an example:

.. code:: cpp

   #include <qplhl/operations/deflate_operation.hpp>

   // ...
   auto deflate_operation             = qpl::deflate_operation();
   auto deflate_gzip_operation        = qpl::deflate_operation::builder()
           .gzipMode(true)
           .build();
   auto deflate_high_compression_level = qpl::deflate_operation::builder()
           .gzip_mode(false)
           .compression_level(qpl::compression_levels::high_level)
           .build();
   auto deflate_fixed_compression_mode = qpl::deflate_operation::builder()
           .compression_mode<qpl::compression_modes::fixed_mode>()
           .build();
   // ...
