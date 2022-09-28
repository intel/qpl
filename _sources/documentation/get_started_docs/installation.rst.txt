 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

Installation
############


.. _building_library_reference_link:

Building the Library
********************


.. _building_library_prerequisites_reference_link:

Prerequisites
=============


Before building Intel® Query Processing Library (Intel® QPL), install and set up
the following tools:

**Linux\* OS:**

- nasm 2.15.0 or higher (can be obtained from https://www.nasm.us, for example,
  `nasm-2.15.05.tar.gz <https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/nasm-2.15.05.tar.gz>`__)

- GCC 8.2 or higher (Clang 12.0.1 or higher for building fuzz tests)

- Universally Unique ID library ``uuid-dev`` version 2.35.2 or higher

- CMake* version 3.15.0 or higher

- GNU Make


**Windows\* OS:**

- nasm 2.15.0 or higher (can be obtained from https://www.nasm.us, for example,
  `nasm-2.15.05-installer-x64.exe <https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/win64/nasm-2.15.05-installer-x64.exe>`__)

- MSVC 14.1 or higher

- CMake* version 3.15.0 or higher

- Microsoft NMAKE


.. _building_library_build_reference_link:

Build
=====


To build Intel QPL, complete the following steps:

1. Make sure that all the tools from the :ref:`building_library_prerequisites_reference_link`
   section are available in your environment.

2. Clone git sources using the following command:


   .. code-block:: shell

      # Using HTTPS
      git clone --recursive https://github.com/intel/qpl.git <qpl_library>

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


   You can use the flag ``CMAKE_BUILD_TYPE`` to specify a different build type,
   for example, ``Debug`` or ``RelWithDebInfo``.


4. The resulting library will be available in the folder ``<install_dir>/lib/``.
   In Linux* OS, ``<install_dir>`` defaults to ``/usr/local/``.


Installed Package Structure
===========================


::

     ┌── bin
     ├── include  
     │   └── qpl  
     |       ├── c_api
     │       └── cpp_api    
     │           ├── chaining  
     │           ├── common
     │           ├── operations                
     │           ├── results    
     │           └── util    
     └── lib64
         └── cmake 


.. _building_library_build_options_reference_link:

Build Options
=============


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


Building the Documentation
**************************


Prerequisites
=============


If you plan to build an offline copy of documentation,
make sure the following documentation generation tools are installed and set up:

- `Doxygen <https://www.doxygen.nl/index.html>`__ 1.8.17 or higher: ``apt install doxygen``
- `Python <https://www.python.org/>`__ 3.8.5 or higher: ``apt install python3.X``
- `Sphinx <https://www.sphinx-doc.org/en/master/>`__ 3.5.4 or higher: ``pip3 install sphinx``
- `sphinx_book_theme <https://executablebooks.org/en/latest/>`__ : ``pip3 install sphinx-book-theme``
- `Breathe <https://breathe.readthedocs.io/en/latest/>`__ 4.29.0 or higher: ``pip3 install breathe``


.. note::

   Linux* OS shell (or Windows* OS shell alternatives) is required to run the build script.


Build
=====


To generate the full offline documentation from sources, use the following command:

.. code-block:: shell

   /bin/bash <qpl_library>/doc/_get_docs.sh


After the generation process is completed, open the ``<qpl_library>/doc/build/html/index.html`` file.


