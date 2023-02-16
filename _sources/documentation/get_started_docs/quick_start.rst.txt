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
The key agent of this API is the :c:struct:`qpl_job` data structure.
To work with Intel QPL low-level C API, the application will need to:

1. Query the required memory size.
2. Allocate memory according to the queried size.
3. Initialize the job structure and fill in necessary parameters.
4. Pass the job structure (along with the allocated memory) to Intel QPL.
5. When the operations are finished, free the resources.

The example below compresses and decompresses data with Deflate dynamic Huffman encoding via Intel QPL
low-level C API. For our purpose to understand the workflow, we only focus on the compression part
here. See the comments after the code block.

.. literalinclude:: ../../../../examples/low-level-api/compression_example.cpp
    :language: cpp
    :lines: 1-16,29-77,94-99,110-
    :emphasize-lines: 14-15,39,44-46,52-58,61,67
    :linenos:

The application only needs to include one header file ``qpl/qpl.h``, which specifies
the prototypes of all the functions.

At line 29, we call :c:func:`qpl_get_job_size` to query the required memory size
based on the specified execution path.

At lines 34 and 35, we allocate memory according to the returned value of ``size``.
Note that the value of ``size`` is greater than the size of the job structure
:c:struct:`qpl_job`. The leading portion of the allocated memory is used to store
the job structure, while the remaining portion is a buffer for internal usages.

At line 36, we call :c:func:`qpl_init_job` to initialize the job structure
and buffer, then we fill in necessary parameters at lines 42 to 48.

The job structure and the allocated buffer are passed to Intel QPL at line 51. After
:c:func:`qpl_execute_job` completes successfully, we can retrieve the results stored
in the job structure.

Finally, we call :c:func:`qpl_fini_job` at line 57 to free the resources.

In order to build the library and all the examples, including the one above, follow steps at :ref:`building_library_reference_link`.
Compiled examples then would be located in ``<qpl_library>/build/examples/low-level-api/``.

Alternatively, in order to build ``compression_example.cpp`` individually using existing Intel QPL installation, use:

.. code-block:: shell

    g++ -I/<install_dir>/include -o compression_example compression_example.cpp /<install_dir>/lib64/libqpl.a -ldl

.. attention::

   Intel QPL could be also used from C applications.
   This would still require C++ runtime library installed on the system, and adding `-lstdc++`.

To run the example on the Hardware Path, use:

.. code-block:: shell

    sudo ./compression_example hardware_path

.. attention::

   With the Hardware Path, the user must either place the ``libaccel-config`` library in ``/usr/lib64/``
   or specify the location of ``libaccel-config`` in ``LD_LIBRARY_PATH`` for the dynamic loader to find it.

To run the example on the Software Path, use:

.. code-block:: shell

    ./compression_example software_path

.. attention::

   If Intel QPL is built with ``-DDYNAMIC_LOADING_LIBACCEL_CONFIG=OFF`` (see :ref:`building_library_build_options_reference_link`
   for details), replace ``-ldl`` with ``-laccel-config`` in the compilation command. The user must either place ``libaccel-config`` in
   ``/usr/lib64/`` or specify the location of ``libaccel-config`` (for example, using ``LD_LIBRARY_PATH`` and ``LIBRARY_PATH``).

Refer to :ref:`Developer Guide <developer_guide_low_level_reference_link>`
for more information about Intel QPL low-level C API.
For more examples, see :ref:`code_examples_c_reference_link`.
