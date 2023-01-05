 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

Installation
############

Intel® Query Processing Library (Intel® QPL) currently doesn't provide a
binary distribution and can be built only from the sources.

.. _system_requirements_reference_link:

System Requirements
*******************

Intel® QPL supports only the Intel 64 platform.

Software Path Requirements
==========================

| **Minimum requirements**
| x86-64 CPU with Intel® Advanced Vector Extensions 2 support
  (Intel® microarchitecture code name Broadwell).

| **Recommended requirements for better performance**
| x86-64 CPU with Intel® Advanced Vector Extensions 512 support
  (Intel® microarchitecture code name Skylake (Server) processor or higher).

.. _system_requirements_hw_path_reference_link:

Hardware Path Requirements
==========================

Execution using Hardware Path available only on Linux* OS.

Additionally, the operating system must meet the following requirements:

- Linux kernel version 5.18 or later. Public versions of Linux
  kernels can be found here: https://www.kernel.org.
- Virtualization technology for directed I/O (VT-d) is enabled through the BIOS menu.

.. _accelerator_configuration_reference_link:

Accelerator Configuration
*************************

An Intel® In-Memory Analytics Accelerator (Intel® IAA) device can be
configured with the ``libaccel-config`` library, which can be found at
https://github.com/intel/idxd-config.

Intel® QPL provides a way to configure Intel® IAA through calling
either of the following commands based on whether you are setting up
via the Intel® QPL source or from the Intel® QPL installed directory:

.. code-block:: shell

   sudo python3 <qpl-library>/tools/scripts/accel_conf.py --load=<path to config file>

.. code-block:: shell

   sudo python3 <install-dir>/bin/scripts/accel_conf.py <config file> --load=<path to config file>

With configuration files found at either ``<qpl-library>/tools/configs/`` or ``<install-dir>/bin/configs/``.
With configuration files of the format ``<# nodes>n<# devices>d<# engines>e<# of workqueues>w-s.conf`` or
``<# nodes>n<# devices>d<# engines>e<# of workqueues>w-s-n<which node>.conf``.

.. attention::

   Sudo privileges are required to configure Intel® IAA instance.

.. note::

  By default, Intel® QPL uses the ``Block On Fault`` feature
  required to handle page faults on the Intel® IAA side. The
  ``block on fault`` flag must be set with the ``accel-config`` for each
  device. Performance can be increased if an application performs its own
  ``pre-faulting``. In this case, the ``block on fault`` flag is
  not required and the ``Block On Fault`` feature must be disabled with the
  CMake build option ``-DBLOCK_ON_FAULT=OFF`` (see the
  :ref:`building_library_build_reference_link` and
  :ref:`building_library_build_options_reference_link` sections).

.. _building_library_reference_link:

Building the Library
********************

.. _building_library_prerequisites_reference_link:

Prerequisites
=============

Before building Intel® QPL, install and set up the following tools:

**Linux\* OS:**

