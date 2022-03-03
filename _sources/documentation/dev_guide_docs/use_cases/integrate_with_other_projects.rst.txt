 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


Integrating into Other Projects
###############################


The Intel® Query Processing Library (Intel® QPL) library 
can be integrated into your project in two ways.


Integration into Any Build System
*********************************


The ``lib64`` folder of ``<install_dir>`` contains the high-level API
library called ``libqplhl.a`` (Linux* OS) or ``qplhl.lib``
(Windows* OS). Link to ``libqplhl.a`` (Linux* OS) or ``qplhl.lib``
(Windows* OS) and specify correct include directory for you build system.
After that you are ready to use the Intel QPL high-level API.


Integration into CMake-based Project
************************************


The library provides a specific CMake export file. To enable CMake
integration, use the standard ``find_package`` function after library
installation.

A simple example of ``CMakeLists.txt``:


.. code:: cmake

   find_package(Qpl REQUIRED COMPONENTS qplhl)

   add_executable(test main.cpp)
   target_link_libraries(test Qpl::qplhl)


The Intel QPL library package consists of several components. High-level
API component is called ``qplhl`` and is placed in the
``Qpl::`` namespace. To start using it, link to the component as it is
shown in the example earlier.
