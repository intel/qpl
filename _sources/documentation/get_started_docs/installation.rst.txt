 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/

Installation
############


Install Package Structure
*************************


::

     ├── bin
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


.. _library_presetting_and_building_reference_link:


Library Presetting and Building
*******************************


Intel® Query Processing Library (Intel® QPL) requires presetting 
of hardware configuration before usage. Library does not perform 
hardware setup independently. System administrator is responsible 
for correctness of accelerator configuration. If configuration is 
invalid or does not exist, library returns an appropriate status 
code in case if hardware execution path used.

.. note::
   
   Intel QPL does not support all hardware possibilities. Library
   limitations are described in the :ref:`Library Limitations 
   <library_limitations_reference_link>` section.


System Requirements
===================


For more information about system requirements,
see the :ref:`System Requirements <system_requirements_reference_link>` chapter.


Build Prerequisites
===================


Before building the library, install and set up the following tools:



- **Assembler**: ``nasm 2.15.0`` or higher (can be installed from 
  https://www.nasm.us/, for example build `2.15.05 
  <https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/>`__, 
  *win64/nasm-2.15.05-installer-x64.exe* for use on 64-bit Windows* OS, 
  or *nasm-2.15.05.tar.gz* for Linux* OS).
- Compiler:
  
  - Linux* OS: GCC 8.2 or higher;
  - Windows* OS: VC 14.1 or higher;
  - Linux* OS: Compiler for building fuzzy tests - Clang 12.0.1 or higher;

- Libraries:

  - Linux* OS: Universally Unique ID library: ``uuid-dev`` version 2.35.2 or higher;
  - Accelerator configuration library (https://github.com/intel/idxd-config) version 3.2 or higher;

-  Cross-platform build tool: CMake* version 3.15.0 or higher.
-  Make: GNU ``make`` (Linux* OS) or ``nmake`` (Windows* OS).


Documentation Build Prerequisites
=================================


Documentation is delivered using GitHub Pages. 
See full Intel QPL `online documentation <https://intel.github.io/qpl/index.html>`__.

If you plan to build an offline version of documentation,
make sure the following documentation generation tools are installed and set up:

- `Doxygen <https://www.doxygen.nl/index.html>`__ 1.8.17 or higher: ``apt install doxygen``
- `Python <https://www.python.org/>`__ 3.8.5 or higher: ``apt install python3.X``
- `Sphinx <https://www.sphinx-doc.org/en/master/>`__ 3.5.4 or higher: ``pip3 install sphinx``
- `sphinx_book_theme <https://executablebooks.org/en/latest/>`__ : ``pip3 install sphinx-book-theme``
- `Breathe <https://breathe.readthedocs.io/en/latest/>`__ 4.29.0 or higher: ``pip3 install breathe``


.. note::

   Linux* OS shell (or Windows* OS shell alternatives) is required to run the build script.

To generate full offline documentation from sources, use the following commands:

.. code-block:: shell

   cmd> cd <qpl_library catalog path>/doc
   cmd> _get_docs.sh


After generation process is completed, open the `<qpl_library>/doc/build/html/index.html` file.


Build
=====


To build the library, complete the following steps:

1. Make sure that all the tools from the `Build Prerequisites <#build-prerequisites>`__
   section are available from your environment.

2. Extract git sources using the following command:


   .. code-block:: shell

      # Using HTTPS
      git clone --recursive <qpl_git_repository> <qpl_library>

3. Build the library and tests by executing the following list of
   commands from ``<qpl_library>`` depending on OS:


**Windows* OS:**

.. code-block:: shell

   mkdir build
   cd build
   cmake -DCMAKE_INSTALL_PREFIX=<install_dir> -G "NMake Makefiles" ..
   cmake --build . --target install


**Linux\* OS:**

.. code-block:: shell

   mkdir build
   cd build
   cmake -DCMAKE_INSTALL_PREFIX=<install_dir> ..
   cmake --build . --target install


**Build type**: CMake generates the script for release build as default. 
If you want to build a particular type of the library and tests (DEBUG|RELEASE), 
use the flag ``CMAKE_BUILD_TYPE`` like:


.. code-block:: shell

   # Debug
   cmake -DCMAKE_BUILD_TYPE=Debug <path_to_cmake_folder>

   # Release
   cmake -DCMAKE_BUILD_TYPE=Release <path_to_cmake_folder>

   # Release with debug information
   cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo <path_to_cmake_folder>


4. The resulting library will be available in the folder ``<install_dir>/lib``.
   In Linux* OS default ``<install_dir>`` is ``/usr/local/``.


.. _cpecific_build_options_reference_link:


Specific Build Options
======================


To achieve the best experience depending on the user's needs, the
library has different buil options:

-  ``-DSANITIZE_MEMORY=[ON|OFF]`` - Enables memory sanitizing (OFF by default)
-  ``-DSANITIZE_THREADS=[ON|OFF]`` - Enables threads sanitizing (OFF by default)
-  ``-DLOG_HW_INIT=[ON|OFF]`` - Enables HW initialization log (OFF by default)
-  ``-DEFFICIENT_WAIT=[ON|OFF]`` - Enables usage of efficient wait instructions (OFF by default)
-  ``-DLIB_FUZZING_ENGINE=[ON|OFF]`` - Enables fuzzy testing (OFF by default)
-  ``-DBLOCK_ON_FAULT=[ON|OFF]`` - Enables Page Fault Processing on the accelerator side (ON by default)

.. note:: 
   
   Right value of ``BLOCK_ON_FAULT`` option has strong
   effect on ``hw-path`` usage experience. Read more in the :ref:`Accelerator
   Configuration <accelerator_configuration_reference_link>` section.