- nasm 2.15.0 or higher (e.g., can be obtained from https://www.nasm.us)

- GCC 8.2 or higher (Clang 12.0.1 or higher for building fuzz tests)

- Universally Unique ID library ``uuid-dev`` version 2.35.2 or higher

- CMake* version 3.16.3 or higher

- GNU Make

Additionally, ``libaccel-config`` library version 3.2 or higher is required
for building and running Intel QPL even for Software Path execution.
Refer to `accel-config releases <https://github.com/intel/idxd-config/releases>`__ for the
latest version.

.. attention::

   Either the ``libaccel-config`` library must be placed in ``/usr/lib64/``
   or the user must update ``LD_LIBRARY_PATH`` and ``LIBRARY_PATH`` with its location.

.. attention::

   It is required to add ``libaccel-config`` library to the link line (``-laccel-config``)
   when building an application with the Intel QPL.

**Windows\* OS:**

- nasm 2.15.0 or higher (can be obtained from https://www.nasm.us, for example,
  `nasm-2.15.05-installer-x64.exe <https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/win64/nasm-2.15.05-installer-x64.exe>`__)

- MSVC 14.1 or higher

- CMake* version 3.16.3 or higher

- Microsoft NMAKE

.. _building_library_build_options_reference_link:

Available Build Options
=======================

Intel QPL supports the following build options:

-  ``-DSANITIZE_MEMORY=[ON|OFF]`` - Enables memory sanitizing (``OFF`` by default).
-  ``-DSANITIZE_THREADS=[ON|OFF]`` - Enables threads sanitizing (``OFF`` by default).
-  ``-DLOG_HW_INIT=[ON|OFF]`` - Enables hardware initialization log (``OFF`` by default).
-  ``-DEFFICIENT_WAIT=[ON|OFF]`` - Enables usage of efficient wait instructions (``OFF`` by default).
-  ``-DLIB_FUZZING_ENGINE=[ON|OFF]`` - Enables fuzz testing (``OFF`` by default).
-  ``-DBLOCK_ON_FAULT=[OFF|ON]`` - Disables Page Fault Processing on the accelerator side (``ON`` by default).

.. note::

   The value of ``BLOCK_ON_FAULT`` can affect the performance of hardware path
   applications. Read more in the :ref:`accelerator_configuration_reference_link` section.

-  ``-DQPL_BUILD_EXAMPLES=[OFF|ON]`` - Enables building library examples (``ON`` by default).
   For more information on existing examples, see :ref:`code_examples_c_reference_link`.

-  ``-DQPL_BUILD_TESTS=[OFF|ON]`` - Enables building library testing and benchmarks frameworks (``ON`` by default).
   For more information on library testing, see :ref:`library_testing_reference_link` section.
   For information on benchmarking the library, see :ref:`library_benchmarking_reference_link`.

.. attention::

   To build Intel QPL from the GitHub release package (``.tar``, ``.tgz``)
   without downloading sub-module dependencies for testing and benchmarking,
   use ``-DQPL_BUILD_TESTS=OFF``.

.. _building_library_build_reference_link:

Build Steps
===========

To build Intel QPL (by default it includes building examples, tests
and benchmarks framework as well), complete the following steps:

1. Make sure that :ref:`system_requirements_reference_link` are met
   and all the tools from the :ref:`building_library_prerequisites_reference_link`
   section are available in your environment.

2. Clone git sources using the following command:


   .. code-block:: shell

      git clone --recursive https://github.com/intel/qpl.git <qpl_library>

.. attention::

   ``--recursive`` is required for downloading sub-module dependencies for testing
   and benchmarking Intel QPL.

3. Build the library and tests by executing the following commands in ``<qpl_library>``:


   **Linux\* OS:**

   .. code-block:: shell

      mkdir build
      cd build
      cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=<install_dir> ..
      cmake --build . --target install


   **Windows\* OS:**

   .. code-block:: shell

      mkdir build
      cd build
      cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=<install_dir> -G "NMake Makefiles" ..
      cmake --build . --target install

4. The resulting library will be available in the folder ``<install_dir>/lib/``.

Installed Package Structure
===========================

::

     ┌── bin
     ├── include
     │   └── qpl
     |       ├── c_api
     |       └── qpl.h
     └── lib or lib64
         ├── cmake
         └── libqpl.a

Executables for tests and benchmarks framework, as well as
configuration files for Intel® IAA
(see :ref:`accelerator_configuration_reference_link` for more details)
are available in ``bin/`` folder.

Examples are located in ``<qpl_library>/build/examples/``.

Intel QPL could be easily integrated to other CMake projects once installed.
Use ``-DCMAKE_PREFIX_PATH`` to point to the existing installation
and add the next lines to your ``CMakeLists.txt``:

.. code-block:: shell

   find_package(QPL CONFIG REQUIRED)
   target_link_libraries(app_name QPL::qpl)

Building the Documentation
**************************

Prerequisites
=============

To build the offline version of the documentation, the following tools must be installed:

- `Doxygen <https://www.doxygen.nl/index.html>`__ 1.8.17 or higher (e.g., with ``apt install doxygen``)
- `Python <https://www.python.org/>`__ 3.8.5 or higher (e.g., with ``apt install python3.X``)
- `Sphinx <https://www.sphinx-doc.org/en/master/>`__ 3.5.4 or higher (e.g., with ``pip3 install sphinx``)
- `sphinx_book_theme <https://executablebooks.org/en/latest/>`__  (e.g., with ``pip3 install sphinx-book-theme``)
- `Breathe <https://breathe.readthedocs.io/en/latest/>`__ 4.29.0 or higher (e.g., with ``pip3 install breathe``)

Build Steps
===========

To generate the full offline documentation from sources,
use the following command:

.. code-block:: shell

   /bin/bash <qpl_library>/doc/_get_docs.sh

.. attention::

   Linux* OS shell (or Windows* OS shell alternatives) is required to run the
   documentation build script.

After the generation process is completed, open the ``<qpl_library>/doc/build/html/index.html`` file.


