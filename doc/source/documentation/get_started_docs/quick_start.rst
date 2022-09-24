 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _quick_start_reference_link:

Quick Start
###########


Low-Level C API
***************


We walk through an example to learn the basic workflow of
Intel® Query Processing Library (Intel® QPL) low-level C API.
The key agent of low-level C API is the :c:struct:`qpl_job`
data structure. To work with Intel QPL low-level C API, the
application will need to:

1. Query the required memory size.
2. Allocate memory according to the queried size.
3. Initialize the job structure and fill in necessary parameters.
4. Pass the job structure (along with the allocated memory) to Intel QPL.
5. Free resources.

The example below is extracted from the
:ref:`compression example <code_examples_c_compression_reference_link>`,
which compresses and decompresses data with Deflate dynamic Huffman encoding via Intel QPL
low-level C API. For our purpose to understand the workflow, we only focus on the compression part
here. See the comments after the code block.

.. literalinclude:: ../../../../examples/low-level-api/compression_example.cpp
    :language: cpp
    :lines: 1-14,25-65,88-93,104-
    :emphasize-lines: 13,29,34-36,42-48,51,57
    :linenos:

The application only needs to include one header file ``qpl/qpl.h``, which specifies
prototypes of all the functions.

At line 29, we call :c:func:`qpl_get_job_size` to query the required memory size
based on the execution path.

.. attention::
   The redesign of APIs for memory size query is in progress in order to achieve a
   finer granularity than the execution path.

At lines 34 and 35, we allocate memory according to the returned value of ``size``.
Note that the value of ``size`` is greater than the size of the job structure
:c:struct:`qpl_job`. The leading portion of the allocated memory is used to store
the job structure, while the remaining portion is a buffer for internal usages.

At line 36, we call :c:func:`qpl_init_job` to initialize the job structure
and buffer, then we fill in necessary parameters at lines 42 to 48.

The job structure and the allocated buffer are passed to Intel QPL at line 51. After
:c:func:`qpl_execute_job` completes successfully, we can retrieve the results stored
in the job structure.

Finally, we call :c:func:`qpl_fini_job` at line 57 to free resources.

After building Intel QPL (see :ref:`building_library_reference_link`), you can find
compiled examples in ``<qpl_library>/build/examples/low-level-api/``. You can also
compile each example individually with the command:

.. code-block:: shell

    g++ -I/path/to/install_dir/include -o compression_example compression_example.cpp /path/to/install_dir/lib64/libqpl.a -ldl


Refer to :ref:`Developer Guide <developer_guide_low_level_reference_link>`
for more information about Intel QPL low-level C API. For more examples,
see :ref:`code_examples_c_reference_link`.
