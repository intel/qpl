 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

Compression Streams
###################


The Intel® Query Processing Library (Intel® QPL) library implements compression 
stream processing allowing the user to process input data by chunks.
This functionality is available for two types of compression streams:

-  ``qpl::deflate_stream`` - responsible for compression by chunks
-  ``qpl::inflate_stream`` - responsible for decompression by chunks

Each stream has its own set of methods for processing input data.
For more details, see the :ref:`Using compression streams 
<using_compression_streams_reference_link>` chapter.
