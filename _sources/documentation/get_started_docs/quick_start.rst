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

1. Allocate buffer required for compression.
2. Query the memory size required for the job structure.
3. Allocate memory for the job structure according to the queried size.
4. Initialize the job structure and fill in necessary parameters.
5. Pass the job structure (along with the allocated memory) to Intel QPL.
6. When the operations are finished, free the resources.

The example below compresses and decompresses data with Deflate dynamic Huffman encoding via Intel QPL
low-level C API. For our purpose to understand the workflow, we only focus on the compression part
here. See the comments after the code block.

.. literalinclude:: ../../../../examples/low-level-api/compression_example.cpp
    :language: cpp
    :lines: 1-16, 29-35, 40-88, 104-110, 123-
    :emphasize-lines: 26, 34, 41, 47-48, 50, 57-63, 66, 75
    :linenos:

The application only needs to include one header file ``qpl/qpl.h``, which specifies
the prototypes of all the functions.

At line 26, we call :c:func:`qpl_get_safe_deflate_compression_buffer_size` to estimate
the size of the output buffer required for compression operations. The size is calculated
based on the input data size and return 0 if the source size exceeds the maximum supported size.
For more details, refer to
:ref:`Estimating the Size of the Deflate Compression Buffer <deflate_estimation_reference_link>`
page.

At line 34, we allocate the output buffer based on the estimation we obtained earlier.

At line 41, we call :c:func:`qpl_get_job_size` to query the required memory size
based on the specified execution path.

At lines 47-48, we allocate memory according to the returned value of ``size``.
Note that the value of ``size`` is greater than the size of the job structure
:c:struct:`qpl_job`. The leading portion of the allocated memory is used to store
the job structure, while the remaining portion is a buffer for internal usages.

At line 50, we call :c:func:`qpl_init_job` to initialize the job structure
and buffer, then we fill in necessary parameters at lines 57 to 63.

The job structure and the allocated buffer are passed to Intel QPL at line 66. After
:c:func:`qpl_execute_job` completes successfully, we can retrieve the results stored
in the job structure.

Finally, we call :c:func:`qpl_fini_job` at line 75 to free the resources.

In order to build the library and all the examples, including the one above, follow steps at :ref:`building_library_reference_link`.
Compiled examples then would be located in ``<qpl_library>/build/examples/low-level-api/``.

Alternatively, in order to build ``compression_example.cpp`` individually using existing Intel QPL installation, use:

.. code-block:: shell

    g++ -I/<install_dir>/include -o compression_example compression_example.cpp /<install_dir>/lib64/libqpl.a -ldl

.. attention::

   Intel QPL could be also used from C applications.
   This would still require C++ runtime library installed on the system.
   You would also need to add `-lstdc++` if you are using the static library ``libqpl.a``.

On Linux, if you installed Intel QPL system wide, you can use the dynamic
library to compile the examples with:

.. code-block:: shell

    g++ -I/<install_dir>/include -o compression_example compression_example.cpp -lqpl

In order to build an example using pkg-config for the dynamic library, set the
``PKG_CONFIG_PATH`` and compile the example using ``qpl.pc``:

.. code-block:: shell

    g++ `pkg-config --cflags --libs qpl` -o compression_example compression_example.cpp

To run the example on the ``Hardware Path`` (see :ref:`library_execution_paths_reference_link`), use:

.. code-block:: shell

    ./compression_example hardware_path

.. attention::
    Either sudo privileges or elevated permissions are required to initialize Intel QPL job with `qpl_path_hardware`.

    Refer to the :ref:`accelerator_configuration_reference_link` section for more details about getting permissions.

.. attention::

   With the Hardware Path, the user must either place the ``libaccel-config`` library in ``/usr/lib64/``
   or specify the location of ``libaccel-config`` in ``LD_LIBRARY_PATH`` for the dynamic loader to find it.

.. attention::

    In the example above we do not set :c:member:`qpl_job.numa_id` value.

    If Intel QPL version is **`< 1.6.0`**, the library will auto-detect NUMA node of the calling process
    and use Intel® In-Memory Analytics Accelerator (Intel® IAA) device(s) located on the same **NUMA node**.

    If Intel QPL version is **`>= 1.6.0`**, the library will use Intel IAA device(s) located on the **socket** of the calling thread.

    Refer to :ref:`library_device_selection_reference_link` section for more details.

To run the example on the ``Software Path`` (see :ref:`library_execution_paths_reference_link`), use:

.. code-block:: shell

    ./compression_example software_path

To run the example on the ``Auto Path`` (see :ref:`library_execution_paths_reference_link`), use:

.. code-block:: shell

    ./compression_example auto_path

.. attention::

   If Intel QPL is built with ``-DDYNAMIC_LOADING_LIBACCEL_CONFIG=OFF`` (see :ref:`building_library_build_options_reference_link`
   for details), replace ``-ldl`` with ``-laccel-config`` in the compilation command. The user must either place ``libaccel-config`` in
   ``/usr/lib64/`` or specify the location of ``libaccel-config`` (for example, using ``LD_LIBRARY_PATH`` and ``LIBRARY_PATH``).

Refer to :ref:`Developer Guide <developer_guide_low_level_reference_link>`
for more information about Intel QPL low-level C API.
For more examples, see :ref:`code_examples_c_reference_link`.
