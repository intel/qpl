 .. ***************************************************************************
 .. * Copyright (C) 2022 Intel Corporation
 .. *
 .. * SPDX-License-Identifier: MIT
 .. ***************************************************************************/


.. _system_requirements_reference_link:


System Requirements
###################


Software Path requirements
**************************

.. list-table::
  :header-rows: 0

  * - **Minimal**
    - x86-64 CPU with Intel® Advanced Vector Extensions 2 support (Intel® microarchitecture code name Broadwell) not optimized code.
  * - **Recommended**
    - x86-64 CPU with Intel® Advanced Vector Extensions 512 (Intel® AVX-512) support (Intel® microarchitecture code name Skylake (Server) processor or higher).  

Hardware Path Requirements
**************************

- **Linux* OS:**

  - The accelerator configuration library ``libaccel-config.so`` 
    (see :ref:`Accelerator Configuration <accelerator_configuration_reference_link>`) 
    should be placed in ``/usr/lib64/``.
- Linux kernel version 5.18 or later. Public versions of Linux 
  kernels can be found here: https://www.kernel.org.
- Virtualization technology for directed I/O (VT-d) is enabled through the BIOS menu.
- Sudo privileges are required to configure accelerators 
  (see :ref:`Accelerator Configuration <accelerator_configuration_reference_link>`) 
  and run hardware path applications.


.. _accelerator_configuration_reference_link:


Accelerator Configuration
*************************


Intel® In-Memory Analytics Accelerator (Intel® IAA) devices can be configured with 
``libaccel-config`` library (version 3.2 or higher), 
which can be found here: https://github.com/intel/idxd-config.

For detailed instructions on configuration, refer to:

- `accel-config README <https://github.com/intel/idxd-config/blob/master/README.md>`__ 
- `accel-config Wiki <https://github.com/intel/idxd-config>`__


.. note:: 
  
  Intel® QPL supports ``libaccel-config`` starting from
  version 3.2. Refer to `accel-config releases 
  <https://github.com/intel/idxd-config/releases/tag/>`__ for
  latest version.

.. note::
  
  By default, Intel® QPL uses ``Block On Fault`` feature
  required to handle page faults on the Intel® IAA side. The
  ``block on fault`` flag must be set with ``accel-config`` for each
  device. Performance can be increased if an application performs
  ``pre-faulting`` by own. In this case, the ``block on fault`` flag is
  not required and ``Block On Fault`` feature must be disabled using
  CMake build option ``-DBLOCK_ON_FAULT``.


.. code-block:: shell

   # Disabling `block on fault`
   cmake -DCMAKE_BUILD_TYPE=Release -DBLOCK_ON_FAULT=OFF <path_to_cmake_folder>
