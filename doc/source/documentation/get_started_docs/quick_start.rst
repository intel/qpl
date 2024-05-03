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
    :lines: 1-13, 28-35, 42-44, 46-82, 100-105, 118-
    :emphasize-lines: 13, 31, 37-38, 40, 47-53, 56, 63
    :linenos:

The application only needs to include one header file ``qpl/qpl.h``, which specifies
the prototypes of all the functions.

At line 31, we call :c:func:`qpl_get_job_size` to query the required memory size
based on the specified execution path.

At lines 37-38, we allocate memory according to the returned value of ``size``.
Note that the value of ``size`` is greater than the size of the job structure
:c:struct:`qpl_job`. The leading portion of the allocated memory is used to store
the job structure, while the remaining portion is a buffer for internal usages.

At line 40, we call :c:func:`qpl_init_job` to initialize the job structure
and buffer, then we fill in necessary parameters at lines 47 to 53.

The job structure and the allocated buffer are passed to Intel QPL at line 56. After
:c:func:`qpl_execute_job` completes successfully, we can retrieve the results stored
in the job structure.

Finally, we call :c:func:`qpl_fini_job` at line 63 to free the resources.

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

    In the example above we do not set :c:member:`qpl_job.numa_id` value, so the library will auto detect NUMA node
    of the calling process and use Intel® In-Memory Analytics Accelerator (Intel® IAA) device(s) located on the same node.

    Alternatively, user can set :c:member:`qpl_job.numa_id` and set matching ``numactl`` policy to ensure
    that the calling process will be located on the same NUMA node as specified with ``numa_id``.

    It is user responsibility to configure accelerator and ensure device(s) availability on the NUMA node.

    Refer to :ref:`library_numa_support_reference_link` section for more details.

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
