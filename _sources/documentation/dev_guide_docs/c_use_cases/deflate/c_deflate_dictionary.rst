 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Compressing with a Dictionary
#############################

.. _compressing_with_dictionary_reference_link:


Intel® Query Processing Library (Intel® QPL) supports compression with
preset dictionary by implementing several auxiliary functions that work
with :c:struct:`qpl_dictionary`. The dictionary itself is just a block of text
conceptually prepended to the input stream. The combined dictionary and
input stream is compressed, and the compressed tokens associated with the
dictionary are dropped. Another way to look at this is that, with dictionary
compression, a given bit of data to be compressed can be matched against a
location before the start of the buffer. The dictionary can be used to improve
the compression ratio of small buffers.


.. _building_dictionary_reference_link:

Building Dictionary
*******************


First, you need to allocate the buffer for :c:struct:`qpl_dictionary`. The size
of that buffer can be obtained by the :c:func:`qpl_get_dictionary_size`
function.

.. code:: c

   size_t qpl_get_dictionary_size(sw_compression_level, hw_compression_level, size_t);


This function accepts 3 arguments: the software and hardware dictionary
compression levels, and the size (in bytes) of a raw dictionary.

A higher dictionary compression level generally results in a better compression ratio,
but it will also cause a higher latency for the compress operation. Some applications
may find that the improvement in compression ratio is not worth the increase in
compress latency and opt for a lower dictionary compression level.

To do the compression only on software path, set the
:c:enum:`hw_compression_level` parameter to ``HW_NONE``. In this case, the
buffer size needed for :c:struct:`qpl_dictionary` will be smaller. Similarly,
to do the compression only on hardware path, set the :c:enum:`sw_compression_level` parameter
to ``SW_NONE``.

On software path, the maximum size of a raw dictionary is ``4K`` bytes. On hardware path,
the maximum size of a raw dictionary depends on the ``hw_compression_level``:

+---------------------------------+----------------------------+
| HW dictionary compression level | Max size of raw dictionary |
+=================================+============================+
|           HW_LEVEL_1            |            2 KB            |
+---------------------------------+----------------------------+
|           HW_LEVEL_2            |            4 KB            |
+---------------------------------+----------------------------+
|           HW_LEVEL_3            |            4 KB            |
+---------------------------------+----------------------------+

If the size of the raw dictionary specified by the user is larger than the maximum size,
the maximum size will be used to calculate the dictionary size. And the last bytes
of the raw dictionary will be used to build the dictionary.

After allocating the buffer, :c:func:`qpl_build_dictionary` function
should be used to fill the :c:struct:`qpl_dictionary`.


.. code:: c

   qpl_status qpl_build_dictionary(qpl_dictionary       *dict_ptr,
                                   sw_compression_level sw_level,
                                   hw_compression_level hw_level,
                                   const uint8_t        *raw_dict_ptr,
                                   size_t               raw_dict_size))


This function accepts a pointer to allocated dictionary buffer, the software
and hardware dictionary compression levels, a pointer to the array containing
the raw dictionary data to use, and its length.

.. attention::

    To get the most benefit out of the dictionary, set ``raw_dict_size``
    to the maximum size of the raw dictionary. If ``raw_dict_size`` is
    larger than the maximum size, then only the last bytes will be used.

Several auxiliary functions can be used to work with dictionary:

-  :c:func:`qpl_get_dictionary_id` and :c:func:`qpl_set_dictionary_id` sets
   and gets dictionary ID of :c:struct:`qpl_dictionary` (can be used in case of
   zlib header).
-  :c:func:`qpl_get_existing_dict_size` is used to get the size (in
   bytes) of the :c:struct:`qpl_dictionary` structure built.


Compressing with Dictionary
***************************


Compression with dictionary is supported only on certain generations of
Intel® In-Memory Analytics Accelerator (Intel® IAA). The :c:macro:`QPL_STS_NOT_SUPPORTED_MODE_ERR`
error will be returned if the operation is not supported. The software path can be
used as an alternative.

On hardware path, compression with dictionary can be done for dynamic, fixed, static,
and canned compression modes.

.. attention::

    Multi-job execution (i.e. :c:macro:`QPL_FLAG_FIRST` and :c:macro:`QPL_FLAG_LAST`
    are not set in the same job) is not supported for dictionary compression on hardware path.

On software path, compression with dictionary can be done for dynamic, fixed, static,
and canned compression modes. 

.. attention::

    Verification is not supported with dictionary compression on either software or
    hardware path. :c:macro:`QPL_FLAG_OMIT_VERIFY` needs to be set when dictionary is used.

In order to do the compression,
the :c:member:`qpl_job.dictionary` field should point to the built dictionary:

.. code:: c

   // ...
   job_ptr->op = qpl_op_compress;
   job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST | QPL_FLAG_DYNAMIC_HUFFMAN;
   job_ptr->dictionary = dictionary_ptr;

   qpl_status = qpl_execute_job(job_ptr);


The dictionary cannot be set in the middle of the compression stream.
The job should be marked with :c:macro:`QPL_FLAG_FIRST`.

.. warning::

    On ``software path`` and ``auto path``, the user must use the same value for the dictionary level
    (i.e. :c:enum:`sw_compression_level`) and the compression level (i.e. :c:member:`qpl_job.level`).

Decompressing with Dictionary
*****************************


To decompress the stream previously compressed with the dictionary, the
same dictionary should be specified for the first decompression job:

.. code:: c

   // ...
   job_ptr->op = qpl_op_decompress;
   job_ptr->flags = QPL_FLAG_FIRST | QPL_FLAG_LAST;
   job_ptr->dictionary = dictionary_ptr;

   qpl_status = qpl_execute_job(job_ptr);

.. attention::

    Canned mode decompression with dictionary is not supported on hardware path. The software
    path can be used as an alternative.

Mixing Software Path and Hardware Path with Dictionary
******************************************************


To make sure that the dictionary data used for compression and decompression matches when compression
and decompression take different paths, the user must provide a raw dictionary with a size equal to
the maximum size of the raw dictionary
(see :ref:`building dictionary <building_dictionary_reference_link>` for maximum size of the raw dictionary).
