 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Deflate with Dictionary
#######################


Intel® Query Processing Library (Intel® QPL) supports compression with
preset dictionary by implementing several auxiliary functions that work
with ``qpl_dictionary``.


Building Dictionary
*******************


First, you need to allocate the buffer for ``qpl_dictionary``. The size
of that buffer can be obtained by the ``qpl_get_dictionary_size(...)``
function.

.. code:: c

   qpl_get_dictionary_size(sw_compression_level, hw_compression_level, size_t);


This function accepts 3 arguments: the software and hardware compression
levels, and the size (in bytes) of a raw dictionary.

To do the compression only on software path, set the
``hw_compression_level`` parameter to ``HW_NONE``. In this case, the
buffer size needed for ``qpl_dictionary`` will be smaller.


After allocating the buffer, ``qpl_build_dictionary(...)`` function
should be used to fill the ``qpl_dictionary``.


.. code:: c

   qpl_build_dictionary(qpl_dictionary *dict_ptr,
                        sw_compression_level sw_level,
                        hw_compression_level hw_level,
                        const uint8_t *raw_dict_ptr,
                        size_t raw_dict_size))


This function accepts a pointer to allocated dictionary buffer, the software
and hardware compression levels, a pointer to the array containing the raw
dictionary data to use, and its length.

**Note**: If the dictionary length is larger than 4,096
bytes, then only the last 4,096 bytes will be used.

Several auxiliary functions can be used to work with dictionary:

-  ``qpl_get_dictionary_id(..)`` and ``qpl_set_dictionary_id(...)`` sets
   and gets dictionary ID of ``qpl_dictionary`` (can be used in case of
   zlib header).
-  ``qpl_get_existing_dict_size(...)`` is used to get the size (in
   bytes) of the ``qpl_dictionary`` structure built.


Compressing with Dictionary
***************************


Compression with preset dictionary can be done for dynamic, fixed,
static, and canned compression modes. In order to do the compression,
the ``qpl_job->dictionary`` field should point to the built dictionary:

.. code:: c

   // ...
   job_ptr->op = qpl_op_compress;
   job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN;
   job_ptr->dictionary = dictionary_ptr;

   qpl_status = qpl_execute_job(job_ptr);


The dictionary cannot be set in the middle of the compression stream.
The job should be marked as FIRST.


To decompress the stream previously compressed with the dictionary, the
same dictionary should be specified for the first decompression job:


.. code:: c

   // ...
   job_ptr->op = qpl_op_decompress;
   job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST;
   job_ptr->dictionary = dictionary_ptr;

   qpl_status = qpl_execute_job(job_ptr);
